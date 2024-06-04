#include "GFE_ComboBox.h"
#include <QCompleter>
#include <QListView>

GFE_ComboBox::GFE_ComboBox(QWidget* parent) :
    QComboBox(parent)
{

}

void GFE_ComboBox::SetFilterable(QComboBox* combo, bool flag)
{
    if(flag) {
        combo->setEditable(true);
        //! Completer
        auto comp = new QCompleter(combo->model(), combo->parent() ? combo->parent() : combo);
        comp->setFilterMode(Qt::MatchContains);
        comp->setCaseSensitivity(Qt::CaseInsensitive);
        comp->setModelSorting(QCompleter::CaseSensitivelySortedModel);
        //! Optimize completer performance
        auto popup = qobject_cast<QListView*>(comp->popup());
        popup->setUniformItemSizes(true);
        popup->setLayoutMode(QListView::Batched);
        combo->setCompleter(comp);
    }
    else {
        combo->setEditable(false);
    }
}

#include <QLineEdit>
void GFE_ComboBox::AlignCenter(QComboBox* combo)
{
    if(combo->isEditable()) {
        combo->lineEdit()->setAlignment(Qt::AlignCenter);
    }
    else {
        auto le = new QLineEdit(combo);
        le->setAlignment(Qt::AlignCenter);
        le->setReadOnly(true);
        combo->setLineEdit(le);
    }
}
