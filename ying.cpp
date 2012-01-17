#include "yingwin.h"

#include <QApplication>
#include <QDir>


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    YingWin yingWin;
    yingWin.showMaximized();

    /* check argv */
    QStringList args = QCoreApplication::arguments();
    if (args.length() > 1) {
        yingWin.analyzeFile(args[1]);
    } else {
        yingWin.analyzeFile(QDir::currentPath());
    }

    return app.exec();
}
