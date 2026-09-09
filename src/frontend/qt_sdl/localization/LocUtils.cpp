#include "LocUtils.h"

#include <QAction>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHeaderView>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSet>
#include <QStatusBar>
#include <QTextCursor>
#include <QTreeWidgetItem>

#include "Worker.h"

#include "localization/lang.h"
#include "localization/strings.h"
#include "localization/handler.h"

namespace
{
    constexpr int ColumnName = 0;
    constexpr int ColumnSize = 1;
    constexpr int ColumnType = 2;

    constexpr int PathRole   = Qt::UserRole + 1;
    constexpr int SizeRole   = Qt::UserRole + 2;
    constexpr int IsFileRole = Qt::UserRole + 3;

    void setExpandedRecursive(QTreeWidgetItem* item, bool expanded)
    {
        item->setExpanded(expanded);
        for (int i = 0; i < item->childCount(); ++i)
            setExpandedRecursive(item->child(i), expanded);
    }

    QString humanSize(quint64 bytes)
    {
        if (bytes < 1024)
            return QString("%1 B").arg(bytes);
        if (bytes < 1024ull * 1024ull)
            return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
}

LocUtils::LocUtils(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    auto font = QFont(ui.textEditLog->font());
    font.setPointSize(8);
    ui.textEditLog->setFont(font);

    ui.splitterMain->setStretchFactor(0, 2);   // tree
    ui.splitterMain->setStretchFactor(1, 3);   // log

    ui.treeFiles->header()->setSectionResizeMode(ColumnName, QHeaderView::Interactive);
    ui.treeFiles->header()->setStretchLastSection(false);
    ui.treeFiles->setColumnWidth(ColumnName, 260);
    ui.treeFiles->setColumnWidth(ColumnSize, 80);

    qRegisterMetaType<NdsFileEntryList>("NdsFileEntryList");
    qRegisterMetaType<NdsExportResult>("NdsExportResult");

    m_worker = new Worker();
    m_worker->moveToThread(&m_workerThread);

    connect(m_worker, &Worker::log, this, &LocUtils::appendLog);
    connect(m_worker, &Worker::romLoaded, this, &LocUtils::onRomLoaded);
    connect(m_worker, &Worker::filesystemListed,  this, &LocUtils::onFilesystemListed);
    connect(m_worker, &Worker::taskFinished, this, &LocUtils::onTaskFinished);
    connect(m_worker, &Worker::exportFinished, this, &LocUtils::onExportFinished);

    connect(this, &LocUtils::requestLoadRom, m_worker, &Worker::loadRom);
    connect(this, &LocUtils::requestLoadRomData, m_worker, &Worker::loadRomData);
    connect(this, &LocUtils::requestExtractP2Files, m_worker, &Worker::extractP2Files);
    connect(this, &LocUtils::requestExtractZFiles, m_worker, &Worker::extractZFiles);
    connect(this, &LocUtils::requestExtractRawFiles, m_worker, &Worker::extractRawFiles);
    connect(this, &LocUtils::requestExportStrings, m_worker, &Worker::exportStrings);

    ui.comboLanguage->addItem("All", 0);

    for (int i = 0; i < ndsloc::Language::LANG_COUNT; i++)
    {
        auto langName = ndsloc::getLanguageName(static_cast<ndsloc::Language>(i));
        ui.comboLanguage->addItem(QString::fromStdString(langName), i + 1);
    }

    ui.comboLanguage->setCurrentIndex(ndsloc::Language::LANG_EN + 1);

    connect(ui.treeFiles, &QTreeWidget::itemChanged, this, &LocUtils::onTreeItemChanged);

    ui.treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeFiles, &QTreeWidget::customContextMenuRequested, this, &LocUtils::onTreeContextMenu);

    m_workerThread.start();

    updateUiState();
}

LocUtils::~LocUtils()
{
    m_workerThread.quit();
    m_workerThread.wait();

    delete m_worker;
    m_worker = nullptr;
}

