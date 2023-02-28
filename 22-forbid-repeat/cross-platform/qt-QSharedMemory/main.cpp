#include "dialog.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory share("forbid_repeat_start");
    if(!share.create(1)) {
        QMessageBox::information(NULL, "提示", "禁止程序重复启动", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return -1;
    }

    Dialog w;
    w.show();
    return a.exec();
}
