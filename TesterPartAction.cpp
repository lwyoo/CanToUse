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
#include <QCanBus>
#include <QCanBusFrame>
#include "CANFile_global.h"

bool Tester::Init2()
{
    qDebug() << Q_FUNC_INFO;
    QString errorString;
    qDebug() << Q_FUNC_INFO;
    qDebug() << "WebSocket server:" << QUrl(QStringLiteral("ws://localhost:1234"));
    QUrl tempUrl = QUrl(QStringLiteral("ws://localhost:1234"));
    connect(&m_webSocket, &QWebSocket::connected, this, &Tester::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Tester::closed);
    m_webSocket.open(QUrl(tempUrl));
    return true;
}

QMutex mutex;
QVector<CANMSG*> vec;
static bool runSenderThread = true;

SYMBOL_TABLE::SYMBOL_TABLE()
{
    qDebug() << Q_FUNC_INFO;

}

SYMBOL_TABLE::~SYMBOL_TABLE()
{
    qDebug() << Q_FUNC_INFO;

}

void SYMBOL_TABLE::Clear()
{
    qDebug() << Q_FUNC_INFO;
    m_hash.clear();
    mVars.clear();
}

SYMBOL* SYMBOL_TABLE::Insert(const char* name, dataType dt)
{
    qDebug() << Q_FUNC_INFO;
    printf("Insert (%d)%s\n", dt, name);
    SYMBOL* newSymbol = new SYMBOL(name, dt);
    m_hash.insert(QString(name), newSymbol);
    return newSymbol;
}

SYMBOL* SYMBOL_TABLE::Search(const char* name)
{
    qDebug() << Q_FUNC_INFO;
    SYMBOL*ret = nullptr;

    QHash<QString, SYMBOL*>::const_iterator i = m_hash.find(QString(name));

    if (i != m_hash.end()) {
        ret = i.value();
    }
    return ret;
}

class Sender : public QThread
{
public:
    virtual void run() override;
private:
    unsigned long mCycle;
};


void Sender::run()
{
    qDebug() << Q_FUNC_INFO;
    mCycle = 10;
    long timeStamp = 0;
    QVector<CANMSG> localvec;
    bool loop = true;

    for (;;)
    {
        int nCount = 0;

        nCount = 0;
        mutex.lock();
        if (runSenderThread)
        {
            localvec.clear();
            for (CANMSG* m : vec)
            {
                m->timeStamp -= mCycle;
                if (m->timeStamp <= 0)
                {
                    m->timeStamp += m->cycle;
                    localvec.append(*m);
                    nCount++;
                }
            }
        }
        else
        {
            loop = false;
        }
        mutex.unlock();
        if (loop == false)
            break;
        if (localvec.empty() == false)
        {
            printf("Sending %ld %d msgs\n", timeStamp, localvec.size());
            //assa VCI_CAN_OBJ *obj = new VCI_CAN_OBJ[localvec.size()];
            //assa VCI_CAN_OBJ *objBase = obj;
            // memset(obj, 0, sizeof(VCI_CAN_OBJ) * localvec.size());


            for (CANMSG& m : localvec)
            {
                // obj->ID = m.id;
                // memcpy(obj->Data , m.data, m.dlc);
                // obj->DataLen = m.dlc;
                // obj->TimeStamp = timeStamp;
                // obj++;
                //                char spedd[8] = {0x80, 0x20, 0x80, 0x20,0x80, 0x20,0x80, 0x20 };
                char spedd[8] = {0x10, 0x20, 0x10, 0x20,0x10, 0x20,0x10, 0x20 };

                QByteArray data((const char *)m.data, m.dlc);
                QCanBusFrame frame = QCanBusFrame(m.id, data);
                frame.setExtendedFrameFormat(false);
                frame.setFlexibleDataRateFormat(false);
                frame.setBitrateSwitch(false);

                qDebug() << "write data !!!!!!";
                //                 m_canDevice->writeFrame(frame);

            }
            //assa VCI_Transmit(VCI_USBCAN2, 0, 0, objBase, localvec.size());
            //delete [] objBase;

        }
        else
        {
            //printf("Sending %ld %d msgs\n", timeStamp, 0);
        }
        msleep(mCycle);
        timeStamp += mCycle;
    }
    exit(0);
    // VCI_CloseDevice(VCI_USBCAN2, 0);
    printf("Sender Thread Terminated\n");
}