void LocUtils::updateUiState()
{
    const bool idle = !m_bIsBusy;

    ui.treeFiles->setEnabled(idle);
    ui.buttonSelectAll->setEnabled(idle && m_bRomLoaded);
    ui.buttonSelectNone->setEnabled(idle && m_bRomLoaded && m_selectedCount > 0);

    ui.comboLanguage->setEnabled(idle && m_bRomLoaded);
    ui.buttonExportStringsToCsv->setEnabled(idle && m_bRomLoaded && m_selectedCount > 0);

    emit busyChanged(m_bIsBusy);
}

void LocUtils::beginTask()
{
    m_bIsBusy = true;
    updateUiState();
}

void LocUtils::loadRom(QString romPath)
{
    m_bRomLoaded = false;
    m_selectedCount = 0;

    clearFileTree();

    beginTask();
    emit requestLoadRom(romPath);
}

void LocUtils::loadRomData(uint8_t* data, uint32_t size)
{
    m_bRomLoaded = false;
    m_selectedCount = 0;

    clearFileTree();

    beginTask();
    emit requestLoadRomData(data, size);
}

void LocUtils::on_buttonExportStringsToCsv_clicked()
{
    const int8_t lang = selectedLanguage();
    QString startDir;

    if (m_handler != nullptr)
    {
        const auto probeLang = (lang == -1) ? ndsloc::Language::LANG_EN : static_cast<ndsloc::Language>(lang);
        const std::string localizationFilePath = m_handler->getLocalizationFilePath(ndsloc::getLanguageFileName(probeLang));

        if (!localizationFilePath.empty())
        {
            const QString folder = QFileInfo(QString::fromStdString(localizationFilePath)).path();
            bool result = QDir().mkpath(folder);
            startDir = folder;
        }
    }

    const QString path = QFileDialog::getExistingDirectory(this, tr("Select target folder"), startDir);

    if (path.isEmpty())
        return;

    const QStringList files = selectedFiles();
    if (files.isEmpty())
        return;

    beginTask();
    m_worker->setLocHandler(m_handler);
    emit requestExportStrings(path, files, ndsloc::ExportFormat::Csv, lang);
}

int8_t LocUtils::selectedLanguage() const
{
    const QVariant data = ui.comboLanguage->currentData();
    return data.isValid() ? (data.toInt() - 1) : -1;
}

void LocUtils::on_buttonSelectAll_clicked()
{
    setAllChecked(Qt::Checked);
}

void LocUtils::on_buttonSelectNone_clicked()
{
    setAllChecked(Qt::Unchecked);
}

void LocUtils::appendLog(const std::string& text)
{
    ui.textEditLog->moveCursor(QTextCursor::End);
    ui.textEditLog->insertPlainText(QString::fromStdString(text));
    ui.textEditLog->moveCursor(QTextCursor::End);
}

void LocUtils::appendLogLine(const QString& text)
{
    appendLog(text.toStdString() + "\n");
}

void LocUtils::onRomLoaded(bool success)
{
    m_bRomLoaded = success;
    m_bIsBusy = false;

    if (!success)
    {
        clearFileTree();
        m_selectedCount = 0;
    }

    updateUiState();

    if (success)
        appendLog("~~ rom loaded ~~\n");
    else
        appendLog("~~ failed to load rom ~~\n");
}

void LocUtils::onFilesystemListed(const NdsFileEntryList& entries)
{
    populateFileTree(entries);
}

void LocUtils::onTaskFinished(bool success)
{
    m_bIsBusy = false;
    updateUiState();

    if (success)
        appendLog("~~ finished! ~~\n");
}

void LocUtils::onExportFinished(const NdsExportResult& result)
{
    m_bIsBusy = false;
    updateUiState();

    if (!result.success)
    {
        appendLogLine(QStringLiteral("~~ export failed ~~"));
        return;
    }

    if (result.files.isEmpty())
    {
        appendLogLine(QStringLiteral("~~ export finished: no strings found ~~"));
        return;
    }

    int widest = 0;
    for (const NdsExportedFile& file : result.files)
    {
        widest = qMax(widest, file.path.size());
    }

    appendLogLine(QStringLiteral("~~ exported %1 file(s), %2 string(s) ~~").arg(result.files.size()).arg(result.totalStrings()));

    for (const NdsExportedFile& file : result.files)
    {
        appendLogLine(QStringLiteral("  %1  %2").arg(file.path.leftJustified(widest, ' ')).arg(file.stringCount, 6));
    }
}

