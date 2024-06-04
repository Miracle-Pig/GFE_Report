#include <QThread>
#include <atomic>
#include <QMutex>

class GFE_ProgDlg_Simple;
class GFE_ProgDlg_Thread : public QThread
{

    Q_OBJECT
public:
    GFE_ProgDlg_Thread(QObject *parent = nullptr);
    ~GFE_ProgDlg_Thread() override;

    enum State
    {
        Stoped,     ///<停止状态，包括从未启动过和启动后被停止
        Running,    ///<运行状态
        Paused,      ///<暂停状态
        Crash,      ///<崩溃
        ToStop,     ///<即将正常退出
        Abort       ///<手动中止
    };

    State state() const;

    static GFE_ProgDlg_Thread *create(std::function<void()> routine);

public slots:
    void start(Priority pri = InheritPriority);
    void stop();

    void pause();
    void control();
    void retry(Priority pri = InheritPriority);

    void toResumePause();
    void toStopPause();

    std::function<void()> Routine() { return routine; }

protected:
    void run() override final;


private:
    std::atomic_bool fPause = false;
    std::atomic_bool fStopPause = false;    // 暂停后选择中止，不再继续
    QMutex mutex;
    std::function<void()> routine;
    GFE_ProgDlg_Thread* t_thread;
    bool fCrash = false, fToStop = false;
    bool fAbort = false;
    bool fExit = false;
    // std::atomic_bool fPause, fAbort, fCrash;
};

//! @brief 手动中止
class ThreadAbortException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Thread exit requested";
    }
};

//! @brief 在检查点识别到错误，退出
class ThreadExitException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Thread exit requested";
    }
};