int Tester::GenPushSymbol(const char* id, dataType dt)
{
    qDebug() << Q_FUNC_INFO;
    SYMBOL* ret = mSymTbl.Search(id);
    if (ret == nullptr)
    {
        ret = mSymTbl.Insert(id, dt);
    }
    mProg.append(PUSH);
    mProg.append(ret);

    return 0;
}

int Tester::GenAssignment(const char* id)
{
    qDebug() << Q_FUNC_INFO;
    SYMBOL* ret = mSymTbl.Search(id);
    if (ret == nullptr)
        return -1;

    mProg.append(COPY);
    mProg.append(ret);
    return 0;
}
void writeBits(FRAME_SIGNAL* s, double val, unsigned char data[]);

int Tester::GenActivate(const char* id)
{
    qDebug() << Q_FUNC_INFO;
    SYMBOL* ret = mSymTbl.Search(id);
    //if (ret != nullptr)
    //  return -1;

    ret = mSymTbl.Insert(id, DT_CANMSG);
    QString sf(id);
    int count = dl->GetFrameCount();
    FRAME * frame = dl->GetFrames();

    for (int i = 0; i < count; i++)
    {
        if (frame[i].name == sf)
        {
            printf("find message %s\n", id);
            CANMSG *m = new CANMSG;
            ret->val.msg = m;
            ret->type = DT_CANMSG;
            m->id = frame[i].id;
            m->dlc = frame[i].dlc;
            m->frame = &frame[i];

            m->cycle = 10;
            m->timeStamp = 0;
            memset(m->data, 0, m->dlc);
            SYMBOL* symWait;
            for (int j = 0; j < frame[i].sigCount; j++)
            {
                QString signame;

                signame = sf + QString("::") + frame[i].sigs[j].name;
                QByteArray ba2;

                ba2 = signame.toUtf8();
                symWait = mSymTbl.Insert(ba2.data(), DT_CANSIG);
                CANSIG *sig = new CANSIG;
                symWait->val.sig = sig;
                sig->msg = m;
                sig->s = &frame[i].sigs[j];
            }

            for (int j = 0; j < frame[i].attrCount; j++)
            {
                if (QString(frame[i].attrs[j].name) == "GenMsgCycleTime" )
                {
                    m->cycle = frame[i].attrs[j].val->ival;
                }
                else if (QString(frame[i].attrs[j].name) == "GenSigStartValue")
                {
                    int iii = 0;
                    iii++;
                    QString signame;

                    signame = sf + QString("::") + frame[i].attrs[j].obj->name;
                    QByteArray ba2;

                    ba2 = signame.toUtf8();

                    symWait = mSymTbl.Search(ba2.data());
                    double val = frame[i].attrs[j].val->ival;
                    writeBits(symWait->val.sig->s, val, symWait->val.sig->msg->data);
                }
            }
            break;
        }
    }
    mProg.append(PUSH);
    mProg.append(ret);
    mProg.append(ACTIVATE);
    return 0;
}

int Tester::GenCallFunc(char *szFilePath)
{
    qDebug() << Q_FUNC_INFO;
    SYMBOL* ret = mSymTbl.Search(szFilePath);
    if (ret == nullptr)
        return -1;

    mProg.append(PUSH);
    mProg.append(ret);
    mProg.append(CALL);

    // printf("GenCallFunc = %s\n", szFilePath);
    return 0;
}

