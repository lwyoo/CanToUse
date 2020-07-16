#include "Tester.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#include "CANFile_global.h"

extern QMutex mutex;
extern QVector<CANMSG*> vec;

void Tester::OnConsoleBootComplete()
{
    qDebug() << Q_FUNC_INFO;
    printf("Boot Complete\n");
    mBootReady = true;
}

void Tester::waitUntil(int flags)
{
    qDebug() << Q_FUNC_INFO;
    //mProg.append(WAIT_UNTIL_BOOT_READY);

    printf("waitUntil = %d\n", flags);
}

void Tester::readData()
{
    qDebug() << Q_FUNC_INFO;
    const QByteArray data = mConsole->readAll();
    mLog.append(data);
    int from = 0;
    int index;

    while ((index = mLog.indexOf(13)) != -1)
    {
        QByteArray line = mLog.mid(0, index + 1 );
        int i;
        if (mLog.contains("Capture") && mCaptureDone == false)
           {
               mCaptureDone = true;
           }
        
        for (i = 0; i < line.length(); i++)
            putchar(line.data()[i]);
        mLog.remove(0, index + 1);
    }
    if (mLog.contains("CLU login:") && mBootReady == false)
    {
        mConsole->write("root\n", 5);
    }
    else if (mLog.contains("Password:") && mBootReady == false)
    {
        mConsole->write("tanggu\n", 7);
    }

    else if (mLog.contains("root@CLU:~#") && mBootReady == false)
    {
        OnConsoleBootComplete();
        mConsole->write("./TestAgent\n", 12);
        //LayerManagerControl dump screen 0 to /data/ccc.bmp
        mConsole->waitForBytesWritten(-1);
        mConsole->flush();
        printf("Loggin Successfully\n");
    }
}

void Tester::WaitForBootReady()
{
    qDebug() << Q_FUNC_INFO;
    //printf("WaitForBootReady\n");
    //return;

}

void writeBits(FRAME_SIGNAL* s, double val, unsigned char data[])
{
    qDebug() << Q_FUNC_INFO;
    int n = s->start / 8;
    int nn = s->start % 8;
    int vvv = 0;
    double wval = val;

    if (wval < s->min)
        wval = s->min;
    if (wval > s->max)
        wval = s->max;

    unsigned char *p = (unsigned char *) &vvv;
    int nsize = (s->length + 7) / 8;
    vvv = -1;
    vvv <<= s->length;
    vvv <<= nn;

    for (int i = 0; i < nsize; i++)
        data[n + i] &= p[i];

    vvv = (wval - s->offset) / s->factor;
    vvv <<= nn;

    for (int i = 0; i < nsize; i++)
        data[n + i] |= p[i];
}


void Tester::DoCopy(SYMBOL *a, SYMBOL *b)
{
    qDebug() << Q_FUNC_INFO;
    if (a->type == DT_CANSIG)
    {
        printf("can signal %s = %s\n", a->name, b->name);
        mutex.lock();
        writeBits(a->val.sig->s, atof(b->name), a->val.sig->msg->data);
        mutex.unlock();
    }
    else if (a->type == DT_VAR)
    {
        const QString name(a->name);

        if (name == "Bp")
        {
            if (b->val.b)
                RelayConnectChanels(1);
            else
                RelayDisconnectChanels(1);
        }
        else if (name == "Ign")
        {
            if (b->val.b)
                RelayConnectChanels(2);
            else
                RelayDisconnectChanels(2);
        }
        else if (name == "CAN")
        {
             if (b->val.b)
                 this->CANConnectChanels(0);
             else
                 this->CANDisconnectChanels(0);
        }
        printf("%s = %s\n", a->name, b->name);
    }
}

void Tester::DoActivate(SYMBOL *sym)
{
    qDebug() << Q_FUNC_INFO;
    mutex.lock();
    printf("activate : %s\n", sym->name);
    vec.append(sym->val.msg);
    mutex.unlock();
}

void Tester::DoCallFunc(SYMBOL *sym)
{
    qDebug() << Q_FUNC_INFO;
    QHash<QString, fff>::const_iterator i =  m_funcTable.find(QString(sym->name));
    fff f = nullptr;

    if (i != m_funcTable.end()) {
        f = i.value();
        (this->*f)(sym, mStack);
    }

    if (sym->val.f->retType != 0)
    {
        SYMBOL * ret = new SYMBOL("ret", DT_VOID);
        mStack.push(ret);
    }
}

bool Tester::ReadCANDB(const char *filePath)
{
    qDebug() << Q_FUNC_INFO;
    return dl->Load(filePath);
}
