#pragma once

#include <QObject>
#include <QVector>
#include <QStack>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>
#include <QList>
#include <QtWebSockets/QWebSocket>
#include <pthread.h>
class QThread;
class QSerialPort;
class DBCLoader;
class Sender;

enum INSTRUCTION
{
    PUSH = 0,
    ACTIVATE = 1,
    CALL = 2,
    REBOOT = 3,
    POP = 4,
    COPY = 5,
    TERMINATE = 6
};

struct FRAME;
struct FRAME_SIGNAL;

struct CANMSG
{
    FRAME *frame;
    long timeStamp;
    long cycle;
    int channel;
    int dlc;
    int id;
    unsigned char data[8];
};

struct CANSIG
{
    CANMSG *msg;
    FRAME_SIGNAL* s;
};

struct FUNCTION
{
    int numParam;
    int retType;
};

struct VARIABLE
{
    int type;
};

enum dataType
{
    DT_STRING,
    DT_CANSIG,
    DT_CANMSG,
    DT_FUNC,
    DT_INT,
    DT_FLOAT,
    DT_BOOL,
    DT_VAR,
    DT_VOID
};

union evalType
{
    QString *s;
    CANSIG *sig;
    CANMSG *msg;
    FUNCTION *f;
    int i;
    double d;
    bool b;
    VARIABLE* v;
};

struct SYMBOL
{
    char name[64];
    int blk;
    dataType type;
    int addr;
    evalType val;

    SYMBOL(const char* _name, dataType _type)
    {
        strcpy(name, _name);
        type = _type;
        memset(&val, 0, sizeof(val));
        if (type == DT_STRING)
        {
            val.s = new QString(_name);
        }
    }
};

union MY_WORD
{
    int instruction;
    SYMBOL* symbol;
    MY_WORD(int inst = 0)
    {
        instruction = inst;
    }
    MY_WORD(SYMBOL* sym)
    {
        symbol = sym;
    }
};

class SYMBOL_TABLE
{
private:
    QList<SYMBOL*> mVars;
    QHash<QString, SYMBOL*> m_hash;
public:
    SYMBOL_TABLE();
    ~SYMBOL_TABLE();
    SYMBOL* Insert(const char* name, dataType dt );
    SYMBOL* Search(const char* name);
    void Clear();
};

class Tester;

typedef int (Tester::*fff)(SYMBOL *, QStack<SYMBOL*>&);

class Tester : public QObject
{
    Q_OBJECT
public slots:
   // void writeData(const QByteArray &data);
    void readData();
private:
    void Init();
    bool RelayOpen();
    bool RelayClose();
    bool RelayConnectChanels(int channels);
    bool RelayDisconnectChanels(int channels);
    bool CANConnectChanels(int channels);
    bool CANDisconnectChanels(int channels);

    bool AttachConsole(QSerialPort *port);
    void DetachConsole();
    void OnConsoleBootComplete();
    void WaitForBootReady();
//    int CBWaitForBootReady(SYMBOL *sym, QStack<SYMBOL*> &stack);
//    int CBCapture(SYMBOL *sym, QStack<SYMBOL*> &stack);
//    int CBSleep(SYMBOL *sym, QStack<SYMBOL*> &stack);
//    int CBClearMsg(SYMBOL *sym, QStack<SYMBOL*> &stack);
protected:
    QVector<MY_WORD> mProg;
    QStack<SYMBOL*> mStack;
    QHash<QString, fff> m_funcTable;
    QMutex mLock;
    QMutex mLockBootReady;
    QWaitCondition bufferNotEmpty;
    QByteArray mLog;
private:
    QSerialPort* mRelay;
    unsigned int mRelayStatus;
    SYMBOL_TABLE mSymTbl;
    QSerialPort* mConsole;
    bool mBootReady;
    bool mCaptureDone;
    void ExecFile(int id);
    DBCLoader *dl;
    Sender *sender;
    MY_WORD *ip;
public:
    bool ReadTestCase(const char *filePath);
    bool ReadCANDB(const char *filePath);
    void waitUntil(int flags);
    void Run();
    void GenReport(const char *filePath);
private:
    void DoCallFunc(SYMBOL* sym);
    void DoActivate(SYMBOL* sym);
    void DoCopy(SYMBOL *a, SYMBOL *b);
public:
    int GenPushSymbol(const char* id, dataType dt);
    int GenAssignment(const char* id);
    int GenActivate(const char* id);
    int GenCallFunc(char *szFilePath);
public:
    Tester(QObject *parent = nullptr);
    virtual ~Tester() override;
public:
    bool Init2();
    static void* ReadEthernetThread(void* _ptr);
    pthread_t m_thread = 0;
    pthread_mutex_t m_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_thread_cond = PTHREAD_COND_INITIALIZER;

    void connectEthernet();
    bool mConnectedEthernetState = false;

    void printSYMBOL(SYMBOL* value);
    void printevalType(evalType value);
    void printCANMSG(CANMSG* value);
    void printFRAME_SIGNAL(FRAME_SIGNAL* value);


Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
};
