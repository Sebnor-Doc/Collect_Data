#include "TTS_GUI.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    a.exec();

    return 1;
}