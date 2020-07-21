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
#include <QMetaType>
#include <cmath>
bool Tester::Init2()
{
    qDebug() << Q_FUNC_INFO;
    return true;
}

QMutex mutex;
QVector<CANMSG*> vec;
static bool runSenderThread = true;


class Sender : public QThread
{
public:
    virtual void run() override;
    Sender(Tester* temp);
private:
    unsigned long mCycle;
    Tester* mTester;
};

//void* Tester::ReadEthernetThread(void* _ptr)
//{
//    qDebug() << Q_FUNC_INFO;
//    QVector<int> vector(10);
//    int *data = vector.data();
//    for (int i = 0; i < 10; ++i)
//        data[i] = 2 * i;


//    Tester* hndl = static_cast< Tester* > (_ptr);
//    qDebug() << "mProg size : " << hndl->mProg.size();
//    MY_WORD *prog = hndl->mProg.data();

//    hndl->ip = prog;

//    qDebug("Start running\n");
//    vec.clear();
//    runSenderThread = true;
//    hndl->mCaptureDone= false;
//    hndl->mBootReady = false;
//    hndl->mRelayStatus = 0;
//    hndl->mLog.clear();
//    hndl->mStack.clear();
//    hndl->sender->start();
//    while (hndl->ip->instruction != TERMINATE)
//    {
//        qDebug() << "hndl->ip->instruction : " << hndl->ip->instruction;
//        switch (hndl->ip->instruction)
//        {
//        case PUSH:
//            hndl->ip++;
//            hndl->mStack.push(hndl->ip->symbol);
//            hndl->ip++; break;
//        case ACTIVATE:
//        {
//            SYMBOL* onoff = hndl->mStack.pop();
//            hndl->ip += 1;
//            hndl->DoActivate(onoff);
//            break;
//        }
//        case COPY:
//        {
//            hndl->ip++;
//            SYMBOL* var = hndl->mStack.top();
//            hndl->DoCopy(hndl->ip->symbol, var);

//            hndl->ip++;
//        }
//            break;
//        case CALL:
//        {
//            SYMBOL* onoff = hndl->mStack.pop();
//            hndl->ip += 1;
//            hndl->DoCallFunc(onoff);

//            break;
//        }
//        default:
//        {
//            qDebug("Unhandled instruction = %d\n", *hndl->ip );
//            *hndl->ip = TERMINATE;
//            break;
//        }
//        }
//    }
//    //    this->mConsole->write("2 exit\n", 7);
//    qDebug() << "?????????????";
//    mutex.lock();
//    runSenderThread = false;
//    mutex.unlock();
//    hndl->sender->wait();
//    qDebug("Terminated\n");
//}
void* Tester::ReadEthernetThread(void* _ptr)
{
    qDebug() << Q_FUNC_INFO;
    //    QVector<int> vector(10);
    //    int *data = vector.data();
    //    for (int i = 0; i < 10; ++i)
    //        data[i] = 2 * i;


    Tester* hndl = static_cast< Tester* > (_ptr);
    qDebug() << "mProg size : " << hndl->mProg.size();
    MY_WORD *prog = hndl->mProg.data();

    hndl->ip = prog;

    qDebug("Start running\n");
    vec.clear();
    runSenderThread = true;
    hndl->mCaptureDone= false;
    hndl->mBootReady = false;
    hndl->mRelayStatus = 0;
    hndl->mLog.clear();
    hndl->mStack.clear();

    while (hndl->ip->instruction != TERMINATE)
    {
        qDebug() << "hndl->ip->instruction : " << hndl->ip->instruction;
        switch (hndl->ip->instruction)
        {
        case PUSH:
            hndl->ip++;
            hndl->mStack.push(hndl->ip->symbol);
            hndl->ip++; break;
        case ACTIVATE:
        {
            SYMBOL* onoff = hndl->mStack.pop();
            hndl->ip += 1;
            hndl->DoActivate(onoff);
            break;
        }
        case COPY:
        {
            hndl->ip++;
            SYMBOL* var = hndl->mStack.top();
            hndl->DoCopy(hndl->ip->symbol, var);

            hndl->ip++;
        }
            break;
        case CALL:
        {
            SYMBOL* onoff = hndl->mStack.pop();
            hndl->ip += 1;
            hndl->DoCallFunc(onoff);

            break;
        }
        default:
        {
            qDebug("Unhandled instruction = %d\n", *hndl->ip );
            *hndl->ip = TERMINATE;
            break;
        }
        }
    }
    //    this->mConsole->write("2 exit\n", 7);
    qDebug() << "?????????????";
    mutex.lock();
    //    runSenderThread = false;
    mutex.unlock();
    hndl->sender->start();
    //    hndl->sender->wait();
    qDebug("Terminated\n");
}

