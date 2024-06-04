#pragma once

#include "macrodef.h"
#include <regex>
#include <vector>
#include <vtk-8.2/vtkAOSDataArrayTemplate.h>

class vtkDataSet;
class QColor;
class QIcon;
class QPixmap;
class QPushButton;
class QString;

namespace GFE_Utility {

// VTK
template<typename T>
std::vector<T> vtkArrayToStdVec(vtkAOSDataArrayTemplate<T>* _arr) {
    std::vector<T> RET;
    // 仅支持components = 1的情况
    if(!_arr || _arr->GetNumberOfComponents() != 1) return RET;
    auto n = _arr->GetNumberOfTuples();
    RET.reserve(n);
    for(vtkIdType i = 0; i < n; i++)
        RET.push_back(_arr->GetTuple1(i));
    return RET;
}

template<typename T>
vtkSmartPointer<vtkAOSDataArrayTemplate<T>> stdVecToVtkArray(const std::vector<T>& _v) {
    VTK_SP(vtkAOSDataArrayTemplate<T>, RET);
    RET->Allocate(_v.size());
    for(const auto& i : _v)
        RET->InsertNextTuple1(i);
    return RET;
}

vtkSmartPointer<vtkDataArray> CellData2PointData(vtkSmartPointer<vtkDataSet> _ds, vtkSmartPointer<vtkDataArray> _arr);

// Qt
void UpdateProgressButton(QPushButton* b, const QString& text, float rate);
QPixmap ChangePixmapColor(const QPixmap& source, const QColor& orig, const QColor& dest);

// Cpp
std::vector<std::string> split(const std::string& input, const std::string& delim);
}
