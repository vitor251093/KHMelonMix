#include "Worker.h"

#include "LocUtils.h"

#include "localization/nds.h"
#include "localization/lang.h"
#include "localization/utils.h"
#include "localization/p2.h"
#include "localization/strings.h"
#include "localization/stringtable.h"
#include "localization/lzss.h"

#include <QFileInfo>
#include <QDir>

#include <fstream>

Worker::Worker(QObject* parent)
    : QObject(parent)
{
}

Worker::~Worker()
{
}

void Worker::clearRom()
{
    m_fs.reset();
    m_romBuffer.clear();
    m_romData = nullptr;
    m_romDataSize = 0;
}

void Worker::loadRom(const QString& romPath)
{
    const std::string path = romPath.toStdString();

    emit log("Reading file: " + path + "\n");

    auto romData = utils::readBinaryFile(path);
    if (romData.empty())
    {
        clearRom();
        emit log("Failed to read file\n");
        emit romLoaded(false);
        return;
    }

    m_romBuffer = std::move(romData);

    loadRomData(m_romBuffer.data(), m_romBuffer.size());
}

void Worker::loadRomData(uint8_t* data, uint32_t size)
{
    clearRom();

    m_romData = data;
    m_romDataSize = size;

    emit log("Parsing filesystem...\n");
    m_fs = std::make_unique<NDS::NDSFileSystem>(data, size);
    m_fs->getRomFileSystem(true);

    const NdsFileEntryList entries = buildEntryList();

    emit log("Loaded " + std::to_string(size) + " bytes, " + std::to_string(entries.size()) + " files.\n");

    emit filesystemListed(entries);
    emit romLoaded(true);
}

NdsFileEntryList Worker::buildEntryList() const
{
    NdsFileEntryList list;
    if (!m_fs)
        return list;

    const auto& entries = m_fs->getAllEntries();
    list.reserve(static_cast<int>(entries.size()));

    for (const auto& entry : entries)
    {
        NdsFileEntry e;
        e.path = QString::fromStdString(entry.filename);
        e.type = QString::fromStdString(entry.type);
        e.size = static_cast<quint64>(entry.size);

        list.push_back(e);
    }

    return list;
}

void Worker::printFilesystem()
{
    if (!m_fs)
    {
        emit log("No ROM loaded.\n");
        emit taskFinished(false);
        return;
    }

    std::string strings;
    for (const auto& entry : m_fs->getAllEntries())
    {
        strings += "  File: " + entry.filename + "\n";
    }

    emit log(strings);
    emit taskFinished(true);
}

void Worker::extractP2Files(const QString& outFolder, const QStringList& files)
{
    assert(m_romData && m_romDataSize > 0);

    for (const QString& file : files)
    {
        if (auto* entry = m_fs->findEntryByName(file.toStdString()))
        {
            auto* data = m_romData + entry->start;

            if (entry->type == "p2")
            {
                auto p2file = ndsloc::P2File(data, entry->size);

                auto& subfiles = p2file.readAndGetFileTable();
                for (auto& subfile : subfiles)
                {
                    auto outPath = QDir(outFolder).filePath(QString::fromStdString(subfile.getFilename()));

                    if (subfile.isCompressed())
                    {
                        ndsloc::LZSSFile file(subfile.inputPtr, subfile.fileSize);
                        file.decompress();
                        file.saveToDisk(outPath.toStdString());
                    }
                    else
                    {
                        std::ofstream os(outPath.toStdString(), std::ofstream::binary);
                        os.write((char*)subfile.inputPtr, subfile.fileSize);
                    }
                }
            }
            else
            {
                assert(false);
            }
        }
    }

    emit taskFinished(true);
}

