#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CHtmlRender rd;
    MainWindow w(rd);
    auto width = QApplication::desktop()->width();
    auto height = QApplication::desktop()->height();
    w.setMinimumSize(QSize(2/3*width,1/2*height));
    w.show();
    w.move((width - w.width())/2,(height - w.height())/2);
    return a.exec();
}
