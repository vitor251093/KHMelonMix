#include "FileSystemDialog.h"

#include <QFileDialog>
#include <QVBoxLayout>

#include "Platform.h"
#include "main.h"

#include "LocUtils.h"

using namespace melonDS::Platform;
namespace Platform = melonDS::Platform;

FileSystemDialog* FileSystemDialog::currentDlg = nullptr;


FileSystemDialog::FileSystemDialog(QWidget* parent)
    : QDialog(parent)
{
    emuInstance = ((MainWindow*)parent)->getEmuInstance();

    QVBoxLayout* layout = new QVBoxLayout(this);

    auto* locUtils = new LocUtils(this);
    locUtils->setPlugin(emuInstance->plugin);

    auto* cart = emuInstance->getNDS()->GetNDSCart();
    locUtils->loadRomData((uint8_t*)cart->GetROM(), cart->GetROMLength());

    layout->addWidget(locUtils);

    setWindowTitle("File System - melonDS");
    setAttribute(Qt::WA_DeleteOnClose);
}

FileSystemDialog::~FileSystemDialog()
{
}

void FileSystemDialog::done(int r)
{
    QDialog::done(r);
    closeDlg();
}

FileSystemDialog* FileSystemDialog::openDlg(QWidget* parent)
{
    if (currentDlg)
    {
        currentDlg->activateWindow();
        return currentDlg;
    }

    currentDlg = new FileSystemDialog(parent);
    currentDlg->open();
    return currentDlg;
}
