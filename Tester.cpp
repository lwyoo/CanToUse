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
    qDebug("Boot Complete\n");
    mBootReady = true;
}

void Tester::waitUntil(int flags)
{
    qDebug() << Q_FUNC_INFO;
    //mProg.append(WAIT_UNTIL_BOOT_READY);

    qDebug("waitUntil = %d\n", flags);
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
        qDebug("Loggin Successfully\n");
    }
}

void Tester::WaitForBootReady()
{
    qDebug() << Q_FUNC_INFO;
    //qDebug("WaitForBootReady\n");
    //return;

}

void writeBits(FRAME_SIGNAL* s, double val, unsigned char data[])
{
    qDebug() << Q_FUNC_INFO << "input arg val : " << val ;


    //    SG_ CF_Gway_PDWRsound : 6|2@1+ (1,0) [0|3] ""  CLU
    //    startbit | length @1+ (factor,offset) [min|max]
    int startByte = s->start / 8;
    qDebug() << Q_FUNC_INFO << "s->start / 8 : " << startByte; // 시작 비트 n
    int startBitValue = s->start % 8;
    qDebug() << Q_FUNC_INFO << "s->start % 8 : " << startBitValue; // 시프트 연산을 위한 값 startBitValue
    int maskValue = 0;
    double rawValue = val;

    if (rawValue < s->min)
        rawValue = s->min;
    if (rawValue > s->max)
        rawValue = s->max;

    unsigned char *p = (unsigned char *) &maskValue;
    int nsize = (s->length + 7) / 8; // 14 + 7 /8
    maskValue = -1;
    maskValue <<= s->length;
    maskValue <<= startBitValue;


    //    0000 0000

    //    0010 0000
    //    0100 0000
    //    0110 0000
    //    1000 0000

//    1000 0000 0110 0000 0100 0000 0010 0000

    //8byte//1111 1111     1111 1111      1111 1111      1111 1111      1111 1111      1111 1111      1100 0000      0000 0000
    //data //0000 0000     0000 0000      0101 0101      1010 1100      1010 1101      1101 1101      1010 1111      1000 0100
    //------------------------------------------------------------------------------------------------------------------------
    //&&&&&//0000 0000     0000 0000      0101 0101      1010 1100      1010 1101      1101 1101      1000 0000      0000 0000

    for (int i = 0; i < nsize; i++) {
        qDebug() << Q_FUNC_INFO << "data[" << startByte + i << "] : " << data[startByte + i] <<  " p["<< i << "] :  " << p[i];
        data[startByte + i] &= p[i];
    }


    // rawValue = (physValue - s->offset) / s->factor;
    // rawValue * s->factor + s->offset = physValue
    // s->factor * rawValue + s->offset =  physValue



    maskValue = (rawValue - s->offset) / s->factor;
    qDebug() << "s->offset : " << s->offset << " s->factor : " << s->factor << " maskValue : " << maskValue  << " rawValue : " << rawValue;
    maskValue <<= startBitValue;
    qDebug() << "s->offset : " << s->offset << " s->factor : " << s->factor << " maskValue : " << maskValue  << " rawValue : " << rawValue;


    for (int i = 0; i < nsize; i++)
        data[startByte + i] |= p[i];

    /////////////////
    //get data test//
    /////////////////

    for (int i = 0; i < nsize; i++) {

        unsigned char tmepValue;
        qDebug() << "dldyddn data[startByte + " << i << "] : " << data[startByte + i];
    }





    qDebug() << Q_FUNC_INFO << "end";
}


void Tester::DoCopy(SYMBOL *a, SYMBOL *b)
{
    qDebug() << Q_FUNC_INFO;
    if (a->type == DT_CANSIG)
    {
        qDebug() << Q_FUNC_INFO << "if (a->type == DT_CANSIG)";
        qDebug("can signal %s = %s\n", a->name, b->name);
        mutex.lock();
        writeBits(a->val.sig->s, atof(b->name), a->val.sig->msg->data);
        for (int i = 0 ; i < a->val.sig->msg->dlc ; i++) {
            qDebug("@@@@@@@@@@@ make data index [%d] data [%d]", i, a->val.sig->msg->data[i]);
        }
        mutex.unlock();
    }
    else if (a->type == DT_VAR)
    {
        qDebug() << Q_FUNC_INFO << "else if (a->type == DT_VAR)";
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
        qDebug("%s = %s\n", a->name, b->name);
    } else {
        qDebug() << Q_FUNC_INFO << "else";
    }
}

void Tester::DoActivate(SYMBOL *sym)
{


    mutex.lock();
    qDebug("activate : %s\n", sym->name);
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
