#include "GFE_ProgDlg.h"
#include "ui_GFE_ProgDlg.h"
#include <QLabel>

GFE_ProgDlg::GFE_ProgDlg(QWidget *parent) :
      GFE_Dialog(parent),
      ui(new Ui::GFE_ProgDlg)
{
    ui->setupUi(this);
    connect(ui->pbClose, &QPushButton::clicked, this, &GFE_ProgDlg::close);
    connect(ui->pbRetry, &QPushButton::clicked, this, [this] { emit Retried(); });

    connect(this, QOverload<int, double>::of(&GFE_ProgDlg::ToUpdate), this, QOverload<int, double>::of(&GFE_ProgDlg::Update));
    connect(this, QOverload<int, double, const QString&>::of(&GFE_ProgDlg::ToUpdate), this, QOverload<int, double, const QString&>::of(&GFE_ProgDlg::Update));
    connect(this, &GFE_ProgDlg::ToFinish, this, &GFE_ProgDlg::Finish);
}

GFE_ProgDlg::~GFE_ProgDlg()
{
    delete ui;
}

void GFE_ProgDlg::Init(const QStringList& msgList)
{
    auto n = msgList.size();
    auto layout = ui->scrollAreaWidgetContents->layout();
    for(int i = 0; i < n; i++) {
        auto item = new GFE_ProgItem(msgList[i]);
        item->setFixedHeight(40);
        item->circle->setMaximum(10000);
        layout->addWidget(item);
    }
}

void GFE_ProgDlg::Update(int row, double rate)
{
    auto layout = ui->scrollAreaWidgetContents->layout();
    auto item = (GFE_ProgItem*)layout->itemAt(row)->widget();
    if(std::isnan(rate)) {
        item->circle->setValue(0);
        item->circle->setMaximum(0);
    }
    else {
        item->circle->setValue(rate*10000);
    }
}

void GFE_ProgDlg::Update(int row, double rate, const QString& msg)
{
    auto layout = ui->scrollAreaWidgetContents->layout();
    auto item = (GFE_ProgItem*)layout->itemAt(row)->widget();
    if(std::isnan(rate)) {
        item->circle->setValue(0);
        item->circle->setMaximum(0);
    }
    else {
        item->circle->setValue(rate*10000);
    }
    item->UpdateText(msg);
}

void GFE_ProgDlg::Finish(int row, int status, const QString& /*errMsg*/)
{
    auto layout = ui->scrollAreaWidgetContents->layout();
    auto item = (GFE_ProgItem*)layout->itemAt(row)->widget();
    if(status == 0) {
        item->circle->setMaximum(10000);
        item->circle->setValue(10000);
        item->Finish(true);
    }
    else if(status == 1) {
        item->Finish(false);
    }
    else {
        // todo: 警告
        item->Finish(false);
    }
    // todo: 处理errMsg
    if(row == layout->count()-1) {
        if(status == 0) _isSuccess = true;
        if(autoClose) close();
    }
}

void GFE_ProgDlg::closeEvent(QCloseEvent* e)
{
    emit Closed(_isSuccess);
    auto layout = ui->scrollAreaWidgetContents->layout();
    for(int i = layout->count()-1; i >= 0; i--) {
        auto w = layout->itemAt(i)->widget();
        layout->removeWidget(w);
        delete w;
    }
    GFE_Dialog::closeEvent(e);
}

GFE_ProgItem::GFE_ProgItem(QWidget* parent) :
      QWidget(parent)
{
    setStyleSheet(".QWidget{border-bottom: 1px solid lightgrey;}");
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setSpacing(12);
    Init();
}

GFE_ProgItem::GFE_ProgItem(const QString& text, QWidget* parent) :
      QWidget(parent)
{
    setStyleSheet(".QWidget{border-bottom: 1px solid lightgrey;}");
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setSpacing(12);
    Init();
    label->setText(text);
}

GFE_ProgItem::~GFE_ProgItem()
{
    delete circle;
    delete label;
}

void GFE_ProgItem::UpdateText(const QString& text)
{
    if(label) {
        label->setText(text);
        this->adjustSize();
        auto parent = parentWidget();
        if(parent)
            parent->adjustSize();
    }
}

void GFE_ProgItem::Finish(bool f)
{
    QPropertyAnimation * animation = new QPropertyAnimation(circle, "outerRadius", circle);
    animation->setDuration(1500);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setEndValue(0.5);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    animation = new QPropertyAnimation(circle, "innerRadius", circle);
    animation->setDuration(750);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setEndValue(0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    QColor color = f ? QColor(155,219,58) : QColor(255,100,100);

    animation = new QPropertyAnimation(circle, "color", circle);
    animation->setDuration(750);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setEndValue(color);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GFE_ProgItem::Init()
{
    auto layout = qobject_cast<QHBoxLayout*>(this->layout());
    if(circle) {
        layout->removeWidget(circle);
        layout->removeWidget(label);
        delete circle;
        delete label;
    }

    circle = new GFE_ProgCircle(this);
    circle->setFixedSize(30, 30);
    layout->addWidget(circle);
    label = new QLabel(this);
    label->setWordWrap(true);
    layout->addWidget(label, 1);

    QPropertyAnimation * animation = new QPropertyAnimation(circle, "outerRadius", circle);
    animation->setDuration(750);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
