#pragma once

#include <QDialog>

class EmuInstance;
class FileSystemDialog;

class FileSystemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileSystemDialog(QWidget* parent);
    ~FileSystemDialog();

    static FileSystemDialog* currentDlg;
    static FileSystemDialog* openDlg(QWidget* parent);

    static void closeDlg()
    {
        currentDlg = nullptr;
    }

private slots:
    void done(int r);

private:
    EmuInstance* emuInstance = nullptr;
};
