#include "Tester.h"
#include <QApplication>
#include <QThread>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Tester t;
//    t.ReadCANDB("../../data/a.dbc");
//    t.ReadTestCase("./telltale.tc");
//    t.Run();

//    for (int i = 0 ; i < 100; i++) {
//        qDebug() << "test index : " << i ;
//        QThread::sleep(1);
//    }

    //t.GenReport("telltale.tex");


    return a.exec();
    //return 0;
}
