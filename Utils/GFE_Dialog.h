#pragma once

#include <QDialog>

class QPropertyAnimation;
class QComboBox;
class QTableWidget;
class QLabel;
class QAbstractButton;
class GFE_Dialog : public QDialog
{
    Q_OBJECT;
public:
    explicit GFE_Dialog(QWidget* parent = nullptr, QAbstractButton* relatedButton = nullptr);
    static void PopupRight(bool state, QWidget* parent, QWidget* target);
    void PopupRight(bool state, QWidget* target) { GFE_Dialog::PopupRight(state, this, target); }
    void MoveToRight(QWidget* parent) { MoveToRight(parent, this); }
    static void MoveToRight(QWidget* parent, QWidget* target);
    void MoveToCen() { MoveToCen(parentWidget(), this); }
    void MoveToCen(QWidget* parent) { MoveToCen(parent, this); }
    static void MoveToCen(QWidget* parent, QWidget* target);
    void MoveToGlobalRight() { MoveToGlobalRight(this); }
    static void MoveToGlobalRight(QWidget* target);
    void MoveToGlobalCen() { MoveToGlobalCen(this); }
    static void MoveToGlobalCen(QWidget* target);

    //! 淡出
//    static void FadeOut(QWidget* widget, QPropertyAnimation* anm = DefaultAnm_FadeOut());
//    static QPropertyAnimation* DefaultAnm_FadeOut();

    //! ComboBox支持搜索功能
    static void CBoxFilterOn(QComboBox*);

    //! 隐藏, 但占位(即仅改变透明度和可交互性)
    static void Occlude(QWidget*, bool state);

    //! 通过qss文件设置样式表
    void SetQSS(const QString& qssFile);

    //!
    void LinkEnable(QLabel*, const QString& text, bool state);

    //! @brief 弹出文件选择对话框
    //! @return 被选择的文件名
    QStringList SelectedFiles(const QString& nameFilter);
    QString SelectedFile(const QString& nameFilter) {
        auto ret = SelectedFiles(nameFilter);
        if(ret.empty()) return QString();
        return ret[0];
    }
    QString SelectedDir();
    QString ExportFile(const QString& suffix);

    void ShowToggle() { setVisible(!isVisible()); }

protected:
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    QAbstractButton* _relatedButton;
};