void LocUtils::clearFileTree()
{
    m_updatingTree = true;
    m_fileItems.clear();
    ui.treeFiles->clear();
    m_updatingTree = false;
}

void LocUtils::populateFileTree(const NdsFileEntryList& entries)
{
    m_updatingTree = true;
    ui.treeFiles->setUpdatesEnabled(false);

    m_fileItems.clear();
    ui.treeFiles->clear();

    QHash<QString, QTreeWidgetItem*> folders;

    for (const NdsFileEntry& entry : entries)
    {
        const QStringList parts = entry.path.split('/', Qt::SkipEmptyParts);
        if (parts.isEmpty())
            continue;

        QTreeWidgetItem* parent = nullptr;
        QString folderPath;

        for (int i = 0; i < parts.size() - 1; ++i)
        {
            folderPath += '/' + parts[i];

            auto it = folders.find(folderPath);
            if (it == folders.end())
            {
                auto* folder = parent ? new QTreeWidgetItem(parent)
                                      : new QTreeWidgetItem(ui.treeFiles);
                folder->setText(ColumnName, parts[i]);
                folder->setText(ColumnType, QStringLiteral("Folder"));
                folder->setFlags(folder->flags() | Qt::ItemIsUserCheckable);
                folder->setCheckState(ColumnName, Qt::Unchecked);
                folder->setData(ColumnName, PathRole, folderPath);
                folder->setData(ColumnName, IsFileRole, false);

                it = folders.insert(folderPath, folder);
            }

            parent = it.value();
        }

        const QString& fileName = parts.last();

        auto* file = parent ? new QTreeWidgetItem(parent)
                            : new QTreeWidgetItem(ui.treeFiles);
        file->setText(ColumnName, fileName);
        file->setText(ColumnSize, humanSize(entry.size));
        file->setTextAlignment(ColumnSize, Qt::AlignRight | Qt::AlignVCenter);
        file->setText(ColumnType, entry.type);
        file->setFlags(file->flags() | Qt::ItemIsUserCheckable);
        file->setCheckState(ColumnName, Qt::Unchecked);
        file->setData(ColumnName, PathRole, entry.path);
        file->setData(ColumnName, SizeRole, QVariant::fromValue(entry.size));
        file->setData(ColumnName, IsFileRole, true);

        m_fileItems.insert(entry.path, file);
    }

    ui.treeFiles->expandToDepth(0);
    ui.treeFiles->setUpdatesEnabled(true);
    m_updatingTree = false;

    m_selectedCount = 0;

    applyDefaultSelection();

    updateSelectionInfo();
    updateUiState();
}

void LocUtils::onTreeItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updatingTree || column != ColumnName || item == nullptr)
        return;

    m_updatingTree = true;

    const Qt::CheckState state = item->checkState(ColumnName);
    if (item->childCount() > 0 && state != Qt::PartiallyChecked)
        setCheckStateRecursive(item, state);

    refreshParentCheckState(item->parent());

    m_updatingTree = false;

    updateSelectionInfo();
    updateUiState();
}

void LocUtils::setCheckStateRecursive(QTreeWidgetItem* item, Qt::CheckState state)
{
    for (int i = 0; i < item->childCount(); ++i)
    {
        QTreeWidgetItem* child = item->child(i);
        child->setCheckState(ColumnName, state);
        setCheckStateRecursive(child, state);
    }
}