void Tester::connectEthernet()
{

    //    qDebug() << Q_FUNC_INFO << "WebSocket server:" << QUrl(QStringLiteral("ws://localhost:1234"));
    //    QUrl tempUrl = QUrl(QStringLiteral("ws://localhost:1234"));
    //    connect(&m_webSocket, &QWebSocket::connected, this, &Tester::onConnected);
    //    connect(&m_webSocket, &QWebSocket::disconnected, this, &Tester::closed);
    //    m_webSocket.open(QUrl(tempUrl));

    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::LocalHost, 1234);
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(onConnected()));

}

void Tester::printSYMBOL(SYMBOL* value)
{
    qDebug( "SYMBOL - char name[64] [%s]", value->name );
    qDebug( "SYMBOL - int blk       [%d]", value->blk  );
    qDebug( "SYMBOL - dataType type [%d]", value->type );
    qDebug( "SYMBOL - int addr      [%d]", value->addr );
    //            qDebug( "SYMBOL - evalType val  [%d]", value->val  );
    printevalType(value->val);
}


void Tester::printevalType(evalType value)
{

    qDebug("evalType - QString *s  [%s]", value.s->toStdString().c_str());
    //  qDebug("evalType - CANSIG *sig [%]", value.
    //  qDebug("evalType - CANMSG *msg [%]", value.
    //  qDebug("evalType - FUNCTION *f [%]", value.
    qDebug("evalType - int i       [%d]", value.i);
    qDebug("evalType - double d    [%f]", value.d);
    qDebug("evalType - bool b      [%d]", value.b);
    //  qDebug("evalType - VARIABLE* v [%]", value.
}

void Tester::printFRAME(FRAME* value)
{
    qDebug() << Q_FUNC_INFO;
    qDebug (" value->id     [%d]", value->id     );
    qDebug (" value->dlc    [%d]", value->dlc    );
    qDebug (" value->type   [%d]", value->type   );
    qDebug (" value->name   [%s]", value->name   );
    qDebug (" value->comment[%s]", value->comment);
    qDebug (" value->senders[%s]", value->senders);

    printFRAME_SIGNAL(value->sigs);

}

void Tester::printCANMSG(CANMSG* value)
{
    //    qDebug("CANMSG - FRAME *frame          [%]", );

    qDebug() << Q_FUNC_INFO;

    printFRAME(value->frame);
    qDebug("CANMSG - long timeStamp        [%lu]", value->timeStamp);
    qDebug("CANMSG - long cycle            [%lu]", value->cycle);
    qDebug("CANMSG - int channel           [%d]", value->channel);
    qDebug("CANMSG - int dlc               [%d]", value->dlc);
    qDebug("CANMSG - int id                [%d]", value->id);
    qDebug("CANMSG - unsigned char data[8] [%s]", value->data);
}

void Tester::printFRAME_SIGNAL(FRAME_SIGNAL* value)
{
    qDebug() << Q_FUNC_INFO;
    qDebug(" FRAME_SIGNAL - char     *name     [%s]",value->name);
    qDebug(" FRAME_SIGNAL - char     *comment  [%s]",value->comment);
    qDebug(" FRAME_SIGNAL - uint32_t  start    [%d]",value->start);
    qDebug(" FRAME_SIGNAL - uint32_t  length   [%d]",value->length);
    qDebug(" FRAME_SIGNAL - uint32_t endianess [%d]",value->endianess);
    qDebug(" FRAME_SIGNAL - uint32_t   signess [%d]",value->signess);
    qDebug(" FRAME_SIGNAL - uint32_t      type [%d]",value->type);
    qDebug(" FRAME_SIGNAL - double  factor     [%f]",value->factor);
    qDebug(" FRAME_SIGNAL - double  offset     [%f]",value->offset);
    qDebug(" FRAME_SIGNAL - double  min        [%f]",value->min);
    qDebug(" FRAME_SIGNAL - double  max        [%f]",value->max);
    qDebug(" FRAME_SIGNAL - char   *unit       [%s]",value->unit);
    qDebug(" FRAME_SIGNAL - int numValues      [%d]",value->numValues);

    //    qDebug(" FRAME_SIGNAL - value_string - int32_t  value      [%d]",value->values->value);
    //    qDebug(" FRAME_SIGNAL - value_string - char    *strptr      [%s]",value->values->strptr);
}

