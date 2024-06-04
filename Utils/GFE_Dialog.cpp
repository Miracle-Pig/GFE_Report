#include "GFE_Dialog.h"
#include <QShortcut>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QAbstractButton>
#include <QListView>
#include <QComboBox>
#include <QFile>

GFE_Dialog::GFE_Dialog(QWidget* parent, QAbstractButton* relatedButton) :
    QDialog(parent),
    _relatedButton(relatedButton)
{
    //    auto scOK = new QShortcut(Qt::Key_Return, this);
    //    auto scCancel = new QShortcut(Qt::Key_Escape, this);
    //    connect(scOK, &QShortcut::activated, [this] { accept(); });
    //    connect(scCancel, &QShortcut::activated, [this] { reject(); });
    if(relatedButton)
        connect(relatedButton, &QAbstractButton::toggled, this, [=](bool f) { PopupRight(f, parent, this); });
}


void GFE_Dialog::PopupRight(bool state, QWidget* parent, QWidget* target)
{
    if(state) {
        constexpr int MARGIN = 16;
        target->setGeometry(parent->frameGeometry().topRight().x() + MARGIN,
                            parent->geometry().y(),
                            target->width(),
                            target->height());
        target->show();
    }
    else target->hide();
}

void GFE_Dialog::MoveToRight(QWidget* parent, QWidget* target)
{
    constexpr int MARGIN = 16;
    target->setGeometry(parent->frameGeometry().topRight().x() + MARGIN,
                        parent->geometry().y(),
                        target->width(),
                        target->height());
}

//void GFE_Dialog::FadeOut(QWidget* widget, QPropertyAnimation* anm)
//{
//    if(!widget)
//        return;

//    auto ge = widget->graphicsEffect();
//    if(!ge) {
//        ge =new QGraphicsOpacityEffect(widget);
//        widget->setGraphicsEffect(ge);
//    }

//    anm->setTargetObject(ge);
//    if(anm->state() == QPropertyAnimation::Running) {
//        anm->stop();
//    }
//    anm->start();
//}

//QPropertyAnimation* GFE_Dialog::DefaultAnm_FadeOut()
//{
//    static bool init = false;
//    static auto anm = new QPropertyAnimation(App::Get());
//    if(!init) {
//        anm->setPropertyName("opacity");
//        anm->setEasingCurve(QEasingCurve::Linear);
//        anm->setDuration(2000);
//        anm->setStartValue(2);
//        anm->setEndValue(0);
//        //        connect(ani, &QPropertyAnimation::finished, [] {
//        //            std::cout << "property animation" << std::endl;
//        //        });
//        init = true;
//    }
//    return anm;
//}

#include <QCompleter>
void GFE_Dialog::CBoxFilterOn(QComboBox* cbox)
{
    auto completer = new QCompleter(cbox->model(), cbox);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    auto popup = qobject_cast<QListView*>(completer->popup());
    popup->setUniformItemSizes(true);
    popup->setLayoutMode(QListView::Batched);
    cbox->setCompleter(completer);
    cbox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
}

void GFE_Dialog::Occlude(QWidget* widget, bool state)
{
    auto opacity = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if(!opacity) {
        opacity = new QGraphicsOpacityEffect;
    }
    if(state) {
        widget->setGraphicsEffect(opacity);
        opacity->setOpacity(1);
        widget->setEnabled(true);
    }
    else {
        widget->setGraphicsEffect(opacity);
        opacity->setOpacity(0);
        widget->setEnabled(false);
    }
}

void GFE_Dialog::SetQSS(const QString& qssFile)
{
    QFile f(qssFile);
    f.open(QFile::ReadOnly);
    this->setStyleSheet(f.readAll());
    f.close();
}

void GFE_Dialog::MoveToCen(QWidget* parent, QWidget* target)
{
    if(!parent)
        return;
    auto globalPos = parent->mapToGlobal({0,0});
    int x = globalPos.x() + (parent->width() - target->width())/2;
    int y = globalPos.y() + (parent->height() - target->height())/2;
    target->move(x, y);
}

#include <QScreen>
void GFE_Dialog::MoveToGlobalRight(QWidget* target)
{
    auto screen = QGuiApplication::primaryScreen()->availableVirtualGeometry();
    int x = screen.width()/1.5;
    int y = screen.height()/4;
    target->move(x, y);
}

void GFE_Dialog::MoveToGlobalCen(QWidget* target)
{
    if(!target)
        return;

    auto screen = QGuiApplication::primaryScreen()->availableVirtualGeometry();

    auto screenWidth = screen.width();
    auto screenHeight = screen.height();
    auto targetWidth = target->width();
    auto targetHeight = target->height();

    target->move(screenWidth/2 - 0.6*targetWidth, screenHeight/2 - 0.4*targetHeight);
}

#include <QLabel>
void GFE_Dialog::LinkEnable(QLabel* label, const QString& text, bool state)
{
    label->setEnabled(state);
    if(state) {
        label->setText(QString("<html><head/><body><p><a href=\"dummy\">%1</a></p></body></html>").arg(text));
    }
    else {
        label->setText(QString("<html><head/><body><p><a style='color: gray;' href=\"dummy\">%1</a></p></body></html>").arg(text));
    }
}

#include <QFileDialog>
QStringList GFE_Dialog::SelectedFiles(const QString& nameFilter)
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setNameFilter(nameFilter);
    dlg.exec();
    if(dlg.result()) return dlg.selectedFiles();
    return QStringList();
}

QString GFE_Dialog::SelectedDir()
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::DirectoryOnly);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.exec();
    if(dlg.result()) return dlg.selectedFiles()[0];
    return QString();
}

QString GFE_Dialog::ExportFile(const QString& suffix)
{
    QFileDialog dlg(this);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setDefaultSuffix(suffix);
    dlg.setNameFilter("*."+suffix);
    dlg.exec();
    if(dlg.result()) return dlg.selectedFiles()[0];
    return QString();
}

void GFE_Dialog::showEvent(QShowEvent* event)
{
    if(_relatedButton)
        _relatedButton->setChecked(true);
    QDialog::showEvent(event);
}

void GFE_Dialog::hideEvent(QHideEvent* event)
{
    // 作为父控件，隐藏时所有子控件也隐藏
    for(auto& dlg : this->findChildren<QDialog*>())
        dlg->hide();

    // 作为子控件, 隐藏时对应的按钮设置为unchecked
    if(_relatedButton)
        _relatedButton->setChecked(false);

    QDialog::hideEvent(event);
}