void LocUtils::refreshParentCheckState(QTreeWidgetItem* item)
{
    while (item != nullptr)
    {
        int checked = 0;
        int partial = 0;
        const int count = item->childCount();

        for (int i = 0; i < count; ++i)
        {
            switch (item->child(i)->checkState(ColumnName))
            {
            case Qt::Checked: ++checked; break;
            case Qt::PartiallyChecked: ++partial; break;
            default: break;
            }
        }

        Qt::CheckState state = Qt::Unchecked;
        if (partial > 0 || (checked > 0 && checked < count))
            state = Qt::PartiallyChecked;
        else if (count > 0 && checked == count)
            state = Qt::Checked;

        item->setCheckState(ColumnName, state);
        item = item->parent();
    }
}

void LocUtils::recomputeFolderStates(QTreeWidgetItem* item)
{
    const int count = item->childCount();

    int checked = 0;
    int partial = 0;

    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* child = item->child(i);

        if (!child->data(ColumnName, IsFileRole).toBool())
            recomputeFolderStates(child);

        switch (child->checkState(ColumnName))
        {
        case Qt::Checked:          ++checked; break;
        case Qt::PartiallyChecked: ++partial; break;
        default:                             break;
        }
    }

    if (item == ui.treeFiles->invisibleRootItem())
        return;

    Qt::CheckState state = Qt::Unchecked;
    if (partial > 0 || (checked > 0 && checked < count))
        state = Qt::PartiallyChecked;
    else if (count > 0 && checked == count)
        state = Qt::Checked;

    item->setCheckState(ColumnName, state);
}

void LocUtils::setAllChecked(Qt::CheckState state)
{
    m_updatingTree = true;
    ui.treeFiles->setUpdatesEnabled(false);

    setCheckStateRecursive(ui.treeFiles->invisibleRootItem(), state);

    ui.treeFiles->setUpdatesEnabled(true);
    m_updatingTree = false;

    updateSelectionInfo();
    updateUiState();
}

void LocUtils::setItemsChecked(const QList<QTreeWidgetItem*>& items, Qt::CheckState state)
{
    if (items.isEmpty())
        return;

    m_updatingTree = true;
    ui.treeFiles->setUpdatesEnabled(false);

    for (QTreeWidgetItem* item : items)
    {
        item->setCheckState(ColumnName, state);
        setCheckStateRecursive(item, state);
    }

    recomputeFolderStates(ui.treeFiles->invisibleRootItem());

    ui.treeFiles->setUpdatesEnabled(true);
    m_updatingTree = false;

    updateSelectionInfo();
    updateUiState();
}

void LocUtils::collectFilePaths(const QTreeWidgetItem* item, QStringList& out) const
{
    for (int i = 0; i < item->childCount(); ++i)
    {
        const QTreeWidgetItem* child = item->child(i);

        if (child->data(ColumnName, IsFileRole).toBool())
            out << child->data(ColumnName, PathRole).toString();
        else
            collectFilePaths(child, out);
    }
}

void LocUtils::collectCheckedFiles(const QTreeWidgetItem* item, QStringList& out) const
{
    for (int i = 0; i < item->childCount(); ++i)
    {
        const QTreeWidgetItem* child = item->child(i);

        if (child->data(ColumnName, IsFileRole).toBool())
        {
            if (child->checkState(ColumnName) == Qt::Checked)
                out << child->data(ColumnName, PathRole).toString();
        }
        else
        {
            collectCheckedFiles(child, out);
        }
    }
}

QStringList LocUtils::selectedFiles() const
{
    QStringList out;
    collectCheckedFiles(ui.treeFiles->invisibleRootItem(), out);
    return out;
}

void LocUtils::updateSelectionInfo()
{
    m_selectedCount = selectedFiles().size();

    ui.statusBar->setText(m_selectedCount == 0
        ? QStringLiteral("No file selected")
        : QStringLiteral("%1 file(s) selected").arg(m_selectedCount));
}

