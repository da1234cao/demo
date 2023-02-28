#include "dialog.h"

#include <QApplication>
#include <QLockFile>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QLockFile lock("./test.lock");
    if(!lock.tryLock(1)) {
        QMessageBox::information(NULL,"提示","禁止重复启动程序",QMessageBox::Ok,QMessageBox::Ok);
        return -1;
    }

    Dialog w;
    w.show();
    return a.exec();
}