void Worker::extractZFiles(const QString& outFolder, const QStringList& files)
{
    assert(m_romData);

    for (const QString& file : files)
    {
        if (auto* entry = m_fs->findEntryByName(file.toStdString()))
        {
            auto fileInfo = QFileInfo(file);
            auto outPath = QDir(outFolder).filePath(fileInfo.fileName());

            auto suffix = fileInfo.suffix().toLower();
            if (suffix == "z")
            {
                auto* data = m_romData + entry->start;
                ndsloc::LZSSFile file(data, entry->size);
                file.decompress();
                file.saveToDisk(outPath.toStdString());
            }
            else
            {
                assert(false);
            }
        }
    }
}

void Worker::extractRawFiles(const QString& outFolder, const QStringList& files)
{
    assert(m_romData);

    for (const QString& file : files)
    {
        if (auto* entry = m_fs->findEntryByName(file.toStdString()))
        {
            auto outPath = QDir(outFolder).filePath(QFileInfo(file).fileName());

            std::ofstream os(outPath.toStdString(), std::ofstream::binary);

            auto* data = m_romData + entry->start;
            os.write((char*)data, entry->size);
        }
    }
}

void Worker::exportStrings(const QString& outFolder, const QStringList& files, uint8_t fmt, uint8_t language)
{
    assert(m_romData);

    using namespace ndsloc;

    if (!m_fs)
    {
        emit log("No ROM loaded.\n");
        emit taskFinished(false);
        return;
    }

    if (files.isEmpty())
    {
        emit log("No files selected.\n");
        emit taskFinished(false);
        return;
    }

    emit log("Exporting strings to: " + outFolder.toStdString() + "\n");

    auto format = (ndsloc::ExportFormat)fmt;

    std::map<uint8_t, std::ofstream> streams;

    auto filePathFromLang = [&](auto lng)
    {
        auto filename = getLanguageFileName(static_cast<Language>(lng));
        return QDir(outFolder).filePath(QString::fromStdString(filename));
    };

    if (language == -1)
    {
        for (int i = 0; i < Language::LANG_COUNT; i++)
        {
            auto outPathNoExt = filePathFromLang(i);
            streams[i] = std::move(strings::startFile(outPathNoExt.toStdString(), format));
        }
    }
    else
    {
        auto outPathNoExt = filePathFromLang(language);
        streams[language] = std::move(strings::startFile(outPathNoExt.toStdString(), format));
    }

    NdsExportResult result;

    for (const QString& file : files)
    {
        const std::string filePath = file.toStdString();

        auto* entry = m_fs->findEntryByName(filePath);
        if (entry)
        {
            auto* data = m_romData + entry->start;
            auto fileInfo = QFileInfo(QString::fromStdString(filePath));
            auto fileName = fileInfo.baseName();

            auto filenameInCsv = entry->filename; // fileName.toStdString();

            if (entry->type == "p2")
            {
                P2File p2(data, entry->size);

                for (auto& os : streams)
                {
                    auto lang = static_cast<Language>(os.first);
                    if (shouldIgnoreFile(entry->filename, lang))
                        continue;

                    std::vector<String> out_strings;
                    p2.setLanguage(lang);
                    p2.extractStrings(out_strings);
                    strings::writeStrings(format, os.second, entry->start, filenameInCsv, out_strings);

                    if (!out_strings.empty())
                    {
                        NdsExportedFile f;
                        f.path = file;
                        f.stringCount = out_strings.size();
                        result.files.append(std::move(f));
                    }
                }
            }
            else if (entry->type == "z")
            {
                LZSSFile lzss(data, entry->size);
                lzss.decompress();

                for (auto& os : streams)
                {
                    auto lang = static_cast<Language>(os.first);
                    if (shouldIgnoreFile(entry->filename, lang))
                        continue;

                    uint32_t count = StringTableFile::exportStrings(os.second, lzss.getConvertedData(), format, filenameInCsv, fileInfo.suffix().toStdString(), entry->start);

                    if (count > 0)
                    {
                        NdsExportedFile f;
                        f.path = file;
                        f.stringCount = count;
                        result.files.append(std::move(f));
                    }
                }
            }
            else
            {
                // Not implemented
            }
        }
    }

    result.success = true;
    emit exportFinished(result);
}

void Worker::logCallback(const std::string& str)
{
    emit log(str);
}