QList<QTreeWidgetItem*> LocUtils::matchPattern(const QString& pattern) const
{
    QList<QTreeWidgetItem*> matches;

    const bool isWildcard = pattern.contains('*') || pattern.contains('?');
    const bool isFullPath = pattern.startsWith('/');

    if (!isWildcard && isFullPath)
    {
        const auto it = m_fileItems.find(pattern);
        if (it != m_fileItems.end())
            matches << it.value();
        return matches;
    }

    QRegularExpression regex;
    if (isWildcard)
    {
        regex = QRegularExpression(
            QRegularExpression::anchoredPattern(
                QRegularExpression::wildcardToRegularExpression(pattern)),
            QRegularExpression::CaseInsensitiveOption);
    }

    for (auto it = m_fileItems.constBegin(); it != m_fileItems.constEnd(); ++it)
    {
        const QString& fullPath = it.key();
        const QString fileName = fullPath.section('/', -1);

        const bool hit = isWildcard
            ? (regex.match(fullPath).hasMatch() || regex.match(fileName).hasMatch())
            : (isFullPath ? fullPath.compare(pattern, Qt::CaseInsensitive) == 0
                          : fileName.compare(pattern, Qt::CaseInsensitive) == 0);

        if (hit)
            matches << it.value();
    }

    return matches;
}

void LocUtils::expandAncestors(QTreeWidgetItem* item)
{
    for (QTreeWidgetItem* parent = item->parent(); parent != nullptr; parent = parent->parent())
        parent->setExpanded(true);
}

void LocUtils::applyDefaultSelection()
{
    for (auto* item : m_fileItems)
    {
        item->setCheckState(ColumnName, Qt::Checked);
    }
}


void LocUtils::onTreeContextMenu(const QPoint& pos)
{
    if (m_bIsBusy)
        return;

    QTreeWidgetItem* clicked = ui.treeFiles->itemAt(pos);

    QMenu menu(this);

    if (clicked == nullptr)
    {
        QAction* expandAll = menu.addAction(tr("Expand all"));
        connect(expandAll, &QAction::triggered, ui.treeFiles, &QTreeWidget::expandAll);

        QAction* collapseAll = menu.addAction(tr("Collapse all"));
        connect(collapseAll, &QAction::triggered, ui.treeFiles, &QTreeWidget::collapseAll);

        menu.exec(ui.treeFiles->viewport()->mapToGlobal(pos));
        return;
    }

    if (!ui.treeFiles->selectedItems().contains(clicked))
        ui.treeFiles->setCurrentItem(clicked);

    const TreeContext ctx = buildTreeContext(clicked);

    if (ctx.hasFolders)
        addFolderActions(menu, ctx);

    if (ctx.hasFiles)
        addFileActions(menu, ctx);

    addCommonActions(menu, ctx);

    if (!menu.isEmpty())
    {
        menu.exec(ui.treeFiles->viewport()->mapToGlobal(pos));
    }
}

LocUtils::TreeContext LocUtils::buildTreeContext(QTreeWidgetItem* clicked) const
{
    TreeContext ctx;
    ctx.clicked = clicked;

    QList<QTreeWidgetItem*> selection = ui.treeFiles->selectedItems();
    if (selection.isEmpty())
        selection << clicked;

    const QSet<QTreeWidgetItem*> selectionSet(selection.begin(), selection.end());

    for (QTreeWidgetItem* item : selection)
    {
        bool coveredByAncestor = false;
        for (QTreeWidgetItem* p = item->parent(); p != nullptr; p = p->parent())
        {
            if (selectionSet.contains(p))
            {
                coveredByAncestor = true;
                break;
            }
        }

        if (!coveredByAncestor)
            ctx.items << item;
    }


    QStringList types;

    for (QTreeWidgetItem* item : ctx.items)
    {
        if (item->data(ColumnName, IsFileRole).toBool())
        {
            ctx.hasFiles = true;
            ctx.filePaths << item->data(ColumnName, PathRole).toString();

            types.append(item->text(ColumnType));
        }
        else
        {
            ctx.hasFolders = true;
            collectFilePaths(item, ctx.filePaths);
        }
    }

    QString commonType = "";
    bool uniform = !ctx.filePaths.isEmpty();
    for (auto& type : types)
    {
        if (commonType.isEmpty())
            commonType = type;
        else if (commonType != type)
        {
            uniform = false;
            break;
        }
    }

    if (uniform)
        ctx.commonType = commonType;

    return ctx;
}