void Tester::testWriteDatagrame(const QByteArray& datagram, const QHostAddress& host, quint16 port)
{
    //    m_udpSocket->writeDatagram(datagram, QHostAddress::LocalHost, 1234);
    qDebug() << Q_FUNC_INFO;
    emit testSignalWriteDatagrame(datagram, host, port);
}

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
    qDebug("Insert (%d)%s\n", dt, name);
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


//void Sender::run()
//{
//    qDebug() << Q_FUNC_INFO << "runSenderThread : " << runSenderThread ;
//    mCycle = 10;
//    long timeStamp = 0;
//    QVector<CANMSG> localvec;
//    bool loop = true;

//    for (;;)
//    {
//        int nCount = 0;

//        nCount = 0;
//        mutex.lock();
//        if (runSenderThread)
//        {
//            qDebug() << Q_FUNC_INFO << "runSenderThread is true";
//            localvec.clear();
//            for (CANMSG* m : vec)
//            {
//                m->timeStamp -= mCycle;
//                if (m->timeStamp <= 0)
//                {
//                    m->timeStamp += m->cycle;
//                    localvec.append(*m);
//                    nCount++;
//                }
//            }
//        }
//        else
//        {
//            loop = false;
//        }
//        mutex.unlock();
//        if (loop == false)
//            break;
//        if (localvec.empty() == false)
//        {
//            qDebug() << Q_FUNC_INFO << "Sending : " << timeStamp << "\t localvec.size() : " << localvec.size() ;
//            qDebug("Sending %ld %d msgs\n", timeStamp, localvec.size());
//            //assa VCI_CAN_OBJ *obj = new VCI_CAN_OBJ[localvec.size()];
//            //assa VCI_CAN_OBJ *objBase = obj;
//            // memset(obj, 0, sizeof(VCI_CAN_OBJ) * localvec.size());


//            //            qDebug() << "write data !!!!!!@@@@@@@@@@@@@@@@@@@@@@@" << localvec.size();
//            for (CANMSG& m : localvec)
//            {
//                // obj->ID = m.id;
//                // memcpy(obj->Data , m.data, m.dlc);
//                // obj->DataLen = m.dlc;
//                // obj->TimeStamp = timeStamp;
//                // obj++;
//                //                char spedd[8] = {0x80, 0x20, 0x80, 0x20,0x80, 0x20,0x80, 0x20 };
//                qDebug() << "write data !!!!!!@@@@@@@@@@@@@@@@@@@@@@@";

////                char spedd[8] = {0x10, 0x20, 0x10, 0x20,0x10, 0x20,0x10, 0x20 };
////                char spedd[8] = {0x01, 0x2, 0x3, 0x4,0x5, 0x6,0x7, 0x8 };

//                QByteArray data((const char *)m.data, m.dlc);
//                QCanBusFrame frame = QCanBusFrame(m.id, data);
//                frame.setExtendedFrameFormat(false);
//                frame.setFlexibleDataRateFormat(false);
//                frame.setBitrateSwitch(false);

//                for (int i = 0 ; i < m.dlc ; i++) {
//                    qDebug("@@@@@@@@ send data  index [%d] data [%d]", i, m.data[i]);
//                }



////                mTester->m_webSocket.sendBinaryMessage(data);
//                mTester->m_udpSocket->writeDatagram(data, QHostAddress::LocalHost, 1234);



//                //                 m_canDevice->writeFrame(frame);

//            }
//            //assa VCI_Transmit(VCI_USBCAN2, 0, 0, objBase, localvec.size());
//            //delete [] objBase;

//        }
//        else
//        {
//            //qDebug("Sending %ld %d msgs\n", timeStamp, 0);
//            qDebug() << "Sending : " << timeStamp << "\t localvec.size() : " << 0 ;
//        }
//        msleep(mCycle);
//        timeStamp += mCycle;
//    }
//    qDebug() << "Sender Thread Terminated\n";
//    exit(0);
//    // VCI_CloseDevice(VCI_USBCAN2, 0);
//    qDebug("Sender Thread Terminated\n");
//}

