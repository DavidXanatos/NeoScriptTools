#include "DebuggerDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DebuggerDemo w;
    w.show();
    return a.exec();
}
