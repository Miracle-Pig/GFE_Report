#include "GFE_Utility.h"

#include <vtk-8.2/vtkCellData.h>
#include <vtk-8.2/vtkDataSet.h>
#include <vtk-8.2/vtkFloatArray.h>
#include <vtk-8.2/vtkPointData.h>
#include <vtk-8.2/vtkPCellDataToPointData.h>
#include <QBitmap>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QString>

vtkSmartPointer<vtkDataArray> GFE_Utility::CellData2PointData(vtkSmartPointer<vtkDataSet> ds, vtkSmartPointer<vtkDataArray> arr)
{
    if(!ds || !arr) return nullptr;

    VTK_SP(vtkFloatArray, RET);
    auto pn = ds->GetNumberOfPoints();
    auto cn = ds->GetNumberOfCells();
    RET->SetNumberOfTuples(pn);
    RET->SetName(arr->GetName());
    auto arr_size = arr->GetNumberOfTuples();
    if(arr_size != cn) return nullptr;
    VTK_SP(vtkIdList, cl);
    for(int i = 0; i < pn; i++) {
        ds->GetPointCells(i, cl);
        auto cl_size = cl->GetNumberOfIds();
        float p = 0;
        int n = cl_size;
        for(int j = 0; j < cl_size; j++) {
            auto v = arr->GetTuple1(cl->GetId(j));
            if(isnan(v)) {
                n--;
                continue;
            }
            p+=v;
        }
        p/=n;
        RET->SetTuple1(i, p);
    }

    return RET;
}

void GFE_Utility::UpdateProgressButton(QPushButton* b, const QString& text, float rate)
{
    static QString ss = "background:qlineargradient("
                        "x1:0, "
                        "x2:1, "
                        "stop:0 green, "
                        "stop:%1 green, "
                        "stop:%2 white, "
                        "stop:1 white);";
    if(rate >= 1) {
        b->setText(text);
        b->setStyleSheet("");
        b->repaint();
    }
    else if(rate > 0) {
        b->setText(QString::number(rate*100)+"%");
        b->setStyleSheet(ss.arg(rate).arg(fmin(rate+rate/100, 1.0)));
        b->repaint();
    }
    else {
        return;
    }
}

std::vector<std::string> GFE_Utility::split(const std::string& input, const std::string& delim)
{
    using namespace std;
    vector<string> RET;
    regex re(delim);

    sregex_token_iterator p(input.begin(), input.end(), re, -1);
    sregex_token_iterator end;
    while (p != end)
        RET.push_back(*p++);

    return RET;
}

QPixmap GFE_Utility::ChangePixmapColor(const QPixmap &source, const QColor& orig, const QColor& dest)
{
    static QPainter p;
    auto mask = source.createMaskFromColor(orig, Qt::MaskOutColor);
    QPixmap ret(mask.size());
    ret.fill(dest);
    ret.setMask(mask);
    p.drawPixmap(100, 100, ret);
    return ret;
}