void Sender::run()
{
    qDebug() << Q_FUNC_INFO << "runSenderThread : " << runSenderThread ;
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
            qDebug() << Q_FUNC_INFO << "runSenderThread is true";
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
            qDebug() << Q_FUNC_INFO << "Sending : " << timeStamp << "\t localvec.size() : " << localvec.size() ;
            qDebug("Sending %ld %d msgs\n", timeStamp, localvec.size());

            /////////////////
            //  CAN FRAME  //
            /////////////////
            //            qDebug() << "write data !!!!!!@@@@@@@@@@@@@@@@@@@@@@@" << localvec.size();
            //            for (CANMSG& m : localvec)
            //            {
            //                qDebug() << "write data !!!!!!@@@@@@@@@@@@@@@@@@@@@@@";

            //                QByteArray data((const char *)m.data, m.dlc);
            //                QCanBusFrame frame = QCanBusFrame(m.id, data);
            //                frame.setExtendedFrameFormat(false);
            //                frame.setFlexibleDataRateFormat(false);
            //                frame.setBitrateSwitch(false);

            //                for (int i = 0 ; i < m.dlc ; i++) {
            //                    qDebug("@@@@@@@@ send data  index [%d] data [%d]", i, m.data[i]);
            //                }

            //                qDebug("@@@@@@@@ start ");
            //                //                mTester->m_udpSocket->writeDatagram(data, QHostAddress::LocalHost, 1234);

            //                mTester->testWriteDatagrame(data, QHostAddress::LocalHost, 1234);

            //                qDebug("@@@@@@@@ end ");
            //            }

            /////////////////////
            // signal + value  //
            /////////////////////
            for (CANMSG& m : localvec)
            {
                qDebug() << "write data !!!!!!@@@@@@@@@@@@@@@@@@@@@@@";

                QByteArray data((const char *)m.data, m.dlc);
                QCanBusFrame frame = QCanBusFrame(m.id, data);
                frame.setExtendedFrameFormat(false);
                frame.setFlexibleDataRateFormat(false);
                frame.setBitrateSwitch(false);

                for (int i = 0 ; i < m.dlc ; i++) {
                    qDebug("@@@@@@@@ send data  index [%d] data [%d]", i, m.data[i]);
                }

                qDebug("@@@@@@@@ start ");
                mTester->printCANMSG(&m);
                //                QByteArray ba("aaaaaaaaaa", m.dlc);
                //                char *dataC = data.data();
                //                while (*dataC) {
                //                    qDebug() << "[" << *dataC<< "]" << endl;
                //                    ++dataC;
                //                }

                //                mTester->testWriteDatagrame(ba, QHostAddress::LocalHost, 1234);

                /////////////////////////
                //  get signal data!!  //
                /////////////////////////

                FRAME *temp = m.frame;
                QByteArray sendData;

                qDebug() << "signal count : "  << temp->sigCount;
                for (unsigned int i = 0 ; i < temp->sigCount; i++) {
                    char* attName = temp->sigs[i].name;
                    qDebug("signale[%d] name [%s] ", i, attName);
                    double rawValue = 0;
                    //                    char* attValue = (char*)(&rawValue);

                    unsigned int nsize = (temp->sigs[i].length + 7) / 8; // 14 + 7 /8
                    int startByte = temp->sigs[i].start / 8;
                    qDebug() << "nsize : " << nsize << "temp->sigs[i]. " << temp->sigs[i].start/8;
                    for (unsigned int j = 0 ; j < nsize; j++) {
                        rawValue +=  static_cast<double>((m.data[startByte + j])* pow(256,j)   ) * temp->sigs[i].factor + temp->sigs[i].offset;
                        qDebug (" start [%d][%d]:%d  ---- %f", i, j, m.data[startByte + j], rawValue);
                    }

                    qDebug ("rawValue : %f", rawValue);
                    sendData.push_back(attName);
                    sendData.push_back("::");





                    sendData.push_back(QString::number(rawValue).toStdString().c_str());

                    if (i == temp->sigCount -1) {
                        sendData.push_back("\n");
                    } else {
                        sendData.push_back(",");
                    }

                }

                //                char *dataC = sendData.data();
                //                while (*dataC) {
                //                    qDebug() << "[" << *dataC<< "]" << endl;
                //                    ++dataC;
                //                }


                mTester->testWriteDatagrame(sendData, QHostAddress::LocalHost, 1234);




                qDebug() << "dldyddn : " << sendData;
            }
            break;
        }
        else
        {
            qDebug() << "Sending : " << timeStamp << "\t localvec.size() : " << 0 ;
        }
        msleep(mCycle);
        timeStamp += mCycle;
    }
    qDebug() << "Sender Thread Terminated\n";
    exit(0);
    qDebug("Sender Thread Terminated\n");
}

Sender::Sender(Tester* temp)
{
    mTester = temp;
}