void Tester::Init()
{
    qDebug() << Q_FUNC_INFO;
    mSymTbl.Clear();

    SYMBOL* symWait = mSymTbl.Insert("WaitForBootReady", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 0;
    symWait = mSymTbl.Insert("Exec", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 1;
    symWait = mSymTbl.Insert("AddToList", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 1;
    symWait = mSymTbl.Insert("Capture", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 1;
    symWait = mSymTbl.Insert("sleep", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 1;
    symWait = mSymTbl.Insert("ClearMsg", DT_FUNC);
    symWait->val.f = new FUNCTION;
    symWait->val.f->retType = DT_VOID;
    symWait->val.f->numParam = 0;
    symWait = mSymTbl.Insert("Bp", DT_VAR);
    symWait->val.v = new VARIABLE;
    symWait->val.v->type = DT_BOOL;
    symWait = mSymTbl.Insert("Ign", DT_VAR);
    symWait->val.v = new VARIABLE;
    symWait->val.v->type = DT_BOOL;
    symWait = mSymTbl.Insert("CAN", DT_VAR);
    symWait->val.v = new VARIABLE;
    symWait->val.v->type = DT_BOOL;
    symWait = mSymTbl.Insert("0", DT_BOOL);
    symWait->val.b = false;
    symWait = mSymTbl.Insert("1", DT_BOOL);
    symWait->val.b = true;
}

Tester::Tester(QObject* parent) :
    QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
    mRelayStatus = 0;
    mRelay = new QSerialPort(nullptr);
    //    mConsole = new QSerialPort(nullptr); //nullptr;
    mBootReady = false;
    mCaptureDone = false;

    Init();
    dl = new DBCLoader;
    sender = new Sender;

    Init2();
}

Tester::~Tester()
{
    qDebug() << Q_FUNC_INFO;
    if (mRelay != nullptr)
        delete mRelay;
    if (dl != nullptr)
        delete dl;
}

void Tester::onConnected()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &Tester::onTextMessageReceived);
    m_webSocket.sendTextMessage(QStringLiteral("Hello, world!!!!!!!!!!!!!!"));

    QByteArray ba("Hello world");
    char *data = ba.data();
    while (*data) {
        qDebug() << "[" << *data << "]" << endl;
        ++data;
    }

    m_webSocket.sendBinaryMessage(ba);
}

void Tester::onTextMessageReceived(QString message)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Message received:" << message;
    m_webSocket.close();
}

static unsigned char buff[16] = {0x10, 0x20, 0};

static unsigned char CheckSum2(const unsigned char buff[])
{
    qDebug() << Q_FUNC_INFO;
    unsigned char MyCheck = buff[1];

    for (int i = 2; i < 16; i++)
    {
        if (MyCheck > buff[i])
            MyCheck = (MyCheck - buff[i]);
        else
            MyCheck = (buff[i] - MyCheck);

    }
    //for (int i=0; i<(16 - 3); i++)
    //      MyCheck = MyCheck ^ buff[i + 2];
    return MyCheck;
}

void sendRelayCmd232(QSerialPort *pPort, unsigned int devices)
{
    qDebug() << Q_FUNC_INFO;
    buff[2] = devices;

    buff[15] = CheckSum2(buff);

    pPort->write((const char *)buff, 16);
    pPort->waitForBytesWritten(-1);
    pPort->flush();
}

static unsigned char buff3[16] = {0x10, 0x97, 1};
void sendRelayCmd232_1(QSerialPort *pPort, unsigned int devices)
{
    qDebug() << Q_FUNC_INFO;
    buff3[2] = devices;

    buff3[15] = CheckSum2(buff3);

    pPort->write((const char *)buff3, 16);
    pPort->waitForBytesWritten(-1);
    pPort->flush();
}

static unsigned char buff2[16] = {0x10, 0x35, 0x01, 0x00, 0xFF, 0x00, 0x01, 0x01};

static unsigned char CheckSum(const unsigned char buff[])
{
    qDebug() << Q_FUNC_INFO;
    unsigned char MyCheck = buff[1];

    for (int i = 0; i < 16 - 3; i++)
    {
        MyCheck ^= buff[i + 2];

    }
    //for (int i=0; i<(16 - 3); i++)
    //      MyCheck = MyCheck ^ buff[i + 2];
    return MyCheck;
}

void sendRelayCmd(QSerialPort *pPort, unsigned int devices)
{
    qDebug() << Q_FUNC_INFO;
    buff2[3] = devices;

    buff2[15] = CheckSum(buff2);

    pPort->write((const char *)buff2, 16);
    pPort->waitForBytesWritten(-1);
    pPort->flush();
}
bool Tester::RelayConnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;
    mRelayStatus |= channels;
    sendRelayCmd232(mRelay, mRelayStatus);

    printf("Relay Turn on %d\n", channels);
    return true;
}

