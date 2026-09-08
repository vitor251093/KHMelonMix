#pragma once

#include <QtWidgets/QWidget>
#include <QStringList>
#include <QThread>

#include "ui_LocUtils.h"

#include "FileEntry.h"
#include "ExportResult.h"

#include <string>

class QTreeWidgetItem;
class Worker;

namespace Plugins { class Plugin; }

class LocUtils : public QWidget
{
    Q_OBJECT

public:
    LocUtils(QWidget* parent = nullptr);
    ~LocUtils();

    void loadRom(QString romPath);
    void loadRomData(uint8_t* data, uint32_t size);

    bool isBusy() const { return m_bIsBusy; }

    void setPlugin(Plugins::Plugin* plugin) { m_plugin = plugin; }
signals:
    void busyChanged(bool busy);
    void requestLoadRom(const QString& romPath);
    void requestLoadRomData(uint8_t* data, uint32_t size);
    void requestExtractP2Files(const QString& outFolder, const QStringList& files);
    void requestExtractZFiles(const QString& outFolder, const QStringList& files);
    void requestExtractRawFiles(const QString& outFolder, const QStringList& files);
    void requestExportStrings(const QString& outFolder, const QStringList& files, uint8_t format, int8_t language);

private slots:
    void on_buttonExportStringsToCsv_clicked();
    void on_buttonSelectAll_clicked();
    void on_buttonSelectNone_clicked();

private:
    struct TreeContext
    {
        QTreeWidgetItem* clicked = nullptr;
        QList<QTreeWidgetItem*> items;
        QStringList filePaths;

        bool hasFiles = false;
        bool hasFolders = false;
        QString commonType;
    };

    void appendLog(const std::string& text);
    void appendLogLine(const QString& text);
    void onRomLoaded(bool success);
    void onFilesystemListed(const NdsFileEntryList& entries);
    void onTaskFinished(bool success);
    void onExportFinished(const NdsExportResult& result);

    void clearFileTree();
    void populateFileTree(const NdsFileEntryList& entries);
    void onTreeItemChanged(QTreeWidgetItem* item, int column);
    void setCheckStateRecursive(QTreeWidgetItem* item, Qt::CheckState state);
    void refreshParentCheckState(QTreeWidgetItem* item);
    void recomputeFolderStates(QTreeWidgetItem* item);
    void setAllChecked(Qt::CheckState state);
    void setItemsChecked(const QList<QTreeWidgetItem*>& items, Qt::CheckState state);
    void collectCheckedFiles(const QTreeWidgetItem* item, QStringList& out) const;
    void collectFilePaths(const QTreeWidgetItem* item, QStringList& out) const;
    QStringList selectedFiles() const;
    void updateSelectionInfo();

    void applyDefaultSelection();
    QList<QTreeWidgetItem*> matchPattern(const QString& pattern) const;
    void expandAncestors(QTreeWidgetItem* item);

    void onTreeContextMenu(const QPoint& pos);
    TreeContext buildTreeContext(QTreeWidgetItem* clicked) const;
    void addFolderActions(QMenu& menu, const TreeContext& ctx);
    void addFileActions(QMenu& menu, const TreeContext& ctx);
    void addCommonActions(QMenu& menu, const TreeContext& ctx);

    enum EExtractType : uint8_t { Raw = 0, P2, Z };
    void actionExtractFile(const TreeContext& ctx, EExtractType type);

    int8_t selectedLanguage() const;

    void setBusy(bool busy);
    void beginTask();
    void updateUiState();

    Ui::LocUtilsClass ui;

    QThread m_workerThread;
    Worker* m_worker = nullptr;

    bool m_bIsBusy = false;
    bool m_bRomLoaded = false;
    bool m_updatingTree = false;
    int m_selectedCount = 0;

    QString m_lastExtractFolder;
    QHash<QString, QTreeWidgetItem*> m_fileItems;

    Plugins::Plugin* m_plugin = nullptr;
};