int Tester::GenPushSymbol(const char* id, dataType dt)
{
    qDebug() << Q_FUNC_INFO << "start";
    SYMBOL* ret = mSymTbl.Search(id);
    if (ret == nullptr)
    {
        ret = mSymTbl.Insert(id, dt);
    }
    mProg.append(PUSH);
    mProg.append(ret);

    qDebug() << Q_FUNC_INFO << "end";
    return 0;
}

int Tester::GenAssignment(const char* id)
{
    qDebug() << Q_FUNC_INFO << "start";
    SYMBOL* ret = mSymTbl.Search(id);
    if (ret == nullptr)
        return -1;

    mProg.append(COPY);
    mProg.append(ret);
    qDebug() << Q_FUNC_INFO << "end";
    return 0;
}
void writeBits(FRAME_SIGNAL* s, double val, unsigned char data[]);

int Tester::GenActivate(const char* id)
{
    qDebug() << Q_FUNC_INFO << "id : " << id;
    SYMBOL* ret = mSymTbl.Search(id);
    //if (ret != nullptr)
    //  return -1;

    ret = mSymTbl.Insert(id, DT_CANMSG);
    QString sf(id);
    int count = dl->GetFrameCount();
    FRAME * frame = dl->GetFrames();

    qDebug() << "@@@@@ GetFrameCount : " << count;

    for (int i = 0; i < count; i++)
    {
        qDebug() << "frame[i].name : " << frame[i].name <<  " sf :" << sf;
        if (frame[i].name == sf)
        {
            //fame id [902], dlc [8], type [0], name  [WHL_SPD11], comment  [[P] Periodic], sender  [CGW], sigCount  [4], attrCount [14]
            qDebug("fame id [%d], dlc [%d], type [%d], name  [%s], comment  [%s], sender  [%s], sigCount  [%d], attrCount [%d]", frame[i].id, frame[i].dlc, frame[i].type, frame[i].name, frame[i].comment, frame[i].senders, frame[i].sigCount, frame[i].attrCount);

            //find message WHL_SPD11
            qDebug("find message %s\n", id);
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

            // signale check //
            for (int j = 0; j < frame[i].sigCount; j++)
            {
                QString signame;

                signame = sf + QString("::") + frame[i].sigs[j].name;

                // signame [WHL_SPD11::WHL_SPD_FL]
                // signame [WHL_SPD11::WHL_SPD_FR]
                // signame [WHL_SPD11::WHL_SPD_RL]
                // signame [WHL_SPD11::WHL_SPD_RR]
                qDebug( "signame [%s]", signame.toStdString().c_str());

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
                qDebug() <<  "@@@@@@@@@2 attrs  : " << QString(frame[i].attrs[j].name);
                if (QString(frame[i].attrs[j].name) == "GenMsgCycleTime" )
                {
                    m->cycle = frame[i].attrs[j].val->ival;
                    qDebug( "@@@@@@@@@2 attrs [%s] GenMsgCycleTime [%d]", frame[i].attrs[j].name, m->cycle);
                }
                else if (QString(frame[i].attrs[j].name) == "GenSigStartValue")
                {
                    int iii = 0;
                    iii++;
                    QString signame;

                    signame = sf + QString("::") + frame[i].attrs[j].obj->name;
                    QByteArray ba2;

                    ba2 = signame.toUtf8();
                    // signame [WHL_SPD11::WHL_SPD_FL]
                    // signame [WHL_SPD11::WHL_SPD_FR]
                    // signame [WHL_SPD11::WHL_SPD_RL]
                    // signame [WHL_SPD11::WHL_SPD_RR]
                    qDebug("signame[%s]", signame.toStdString().c_str());

                    // get SYMBOL
                    symWait = mSymTbl.Search(ba2.data());

                    // symWait->name; [WHL_SPD11::WHL_SPD_FL]
                    // symWait->blk;  [1664164609]
                    // symWait->type; [1]
                    // symWait->addr; [1073741824]
                    // symWait->val;  [1675054832]
                    qDebug("\n\n");
                    printSYMBOL(symWait);

                    double val = frame[i].attrs[j].val->ival;
                    qDebug() <<"symWait->val.sig->msg->data : " << symWait->val.sig->msg->data;
                    printFRAME_SIGNAL(symWait->val.sig->s);
                    writeBits(symWait->val.sig->s, val, symWait->val.sig->msg->data);
                    qDebug() <<"symWait->val.sig->msg->data : " << symWait->val.sig->msg->data;
                    printSYMBOL(symWait);

                }
            }
            break;
        } else {
        }
    }
    mProg.append(PUSH);
    mProg.append(ret);
    mProg.append(ACTIVATE);
    qDebug() <<Q_FUNC_INFO << "end";
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

    // qDebug("GenCallFunc = %s\n", szFilePath);
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
    QString errorString;

    qDebug() << Q_FUNC_INFO << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    qRegisterMetaType<QHostAddress>("QHostAddress");
    QObject::connect(this, &Tester::testSignalWriteDatagrame,this , &Tester::testSlotWriteDatagrame);
    qDebug() << Q_FUNC_INFO << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ENDS";
    connectEthernet();

    pthread_mutex_init(&m_thread_mutex, nullptr);
    pthread_cond_init(&m_thread_cond, nullptr);

    Init();
    dl = new DBCLoader;
    sender = new Sender(this);

    //    Init2();
}

