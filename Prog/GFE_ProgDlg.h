#pragma once

#include "../Utils/GFE_Dialog.h"
#include "GFE_ProgCircle.h"

namespace Ui {
class GFE_ProgDlg;
}

//!
//! \brief 列表式的进度对话框. 单行的见GFE_ProgDlg_Simple
//!
class GFE_ProgDlg : public GFE_Dialog
{
    Q_OBJECT
public:
    explicit GFE_ProgDlg(QWidget *parent = nullptr);
    ~GFE_ProgDlg();

    void Init(const QStringList& msgList);

    void Update(int row, double rate);
    void Update(int row, double rate, const QString& msg);
    void Finish(int row, int status, const QString& errMsg);        // status: 0 成功 1 失败 2警告

    virtual void closeEvent(QCloseEvent*) override;

    bool autoClose = false;                 // 执行成功后自动关闭对话框

signals:
    void ToUpdate(int, double);          // 更新第i行的进度, 第二个参数为NAN时表示进度为inifinite
    void ToUpdate(int, double, const QString&);       // 更新第i行的进度, 同时更新信息

    void ToFinish(int, int, const QString&);

    void Retried();
    void Closed(bool);                  // 0: 失败或者中断 1 成功

private:
    Ui::GFE_ProgDlg* ui;
    bool _isSuccess = false;
};

class QLabel;
class GFE_ProgItem : public QWidget
{
public:
    explicit GFE_ProgItem(QWidget* parent = nullptr);
    GFE_ProgItem(const QString& text, QWidget* parent = nullptr);
    ~GFE_ProgItem();

    void UpdateText(const QString&);
    void Init();
    void Finish(bool);

    GFE_ProgCircle* circle = nullptr;

private:
    QLabel* label = nullptr;
};
