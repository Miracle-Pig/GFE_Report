#pragma once

#include "../Utils/GFE_Dialog.h"
#include <functional>

namespace Ui {
class GFE_ProgDlg_Simple;
}

class GFE_ProgDlg_Thread;
class GFE_ProgDlg_Simple : public GFE_Dialog
{
    Q_OBJECT
public:
    enum eStatus {
        Success,
        Failed,
        Warning,
        Pause,
        FailedRetry
    };

    explicit GFE_ProgDlg_Simple(QWidget *parent = nullptr);
    ~GFE_ProgDlg_Simple();

    static GFE_ProgDlg_Simple* GlobalInstance();

    void Init(bool isInfinite);
    void Start(const std::function<void()>&);

    //! @brief 更新当前进度，更新前会发射CanStop信号并休眠$gap毫秒
    //! 若对话框此时处于「等待中止」的状态，会开始中止线程
    //! @param rate: NAN表示inifinite
    //! @param gap： 为负数时表示不休眠
    void Update(double rate, const QString& msg);
    void Update(size_t left, size_t right, const QString& msg);
    void Update(const QString& msg) { Update(NAN, msg); }
    void Finish(int status, const QString& errMsg);

    void SetCanAbort(bool f);
    void Retry();

    bool autoClose = false;         // 成功后自动关闭对话框
    bool result = false;

    //! @brief 结合Finish(status = 3)使用, 用于某些多步操作的场景, 比如
    //! 某迭代计算场景, 每次迭代后输出当次的结果, 若满足要求, 用户选择停止迭代, 取该结果, 也可以继续迭代
    bool resultContinue = false;

signals:
    void ToUpdate(double, const QString&);
    void ToFinish(int, const QString&);
    void CanStop();

    void ResumeSignal(); //!线程恢复信号
    void StopSignal();  //!线程停止信号

private:
    void showEvent(QShowEvent*) override;

    //! @brief 手动中止时，UI相应的动作
    void __Stop();
    void __Update(double rate, const QString& msg);

    //! @brief 各种结束状态，UI相应的动作
    void __Finish(int status, const QString& errMsg);     // status: 0 成功 1 失败 2 警告 3 暂停(不中止线程，通过resultContinue判断) 4 失败（可重试）

    Ui::GFE_ProgDlg_Simple* ui;
//    QSharedPointer<QThread> _thread;
    QSharedPointer<GFE_ProgDlg_Thread> _thread;//!修改
    bool _isInfinite = true;       // 用于retry

    bool _isSubsequent = false;//是否为后续警告都采用此选择
    bool _toAbort = false;
};
