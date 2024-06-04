#include "GFE_ProgDlg_Thread.h"
#include "GFE_ProgDlg_Simple.h"
#include <QDebug>


GFE_ProgDlg_Thread::GFE_ProgDlg_Thread(QObject *parent)
    : QThread(parent)
{
}

GFE_ProgDlg_Thread::~GFE_ProgDlg_Thread()
{
    stop();
}

GFE_ProgDlg_Thread::State GFE_ProgDlg_Thread::state() const
{
    if(fCrash)
        return Crash;
    if(fToStop)
        return ToStop;
    if(fAbort)
        return Abort;
    if(fExit)
        return Stoped;
    if(QThread::isRunning())
        return fPause ? Paused : Running;
    return Stoped;
}

GFE_ProgDlg_Thread *GFE_ProgDlg_Thread::create(std::function<void ()> routine)
{
    auto thread = new GFE_ProgDlg_Thread;
    thread->routine = routine;
    return thread;
}

void GFE_ProgDlg_Thread::start(Priority pri)
{
    QThread::start(pri);
}

void GFE_ProgDlg_Thread::stop()
{
    if (QThread::isRunning())
    {
        fToStop = true;
        QThread::terminate();
        QThread::wait();
    }
}

void GFE_ProgDlg_Thread::pause()
{
    fPause = true;
    //!创建一个监控线程，接收发送来的线程恢复信号
    auto func = std::bind(&GFE_ProgDlg_Thread::control,this);
    QThread *control = QThread::create(func);
    control->start();

    msleep(10);

    mutex.lock();
    mutex.unlock();

    fPause = false;
}

void GFE_ProgDlg_Thread::control()
{
    mutex.lock();
    while(fPause){
        //!如果选择结束线程，直接跳出，结束线程
        if(fStopPause){
            mutex.unlock();
            this->stop();
            break;
        }
    }

    mutex.unlock();
}

void GFE_ProgDlg_Thread::retry(Priority pri)
{
    if(isRunning())
        return;
    start(pri);
}

void GFE_ProgDlg_Thread::toResumePause()
{
    fPause = false;
}

void GFE_ProgDlg_Thread::toStopPause()
{
    fStopPause = true;
}
#ifdef _WIN32
#include <windows.h>
#include <Dbghelp.h>
extern long WINAPI UE_callback(_EXCEPTION_POINTERS* excp);
#endif


void GFE_ProgDlg_Thread::run()
{
    t_thread = this;
    //    qDebug() << "enter thread : " << QThread::currentThreadId();
    fCrash = false;
    fToStop = false;
    fAbort = false;
    fExit = false;

    // 拦截手动中止的异常
    auto TryRoutine = [&] {
        try {
            routine();
        }
        catch(const ThreadAbortException&) {
            fAbort = true;
        }
        catch(const ThreadExitException&) {
            fExit = true;
        }
    };

#ifdef _WIN32
    __try {
        TryRoutine();
    }
    __except((UE_callback(GetExceptionInformation()),EXCEPTION_EXECUTE_HANDLER)) {
        fCrash = true;
    }
#else
    routine();
#endif

    //    qDebug() << "exit thread : " << QThread::currentThreadId();
}
