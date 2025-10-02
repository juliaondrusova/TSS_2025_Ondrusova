#include "TSS_App.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TSS_App window;
    window.show();
    return app.exec();
}
