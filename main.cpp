#include "Tester.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Tester t;

    t.ReadCANDB("../../data/a.dbc");

    t.ReadTestCase("./telltale.tc");
    t.Run();

    //t.GenReport("telltale.tex");

    return a.exec();
    //return 0;
}
