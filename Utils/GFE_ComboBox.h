#pragma once
#include <QComboBox>

class GFE_ComboBox : public QComboBox
{
public:
    explicit GFE_ComboBox(QWidget* parent = nullptr);

    static void SetFilterable(QComboBox* combo, bool flag = true);
    static void AlignCenter(QComboBox*);
};

