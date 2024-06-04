#include "GFE_ProgDlg_Simple.h"
#include "ui_GFE_ProgDlg_Simple.h"
#include <QLabel>
#include <QThread>
#include <QMessageBox>
#include <QCheckBox>
#include "GFE_ProgDlg_Thread.h"
#include <QFile>
//#include "Frame/Application.h"

GFE_ProgDlg_Simple::GFE_ProgDlg_Simple(QWidget *parent) :
    GFE_Dialog(parent),
    ui(new Ui::GFE_ProgDlg_Simple)
{
    ui->setupUi(this);
    QFile qss(":/Res/QSS/Common.qss");
    qss.open(QFile::ReadOnly);
    this->setStyleSheet(qss.readAll());
    qss.close();
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    connect(ui->pbClose, &QPushButton::clicked, this, &GFE_ProgDlg_Simple::__Stop);
    connect(ui->pbRetry, &QPushButton::clicked, this, &GFE_ProgDlg_Simple::Retry);
    connect(this, &GFE_ProgDlg_Simple::ToUpdate, this, &GFE_ProgDlg_Simple::__Update);
    connect(this, &GFE_ProgDlg_Simple::ToFinish, this, &GFE_ProgDlg_Simple::__Finish);

    ui->pbRetry->hide();
}

GFE_ProgDlg_Simple::~GFE_ProgDlg_Simple()
{
    delete ui;
}

GFE_ProgDlg_Simple* GFE_ProgDlg_Simple::GlobalInstance()
{
//    static auto instance = new GFE_ProgDlg_Simple(App::Get());
    static auto instance = new GFE_ProgDlg_Simple();
    return instance;
}

void GFE_ProgDlg_Simple::Init(bool isInfinite)
{
    auto item = ui->widget;
    item->Init();
    item->circle->setMaximum(isInfinite ? 0 : 10000);
    item->circle->setValue(0);
    item->UpdateText(tr("Initialized"));
    ui->pbClose->setText(tr("Abort"));

    result = false;
    _isSubsequent = false;
    _toAbort = false;
    _isInfinite = isInfinite;
}

void GFE_ProgDlg_Simple::Start(const std::function<void ()>& routine)
{
    //_thread.reset(QThread::create(routine)); //!修改
    _thread.reset(GFE_ProgDlg_Thread::create(routine));
    //!连接线程恢复和停止的信号和槽
    connect(this, &GFE_ProgDlg_Simple::ResumeSignal, _thread.get(), &GFE_ProgDlg_Thread::toResumePause);
    connect(this, &GFE_ProgDlg_Simple::StopSignal, _thread.get(), &GFE_ProgDlg_Thread::toStopPause);
    // auto conn_update = connect(this, &GFE_ProgDlg_Simple::ToUpdate, this, &GFE_ProgDlg_Simple::__Update);
    // auto conn_finish = connect(this, &GFE_ProgDlg_Simple::ToFinish, this, &GFE_ProgDlg_Simple::__Finish);
    connect(_thread.get(), &GFE_ProgDlg_Thread::finished, this, [=] {
        auto state = _thread->state();
        if(state == GFE_ProgDlg_Thread::Crash) {
            // 处理崩溃
            resize(400, height());
            Finish(1, tr("The thread stopped due to an unknown error.\n"
                         "\n"
                         "To assist us in enhancing the application, please send the "
                         "PrePo.dmp file from the program's direcotry,which will provide "
                         "essential information for resolving the issue."));
        }
        else if(state == GFE_ProgDlg_Thread::Abort) {
            // 处理手动中止
            GFE_Dialog::close();
            ui->pbClose->setHidden(false);
        }
        // disconnect(conn_update);
        // disconnect(conn_finish);
    });
    _thread->start();
    exec();
}

void GFE_ProgDlg_Simple::Update(double rate, const QString& msg)
{
    if(_toAbort && QThread::currentThread() != QCoreApplication::instance()->thread())
        throw ThreadAbortException();

    if(_isInfinite && !std::isnan(rate))
        emit ToUpdate(rate, QString(msg).append(" (%1%)").arg(rate*100, 0, 'f', 2));
    else
        emit ToUpdate(rate, msg);
}

void GFE_ProgDlg_Simple::Update(size_t left, size_t right, const QString& msg)
{
    if(_toAbort && QThread::currentThread() != QCoreApplication::instance()->thread())
        throw ThreadAbortException();

    auto rate = (double)left/(double)right;
    if(_isInfinite && !std::isnan(rate))
        emit ToUpdate(rate, QString(msg).append(" (%1/%2)").arg(left).arg(right));
    else
        emit ToUpdate(rate, msg);
}