bool Tester::RelayDisconnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;
    mRelayStatus &= ~channels;
    sendRelayCmd232(mRelay, mRelayStatus);
    printf("Relay Turn off %d\n", channels);
    return true;
}

bool Tester::CANConnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;
    Init2();
    printf("CAN on %d\n", channels);
    return true;
}

bool Tester::CANDisconnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;

    printf("CAN off %d\n", channels);
    return true;
}

bool Tester::RelayOpen()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "WebSocket server:" << QUrl(QStringLiteral("ws://localhost:1234"));
    QUrl tempUrl = QUrl(QStringLiteral("ws://localhost:1234"));
    connect(&m_webSocket, &QWebSocket::connected, this, &Tester::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Tester::closed);
    //     m_webSocket.open(QUrl(tempUrl));
    m_webSocket.open(QUrl(QStringLiteral("ws://localhost:1234")));
    return true;
}

bool Tester::RelayClose()
{
    qDebug() << Q_FUNC_INFO;
    mRelay->close();
    return true;
}

bool Tester::AttachConsole(QSerialPort *port)
{
    qDebug() << Q_FUNC_INFO;
    return true;
}

void Tester::DetachConsole()
{
    qDebug() << Q_FUNC_INFO;

}

void Tester::GenReport(const char *filePath)
{
    qDebug() << Q_FUNC_INFO;
    FILE *fp = fopen(filePath, "w");



    fclose(fp);
}

void Tester::Run()
{
    qDebug() << Q_FUNC_INFO;
    MY_WORD *prog = mProg.data();
    ip = prog;

    printf("Start running\n");
    vec.clear();
    runSenderThread = true;
    mCaptureDone= false;
    mBootReady = false;
    mRelayStatus = 0;
    mLog.clear();
    mStack.clear();
    Init();
    sender->start();
    while (ip->instruction != TERMINATE)
    {
        //printf("instruction %d\n", ip->instruction);
        switch (ip->instruction)
        {
        case PUSH:
            ip++;
            mStack.push(ip->symbol);
            //printf("Push %s\n", ip->symbol->name);
            ip++; break;
        case ACTIVATE:
        {
            SYMBOL* onoff = mStack.pop();
            ip += 1;
            DoActivate(onoff);
            break;
        }
        case COPY:
        {
            ip++;
            SYMBOL* var = mStack.top();
            DoCopy(ip->symbol, var);

            ip++;
        }
            break;
        case CALL:
        {
            SYMBOL* onoff = mStack.pop();
            ip += 1;
            DoCallFunc(onoff);

            break;
        }
        default:
        {
            printf("Unhandled instruction = %d\n", *ip );
            *ip = TERMINATE;
            break;
        }
        }
    }
    this->mConsole->write("2 exit\n", 7);
    mutex.lock();
    runSenderThread = false;
    mutex.unlock();
    sender->wait();
    printf("Terminated\n");
}

