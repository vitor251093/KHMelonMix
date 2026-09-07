#pragma once

#include <QObject>

#include "FileEntry.h"
#include "ExportResult.h"

namespace NDS {
class NDSFileSystem;
}

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject* parent = nullptr);
    ~Worker() override;

public slots:
    void loadRom(const QString& romPath);
    void loadRomData(uint8_t* data, uint32_t size);
    void printFilesystem();
    void extractP2Files(const QString& outFolder, const QStringList& files);
    void extractZFiles(const QString& outFolder, const QStringList& files);
    void exportStrings(const QString& outFolder, const QStringList& files, uint8_t format, uint8_t language);
    void extractRawFiles(const QString& outFolder, const QStringList& files);

signals:
    void log(const std::string& str);
    void romLoaded(bool success);
    void filesystemListed(const NdsFileEntryList& entries);
    void exportFinished(const NdsExportResult& result);
    void taskFinished(bool success);

private:
    void logCallback(const std::string& str);

    void clearRom();
    NdsFileEntryList buildEntryList() const;

    std::unique_ptr<NDS::NDSFileSystem> m_fs;

    std::vector<uint8_t> m_romBuffer;
    uint8_t* m_romData = nullptr;
    uint32_t m_romDataSize = 0;
};