void GFE_ProgDlg_Simple::Finish(int status, const QString& errMsg)
{
    // 手动中止不需要触发__Finish
    if(_toAbort && QThread::currentThread() != QCoreApplication::instance()->thread())
        throw ThreadAbortException();

    // 除手动中止外，其他结束状态，需要先发送ToFinish信号让UI做相应动作（异步），同时线程自身开始准备退出或暂停
    emit ToFinish(status, errMsg);
    switch(status) {
    case Success:
    case Failed:
    case FailedRetry:
    {
        if(QThread::currentThread() != QCoreApplication::instance()->thread())
            throw ThreadExitException();
        break;
    }
    case Warning:
    case Pause:
    {
        //! 不为后续警告执行相同操作时，暂停线程
        if(!_isSubsequent)
            _thread->pause();
        break;
    }
    }
}

void GFE_ProgDlg_Simple::SetCanAbort(bool f)
{
    //    ui->pbClose->setEnabled(f);
    if(f) ui->pbClose->setHidden(false);
    else ui->pbClose->setHidden(true);
}

void GFE_ProgDlg_Simple::__Update(double rate, const QString& msg)
{
    auto item = ui->widget;
    if(!std::isnan(rate))
        item->circle->setValue(rate*10000);
    item->UpdateText(msg);
}


void GFE_ProgDlg_Simple::__Finish(int status, const QString& errMsg)
{
    //! 0: 成功
    //! 1：失败
    //! 2：警告
    //! 3：暂停
    //! 4：失败（可重试）

    auto item = ui->widget;
    if(status == 0) {
        item->circle->setMaximum(10000);
        item->circle->setValue(10000);
        item->Finish(true);
        result = true;
        if(autoClose) close();
        if(!errMsg.isEmpty())
            item->UpdateText(errMsg);
    }
    else if(status == 1) {
        item->circle->setMaximum(10000);
        item->circle->setValue(10000);
        item->Finish(false);
        //        ui->pbRetry->show();
        if(!errMsg.isEmpty())
            item->UpdateText(errMsg);
    }
    else if(status == 2){
        if(_isSubsequent){
            return;
        }
        else{
            QMessageBox msgBox(QMessageBox::Warning, tr("Warning"), errMsg, QMessageBox::Ignore | QMessageBox::Abort);
            msgBox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QCheckBox checkBox(tr("Ignore all warnings"));
            msgBox.setCheckBox(&checkBox);
            msgBox.exec();
            if (msgBox.clickedButton() == msgBox.button(QMessageBox::Ignore)) {
                emit ResumeSignal();
                if (checkBox.isChecked()){
                    _isSubsequent = true;
                }
                return;
            }
            else {
                emit StopSignal();
                item->UpdateText(tr("Aborted"));
                item->circle->setMaximum(10000);
                item->circle->setValue(10000);
                item->Finish(false);
            }
        }
    }
    else if(status == 3) {
        QMessageBox msgBox(QMessageBox::Question, tr("Message"), errMsg);
        QPushButton cont(tr("Continue"));
        QPushButton end(tr("End"));
        msgBox.addButton(&cont, QMessageBox::AcceptRole);
        msgBox.addButton(&end, QMessageBox::RejectRole);
        msgBox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        msgBox.exec();
        if (msgBox.clickedButton() == &cont) {
            emit ResumeSignal();
            resultContinue = true;
            return;
        }
        else {
            emit ResumeSignal();        // 要继续进行，子线程函数里可能有些后续处理
            resultContinue = false;
            return;
        }
    }
    else if(status == 4) {
        item->circle->setMaximum(10000);
        item->circle->setValue(10000);
        item->Finish(false);
        //        ui->pbRetry->show();
        if(!errMsg.isEmpty())
            item->UpdateText(errMsg);
        ui->pbRetry->show();
    }

    ui->pbClose->setText(tr("Close"));
    // if(_thread) _thread->stop();
}

void GFE_ProgDlg_Simple::Retry()
{
    Init(_isInfinite);
    Start(_thread->Routine());
    ui->pbRetry->hide();
}

void GFE_ProgDlg_Simple::showEvent(QShowEvent* e)
{
    MoveToGlobalCen();
    GFE_Dialog::showEvent(e);
}

void GFE_ProgDlg_Simple::__Stop()
{
    if(ui->pbClose->text() == tr("Close")) {
        GFE_Dialog::close();
        return;
    }

    auto item = ui->widget;
    item->circle->setMaximum(0);
    item->circle->setValue(0);
    item->UpdateText(tr("Waiting to abort"));
    ui->pbClose->setHidden(true);
    _toAbort = true;
}