Tester::~Tester()
{
    qDebug() << Q_FUNC_INFO;
    if (mRelay != nullptr)
        delete mRelay;
    if (dl != nullptr)
        delete dl;

    pthread_cancel(m_thread);
    pthread_join(m_thread, nullptr);
}

void Tester::onConnected()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &Tester::onTextMessageReceived);
    m_webSocket.sendTextMessage(QStringLiteral("Hello, world!!!!!!!!!!!!!!"));

    mConnectedEthernetState = true;

    //    QByteArray ba("Hello world");
    //    char *data = ba.data();
    //    while (*data) {
    //        qDebug() << "[" << *data << "]" << endl;
    //        ++data;
    //    }

    //    m_webSocket.sendBinaryMessage(ba);
    qDebug() << Q_FUNC_INFO;
    ReadCANDB("../../data/a.dbc");
    //    ReadCANDB("../../data/20200331_NQ5_2021_FD-E_v1.2W_REV1.dbc");
    //        ReadCANDB("../../data/test.dbc");
    //    ReadTestCase("./telltale.tc");
    ReadTestCase("./telltale_origine.tc");

    Run();

}

void Tester::onTextMessageReceived(QString message)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Message received:" << message;
    m_webSocket.close();
    m_udpSocket->close();
}

void Tester::testSlotWriteDatagrame(const QByteArray& datagram, const QHostAddress& host, quint16 port)
{
    qDebug() << Q_FUNC_INFO;
    m_udpSocket->writeDatagram(datagram, host, port);
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

void sendRelayCmd232_Test(QWebSocket *pPort, unsigned int devices)
{
    qDebug() << Q_FUNC_INFO << "devices : " << devices <<  "CheckSum2(buff) : " << CheckSum2(buff);
    buff[2] = devices;

    buff[15] = CheckSum2(buff);

    qDebug() << Q_FUNC_INFO << "devices : " << devices <<  "CheckSum2(buff) : " << buff[15];

    //    pPort->sendTextMessage("test");

    //    pPort->sendBinaryMessage()
    QByteArray temp;
    for (int i = 0 ; i < 16; i++) {
        temp.push_back(buff[i]);
    }
    pPort->sendBinaryMessage(temp);

}


void sendRelayCmd232(QSerialPort *pPort, unsigned int devices)
{
    qDebug() << Q_FUNC_INFO << "devices : " << devices <<  "CheckSum2(buff) : " << CheckSum2(buff);
    buff[2] = devices;

    buff[15] = CheckSum2(buff);

    qDebug() << Q_FUNC_INFO << "devices : " << devices <<  "CheckSum2(buff) : " << buff[15];

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
    //    sendRelayCmd232(mRelay, mRelayStatus);
    sendRelayCmd232_Test(&m_webSocket, mRelayStatus);

    qDebug("Relay Turn on %d\n", channels);
    return true;
}

bool Tester::RelayDisconnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;
    mRelayStatus &= ~channels;
    //    sendRelayCmd232(mRelay, mRelayStatus);
    sendRelayCmd232_Test(&m_webSocket, mRelayStatus);
    qDebug("Relay Turn off %d\n", channels);
    return true;
}

bool Tester::CANConnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;
    Init2();
    qDebug("CAN on %d\n", channels);
    return true;
}

bool Tester::CANDisconnectChanels(int channels)
{
    qDebug() << Q_FUNC_INFO;

    qDebug("CAN off %d\n", channels);
    return true;
}

bool Tester::RelayOpen()
{
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
    int ret = pthread_create(&m_thread, nullptr, &ReadEthernetThread, this);
}