void LocUtils::addFolderActions(QMenu& menu, const TreeContext& ctx)
{
    QAction* expand = menu.addAction(tr("Expand"));
    connect(expand, &QAction::triggered, this, [ctx]() {
        for (QTreeWidgetItem* item : ctx.items)
            setExpandedRecursive(item, true);
    });

    QAction* collapse = menu.addAction(tr("Collapse"));
    connect(collapse, &QAction::triggered, this, [ctx]() {
        for (QTreeWidgetItem* item : ctx.items)
            setExpandedRecursive(item, false);
    });

    menu.addSeparator();
}

void LocUtils::addFileActions(QMenu& menu, const TreeContext& ctx)
{
    int fileCount = ctx.filePaths.size();
    auto getActionName = [&fileCount](const char* name, const char* type) -> QString
    {
        return (fileCount > 1)
            ? tr("%1 %2 %3 files...").arg(name, QString::number(fileCount), type)
            : tr("%1 %2 file...").arg(name, type);
    };

    if (ctx.commonType == QLatin1String("p2"))
    {
        QAction* extract = menu.addAction(getActionName("Extract and decompress", "P2"));
        connect(extract, &QAction::triggered, this, [this, ctx]() { actionExtractFile(ctx, EExtractType::P2); });
    }
    else if (ctx.commonType == QLatin1String("z"))
    {
        QAction* decompress = menu.addAction(getActionName("Decompress", "Z"));
        connect(decompress, &QAction::triggered, this, [this, ctx]() { actionExtractFile(ctx, EExtractType::Z); });
    }

    QAction* extractRaw = menu.addAction(getActionName("Extract", "raw"));
    connect(extractRaw, &QAction::triggered, this, [this, ctx]() { actionExtractFile(ctx, EExtractType::Raw); });

    menu.addSeparator();
}

void LocUtils::addCommonActions(QMenu& menu, const TreeContext& ctx)
{
    QAction* tick = menu.addAction(tr("Tick for export"));
    tick->setEnabled(!ctx.items.isEmpty());
    connect(tick, &QAction::triggered, this, [this, ctx]() {
        setItemsChecked(ctx.items, Qt::Checked);
    });

    QAction* untick = menu.addAction(tr("Untick"));
    untick->setEnabled(!ctx.items.isEmpty());
    connect(untick, &QAction::triggered, this, [this, ctx]() {
        setItemsChecked(ctx.items, Qt::Unchecked);
    });

    menu.addSeparator();

    QAction* copyPath = menu.addAction(ctx.items.size() > 1
        ? tr("Copy %1 paths").arg(ctx.items.size())
        : tr("Copy path"));
    connect(copyPath, &QAction::triggered, this, [ctx]() {
        QStringList paths;
        for (QTreeWidgetItem* item : ctx.items)
            paths << item->data(ColumnName, PathRole).toString();

        QGuiApplication::clipboard()->setText(paths.join('\n'));
    });
}

void LocUtils::actionExtractFile(const TreeContext& ctx, EExtractType type)
{
    if (ctx.filePaths.isEmpty())
        return;

    const QString outFolder = QFileDialog::getExistingDirectory(
        this,
        tr("Extract %1 file(s) to...").arg(ctx.filePaths.size()),
        m_lastExtractFolder);

    if (outFolder.isEmpty())
        return;

    if (!QFileInfo(outFolder).isWritable())
    {
        QMessageBox::warning(this, tr("Extract"), tr("This folder is not writable:\n%1").arg(outFolder));
        return;
    }

    m_lastExtractFolder = outFolder;

    appendLog("Extracting " + std::to_string(ctx.filePaths.size()) + " file(s) to: " + outFolder.toStdString() + "\n");

    switch (type)
    {
    case EExtractType::Raw:
        emit requestExtractRawFiles(outFolder, ctx.filePaths);
        break;
    case EExtractType::P2:
        emit requestExtractP2Files(outFolder, ctx.filePaths);
        break;
    case EExtractType::Z:
        emit requestExtractZFiles(outFolder, ctx.filePaths);
        break;
    default:
        assert(false);
    }
}
