#include "DlgReportUtil.h"

#include <QDebug>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QObject>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QObject>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QTemporaryDir>
#include <QFont>

#include <Windows.h>
#include <Python.h>

#include <sstream>
#include <filesystem>
#include <math.h>

#include <vtk-8.2/vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);   // VTK was built withvtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);

#include <vtk-8.2/vtkIntArray.h>
#include <vtk-8.2/vtkFloatArray.h>
#include <vtk-8.2/vtkStringArray.h>
#include <vtk-8.2/vtkPointData.h>
#include <vtk-8.2/vtkCellType.h>
#include <vtk-8.2/vtkCellData.h>
#include <vtk-8.2/vtkScalarBarActor.h>
#include <vtk-8.2/vtkScalarBarWidget.h>
#include <vtk-8.2/vtkScalarBarRepresentation.h>
#include <vtk-8.2/vtkTextProperty.h>
#include <vtk-8.2/vtkCamera.h>
#include <vtk-8.2/vtkAxesActor.h>
#include <vtk-8.2/vtkOrientationMarkerWidget.h>
#include <vtk-8.2/vtkProperty.h>
#include <vtk-8.2/vtkExtractEdges.h>
#include <vtk-8.2/vtkCellCenters.h>
#include <vtk-8.2/vtkSelectVisiblePoints.h>
#include <vtk-8.2/vtkLabelPlacementMapper.h>
#include <vtk-8.2/vtkPointSetToLabelHierarchy.h>
#include <vtk-8.2/vtkPlot.h>
#include <vtk-8.2/vtkChartLegend.h>
#include <vtk-8.2/vtkAxis.h>
#include <vtk-8.2/vtkContextView.h>
#include <vtk-8.2/vtkContextScene.h>
#include <vtk-8.2/vtkTable.h>
#include <vtk-8.2/vtkDoubleArray.h>
#include <vtk-8.2/vtkChartXY.h>
#include <vtk-8.2/vtkAppendSelection.h>
#include <vtk-8.2/vtkWindowToImageFilter.h>
#include <vtk-8.2/vtkPNGWriter.h>
#include <vtk-8.2/vtkColorTransferFunction.h>
#include <vtk-8.2/vtkPolyDataMapper.h>
#include <vtk-8.2/vtkDataSetSurfaceFilter.h>
#include <vtk-8.2/vtkTextActor.h>
#include <vtk-8.2/vtkNamedColors.h>
#include <vtk-8.2/vtkMatrix4x4.h>
#include <vtk-8.2/vtkConeSource.h>
#include <vtk-8.2/vtkCylinderSource.h>
#include <vtk-8.2/vtkTransform.h>
#include <vtk-8.2/vtkMinimalStandardRandomSequence.h>
#include <vtk-8.2/vtkTransformPolyDataFilter.h>

#include "MemberPJ.h"
#include "GFE_FileInfo.h"
#include "GFE_StrUtil.h"
#include "Utils/GFE_Utility.h"

#define QD qDebug()<<"****************"<<

#define DELETE_PTR(ptr) \
delete ptr; \
    ptr = nullptr; \
 \

ReportBookMark::InsertType ReportBookMark::StrToInsertType(const std::string &str)
{
    if(str == "Text"){
        return InsertType::Text;
    }else if(str == "Table"){
        return InsertType::Table;
    }else if(str == "CurveChart"){
        return InsertType::CurveChart;
    }else if(str == "Picture"){
        return InsertType::Picture;
    }else if(str == "CloudChart"){
        return InsertType::CloudChart;
    }else if(str == "Number"){
        return InsertType::Number;
    }
    return InsertType::UNKNOWInsertType;
}

ReportBookMark::GFEType ReportBookMark::StrToGFEType(const std::string &str)
{
    if(str == "Vibration"){
        return GFEType::Vibration;
    }else if(str == "SDR"){
        return GFEType::SDR;
    }else if(str == "Comb"){
        return GFEType::Comb;
    }else if(str == "SoilDyna"){
        return GFEType::SoilDyna;
    }else if(str == "Property"){
        return GFEType::Property;
    }else if(str == "Comp"){
        return GFEType::Comp;
    }else if(str == "Model"){
        return GFEType::Model;
    }else if(str == "AMP"){
        return GFEType::AMP;
    }else if(str == "Storey"){
        return GFEType::Storey;
    }else if(str == "Damage"){
        return GFEType::Damage;
    }else if(str == "Floor"){
        return GFEType::Floor;
    }else if(str == "Soil"){
        return GFEType::Soil;
    }else if(str == "Material"){
        return GFEType::Material;
    }else if(str == "XYData"){
        return GFEType::XYData;
    }else if(str == "Test"){
        return GFEType::Test;
    }
    return GFEType::UNKNOWGFEType;
}

ReportBookMark::Param ReportBookMark::StrToParam(const std::string &str)
{
    if(str == "E2X"){
        return Param::E2X;
    }else if(str == "E2Y"){
        return Param::E2Y;
    }else if(str == "ShearForce"){
        return Param::ShearForce;
    }else if(str == "AxialForce"){
        return Param::AxialForce;
    }else if(str == "Bending"){
        return Param::Bending;
    }else if(str == "InterForce"){
        return Param::InterForce;
    }else if(str == "Jiegou"){
        return Param::Jiegou;
    }else if(str == "ACR"){
        return Param::ACR;
    }else if(str == "DLQ"){
        return Param::DLQ;
    }else if(str == "Overall"){
        return Param::Overall;
    }else if(str == "Envelope"){
        return Param::Envelope;
    }else if(str == "MatParam"){
        return Param::MatParam;
    }else if(str == "SoilInfo"){
        return Param::SoilInfo;
    }else if(str == "PJ"){
        return Param::PJ;
    }else if(str == "Max"){
        return Param::Max;
    }else if(str == "List"){
        return Param::List;
    }else if(str == "Time"){
        return Param::Time;
    }else if(str == "Attribute"){
        return Param::Attribute;
    }else if(str == "EERA"){
        return Param::EERA;
    }else if(str == "ElemSet"){
        return Param::ElemSet;
    }else if(str == "SeismWave"){
        return Param::SeismWave;
    }else if(str == "Energy"){
        return Param::Energy;
    }else if(str == "Wall"){
        return Param::Wall;
    }else if(str == "Col"){
        return Param::Col;
    }else if(str == "Beam"){
        return Param::Beam;
    }else if(str == "Level"){
        return Param::Level;
    }else if(str == "Spectrum"){
        return Param::Spectrum;
    }else if(str == "Case"){
        return Param::Case;
    }else if(str == "Displacement"){
        return Param::Displacement;
    }else if(str == "Acceleration"){
        return Param::Acceleration;
    }else if(str == "Damagec"){
        return Param::Damagec;
    }else if(str == "Elem"){
        return Param::Elem;
    }else if(str == "Node"){
        return Param::Node;
    }else if(str == "Layer"){
        return Param::Layer;
    }else if(str == "UDiff"){
        return Param::UDiff;
    }else if(str == "HighLight"){
        return Param::HighLight;
    }else if(str == "Hysteretic"){
        return Param::Hysteretic;
    }else if(str == "Temp"){
        return Param::Temp;
    }
    return Param::UNKNOWParam;
}

const QHash<ReportBookMark::InsertType, QHash<ReportBookMark::GFEType, QSet<ReportBookMark::Param> > > &ReportBookMark::ValidBMs()
{
    static QHash<InsertType, QHash<GFEType, QSet<Param>>> validBMs = {
        {
            Number,
            {
             {Comb,{AxialForce,ShearForce,Bending,ACR}},
             {SDR,{Max}},
             {Model,{Elem, Node}},
          {Test, {Temp}},

            }
        },
        {
            Text,
            {
            },
        },
        {
            Table,
            {
             {Comp,{InterForce}},
             {SoilDyna,{MatParam,SoilInfo}},
             {Property,{PJ}},
             {Comb,{List}},
             {Model,{Attribute}},
             {SDR,{Envelope,E2Y,Displacement}},
             {Vibration,{Case}},
             {Soil,{Layer}},
             {Material,{Layer}},
             },
            },
        {
            CurveChart,
            {
             {Vibration, {E2X,E2Y,Time,EERA,Energy,Spectrum}},
             {SDR,{E2Y,E2X,Envelope,Time,Displacement,UDiff}},
             {Comb,{ACR}},
             {AMP,{SeismWave}},
             {Storey,{ShearForce}},
             {XYData,{Hysteretic}}
            }
        },
        {
            CloudChart,
            {
             {Comb,{ShearForce,AxialForce,Bending,ACR}},
             {Vibration,{E2X,E2Y}},
             {Damage,{Damagec,Level}},
            }
        },
        {
            Picture,
            {
             {Model,{ElemSet, HighLight}},
            }
        }
    };

    return validBMs;
}

const QMap<QString, QStringList>& DlgReportUtil::ReportType()
{
    //! todo：补上每个词条的默认状态 0 不打开 1 根据子词条的情况而定 2 打开
    //! todo：补上每个词条的详细解释，在页面中选中时，右侧空白处显示详细解释

    static QMap<QString, QStringList> reportType = {
        {
            "Types",
            {
                tr("Response displacement"),                            // 反应位移法
                tr("Time history(Elastic)"),           // 时程分析（弹性）
//                tr("Time history analysis"), // 旧版弹性时程分析报告
                tr("Shock absorption"),                                 // 减震报告
          tr("Test"),
            }
        },
        {
            tr("Response displacement"),                                // 反应位移法
            {
             tr("SDR envelope curve"),
             tr("SDR max value"),
             tr("Axial force contour(element)"),
             tr("Axial foce max value"),
             tr("Shear force contour(element)"),
             tr("Shear force max value"),
             tr("Bending moment contour(element)"),
             tr("Bending moment max value"),
             tr("Internal force envelope table(component)"),
             tr("Reinforcement table(component)")
            }
        },
        {
            tr("Time history(Elastic)"),               // 时程分析（弹性）
            {
             tr("Node number"),
             tr("Element number"),
             tr("Soil material table"),
             tr("Structure material table"),

             tr("Structure picture"),
             tr("Structure picture(mesh)"),
             tr("Diaphragm wall picture"),
             tr("Structure location in soil picture"),

             tr("Seismic wave curve"),
             tr("Case combination table"),

             tr("Displacement envelope contour of each case"),
             tr("SDR peak table of each case"),
             tr("SDR envelope curve of each case"),
             tr("Max. nodal displacement difference curve of each case"),

             tr("Bending moment contour(element)"),
             tr("ACR peak value"),
             tr("ACR contour"),
            }
        },
        {
            tr("Time history analysis"), // 旧版弹性时程分析报告
            {
                tr("Soil dynamic parameter information"),
                tr("Soil layer distribution information"),
                tr("Combination list info table"),
                tr("Storey drift ratio curve chart"),
                tr("Time history of ground motion"),
                tr("Model information table"),
                tr("ERA analysis curve"),
                tr("Storey drift ratio envelope table"),
                tr("Axial pressure ratio scatter diagram of wall"),
                tr("Axial pressure ratio scatter diagram of column"),
            }
        },
        {
            tr("Shock absorption"),                                         //减震报告
            {

                tr("Acceleration peak table"),                              // 各工况加速度峰值表格
                tr("Seisimic spectrum curve"),                              // 地震谱曲线
                tr("Seismic wave curve"),                                   // 地震动曲线
                tr("SSF envelope curve"),                                   // 层间剪力包络曲线
                tr("SDR envelope curve"),                                   // 层间位移角包络曲线
                tr("Displacement envelope curve"),                          // 楼层最大位移曲线
                tr("Displacement and SDR peak table"),                      // 位移与层间位移角峰值表格
                tr("Energy stack chart"),                                   // 能量堆叠图
                tr("Wall damage contour"),                                  // 墙损伤云图
                tr("Wall performance contour"),                             // 墙性能评价云图
                tr("Beam damage contour"),                                  // 梁损伤云图
                tr("Beam performance contour"),                             // 梁性能评价云图
                // tr("Slab damage contour"),                                  // 板损伤云图
                // tr("Slab performance contour"),                             // 板性能评价云图
                tr("Col damage contour"),                                   // 柱损伤云图
                tr("Col performance contour"),                              // 柱性能评价云图
//             tr("Hysteretic curve chart"),
            }
        },

        {
         tr("Test"),
         {
             tr("template test"),
         }
        }
    };
    return reportType;
}

const QHash<QString, QString> &DlgReportUtil::Type2BMName()
{
    static QHash<QString,QString> type2BMName = {
        //反应位移法
        {tr("SDR envelope curve"),"CurveChart_SDR_Envelope"},
        {tr("SDR max value"),"Number_SDR_Max"},
        {tr("Axial force contour(element)"),"CloudChart_Comb_AxialForce"},
        {tr("Axial foce max value"),"Number_Comb_AxialForce_Max"},
        {tr("Shear force contour(element)"),"CloudChart_Comb_ShearForce"},
        {tr("Shear force max value"),"Number_Comb_ShearForce_Max"},
        {tr("Bending moment contour(element)"),"CloudChart_Comb_Bending"},
        {tr("Bending moment max value"),"Number_Comb_Bending_Max"},
        {tr("Internal force envelope table(component)"),"Table_Comp_InterForce"},
        {tr("Reinforcement table(component)"),"Table_Property_PJ"},
        //三维弹性时程
        {tr("Element number"), "Number_Model_Elem"},                                             // 单元数
        {tr("Node number"), "Number_Model_Node"},                                               // 节点数
        {tr("Soil material table"),"Table_Soil_Layer"},                                         // 土层材料参数
        {tr("Structure material table"),"Table_Material_Layer"},                                // 结构材料参数
        
        {tr("Structure picture"),"Picture_Model_ElemSet_Jiegou"},                               // 主结构截图
        {tr("Structure picture(mesh)"),"Picture_Model_ElemSet_JiegouMesh"},                     // 主结构网格截图
        {tr("Diaphragm wall picture"),"Picture_Model_ElemSet_DLQ"},                             // 地连墙结构截图
        {tr("Structure location in soil picture"),"Picture_Model_HighLight_Jiegou"},            // 主结构相对位置截图（高亮）
        
        {tr("Seismic wave curve"),"CurveChart_Vibration_E2Y"},                                  // 地震动时程（标签名改一下？）
        {tr("Case combination table"),"Table_Comb_List"},                                       // 工况组合系数表
        
        {tr("Displacement envelope contour of each case"),"CloudChart_Vibration_E2Y_Max"},      // 各工况结构位移包络云图
        {tr("SDR peak table of each case"),"Table_SDR_E2Y_Max"},                                // 各工况最大层间位移角统计表
        {tr("SDR envelope curve of each case"),"CurveChart_SDR_Envelope_E2"},                   // 各工况层间位移角包络曲线
        {tr("Max. nodal displacement difference curve of each case"),"CurveChart_SDR_UDiff"},   // 各工况最大节点层间位移差时程曲线
        
        {tr("Bending moment contour(element)"),"CloudChart_Comb_Bending"},                      // 结构组合弯矩云图（单元）
        {tr("ACR peak value"), "Number_Comb_ACR_Max"},                                          // 墙、柱最大轴压比数值
        {tr("ACR contour"),"CloudChart_Comb_ACR"},                                              // 组合轴压比云图
        //抗震
        {tr("Soil dynamic parameter information"),"Table_SoilDyna_MatParam"},
        {tr("Soil layer distribution information"),"Table_SoilDyna_SoilInfo"},
        {tr("Combination list info table"),"Table_Comb_List"},
        {tr("Storey drift ratio curve chart"),"CurveChart_SDR_Time"},
        {tr("Time history of ground motion"),"CurveChart_Vibration_Time"},
        {tr("Model information table"),"Table_Model_Attribute"},
        {tr("ERA analysis curve"),"CurveChart_Vibration_EERA"},
        {tr("Storey drift ratio envelope table"),"Table_SDR_Envelope"},
        {tr("Axial pressure ratio scatter diagram of wall"),"CurveChart_Comb_ACR_Wall"},
        {tr("Axial pressure ratio scatter diagram of column"),"CurveChart_Comb_ACR_Col"},
        //减隔震
        {tr("Acceleration peak table"),"Table_Vibration_Case"},
        {tr("Seisimic spectrum curve"),"CurveChart_Vibration_Spectrum"},
        {tr("Seismic wave curve"),"CurveChart_AMP_SeismWave"},
        {tr("SSF envelope curve"),"CurveChart_Storey_ShearForce"},
        {tr("SDR envelope curve"),"CurveChart_SDR_Envelope"},
        {tr("Displacement envelope curve"),"CurveChart_SDR_Displacement"},
        {tr("Displacement and SDR peak table"),"Table_SDR_Displacement"},
        {tr("Energy stack chart"),"CurveChart_Vibration_Energy"},
        {tr("Wall damage contour"),"CloudChart_Damage_Damagec_Wall"},
        {tr("Wall performance contour"),"CloudChart_Damage_Level_Wall"},
        {tr("Beam damage contour"),"CloudChart_Damage_Damagec_Beam"},
        {tr("Beam performance contour"),"CloudChart_Damage_Level_Beam"},
        {tr("Col damage contour"),"CloudChart_Damage_Damagec_Col"},
        {tr("Col performance contour"),"CloudChart_Damage_Level_Col"},
        {tr("Hysteretic curve chart"),"CurveChart_XYData_Hysteretic"},

        {tr("template test"), "Number_Test_Temp"},

    };

    return type2BMName;
}

QString DlgReportUtil::GenerateRandomString(int length)
{
    const QString characters = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    QString randomString;

    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(characters.length());
        randomString.append(characters.at(index));
    }

    return randomString;
}

QHash<QString,QHash<QString,QString>> ReportWord::Attributes = {
    {"CloudChart2D",{{"height","333"},{"width","395"}}},
    {"CloudChart3D",{{"height","259"},{"width","406"}}},
    {"ModelPicture3D",{{"height","210"},{"width","351"}}},
    {"CurveChart",{{"height","285"},{"width","292"}}},
    {"CurveChart1",{{"height","271"},{"width","410"}}},
    {"CurveChart2",{{"height","254"},{"width","390"}}},
    {"CurveChartSDR",{{"height","304"},{"width","277"}}},
};

QMap<QString,QVector<double>> ReportVTK::Color = {
    {"Default",{0.27, 0.54, 0.45}},
    {"Red",{1, 0, 0}},
    {"Blue",{0, 1, 0}},
    {"Green",{0, 0, 1}},
    {"Yellow",{1, 1, 0}},
    {"Cyan",{0, 1, 1}},
    {"Magenta",{1 ,0 ,1}},
    {"Black",{0, 0, 0}},
};

DlgReportUtil::DlgReportUtil()
{
}

DlgReportUtil::~DlgReportUtil()
{

}

void DlgReportUtil::FloatToReciprocal(double number, int &numerator, int &denominator)
{
    denominator = static_cast<int>(1.0 / number);
    numerator = 1;
}

std::string DlgReportUtil::FloatToReciprocal(double number){
    int denominator = static_cast<int>(1.0 / number);
    int numerator = 1;
    if(number == 0){
        return "0";
    }
    return std::to_string(numerator) + "/" + std::to_string(denominator);
}

bool DlgReportUtil::IsFloat(const std::string &str)
{
    try {
        size_t pos;
        std::stof(str, &pos);
        return pos == str.size();  // 如果转换后的位置与字符串的长度相等，则说明转换成功
    } catch (const std::exception&) {
        return false;  // 转换失败，说明不是浮点数
    }
}

QString DlgReportUtil::ConnectStrs(const QStringList &strs, const QString& delimiter)
{
    QString res;
    for(int i = 0;i < strs.size();++i){
        if(i > 0){
            res += delimiter;
        }
        res += strs[i];
    }
    return res;
}

bool DlgReportUtil::IsMatch(QString input, QString pattern)
{
    input = QString::fromStdString(GFE::ToLower(input.toStdString()));
    pattern = QString::fromStdString(GFE::ToLower(pattern.toStdString()));
    static QRegularExpression regex(pattern);
    return regex.match(input).hasMatch();
}

int DlgReportUtil::Stoi(const std::string &str)
{
    try {
        return std::stoi(str);
    } catch (const std::invalid_argument& e) {
    } catch (const std::out_of_range& e) {
    }

    return std::numeric_limits<int>::min(); // 返回 INT_MIN
}

bool DlgReportUtil::IsParentChildPaths(const QString &parentPath, const QString &childPath)
{
    QDir childDir(childPath);
    childDir.cdUp();
    return childDir.path() == parentPath;
}

QString DlgReportUtil::GetParentDirectory(const QString &childPath)
{
    QDir childDir(childPath);
    childDir.cdUp();
    return childDir.path();
}


ReportBookMark::ReportBookMark()
{

}

ReportBookMark::ReportBookMark(QString bmName):m_bmName(bmName)
{
    QStringList bms = bmName.split('_');
    m_dbsID =  std::stoi(bms[0].toStdString().substr(3));
    m_insertType = StrToInsertType(bms[1].toStdString());
    m_gfeType = StrToGFEType(bms[2].toStdString());
    m_param = StrToParam(bms[3].toStdString());

    for(int i=4;i<bms.size();++i){
        m_extra.append(bms[i]);
    }
}

ReportBookMark::~ReportBookMark()
{

}

ReportBookMark ReportBookMark::GetBM(QString bmName)
{
    return *this;
}

bool ReportBookMark::isValid(QString bmName)
{
    QStringList bms = bmName.split('_');
    for(auto &b:bms){
        if(b.isEmpty()){
            return false;
        }
    }
    //todo
    if(bms.size() < 4){
        return false;
    }

//    int dbsId = GetDBSID(bmName);

    InsertType insertType = StrToInsertType(bms[1].toStdString());

    GFEType gfeType = StrToGFEType(bms[2].toStdString());

    Param param = StrToParam(bms[3].toStdString());

//    if(bmName == "DBS0_CurveChart_SDR_UDiff"){
//        QD insertType<<gfeType<<param<<bmName<<(bms.size()>4?bms[4]:"");
//    }

//    if(dbsId!=dbsID || ReportBookMark::ValidBMs()[insertType][gfeType].find(param)==ReportBookMark::ValidBMs()[insertType][gfeType].end())return false;
    if(ReportBookMark::ValidBMs()[insertType][gfeType].find(param)==ReportBookMark::ValidBMs()[insertType][gfeType].end())return false;
    return true;
}




int ReportBookMark::GetDBSID(QString bmName)
{
    auto bms = bmName.split('_');
    bool ok = false;

    int dbsID = bms[0].mid(3).toInt(&ok);
    if(!ok)return -1;
    return dbsID;
}

ReportWord::ReportWord(QString wordPath,QString outPath):m_outPath(outPath)
{
    HRESULT r = OleInitialize(0);
    //todo
    if (r != S_OK && r != S_FALSE) {
        qDebug()<<QObject::tr("Error::Failed to initialize COM");
        return;
    }
    // 打开word并准备文档
    auto word = new QAxObject("kwps.Application");
    if(!word)
        word = new QAxObject("Word.Application");
    if(!word)
    {
        qDebug()<<QObject::tr("Error::Generate Report needs Microsoft Office Word or WPS Word!");
        return;
    }
    word->setProperty("Visible", false);
    auto documents = word->querySubObject("Documents");
    documents->dynamicCall("Add(QString)", wordPath);
    auto document = word->querySubObject("ActiveDocument");
    m_word.reset(word);
    m_documents.reset(documents);
    m_document.reset(document);
}

ReportWord::~ReportWord()
{

}

QString ReportWord::Save()
{
    m_document->dynamicCall("SaveAs(const QString&)", m_outPath);
    m_document->dynamicCall("Close(boolean)", false);
    m_documents->dynamicCall("Close()");
    m_word->dynamicCall("Quit(bool)", false);

    m_document.reset();
    m_document.reset();
    m_word.reset();
    OleUninitialize();
    return "";
}

QAxObject* ReportWord::GetBM(QString bmName)
{
    QString bmTag = "Bookmarks(" + bmName + ")";
    auto bm = m_document->querySubObject(bmTag.toStdString().c_str());
    return bm;
}

QString ReportWord::WriteText(const QString &bmName, const QString &text,const QString& type)
{
    auto bm = GetBM(bmName);
    if(!bm){
        qDebug() << "未找到书签" << bmName;
    }
    bm->dynamicCall("Select(void)");
    auto range = bm->querySubObject("Range");

    if(!range)return "";
    auto font = range->querySubObject("Font");
    if(font){
        font->setProperty("Name", u8"宋体");  // 设置字体为宋体
        font->setProperty("Size", 12);  // 设置字号为小四
    }
    range->setProperty("Text", text);

    if(type == "title"){
        // 设置文字居中
        QAxObject* paragraphFormat = range->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
    }
    if(type == "error"){
        // 设置文字居中
        QAxObject* paragraphFormat = range->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
        if(font){
            font->setProperty("ColorIndex", "wdRed");
        }

    }

    if(font){
        DELETE_PTR(font);
    }
    DELETE_PTR(range)
    DELETE_PTR(bm)
    return "";
}

QString ReportWord::AddTable(QString bmName, const QStringList &header, const QVector<QStringList> &data)
{
    auto bm = GetBM(bmName);
    QAxObject* range = bm->querySubObject("Range");

    QAxObject* tables = range->querySubObject("Tables");
    tables->dynamicCall("Add(QVariant, QVariant, QVariant, QVariant)", range->asVariant(), 1, header.size(), 1, 1);

    bm->dynamicCall("Select(void)");
    auto table = bm->querySubObject("Range")->querySubObject("Tables")->querySubObject("Item(int)", 1);

    auto rows = table->querySubObject("Rows");
    QAxObject* firstRow = rows->querySubObject("Item(int)", 1); // First row
    auto cells = firstRow->querySubObject("Cells");
    for(int i = 0; i < header.size(); ++i){
        auto cell = cells->querySubObject("Item(int)", i + 1);
        auto cellRange = cell->querySubObject("Range");
        cellRange->setProperty("Text", header[i]);
        QAxObject* font = cellRange->querySubObject("Font");
        if(font){
            font->setProperty("Name", u8"宋体");
            font->setProperty("Size", 12);
        }
        QAxObject* paragraphFormat = cellRange->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
    }

    for (int row = 0; row < data.size(); ++row) {
        auto info = data[row];
        TableAddRow(table,info);
    }

//    table->dynamicCall("AutoFitBehavior(QVariant)",1);//根据单元格内容进行调整

    cells->setProperty("VerticalAlignment", 1); // 垂直居中

    DELETE_PTR(cells);
    DELETE_PTR(rows);

    return "";
}

QString ReportWord::AddTable(QString bmName, const QVector<QStringList> &data)
{
    auto bm = GetBM(bmName);
    QAxObject* range = bm->querySubObject("Range");

    QAxObject* tables = range->querySubObject("Tables");
    tables->dynamicCall("Add(QVariant, QVariant, QVariant, QVariant)", range->asVariant(), 1, data[0].size(), 1, 1);

    bm->dynamicCall("Select(void)");
    auto table = bm->querySubObject("Range")->querySubObject("Tables")->querySubObject("Item(int)", 1);

    auto rows = table->querySubObject("Rows");
    QAxObject* firstRow = rows->querySubObject("Item(int)", 1); // First row
    auto cells = firstRow->querySubObject("Cells");
    for(int i = 0; i < data[0].size(); ++i){
        auto cell = cells->querySubObject("Item(int)", i + 1);
        auto cellRange = cell->querySubObject("Range");
        cellRange->setProperty("Text", data[0][i]);
        QAxObject* font = cellRange->querySubObject("Font");
        if(font){
            font->setProperty("Name", u8"宋体");
            font->setProperty("Size", 12);
        }
        QAxObject* paragraphFormat = cellRange->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
    }

    for (int row = 1; row < data.size(); ++row) {
        auto info = data[row];
        TableAddRow(table,info);
    }

    cells->setProperty("VerticalAlignment", 1); // 垂直居中
//    table->dynamicCall("AutoFitBehavior(QVariant)",1);

    DELETE_PTR(cells);
    DELETE_PTR(rows);
    return "";
}

QString ReportWord::FindReplace(QString findText, QString replaceText)
{
    QAxObject *selection = m_word->querySubObject("Selection");
    QAxObject *find = selection->querySubObject("Find");
    if(!find)return QObject::tr("Error: Can not find the text");

    find->setProperty("Font.Size", 12);  // 设置字号为小四
    find->setProperty("Font.Name", u8"宋体");  // 设置字体为宋体

    QList<QVariant> list;                                               //*****Find Word http://technet.microsoft.com/zh-cn/library/ff193977
    list.append(QString::fromUtf8(findText.toStdString().c_str()));                                         //find text
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QString::fromUtf8(replaceText.toStdString().c_str()));                     //replace text
    list.append(2);
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());
    list.append(QVariant());

    find->dynamicCall("Execute (QVariant&, QVariant&, QVariant&, QVariant&, QVariant&, QVariant&, QVariant&, QVariant&, QVariant& ,QVariant& ,QVariant& , QVariant&, QVariant&, QVariant&, QVariant&)", list);

    return "";
}

QAxObject* ReportWord::GetTable(QString bmName)
{
    auto bm = GetBM(bmName);
    bm->dynamicCall("Select(void)");
    auto table = bm->querySubObject("Range")->querySubObject("Tables")->querySubObject("Item(int)", 1);
    return table;
}


QString ReportWord::TableAddRow(QAxObject *table, QStringList info)
{
    auto rows = table->querySubObject("Rows");
    auto row = rows->querySubObject("Add(void)");
    auto cells = row->querySubObject("Cells");
    int size = info.size();
    for(int i = 0; i < size; ++i){
        auto cell = cells->querySubObject("Item(int)", i + 1);
        auto cellRange = cell->querySubObject("Range");
        cellRange->setProperty("Text", info[i]);
        auto paragraphFormat = cellRange->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐

        DELETE_PTR(paragraphFormat);
        DELETE_PTR(cellRange);
        DELETE_PTR(cell);
    }

    DELETE_PTR(cells)
    DELETE_PTR(row)
    DELETE_PTR(rows)
    return "";
}

QString ReportWord::TableAddCol(QAxObject *table, QStringList info)
{
    auto cols = table->querySubObject("Columns");
    auto col = cols->querySubObject("Add(void)");
    auto cells = col->querySubObject("Cells");
    int size = info.size();
    for(int i = 0; i < size; ++i)
        cells->querySubObject("Item(int)", i + 1)->querySubObject("Range")->setProperty("Text", info[i]);
    DELETE_PTR(cells)
    DELETE_PTR(col)
    DELETE_PTR(cols)
    return "";
}

QString ReportWord::TableMergeCol(QAxObject *table, int colNum, int rowStart, int rowEnd)
{
    QAxObject* rows = table->querySubObject("Rows");
    int rowCount = rows->property("Count").toInt();
    int start = 2,end = rowCount;
    if(rowStart != INT_MAX){
        start = rowStart;
    }
    if(rowEnd != INT_MIN){
        end = rowEnd;
    }

    if(rowStart < 1||rowEnd > rowCount){
        return QObject::tr("Error: The specified row number exceeds the row range of the table");
    }

    for(int i = start;i <= end; ++i){
        auto cell1 = table->querySubObject("Cell(int, int)", start, colNum);
        auto cell2 = table->querySubObject("Cell(int, int)", i, colNum);
        cell1->dynamicCall("Merge(QVariant)", cell2->asVariant());

        DELETE_PTR(cell1);
        DELETE_PTR(cell2);
    }
    DELETE_PTR(rows);
    return "";
}

QString ReportWord::TableMergeRow(QAxObject *table, int rowNum)
{
    QAxObject* cols = table->querySubObject("Columns");
    int colCount = cols->property("Count").toInt();
    for(int i = 2;i <= colCount; ++i){
        auto cell1 = table->querySubObject("Cell(int, int)", rowNum, 1);
        auto cell2 = table->querySubObject("Cell(int, int)", rowNum, i);
        cell1->dynamicCall("Merge(QVariant)", cell2->asVariant());

        DELETE_PTR(cell1);
        DELETE_PTR(cell2);
    }

    DELETE_PTR(cols);
    return "";
}

QString ReportWord::TableWriteCell(QAxObject *table, int row, int col, const QString &text)
{
    auto cell = table->querySubObject("Cell(int, int)", row, col);
    if(!cell){
        return QObject::tr("Error: This cell was not found in the table");
    }
    auto cellRange = cell->querySubObject("Range");
    cellRange->setProperty("Text", text);
    auto paragraphFormat = cellRange->querySubObject("ParagraphFormat");
    paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
    cell->setProperty("VerticalAlignment", 1); // 垂直居中

    DELETE_PTR(paragraphFormat);
    DELETE_PTR(cellRange);
    DELETE_PTR(cell);

    return "";
}

QString ReportWord::AddChart(QString bmName, QVector<QPair<QVector<double>, QVector<double> > > coordDatas, const QString &xName, const QString &yName,
                             int type, const QString& xType, const QString& yType)
{
    auto bm = GetBM(bmName);
    if(!bm)return QObject::tr("Error:Can not find the bookmark when adding chart");
    auto range = bm->querySubObject("Range");

    auto excel = new QAxObject("Excel.Application");
    if(!excel)
        excel = new QAxObject("ket.Application");
    if(!excel)
        return QObject::tr("Generate Report needs Microsoft Office Excel or WPS Excel!");

    excel->setProperty("Visible", false);
    auto books = excel->querySubObject("WorkBooks");
    auto book = books->querySubObject("Add");
    auto sheet = excel->querySubObject("ActiveSheet");

    auto eChartObjs = sheet->querySubObject("ChartObjects(void)");
    auto eChartObj = eChartObjs->querySubObject("Add(double, double, double, double)", 0, 0, 400, 250);
    auto eChart = eChartObj->querySubObject("Chart");
    eChart->setProperty("ChartType", type);
    eChart->setProperty("HasTitle", false);
    eChart->setProperty("HasLegend", false);
    //    eChart->setProperty("Legend.Position", 2);

    auto eSc = eChart->querySubObject("SeriesCollection");
    auto n = eSc->property("Count").toInt();
    for(int i = n; i >= 1; --i)
    {
        eSc->querySubObject("Item(int)", i)->dynamicCall("Delete(void)");
    }
    double min = FLT_MAX;

    for(int i=0;i<coordDatas.size();++i){
        auto coordData = coordDatas[i];
        QAxObject *es = eSc->querySubObject("NewSeries(void)");
        es->setProperty("Name", "Series");
        auto xData = coordData.first;
        auto yData = coordData.second;
        QVariantList xValue, yValue;
        int size = std::min(xData.size(), yData.size());
        for(int i = 0; i < size; ++i)
        {
            xValue << QVariant(xData[i]);
            yValue << QVariant(yData[i]);
            if(yData[i] < min)
                min = yData[i];
        }

        es->querySubObject("Format")->querySubObject("Line")->setProperty("Weight", 1.0);
        es->setProperty("XValues", QVariant(xValue));
        es->setProperty("Values", QVariant(yValue));

//        QAxObject *line = es->querySubObject("Format")->querySubObject("Line");
//        line->setProperty("ForeColor.RGB", QColor(Qt::red).rgb());

        DELETE_PTR(es)
    }

    //    eChart->setProperty("Legend.Name", "My Legend"); // 设置图例名称，你可以根据需要修改名称和位置等属性
    //    eChart->setProperty("Legend.Text", "Series 1, Series 2, Series 3");

    auto xAxis = eChart->querySubObject("Axes(int)", 1);
    auto yAxis = eChart->querySubObject("Axes(int)", 2);
    if(type == 75)
    {
        yAxis->setProperty("MinimumScaleIsAuto", false);
        yAxis->setProperty("MinimumScale", min);
        yAxis->setProperty("CrossesAt", min);
    }
    else
    {
        yAxis->setProperty("MajorUnitIsAuto", false);
        yAxis->setProperty("MajorUnit", 1);
    }

    xAxis->setProperty("HasTitle", true);
    yAxis->setProperty("HasTitle", true);
    xAxis->querySubObject("AxisTitle")->setProperty("Caption", xName);
    yAxis->querySubObject("AxisTitle")->setProperty("Caption", yName);
    if(type == 75)
    {
        xAxis->setProperty("HasMajorGridlines", true);
        yAxis->setProperty("HasMajorGridlines", true);
    }
    else
    {
        xAxis->setProperty("HasMajorGridlines", false);
        yAxis->setProperty("HasMajorGridlines", false);
    }

    QTemporaryDir tDir;
    eChart->dynamicCall("Export(const QString&)", tDir.path() + "/tmp.png");
    auto iS = range->querySubObject("InlineShapes");
    iS->dynamicCall("AddPicture(const QString&)", tDir.path() + "/tmp.png");

    book->dynamicCall("Close(boolean)", false);
    excel->dynamicCall("Quit()");

    DELETE_PTR(iS)

    DELETE_PTR(eChart)
    DELETE_PTR(eChartObj)
    DELETE_PTR(eChartObjs)
    DELETE_PTR(sheet)
    DELETE_PTR(book)
    DELETE_PTR(books)
    DELETE_PTR(excel)
    DELETE_PTR(range);
    DELETE_PTR(bm);


    return "";
}

QString ReportWord::AddChart(QString bmName, std::vector<std::pair<std::vector<double>,std::vector<double>>> coordDatas, const QString &xName, const QString &yName,
                             int type, const QString& xType, const QString& yType)
{
    auto bm = GetBM(bmName);
    if(!bm)return QObject::tr("Error:Can not find the bookmark when adding chart");
    auto range = bm->querySubObject("Range");

    auto excel = new QAxObject("Excel.Application");
    if(!excel)
        excel = new QAxObject("ket.Application");
    if(!excel)
        return QObject::tr("Generate Report needs Microsoft Office Excel or WPS Excel!");

    excel->setProperty("Visible", false);
    auto books = excel->querySubObject("WorkBooks");
    auto book = books->querySubObject("Add");
    auto sheet = excel->querySubObject("ActiveSheet");

    auto eChartObjs = sheet->querySubObject("ChartObjects(void)");
    auto eChartObj = eChartObjs->querySubObject("Add(double, double, double, double)", 0, 0, 400, 250);
    auto eChart = eChartObj->querySubObject("Chart");
    eChart->setProperty("ChartType", type);
    eChart->setProperty("HasTitle", false);
    eChart->setProperty("HasLegend", false);
    //    eChart->setProperty("Legend.Position", 2);

    auto eSc = eChart->querySubObject("SeriesCollection");
    auto n = eSc->property("Count").toInt();
    for(int i = n; i >= 1; --i)
    {
        eSc->querySubObject("Item(int)", i)->dynamicCall("Delete(void)");
    }
    double min = FLT_MAX;

    for(int i=0;i<coordDatas.size();++i){
        auto coordData = coordDatas[i];
        QAxObject *es = eSc->querySubObject("NewSeries(void)");
        es->setProperty("Name", "Series");
        auto xData = coordData.first;
        auto yData = coordData.second;
        QVariantList xValue, yValue;
        int size = std::min(xData.size(), yData.size());
        for(int i = 0; i < size; ++i)
        {
            xValue << QVariant(xData[i]);
            yValue << QVariant(yData[i]);
            if(yData[i] < min)
                min = yData[i];
        }

        es->querySubObject("Format")->querySubObject("Line")->setProperty("Weight", 1.0);
        es->setProperty("XValues", QVariant(xValue));
        es->setProperty("Values", QVariant(yValue));

        //        QAxObject *line = es->querySubObject("Format")->querySubObject("Line");
        //        line->setProperty("ForeColor.RGB", QColor(Qt::red).rgb());

        DELETE_PTR(es)
    }

    //    eChart->setProperty("Legend.Name", "My Legend"); // 设置图例名称，你可以根据需要修改名称和位置等属性
    //    eChart->setProperty("Legend.Text", "Series 1, Series 2, Series 3");

    auto xAxis = eChart->querySubObject("Axes(int)", 1);
    auto yAxis = eChart->querySubObject("Axes(int)", 2);
    if(type == 75)
    {
        yAxis->setProperty("MinimumScaleIsAuto", false);
        yAxis->setProperty("MinimumScale", min);
        yAxis->setProperty("CrossesAt", min);
    }
    else
    {
        yAxis->setProperty("MajorUnitIsAuto", false);
        yAxis->setProperty("MajorUnit", 1);
    }

    xAxis->setProperty("HasTitle", true);
    yAxis->setProperty("HasTitle", true);
    xAxis->querySubObject("AxisTitle")->setProperty("Caption", xName);
    yAxis->querySubObject("AxisTitle")->setProperty("Caption", yName);
    if(type == 75)
    {
        xAxis->setProperty("HasMajorGridlines", true);
        yAxis->setProperty("HasMajorGridlines", true);
    }
    else
    {
        xAxis->setProperty("HasMajorGridlines", false);
        yAxis->setProperty("HasMajorGridlines", false);
    }

    QTemporaryDir tDir;
    eChart->dynamicCall("Export(const QString&)", tDir.path() + "/tmp.png");
    auto iS = range->querySubObject("InlineShapes");
    iS->dynamicCall("AddPicture(const QString&)", tDir.path() + "/tmp.png");

    book->dynamicCall("Close(boolean)", false);
    excel->dynamicCall("Quit()");

    DELETE_PTR(iS)

    DELETE_PTR(eChart)
    DELETE_PTR(eChartObj)
    DELETE_PTR(eChartObjs)
    DELETE_PTR(sheet)
    DELETE_PTR(book)
    DELETE_PTR(books)
    DELETE_PTR(excel)
    DELETE_PTR(range);
    DELETE_PTR(bm);

    return "";
}


QString ReportWord::AddPicture(QString bmName, const QString &pngPath, int width, int height)
{
    auto bm = GetBM(bmName);
    if(!bm)return QObject::tr("Error:Can not find the bookmark when adding picture");
    auto range = bm->querySubObject("Range");
    QAxObject* inlineShapes = range->querySubObject("InlineShapes");

    // 插入图片
    QAxObject *picture = inlineShapes->querySubObject("AddPicture(const QString&)", pngPath);

//    picture->setProperty("Range", range->asVariant());
//    picture->setProperty("Align", 1); // 居中对齐

    // 设置图片大小
    picture->dynamicCall("Select()");
    picture->dynamicCall("SetHeight(int)", height);  // 设置高度
    picture->dynamicCall("SetWidth(int)", width);   // 设置宽度

    DELETE_PTR(range);
    DELETE_PTR(bm);

    return "";
}

QString ReportWord::AddNewBM(const QString& oldBmName,const QString& newBmName,const QString& text,const QString& textType)
{
    QAxObject* bookmarks = m_document->querySubObject("Bookmarks");
    QString bookmarkName = oldBmName;  // 要添加回车和新书签的书签的名称
    QAxObject* bookmark = bookmarks->querySubObject("Item(const QVariant&)", bookmarkName);
    QAxObject* bookmarkRange = bookmark->querySubObject("Range");

//    QAxObject* paragraphs = bookmarkRange->querySubObject("Paragraphs");
//    QAxObject* lastParagraph = paragraphs->querySubObject("Last");
//    QAxObject* paragraphRange = lastParagraph->querySubObject("Range");
//    bookmarkRange->dynamicCall("SetRange(int, int)", paragraphRange->property("End").toInt(), paragraphRange->property("End").toInt());
    bookmarkRange->dynamicCall("InsertParagraphAfter()");// 在书签处添加回车
    QAxObject* newParagraph = m_document->querySubObject("Content")
                                  ->querySubObject("Paragraphs")
                                  ->querySubObject("Add(QVariant)", bookmarkRange->asVariant());
    QAxObject* newParagraphRange = newParagraph->querySubObject("Range");
    newParagraphRange->dynamicCall("SetText(QString)", text); //必须先添加文本在添加书签，书签才不会消失
    QAxObject* font = newParagraphRange->querySubObject("Font");
    if(font){
        font->setProperty("Name", u8"宋体");
        font->setProperty("Size", 12);
    }

    if(textType == "title"){
        QAxObject* paragraphFormat = newParagraphRange->querySubObject("ParagraphFormat");
        paragraphFormat->setProperty("Alignment", 1); // 1 表示居中对齐
    }
    QString newBookmarkName = newBmName;  // 新书签的名称
    QAxObject* newBookmark = bookmarks->querySubObject("Add(const QString&, const QVariant&)", newBookmarkName,
                                                       newParagraphRange->asVariant());

    return "";
}


GFECaculate::GFECaculate()
{

}

GFECaculate::~GFECaculate()
{

}

QString GFECaculate::GFECombPJCalculate(CombStruct& cs)
{
    if(m_opt.NoComb)return "";
    std::vector<Str2> paths;
    std::vector<std::pair<std::vector<double>, int>> combFac;
    int size = std::min(cs.dbPathList.size(), cs.gjdyList.size());
    if(size <= 0){
        return QObject::tr("Error: No gjdy file!");
    }

    for(int i = 0; i < size; ++i)
    {
        Str2 tmp(cs.dbPathList[i].toStdString(), cs.gjdyList[i].toStdString());
        paths.push_back(tmp);
    }

    for(auto &comb : cs.combList)
    {
        std::vector<double> fac;
        int type = comb.last().toInt();
        // 只算基本组合
        if(type != 0)
            continue;
        comb.pop_back();
        for(const auto& c : comb)
            fac.push_back(c.toDouble());
        std::pair<std::vector<double>, int> tmp(fac, type);
        combFac.push_back(tmp);
    }
    auto vAcr = QVector<int>::fromList(cs.acrLayer).toStdVector();
    m_pjResult = GFECaculate::CalcPJ(PJInput{paths, combFac, vAcr, cs.isSimplified,
                                             cs.caseTypeArr, m_vSlabProp, m_vSlabProp});

    auto eleForce = m_pjResult.eleForce;

    for(auto& it1:eleForce){
        auto interForce = it1.first.first;
        auto comb = QString::fromStdString(it1.first.second);
        for(auto& it2:it1.second){
            auto num = abs(it2.second);
            if(interForce == "SF1" && num > m_maxSF1){
                m_maxSF1 = num;
                m_maxSF1Comb = comb;
            }else if(interForce == "SF2" && num > m_maxSF2){
                m_maxSF2 = num;
                m_maxSF2Comb = comb;
            }else if(interForce == "SM1" && num > m_maxSM1){
                m_maxSM1 = num;
                m_maxSM1Comb = comb;
            }
        }
    }
    return "";
}

GFECaculate::PJResult GFECaculate::CalcPJ(const PJInput& input)
{
    using namespace YJK_PJ;

    auto& files = input.files;
    auto& combFactor = input.combFactor;
    auto acrStorey = input.acrStorey;
    auto& isSimplified = input.isSimplified;
    auto& caseType = input.caseType;
    auto& slabProp = input.slabProp;
    auto& wallProp = input.wallProp;

    if(files.empty())
        return PJResult();

    CMemberPJ calcObj;

    // 在第一个db的上级目录创建PJ文件夹，存放配筋txt文件
    {
        auto dbPath = files[0].first;
        auto dbDir = GFE::FileInfo::Dir(dbPath);
        if(GFE::FileInfo::CreateDir(CPStr(dbDir+"\\..\\PJ"), false)) {
            calcObj.m_sWorkPath = dbDir+"\\..\\PJ\\";
        }
    }

    // 设置计算参数
    calcObj.m_vSlabProperty = slabProp;
    calcObj.m_vWallProperty = wallProp;
    int nCase = (int)files.size();
    calcObj.m_iLoadCaseCnt = nCase;
    calcObj.m_pLoadCase = new CMemberPJ::PJ_LOADCASE[nCase];
    for(int i = 0; i < nCase; i++) {
        calcObj.m_pLoadCase[i].sDbPath = files[i].first;
        calcObj.m_pLoadCase[i].sGJDYPath = files[i].second;
        if(caseType[i] < 2)
            calcObj.m_pLoadCase[i].bStaic = true;
    }
    int nComb = (int)combFactor.size();
    calcObj.m_iLoadCombCnt = nComb;
    calcObj.m_pLoadComb = new CMemberPJ::PJ_LOADCOMB[nComb];
    for(int i = 0; i < nComb; i++) {
        calcObj.m_pLoadComb[i].pCombCoeff = new double[nCase];
        calcObj.m_pLoadComb[i].iType = combFactor[i].second;
        for(int j = 0; j < nCase; j++)
            calcObj.m_pLoadComb[i].pCombCoeff[j] = combFactor[i].first[j];
    }

    int pProp[16] = {0};
    pProp[0] = isSimplified;

    // 测试代码
//    calcObj.m_iLoadCombCnt = 1;
//    calcObj.m_pLoadComb = new CMemberPJ::PJ_LOADCOMB[1];
//    calcObj.m_pLoadComb[0].pCombCoeff = new double[1];
//    calcObj.m_pLoadComb[0].pCombCoeff[0] = 1;
//    calcObj.m_pLoadComb[0].iType = YJK_PJ::CMemberPJ::PJ_LOADCOMB_FUNDAMENTAL;

    calcObj.Perform(acrStorey, pProp);


    // 提取配筋结果
    PJResult ret;
    auto& pjData = ret.gjPj;
    for(int i=0;i<calcObj.m_iRectBeamCnt;++i){
        QStringList data;
        QString dataStr;
        PJ_RectBeam *pBeam = &calcObj.m_pRectBeam[i];
        if(pBeam->iOutType&3){
            double *pCur = pBeam->pDstF + pBeam->iCompCnt * 2;
            double fAs = calcObj.ToBendPJ(pBeam->B,pBeam->H,pBeam->fc,pBeam->fy,pCur[3]);
            dataStr.sprintf("%d_%.0f_%.0f_%.0f_%.2f",pBeam->ID,pBeam->H * 1000,abs(pCur[3]),fAs,100 * fAs/(pBeam->B * pBeam->H *1000000));
        }
        if(dataStr.isEmpty())continue;
        data = dataStr.split('_');
        pjData.append(data);
    }
    for(int i=0;i<calcObj.m_iRectColCnt;++i){
        QStringList data;
        QString dataStr;
        PJ_RectCol *pCol = &calcObj.m_pRectCol[i];
        if(pCol->iOutType&3){
            double *pCur = pCol->pDstF + pCol->iCompCnt * 2;
            double fAs = calcObj.ToBendPJ(pCol->B,pCol->H,pCol->fc,pCol->fy,pCur[3]);
            dataStr.sprintf("%d_%.0f_%.0f_%.0f_%.2f",pCol->ID,pCol->H * 1000,abs(pCur[3]),fAs,100 * fAs/(pCol->B * pCol->H *1000000));
        }
        if(dataStr.isEmpty())continue;
        data = dataStr.split('_');
        pjData.append(data);
    }


    // 提取轴压比结果
    auto& ret1 = ret.acr;
    ret1.reserve(nComb);
    auto workDir = GFE::FileInfo::Dir(CPStr(files[0].second));
    GFE::FileInfo::CreateDir(workDir+L"/轴压比", true);
    std::ofstream ofs1(GFE::FileInfo::Dir(CPStr(files[0].second))+L"/轴压比/超限构件.txt");
    for(int j = 0; j < nComb; j++)
    {
        AcrResult resCol, resWall;
        for(int i = 0; i < calcObj.m_iRectColCnt; i++) {
            auto& one = calcObj.m_pRectCol[i];
            resCol[{one.iStyId, one.ID}] = one.pAxisF[j];
        }
        for(int i = 0; i < calcObj.m_iCirColCnt; i++) {
            auto& one = calcObj.m_pCirCol[i];
            resCol[{one.iStyId, one.ID}] = one.pAxisF[j];
        }
        for(int i = 0; i < calcObj.m_iSteelColCnt; i++) {
            auto& one = calcObj.m_pSteelCol[i];
            resCol[{one.iStyId, one.ID}] = one.pAxisF[j];
        }
        for(int i = 0; i < calcObj.m_iSrcRectColCnt; i++) {
            auto& one = calcObj.m_pSrcRectCol[i];
            resCol[{one.iStyId, one.ID}] = one.pAxisF[j];
        }
        for(int i = 0; i < calcObj.m_iSrcCirColCnt; i++) {
            auto& one = calcObj.m_pSrcCirCol[i];
            resCol[{one.iStyId, one.ID}] = one.pAxisF[j];
        }
        for(int i = 0; i < calcObj.m_iPjWallColCnt; i++) {
            auto& one = calcObj.m_pPjWallCol[i];
            for(int k = 0; k < one.iSubWallCnt; k++)
                resWall[{one.iStyId, one.pSubWallId[k]}] = one.pAxisF[j];
            if(one.iSubWallCnt == 0)
                resWall[{one.iStyId, one.ID}] = one.pAxisF[j];
        }

        // 输出到txt文件
        {
            auto filename = L"柱轴压比_"+std::to_wstring(j+1)+L".txt";
            std::ofstream ofs(GFE::FileInfo::Dir(CPStr(files[0].second))+L"/轴压比/"+filename);
            ofs << u8"*** 组合号： " << j+1 << "\n";
            ofs << u8"### 层号, 构件号, 柱轴压比\n";
            for(const auto& [pair, val] : resCol) {
                auto& [iStyId, ID] = pair;
                ofs << iStyId << ", " << ID << ", " << val << "\n";
                if(val >= 0.75) {
                    ofs1 << u8"组合" << j+1 << u8", 层" << iStyId << u8", 柱," << ID << u8", 轴压比: " << val << "\n";
                }
            }
            ofs.close();
        }
        {
            auto filename = L"墙轴压比_"+std::to_wstring(j+1)+L".txt";
            std::ofstream ofs(GFE::FileInfo::Dir(CPStr(files[0].second))+L"/轴压比/"+filename);
            ofs << u8"*** 组合号： " << j+1 << "\n";
            ofs << u8"### 层号, 构件号, 墙轴压比\n";
            for(const auto& [pair, val] : resWall) {
                auto& [iStyId, ID] = pair;
                ofs << iStyId << ", " << ID << ", " << val << "\n";
                if(val >= 0.75) {
                    ofs1 << u8"组合" << j+1 << u8", 层" << iStyId << u8", 墙," << ID << u8", 轴压比: " << val << "\n";
                }
            }
            ofs.close();
        }

        ret1.push_back({std::move(resCol), std::move(resWall)});

        // 提取单元组合后内力
        auto& ret2 = ret.eleForce;
        static const std::vector<std::string> force_components = {"SF1", "SF2", "SF3", "SM1", "SM2", "SM3", "SF4", "SF5"};
        constexpr auto fcnt_beam = 6;
        constexpr auto fcnt_shell = 8;
        std::map<std::string, std::pair<std::string, std::string>> split_str;
        for (auto& fc : force_components) {
            for (auto& minmax : {/*"Max", "Min", */"Abs"}) {
                split_str[fc + "_" + minmax] = {fc, minmax};
            }
        }

        auto tmp = "Comb" + std::to_string(j + 1)/* + "."*/;
        auto& pBeam = calcObj.m_mdl->m_pBeamEle;
        auto beams = calcObj.m_mdl->m_iBeamEle;
        auto& pShell = calcObj.m_mdl->m_pShellEle;
        auto shells = calcObj.m_mdl->m_iShellEle;

        for (int i = 0; i < beams; ++i) {
            auto& ele = pBeam[i];
            for (int k = 0; k < ele.iTargetCnt; ++k) {
                if (ele.pTargetComb[k] == j) {
//                    if(split_str.find(ele.pTargetName[k]) == split_str.end())
//                        continue;
                    if (ele.pTargetName[k] == "Static")
                    {

                        int index = 0;
                        for (auto& comp : force_components) {
                            if (index >= fcnt_beam)
                                break;
                            ret2[{comp, tmp/*+minmax*/}].emplace_back(ele.ID, ele.pTargetForce[k * fcnt_beam + index]);
                            ++index;
                        }
                    }
                    else
                    {
                        auto& [comp, minmax] = split_str[ele.pTargetName[k]];
                        int index = std::find(force_components.begin(), force_components.end(), comp) - force_components.begin();
                        //力类型-组合-数组[单元ID-力的数值]
                        ret2[{comp, tmp/*+minmax*/}].emplace_back(ele.ID, ele.pTargetForce[k * fcnt_beam + index]);
                    }
                }
            }
        }
        for (int i = 0; i < shells; ++i) {
            auto& ele = pShell[i];
            for (int k = 0; k < ele.iTargetCnt; ++k) {
                if (ele.pTargetComb[k] == j) {
//                    if(split_str.find(ele.pTargetName[k]) == split_str.end())
//                        continue;
                    if (ele.pTargetName[k] == "Static")
                    {
                        int index = 0;
                        for (auto& comp : force_components) {
                            ret2[{comp, tmp/*+minmax*/}].emplace_back(ele.ID, ele.pTargetForce[k * fcnt_shell + index]);
                            ++index;
                        }
                    }
                    else
                    {
                        auto& [comp, minmax] = split_str[ele.pTargetName[k]];
                        int index = std::find(force_components.begin(), force_components.end(), comp) - force_components.begin();
                        ret2[{comp, tmp/*+minmax*/}].emplace_back(ele.ID, ele.pTargetForce[k * fcnt_shell + index]);
                    }

                }
            }
        }

    }
    return ret;
}

void GFECaculate::GetSDRInfo(QStringList dbPathList)
{
    if(m_opt.NoSDR)return;
    for(const auto& dbPath : dbPathList)
    {
        auto db = GFE::open(dbPath.toStdString(), false);
        m_dim = DlgReportUtil::Stoi(GFE::getParameter(db, "Model Dim"));//???
        if(m_dim < 0)continue;
        QFileInfo fi(dbPath);
        if(m_dim == 2)
        {
            QFile f(fi.absolutePath() + u8"/XYData/层间位移角包络.txt");
            if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            f.readLine();
            while(!f.atEnd())
            {
                f.readLine();
                m_layer++;
            }
            f.close();
            break;
        }
        else if(m_dim == 3)
        {
            for(const QString& dir : {"X", "Y", "Z"})
            {
                QFile f(fi.absolutePath() + "/XYData/" + dir + u8"向层间位移角包络.txt");

                if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;
                f.readLine();
                bool isFirst = (m_sdrDir1 == "");
                if(isFirst)
                    m_sdrDir1 = dir;
                else
                    m_sdrDir2 = dir;
                if(m_layer == 0)
                {
                    while(!f.atEnd())
                    {
                        f.readLine();
                        m_layer++;
                    }
                }
                f.close();
            }
            if(m_layer != 0)
                break;
        }

    }

    //层间位移角限值
    if(m_layer <= 2)
        m_denoLimit = 550;
    else
        m_denoLimit = 1000;
}

void GFECaculate::GetSDREnvelope(QString sdrPath, int& layerMax, double& sdrMax, QVector<int>&& layer, QVector<double>&& sdr)
{
    QFile f(sdrPath);
    if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }
    while(!f.atEnd())
    {
        QString line = f.readLine();
        auto sp = line.split(",");
        if(sp.size() < 2)
            continue;
        double x = sp[0].trimmed().toDouble(),
            y = sp[1].trimmed().toInt();
        sdr.push_back(x);
        layer.push_back(y);

        if(x > sdrMax){
            sdrMax = x;
            layerMax = y;
        }
    }
}

int GFECaculate::GetEnvelopeFrame(std::shared_ptr<GFE::DB> db, QString var, QString frameStr)
{
    int frame = INT_MIN;
    auto data = GFE::FO::GetData(db,var.toStdString(),true);
    GFE::data_t absMax = -FLT_MAX,absMin = FLT_MAX,Max = -FLT_MAX,Min = FLT_MAX;
    int absMaxFrame = INT_MIN,absMinFrame = INT_MIN,MaxFrame = INT_MIN,MinFrame = INT_MIN;
    //? 第0帧都是无效帧吗?
    for(int i = 1;i < data.size();++i){
        auto it = data[i];
        for(auto it1:it.data){
            if(abs(it1) > absMax){
                absMax = abs(it1);
                absMaxFrame = it.frame;
            }
            if(abs(it1) < absMin){
                absMin = abs(it1);
                absMinFrame = it.frame;
            }
            if(it1 > Max){
                Max = it1;
                MaxFrame = it.frame;
            }
            if(it1 < Min){
                Min = it1;
                MinFrame = it.frame;
            }
        }
    }

    if(frameStr == "AbsMax"){
        frame = absMaxFrame;
    }else if(frameStr == "AbsMin") frame = absMinFrame;
    else if(frameStr == "Max") frame = MaxFrame;
    else frame = MinFrame;

    return frame;
}



std::vector<GFE::data_t> GFECaculate::GetEnvelopeData(std::shared_ptr<GFE::DB> db, const QString &variable,const QString &frameStr)
{
    QString t_frameStr = frameStr.toLower();
    std::vector<GFE::data_t> envelope;
    if(t_frameStr == "absmax"||t_frameStr == "absmin"||
             t_frameStr == "min"||t_frameStr == "max"){
        auto allData = GFE::FO::GetData(db,variable.toStdString(),true);
        int size = allData[0].data.size();
        if(t_frameStr == "max")envelope.resize(size,-FLT_MAX);

        for(int i = 0;i < size;++i){
            for(int j = 0;j < allData.size();++j){
                if(t_frameStr == "max")envelope[i] = std::max(envelope[i],allData[j].data[i]);
            }
        }
    }


    return envelope;
}

int GFECaculate::GetElastoplasticity(std::shared_ptr<GFE::DB> db)
{
    int type = -1;

    auto materialNames = GFE::getAllMaterialName(db);

    for(auto& name : materialNames){
        auto material = GFE::getMaterial(db, name);
        auto entries =  material->entries;
        for(auto& entry : entries){
            if(entry->TypeStr() == "ConcreteDamaged"){
                return 1;
            }
        }
    }

    return 0;
}

ReportPlot::ReportPlot(const QString& outputDir)
{
    m_outputDir = outputDir;
    m_colors = QStringList({
        "black","green","blue","red","yellow","cyan","purple","brown",
        "coral","lime","maroon","orange","pink","wheat"
    });
    m_curExePath = QString::fromStdWString(GFE::GetExeDir());
    QString pythonHome = m_curExePath + "python3.8";
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    Py_SetPythonHome(converter.from_bytes(pythonHome.toStdString()).c_str());
}

ReportPlot::~ReportPlot()
{
    Py_Finalize();
}

void ReportPlot::PyRun(const std::string &runStr)
{
    PyRun_SimpleString(runStr.c_str());
}

std::string ReportPlot::keyWords2ArgsStr(std::unordered_map<std::string,std::string>& keyWords){
    std::string ret;
    for(auto it = keyWords.begin();it != keyWords.end();++it){
        if(it == keyWords.begin()){
            if(DlgReportUtil::IsFloat(it->second)){
                ret += it->first + "=" + it->second;
            }else{
                ret += it->first + "='" + it->second + "'";
            }
        }else{
            if(DlgReportUtil::IsFloat(it->second)){
                ret += ',' + it->first + "=" + it->second;
            }else{
                ret += ',' + it->first + "='" + it->second + "'";
            }
        }
    }

    return ret;
}

std::string ReportPlot::vector2PyStr(const std::vector<double>& vec,std::string name){
    std::string ret;
    ret = name + " = np.array([";

    for(int i = 0;i < vec.size();++i){
        std::ostringstream oss;
        oss << std::scientific << vec[i];
        std::string str = oss.str();
        if(i == 0){
            ret += str;
        }else{
            ret += ',' + str;
        }
    }

    ret += "], dtype=np.float64)";
    return ret;
}

std::string ReportPlot::strVec2PyStr(const std::vector<std::string> &strVec, std::string name)
{
    std::string ret;
    ret = name + "=[";

    for(int i = 0;i < strVec.size();++i){
        if(i == 0){
            ret += "'" + strVec[i] + "'";
        }else{
            ret += ",'" + strVec[i] + "'";
        }
    }

    ret += ']';
    return ret;
}

std::string ReportPlot::plotStr(std::string xName,std::string yName,std::unordered_map<std::string,std::string>& keyWords){
    std::string ret;
    if(keyWords.empty()){
        ret += "plt.plot(" + xName + ',' + yName + ')';
    }else{
        ret += "plt.plot(" + xName + ',' + yName + ',' + keyWords2ArgsStr(keyWords) + ')';
    }
    return ret;
}

std::string ReportPlot::fillBetweenStr(std::string xName,std::string y1Name, std::string y2Name,std::unordered_map<std::string,std::string>& keyWords){
    std::string ret;
    ret += "plt.fill_between(" + xName + ',' + y1Name + ',' + y2Name + ',' + keyWords2ArgsStr(keyWords) + ')';
    return ret;
}

std::string ReportPlot::patchStr(std::unordered_map<std::string,std::string>& keyWords){
    std::string ret;
    ret += "Patch(" + keyWords2ArgsStr(keyWords) + ')';
    return ret;
    return "";
}

std::string ReportPlot::list2PyStr(std::vector<std::string> strs, std::string name){
    std::string ret;
    ret += name + "=[";

    for(int i = 0;i < strs.size();++i){
        if(i == 0){
            ret += strs[i];
        }else{
            ret += ',' + strs[i];
        }
    }

    ret += ']';
    return ret;
}

QString ReportPlot::PrintEnergyCurveChart(const QVector<QVector<double>>& xDatas,const QVector<QVector<double>>& yDatas,
                                          const QString& xAxis, const QString& yAxis,
                                          const QStringList& areaNames, int type, int width, int height)
{
    QString pngPath = m_outputDir + "/tmp" + QString::number(m_cnt++) + ".png";
    Py_Initialize();
    if (!Py_IsInitialized()) {
        const wchar_t* error = Py_GetPythonHome();
        printf("Error: Failed to set Python home: %ws\n", error);
        return "";
    }

    PyRun("import matplotlib");
    PyRun("matplotlib.use('Agg')");
    PyRun("import matplotlib.pyplot as plt");
    PyRun("from matplotlib.patches import Patch");
    PyRun("plt.clf()");

    std::vector<std::string> patchStrs;

    if(type == 0){
        auto xData = xDatas[0].toStdVector();
        std::string xName = "xData";
        PyRun(vector2PyStr(xData,xName));

        std::vector<double> base(yDatas[0].size(),0.0);
        PyRun(vector2PyStr(base,"base"));
        std::unordered_map<std::string,std::string> kw = {{"label","base"}};
        PyRun(plotStr(xName,"base",kw));
        for(int i = 0;i < yDatas.size();++i){
            auto yData = yDatas[i].toStdVector();
            std::string yName = "yData" + std::to_string(i);
            auto areaName = areaNames[i].toStdString();
            PyRun(vector2PyStr(yData,yName));
            std::string color = m_colors[i].toStdString();
            std::unordered_map<std::string,std::string> plotKW = {
                {"label",areaName}
            };
            PyRun(plotStr(xName, yName, plotKW));
            std::unordered_map<std::string,std::string> fillKW = {
                {"alpha","0.5"},{"color",color}
            };
            if(i == 0){
                PyRun(fillBetweenStr(xName,"base",yName,fillKW));
            }else{
                PyRun(fillBetweenStr(xName,"yData" + std::to_string(i - 1),yName,fillKW));
            }
            std::unordered_map<std::string,std::string> patchKW = {
                {"facecolor",color},{"alpha","0.5"},{"label",areaName}
            };
            patchStrs.push_back(patchStr(patchKW));
        }
    }else{
        auto yData = yDatas[0].toStdVector();
        std::string yName = "yData";
        PyRun(vector2PyStr(yData,yName));

        for(int i = 0;i < xDatas.size();++i){
            auto xData = xDatas[i].toStdVector();
            std::string xName = "xData" + std::to_string(i);
            auto lineName = areaNames[i].toStdString();
            PyRun(vector2PyStr(xData,xName));
            std::string color = m_colors[i].toStdString();
            std::unordered_map<std::string,std::string> plotKW = {
                {"label",lineName},{"color",color}
            };
            PyRun(plotStr(xName, yName, plotKW));
        }
    }

//    PyRun("plt.xlabel('" + xAxis.toStdString() + "')");
//    PyRun("plt.ylabel('" + yAxis.toStdString() + "')");
//    std::string patchesName = "patches_list";
//    PyRun(list2PyStr(patchStrs,patchesName));
//    PyRun("plt.legend(handles=" + patchesName + ")");
//    PyRun("plt.savefig('" + pngPath.toStdString() + "')");
//    PyRun("plt.show()");

    if(!areaNames.empty()){
        std::string patchesName = "patches_list";
        PyRun(list2PyStr(patchStrs,patchesName));
        PyRun("plt.legend(handles=" + patchesName + ")");
    }
    double width_inch = (double)width / 28 / 2.54,
        height_inch = (double)height / 28 / 2.54;
    PyRun("plt.grid(True)");
    PyRun("plt.gcf().set_size_inches(" + std::to_string(width_inch) +
          ", " + std::to_string(height_inch) + ")");
    PyRun("plt.savefig('" + pngPath.toStdString() + "',dpi = 300, bbox_inches='tight')");

    return pngPath;
}

QString ReportPlot::PrintCurveChart(const QVector<QVector<double>>& xDatas,const QVector<QVector<double>>& yDatas,
                                    const QString& xAxis, const QString& yAxis,
                                    const QStringList& lineNames, int type, int width, int height)
{

    QString pngPath = m_outputDir + "/tmp" + QString::number(m_cnt++) + ".png";
    double xMin = INFINITY, yMin = INFINITY, xMax = -INFINITY, yMax = -INFINITY;
    if(xDatas.empty() || yDatas.empty() || xDatas.size() != yDatas.size()){
        qCritical() << "Error: wrong XYData input args";
        return "";
    }

    Py_Initialize();
    if (!Py_IsInitialized()) {
        const wchar_t* error = Py_GetPythonHome();
        printf("Error: Failed to set Python home: %ws\n", error);
        return "";
    }

    QVector<QVector<double>> t_xDatas, t_yDatas;

    for(int i = 0;i < xDatas.size();++i){
        QVector<double> tmpX, tmpY;
        for(int j = 0;j < xDatas[i].size();++j){
            if(std::isnan(xDatas[i][j]) || std::isnan(yDatas[i][j])){
                continue;
            }
            tmpX.push_back(xDatas[i][j]);
            tmpY.push_back(yDatas[i][j]);
        }
        t_xDatas.push_back(tmpX);
        t_yDatas.push_back(tmpY);
    }

    PyRun("import numpy as np");
    PyRun("import matplotlib");
    PyRun("matplotlib.use('Agg')");
    PyRun("import matplotlib.pyplot as plt");
    PyRun("plt.clf()");

    if(type == 0){
        auto xData = t_xDatas[0].toStdVector();
        std::string xName = "xData";
        PyRun(vector2PyStr(xData,xName));

        for(int i = 0;i < xData.size();++i){
            if(xData[i] < xMin)xMin = xData[i];
            if(xData[i] > xMax)xMax = xData[i];
        }

        for(int i = 0;i < t_yDatas.size();++i){
            auto yData = t_yDatas[i].toStdVector();
            for(int j = 0;j < yData.size();++j){
                if(yData[j] < yMin)yMin = yData[j];
                if(yData[j] > yMax)yMax = yData[j];
            }
            std::string yName = "yData" + std::to_string(i);
            PyRun(vector2PyStr(yData,yName));
            std::string color = m_colors[i].toStdString();
            std::unordered_map<std::string,std::string> plotKW = {
                {"color",color}
            };
            if(!lineNames.empty()){
                auto lineName = lineNames[i].toStdString();
                plotKW.insert({"label",lineName});
            }
            PyRun(plotStr(xName, yName, plotKW));
        }
    }else{
        auto yData = t_yDatas[0].toStdVector();
        std::string yName = "yData";
        PyRun(vector2PyStr(yData,yName));
        for(int j = 0;j < yData.size();++j){
            if(yData[j] < yMin)yMin = yData[j];
            if(yData[j] > yMax)yMax = yData[j];
        }
        for(int i = 0;i < t_xDatas.size();++i){
            auto xData = t_xDatas[i].toStdVector();
            for(int j = 0;j < xData.size();++j){
                if(xData[j] < xMin)xMin = xData[j];
                if(xData[j] > xMax)xMax = xData[j];
            }
            std::string xName = "xData" + std::to_string(i);
            PyRun(vector2PyStr(xData,xName));
            std::string color = m_colors[i].toStdString();
            std::unordered_map<std::string,std::string> plotKW = {
                {"color",color}
            };
            if(!lineNames.empty()){
                auto lineName = lineNames[i].toStdString();
                plotKW.insert({"label",lineName});
            }
            PyRun(plotStr(xName, yName, plotKW));
            if(type == 1){
                PyRun("plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))");
            }
        }
    }


    if(type == 2){
        PyRun("plt.xlim(" + std::to_string(xMin) + "," + std::to_string(xMax) + ")");
        PyRun("plt.ylim(" + std::to_string(yMin) + "," + std::to_string(yMax) + ")");
        PyRun("plt.yticks(range(" + std::to_string((int)yMin) +", " + std::to_string((int)yMax + 1) + "))");

        std::vector<double> xaixsVec;
        std::vector<std::string> sdrVec;
        double d = (xMax - xMin) / 4, sdr = xMin;

        for(int i = 0;i < 5;++i){
            xaixsVec.push_back(sdr);
            sdrVec.push_back(DlgReportUtil::FloatToReciprocal(sdr));
            sdr += d;
        }

//        std::vector<double> xaixsVec = {0.0};
//        std::vector<std::string> sdrVec = {"0"};
//        double start = 5000.0, num = start, end = -1.0;
//        while(1){
//            if(1 / num > xMax){
//                end = num;
//                break;
//            }
//            num -= 500;
//        }
//        for(double i = start;i >= end;i -= 500){
//            double sdr = 1 / i;
//            xaixsVec.push_back(sdr);
//            sdrVec.push_back(DlgReportUtil::FloatToReciprocal(sdr));
//            QD i << sdr;
//        }

        PyRun(vector2PyStr(xaixsVec, "xaixsVec"));
        PyRun(strVec2PyStr(sdrVec, "sdrVec"));

        PyRun(strVec2PyStr(sdrVec, "sdrVec"));
        PyRun("plt.xticks(xaixsVec, sdrVec)");
//        PyRun("plt.gca().set_aspect('equal')");
    }

    PyRun("plt.xlabel('" + xAxis.toStdString() + "')");
    PyRun("plt.ylabel('" + yAxis.toStdString() + "')");
    if(!lineNames.empty()){
        PyRun("plt.legend()");
    }
    double width_inch = (double)width / 28 / 2.54,
        height_inch = (double)height / 28 / 2.54;
    PyRun("plt.grid(True)");
    PyRun("plt.gcf().set_size_inches(" + std::to_string(width_inch) +
          ", " + std::to_string(height_inch) + ")");
    PyRun("plt.savefig('" + pngPath.toStdString() + "',dpi = 300, bbox_inches='tight')");

    return pngPath;
}

ReportVTK::ReportVTK(std::shared_ptr<GFE::DB> db, int width, int height):
    m_renderer(vtkSmartPointer<vtkRenderer>::New()),
    m_interactor(vtkSmartPointer<vtkRenderWindowInteractor>::New()),
    m_rendererWindow(vtkSmartPointer<vtkRenderWindow>::New()),
    m_ugrid(vtkSmartPointer<vtkUnstructuredGrid>::New()),
    m_db(db)
{
    //设置渲染器和交互器默认属性
    m_renderer->SetBackground(1.0,1.0,1.0);
    m_renderer->SetBackgroundAlpha(1.0);
    m_rendererWindow->AddRenderer(m_renderer);
    m_rendererWindow->SetSize(width, height);
    m_interactor->SetRenderWindow(m_rendererWindow);

    Init();
}

ReportVTK::ReportVTK(Option *opt):
    m_renderer(vtkSmartPointer<vtkRenderer>::New()),
    m_interactor(vtkSmartPointer<vtkRenderWindowInteractor>::New()),
    m_rendererWindow(vtkSmartPointer<vtkRenderWindow>::New()),
    m_opt(opt){

    //设置渲染器和交互器默认属性
    m_renderer->SetBackground(1.0,1.0,1.0);
    m_renderer->SetBackgroundAlpha(0.0);
    m_rendererWindow->AddRenderer(m_renderer);
    m_interactor->SetRenderWindow(m_rendererWindow);
}

ReportVTK::~ReportVTK(){}

QString ReportVTK::Init()
{
    QString retInfo;

    retInfo = LoadModel();
    if(retInfo != "")return retInfo;

    return "";
}

QString ReportVTK::Start(const QString& angle)
{
    //调整相机视角
    auto camera = m_renderer->GetActiveCamera();
    if(m_dim == 3){
        auto mode = Str2ViewMode(angle);
        if(mode == Default){
            camera->SetPosition(-1, -1, 1);
            camera->SetFocalPoint(-1.7, -0.3, 0);
            camera->SetViewUp(-0.25,0.69,0.69);
            camera->SetViewAngle(30);

        }else{
            SetViewAction(mode);
        }
    }
    m_renderer->ResetCamera();

    //动态调整相机视角
    //    vtkSmartPointer<CustomCommand> command = vtkSmartPointer<CustomCommand>::New();
    //    m_interactor->AddObserver(vtkCommand::MouseMoveEvent, command);

    if(m_opt->isZoom) {
        camera->Zoom(m_opt->zoomFactor);
    }
    //坐标轴
    auto axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetShaftTypeToCylinder();
    axes->SetConeRadius(0.6);
    auto Axis = vtkOrientationMarkerWidget::New();
    Axis->SetOrientationMarker(axes);
    Axis->SetInteractor(m_interactor);
    Axis->SetEnabled(true);
    Axis->SetInteractive(false);
    Axis->SetViewport(0, 0, 0.25, 0.35);

    if(!m_opt->isInteract){
        m_rendererWindow->SetOffScreenRendering(true);
        m_rendererWindow->Render();
    }else{
        m_rendererWindow->Render();
        m_interactor->Initialize();
        m_interactor->Start();
    }


    return "";
}

QString ReportVTK::LoadModel()
{
    m_dim = DlgReportUtil::Stoi(GFE::getParameter(m_db, "Model Dim"));
    if(m_dim < 0){
        m_dim = 3;
    }
    m_nodeId2Label = GFE::getNodeAttribute(m_db, "Label");
    m_nNode = m_nodeId2Label.size();
    auto nodeMax = *std::max_element(m_nodeId2Label.begin(), m_nodeId2Label.end());
    m_nodeLabel2Id.resize(nodeMax+1);
    for (std::size_t i = 0; i < m_nNode; ++i)
        m_nodeLabel2Id[m_nodeId2Label[i]] = (int)i;

    m_elementId2Label = GFE::getElementAttribute(m_db, "Label");
    auto m_nElem = m_elementId2Label.size();
    auto elementMax = *std::max_element(m_elementId2Label.begin(), m_elementId2Label.end());
    m_elementLabel2Id.resize(elementMax+1);
    for (std::size_t i = 0; i < m_nElem; ++i)
        m_elementLabel2Id[m_elementId2Label[i]] = (int)i;


    /* 从数据库中读取的对象 */
    auto db_nodes = GFE::getNode(m_db);
    auto db_elements = GFE::getElement(m_db);
    auto db_nset = GFE::getAllNodeSet(m_db);
    auto db_elset = GFE::getAllElementSet(m_db);
    auto& ugrid = m_ugrid;

    /* 构造模型 */
    auto pids = vtkSmartPointer<vtkIntArray>::New();
    auto plbs = vtkSmartPointer<vtkIntArray>::New();
    auto pref = vtkSmartPointer<vtkIntArray>::New();
    auto cids = vtkSmartPointer<vtkIntArray>::New();
    auto clbs = vtkSmartPointer<vtkIntArray>::New();
    auto mat = vtkSmartPointer<vtkIntArray>::New();
    pids->SetName(NodeIdsName);
    plbs->SetName(NodeLabelsName);
    pref->SetName("Node:IsRefPoint");
    cids->SetName(ElementIdsName);
    clbs->SetName(ElementLabelsName);
    mat->SetName("Element:Material");

    // Node
    auto pts = vtkSmartPointer<vtkPoints>::New();
    auto sz_nd = (vtkIdType)db_nodes.size();
    pts->Allocate(sz_nd);
    pids->Allocate(sz_nd);
    plbs->Allocate(sz_nd);
    pref->Allocate(sz_nd);
    for(const auto& nd : db_nodes) {
        auto pid = pts->InsertNextPoint(nd.x, nd.y, nd.z);
        pids->InsertNextValue(pid);
        pref->InsertNextValue(-1);//???
        plbs->InsertNextValue(nd.label);
        m_nodeLabel2Id[nd.label] = pid;
    }
    ugrid->SetPoints(pts);

    // Element
    static QMap<int, int> tmap;
    {
        //初始化: GFE_API单元类型->VTK单元类型
        tmap = {
            {GFE::CT_LINE, VTK_LINE},
            {GFE::CT_TRIANGLE, VTK_TRIANGLE},
            {GFE::CT_QUAD, VTK_QUAD},
            {GFE::CT_TETRA, VTK_TETRA},
            {GFE::CT_HEXA, VTK_HEXAHEDRON},
            {GFE::CT_WEDGE, VTK_WEDGE},
            {GFE::CT_PYRAMID, VTK_PYRAMID},
            {GFE::CT_TETRA_2, VTK_QUADRATIC_TETRA}
        };
    }

    auto sz_el = (vtkIdType)db_elements.size();
    ugrid->Allocate(sz_el);
    cids->Allocate(sz_el);
    clbs->Allocate(sz_el);
    mat->Allocate(sz_el);
    for(const auto& el : db_elements) {
        auto npts = GFE::getElementNodeNum().at(el.type);
        auto cid = ugrid->InsertNextCell(tmap[el.type], npts, el.nodes.data());
        cids->InsertNextValue(cid);
        clbs->InsertNextValue(el.label);
        mat->InsertNextValue(el.material);//???
        m_elementLabel2Id[el.label] = cid;
    }

    // 添加属性数组
    {
        auto pd = ugrid->GetPointData();
        auto cd = ugrid->GetCellData();
        pd->AddArray(pids);
        pd->AddArray(plbs);
        pd->AddArray(pref);
        cd->AddArray(cids);
        cd->AddArray(clbs);
        cd->AddArray(mat);
    }

    return "";
}

QString ReportVTK::OptInit(Option *opt)
{
    QString retInfo;
    m_opt = opt;
    if(opt->isCloud){
        retInfo = SetScalar(0);
    }
    return retInfo;
}

QString ReportVTK::Rendering(const QStringList& setNames,
                             const QString& barTitle, const std::vector<float> &data , const QString& mapperType,
                             const QMap<int, QColor>& cm)
{
    QString retInfo, setNamesKey;
    QStringList t_setNames;
    m_ugrid->GetCellData()->RemoveArray("color_array");

    /* 选择单元集 */
    float rangeMax = -INFINITY, rangeMin = INFINITY, absMax = -INFINITY, absMin = INFINITY;
    int maxId = -1, minId = -1, absMaxId = -1, absMinId = -1;
    auto extractSelection = vtkSmartPointer<vtkExtractSelection>::New();
    auto appendSelection = vtkSmartPointer<vtkAppendSelection>::New();
    auto selectedUGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();

    bool hasSelection = false;
    for(int i = 0;i < setNames.size();++i){
        auto setName = setNames[i];
        auto selection = ElementSelection(setName);
        if(!selection){
            qDebug() <<  "Warning: This element set does not exist in DB: " + setName;
            continue;
        }
        hasSelection = true;
        t_setNames.push_back(setName);
        appendSelection->AddInputData(selection);
        if(!m_opt->isCloud)continue;

        if(setName.isEmpty()){
            for(int j = 0;j < data.size();++j){
                auto d = data[j], absd = std::abs(d);
                if(std::isnan(d))continue;
                if(d > rangeMax){
                    rangeMax = d;
                    maxId = j;
                }
                if(d < rangeMin){
                    rangeMin = d;
                    minId = j;
                }
                if(absd > absMax){
                    absMax = absd;
                    absMaxId = j;
                }
                if(absd < absMin){
                    absMin = absd;
                    absMinId = j;
                }
            }
        }else{
            if(m_opt->isCell){
                auto elementSet = GFE::getElementSet(m_db, setName.toStdString());
                if(!elementSet)return nullptr;
                for(const auto& elemId : elementSet->elements) {
                    auto d = data[elemId], absd = std::abs(d);
                    if(std::isnan(d))continue;
                    if(d > rangeMax){
                        rangeMax = d;
                        maxId = elemId;
                    }
                    if(d < rangeMin){
                        rangeMin = d;
                        minId = elemId;
                    }
                    if(absd > absMax){
                        absMax = absd;
                        absMaxId = elemId;
                    }
                    if(absd < absMin){
                        absMin = absd;
                        absMinId = elemId;
                    }
                }
            }else{
                auto nodes = GFE::getElementSetNodes(m_db, setName.toStdString());
                int idx = 0;
                for(int j = 0;j < data.size();++j){
                    if(j == nodes[idx]){
                        auto d = data[j], absd = std::abs(d);
                        if(std::isnan(d)){
                            idx++;
                            continue;
                        }
                        if(d > rangeMax){
                            rangeMax = d;
                            maxId = j;
                        }
                        if(d < rangeMin){
                            rangeMin = d;
                            minId = j;
                        }
                        if(absd > absMax){
                            absMax = absd;
                            absMaxId = j;
                        }
                        if(absd < absMin){
                            absMin = absd;
                            absMinId = j;
                        }
                        idx++;
                    }
                }
            }
        }

    }

    if(!hasSelection){
        retInfo = "Warning: Element set " +  setNamesKey + " do not exists!";
        return retInfo;
    }

    std::sort(t_setNames.begin(), t_setNames.end());
    setNamesKey = DlgReportUtil::ConnectStrs(t_setNames, "-");

    if(m_opt->isCloud){
        if(rangeMax == -INFINITY || rangeMin == INFINITY ||
            absMax == -INFINITY || absMin == INFINITY ||
            std::isnan(rangeMax) || std::isnan(rangeMin)){
            retInfo = "Warning: " + setNamesKey + ":" + barTitle + " Color data is all NAN!";
            return retInfo;
        }
        m_setMax = rangeMax, m_setMin = rangeMin, m_setAbsMax = absMax, m_setAbsMin = absMin;
        m_setMaxId = maxId, m_setMinId = minId, m_setAbsMaxId = absMaxId, m_setAbsMinId = absMinId;
    }

    vtkSmartPointer<vtkSelection> combinedSelection = appendSelection->GetOutput();
    appendSelection->Update();
    extractSelection->SetInputData(0, m_ugrid);
    extractSelection->SetInputData(1, combinedSelection);
    extractSelection->Update();
    selectedUGrid->ShallowCopy(extractSelection->GetOutput());

    vtkSmartPointer<vtkPolyData> polydata;
    if(m_polyDatas.find(setNamesKey) == m_polyDatas.end()){
        auto surfFtr = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfFtr->SetInputData(selectedUGrid);
        surfFtr->SetNonlinearSubdivisionLevel(0);
        surfFtr->Update();
        polydata = surfFtr->GetOutput();
        m_polyDatas[setNamesKey] = polydata;
    }else{
        polydata = m_polyDatas[setNamesKey];
    }

    if(m_opt->isCell){
        polydata->GetCellData()->RemoveArray("color_arr");
        selectedUGrid->GetCellData()->RemoveArray("color_arr");
        polydata->GetCellData()->SetScalars(nullptr);
        selectedUGrid->GetCellData()->SetScalars(nullptr);
    }else{
        polydata->GetPointData()->RemoveArray("color_arr");
        selectedUGrid->GetPointData()->RemoveArray("color_arr");
        polydata->GetPointData()->SetScalars(nullptr);
        selectedUGrid->GetPointData()->SetScalars(nullptr);
    }

    if(m_opt->isCloud){
        if(data.empty())return "Error: No color data!";
        auto vd = StdVecToVtkArray(data);
        if(vd){
            vtkSmartPointer<vtkFloatArray> array;
            if(m_opt->isCell){
                auto attr = polydata->GetCellData();
                array = FloatArrayFilter(vd, attr->GetArray(ElementIdsName));
                attr->SetScalars(array);
                array->SetName("color_arr");
            }else{
                auto attr = polydata->GetPointData();
                array = FloatArrayFilter(vd, attr->GetArray(NodeIdsName));
                attr->SetScalars(array);
                array->SetName("color_arr");
            }
        }
    }

    if(m_opt->isCloud){
        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polydata);
        if(mapperType == "banded"){
            mapper->InterpolateScalarsBeforeMappingOn();
        }else if(mapperType == "origin"){
            mapper->InterpolateScalarsBeforeMappingOff();
        }
        m_curActor = vtkSmartPointer<vtkActor>::New();
        m_curActor->SetMapper(mapper);
        if(m_opt->isDiscrete){
            SetScalar(2, data, cm);
        }else{
            mapper->SetScalarRange(rangeMin, rangeMax);
            mapper->SetLookupTable(m_lookupTable);
        }
        if(!m_opt->isCell){
            mapper->SetScalarModeToUsePointData();
        }else{
            mapper->SetScalarModeToUseCellData();
        }
        mapper->SetColorModeToMapScalars();
        m_scalarBarWgt->GetScalarBarActor()->SetTitle(barTitle.toStdString().c_str());
        m_curActor->GetMapper()->Update();
        m_renderer->AddActor(m_curActor);
    }else{
        auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
        mapper->SetInputData(selectedUGrid);
        m_curActor = vtkSmartPointer<vtkActor>::New();
        m_curActor->SetMapper(mapper);
        auto color = ReportVTK::Color["Default"];
        m_curActor->GetProperty()->SetColor(color.toStdVector().data());
        m_renderer->AddActor(m_curActor);
    }

    if(m_opt->isMesh){
        auto mesh = vtkSmartPointer<vtkExtractEdges>::New();
        mesh->SetInputConnection(extractSelection->GetOutputPort());
        auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
        auto actor = vtkSmartPointer<vtkActor>::New();
        mapper->SetInputConnection(mesh->GetOutputPort());
        mapper->ScalarVisibilityOff();
        actor->SetMapper(mapper);
        actor->GetProperty()->EdgeVisibilityOn();
        m_renderer->AddActor(actor);
    }

    if(m_opt->isLabel){
        auto vd = GFE_Utility::stdVecToVtkArray(data);
        if(vd){
            vtkSmartPointer<vtkIntArray> array;

            if(m_opt->isCell){
                auto attr = selectedUGrid->GetCellData();
                array = IntArrayFilter(vd,attr->GetArray(ElementIdsName));
                array->SetName("color_arr");
                attr->SetScalars(array);
            }else{
                auto attr = selectedUGrid->GetPointData();
                array = IntArrayFilter(vd,attr->GetArray(NodeIdsName));
                array->SetName("color_arr");
                attr->SetScalars(array);
            }
        }

        vtkSmartPointer<vtkCellCenters> center = vtkSmartPointer<vtkCellCenters>::New();
        center->SetInputData(selectedUGrid);

        vtkSmartPointer<vtkSelectVisiblePoints> visElm = vtkSmartPointer<vtkSelectVisiblePoints>::New();
        visElm->SetRenderer(m_renderer);
        visElm->SetInputConnection(center->GetOutputPort());
        visElm->SetTolerance(1);
        visElm->SelectInvisibleOff();

        vtkSmartPointer<vtkPointSetToLabelHierarchy> e2lLocal = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
        e2lLocal->SetInputConnection(visElm->GetOutputPort());
        e2lLocal->SetLabelArrayName("color_arr");

        vtkSmartPointer<vtkLabelPlacementMapper> labelMapperElmLocal = vtkSmartPointer<vtkLabelPlacementMapper>::New();
        labelMapperElmLocal->SetInputConnection(e2lLocal->GetOutputPort());
        labelMapperElmLocal->SetShapeToNone();
        labelMapperElmLocal->Update();

        vtkSmartPointer<vtkTextProperty> labelTextElm = vtkSmartPointer<vtkTextProperty>::New();
        labelTextElm->SetFontSize(18);
        labelTextElm->SetBold(false);
        labelTextElm->SetColor(0, 0, 0);
        labelTextElm->SetFrame(false);

        e2lLocal->SetTextProperty(labelTextElm);

        vtkSmartPointer<vtkActor2D> localElm = vtkSmartPointer<vtkActor2D>::New();

        localElm->SetMapper(labelMapperElmLocal);
        m_renderer->AddActor2D(localElm);

        localElm->GetMapper()->GetInputAlgorithm()->GetInputAlgorithm()->Update();
        localElm->VisibilityOn();
    }

    return "";
}

vtkSmartPointer<vtkSelection> ReportVTK::ElementSelection(const QString &elementSetName)
{
    auto selection = vtkSmartPointer<vtkSelection>::New();
    auto ids = vtkSmartPointer<vtkIdTypeArray>::New();
    if(elementSetName.isEmpty()){
        auto element = GFE::getElement(m_db);
        if(element.empty())return nullptr;
        for(const auto& el : element) {
            ids->InsertNextValue(el.id);
        }
    }else{
        auto elementSet = GFE::getElementSet(m_db,elementSetName.toStdString());
        if(!elementSet)return nullptr;
        for(const auto& el : elementSet->elements) {
            ids->InsertNextValue(el);
        }
    }
    auto node = vtkSmartPointer<vtkSelectionNode>::New();
    node->SetFieldType(vtkSelectionNode::CELL);
    node->SetContentType(vtkSelectionNode::INDICES);
    node->SetSelectionList(ids);
    selection->AddNode(node);
    return selection;
}

vtkSmartPointer<vtkExtractSelection> ReportVTK::ElementSelection(const QVector<int> &elemIds)
{
    auto selection = vtkSmartPointer<vtkSelection>::New();
    auto ids = vtkSmartPointer<vtkIdTypeArray>::New();
    auto extractSelection = vtkSmartPointer<vtkExtractSelection>::New();
    if(elemIds.empty())return nullptr;
    for(auto& elemId : elemIds){
        ids->InsertNextValue(elemId);
    }
    auto node = vtkSmartPointer<vtkSelectionNode>::New();
    node->SetFieldType(vtkSelectionNode::CELL);
    node->SetContentType(vtkSelectionNode::INDICES);
    node->SetSelectionList(ids);
    selection->AddNode(node);

    extractSelection->SetInputData(0, m_ugrid);
    extractSelection->SetInputData(1, selection);
    extractSelection->Update();

    return extractSelection;
}

vtkSmartPointer<vtkExtractSelection> ReportVTK::SetsSelection(const QStringList &setNames)
{
    if(m_opt->isCell){
        auto extractSelection = vtkSmartPointer<vtkExtractSelection>::New();
        auto appendSelection = vtkSmartPointer<vtkAppendSelection>::New();
        bool hasSelection = false;
        for(int i = 0;i < setNames.size();++i){
            auto setName = setNames[i];
            auto selection = ElementSelection(setName);
            if(!selection){
                qDebug() <<  "Warning: This element set does not exist in DB: " + setName;
                continue;
            }
            hasSelection = true;
            appendSelection->AddInputData(selection);
        }
        if(!hasSelection){
            return nullptr;
        }
        vtkSmartPointer<vtkSelection> combinedSelection = appendSelection->GetOutput();
        appendSelection->Update();
        extractSelection->SetInputData(0, m_ugrid);
        extractSelection->SetInputData(1, combinedSelection);
        extractSelection->Update();
        return extractSelection;
    }

    return nullptr;
}

QString ReportVTK::SetScalar(int type, const std::vector<float> &data, const QMap<int, QColor>& cm)
{
    vtkSmartPointer<vtkMapper> mapper;
    if(m_curActor)mapper = m_curActor->GetMapper();

    // 用于筛选vtkDataArray
    static auto ArrayFilter = [&](vtkDataArray* arr, vtkDataArray* ids, double fac = 1) {
        auto RET = vtkSmartPointer<vtkFloatArray>::New();
        RET->SetName(arr->GetName());
        if(!ids) return RET;
        auto n = ids->GetNumberOfTuples();
        RET->Allocate(n);
        for(int i = 0; i < n; i++)
            RET->InsertNextValue(arr->GetTuple1(ids->GetTuple1(i)) * fac);
        return RET;
    };
    if(type == 0){
        GetScalar(0);
    }
    if(type == 1){
        auto [vd, lut] = GetScalar(type, data, cm);
        if(!lut){
            return "Error: Lookup table Settings are abnormal!";
        }
        auto lookupTable = dynamic_cast<vtkLookupTable*>(lut.GetPointer());
        m_scalarBarWgt->GetScalarBarActor()->SetNumberOfLabels(10);
        m_scalarBarWgt->GetScalarBarActor()->SetLookupTable(lookupTable);
        m_scalarBarWgt->GetScalarBarActor()->SetNumberOfLabels(static_cast<int>(lookupTable->GetNumberOfTableValues()) + 1);
        m_scalarBarWgt->SetRepositionable(false);
        if(vd){
            vtkSmartPointer<vtkFloatArray> array;
            auto attr = m_ugrid->GetCellData();
            if(m_opt->isCell){
                array = ArrayFilter(vd,attr->GetArray(ElementIdsName));
            }else{
                array = ArrayFilter(vd,attr->GetArray(NodeIdsName));
            }
            attr->SetScalars(array);
        }
        if(mapper){
            mapper->SetLookupTable(lut);
        }

    }
    if(type == 2){
        if(!mapper){
            return "Error: VTK mapper is null, can not discrete scalar!";
        }
        auto [vd, lut] = GetScalar(type, data, cm);
        if(vd == nullptr)
            return "Error: Discrete data is empty!";
        mapper->SetLookupTable(lut);
        m_scalarBarWgt->GetScalarBarActor()->SetLookupTable(lut);
        mapper->ScalarVisibilityOn();
        if(m_opt->isCell){
            mapper->SetScalarModeToUseCellData();
        }else{
            mapper->SetScalarModeToUsePointData();
        }
        m_scalarBarRep->VisibilityOn();
        m_scalarBarWgt->GetScalarBarActor()->SetTextPositionToPrecedeScalarBar();
        vtkSmartPointer<vtkFloatArray> array;
        auto attr = m_ugrid->GetCellData();
        if(m_opt->isCell){
            array = ArrayFilter(vd,attr->GetArray(ElementIdsName));
        }else{
            array = ArrayFilter(vd,attr->GetArray(NodeIdsName));
        }
        attr->SetScalars(array);
        mapper->SetColorModeToMapScalars();
    }

    return "";
}

std::pair<vtkSmartPointer<vtkDataArray>,vtkSmartPointer<vtkScalarsToColors>> ReportVTK::GetScalar(int type, const std::vector<float> &data, const QMap<int, QColor>& cm)
{
    if(type == 0){
        //初始化云图映射表
        m_lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        m_lookupTable->SetNumberOfColors(12);
        m_lookupTable->SetTableValue(11, 1, 0, 0);
        m_lookupTable->SetTableValue(10, 1, .3608, 0);
        m_lookupTable->SetTableValue(9, 1, .7255, 0);
        m_lookupTable->SetTableValue(8, .9059, 1, 0);
        m_lookupTable->SetTableValue(7, .5451, 1, 0);
        m_lookupTable->SetTableValue(6, .1804, 1, 0);
        m_lookupTable->SetTableValue(5, 0, 1, .1804);
        m_lookupTable->SetTableValue(4, 0, 1, .5451);
        m_lookupTable->SetTableValue(3, 0, 1, .9059);
        m_lookupTable->SetTableValue(2, 0, .7255, 1);
        m_lookupTable->SetTableValue(1, 0, .3608, 1);
        m_lookupTable->SetTableValue(0, 0, 0, 1);
        m_lookupTable->SetAboveRangeColor(120/255.0, 120/255.0, 120/255.0, 1);
        m_lookupTable->SetUseAboveRangeColor(true);
        m_lookupTable->SetBelowRangeColor(51/255.0, 51/255.0, 51/255.0, 1);
        m_lookupTable->SetUseBelowRangeColor(true);
        m_lookupTable->SetNanColor(1,1,1,1);
        m_lookupTable->Build();

        //初始化色标带
        m_scalarBarWgt = vtkSmartPointer<vtkScalarBarWidget>::New();
        m_scalarBarWgt->GetScalarBarActor()->SetUnconstrainedFontSize(true);
        m_scalarBarWgt->GetScalarBarActor()->SetNumberOfLabels(10);
        m_scalarBarWgt->GetScalarBarActor()->SetLookupTable(m_lookupTable);
        m_scalarBarWgt->GetScalarBarActor()->SetNumberOfLabels(static_cast<int>(m_lookupTable->GetNumberOfTableValues()) + 1);
        m_scalarBarWgt->SetRepositionable(false);

        m_scalarBarWgt->GetScalarBarActor()->GetTitleTextProperty()->SetFontSize(30);
        m_scalarBarWgt->GetScalarBarActor()->GetTitleTextProperty()->SetBold(true);
        m_scalarBarWgt->GetScalarBarActor()->GetTitleTextProperty()->SetColor(0.0,0.0,0.0);
        m_scalarBarWgt->GetScalarBarActor()->GetLabelTextProperty()->SetFontSize(28);
        m_scalarBarWgt->GetScalarBarActor()->GetLabelTextProperty()->SetBold(true);
        m_scalarBarWgt->GetScalarBarActor()->GetLabelTextProperty()->SetColor(0.0,0.0,0.0);

        // m_scalarBarWgt->GetScalarBarActor()->SetTitle(m_barTitle.c_str());
        //       scalarBarWgt->GetScalarBarActor()->SetLabelFormat(("%."+to_string(ui->horizontalSlider_Decimal->value())+"E").data());
        m_scalarBarRep = vtkScalarBarRepresentation::SafeDownCast(m_scalarBarWgt->GetRepresentation());
        m_scalarBarRep->SetOrientation(1);
        m_scalarBarRep->SetAutoOrient(false);
        m_scalarBarRep->GetPosition2Coordinate()->SetValue(.07, .63);
        m_scalarBarRep->SetPosition(0.05, 0.33);
        m_scalarBarRep->SetShowBorderToOff();
        m_scalarBarRep->SetVisibility(true);

        m_scalarBarWgt->SetInteractor(m_interactor);
        m_scalarBarWgt->SetDefaultRenderer(m_renderer);
        m_scalarBarWgt->SetResizable(false);
        m_scalarBarWgt->On();

        m_renderer->AddActor(m_scalarBarWgt->GetScalarBarActor());
    }
    else if(type == 1){
        //todo
        auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        lookupTable->SetNumberOfColors(12);
        lookupTable->SetTableValue(11, 1, 0, 0);
        lookupTable->SetTableValue(10, 1, .3608, 0);
        lookupTable->SetTableValue(9, 1, .7255, 0);
        lookupTable->SetTableValue(8, .9059, 1, 0);
        lookupTable->SetTableValue(7, .5451, 1, 0);
        lookupTable->SetTableValue(6, .1804, 1, 0);
        lookupTable->SetTableValue(5, 0, 1, .1804);
        lookupTable->SetTableValue(4, 0, 1, .5451);
        lookupTable->SetTableValue(3, 0, 1, .9059);
        lookupTable->SetTableValue(2, 0, .7255, 1);
        lookupTable->SetTableValue(1, 0, .3608, 1);
        lookupTable->SetTableValue(0, 0, 0, 1);
        lookupTable->SetAboveRangeColor(120/255.0, 120/255.0, 120/255.0, 1);
        lookupTable->SetUseAboveRangeColor(true);
        lookupTable->SetBelowRangeColor(51/255.0, 51/255.0, 51/255.0, 1);
        lookupTable->SetUseBelowRangeColor(true);
        lookupTable->SetNanColor(1,1,1,1);
        lookupTable->Build();
        if(!data.empty()){
            auto vd = StdVecToVtkArray(data);
            if(!vd)return {nullptr, nullptr};
            return {vd, lookupTable};
        }else{
            return {nullptr, lookupTable};
        }
    }else if(type == 2){
        auto vd = StdVecToVtkArray(data);
        if(!vd)return {nullptr, nullptr};
        auto colorMap = vtkSmartPointer<vtkColorTransferFunction>::New();
        colorMap->SetIndexedLookup(true);
        std::set<int> sdata;
        bool hasNan = false;
        for(int i = 0; i < data.size(); ++i)
        {
            if(data[i] < 0 || std::isnan(data[i]))
                hasNan = true;
            else
                sdata.insert(data[i]);
        }
        int cnt = 0;
        for(auto it = sdata.begin(); it != sdata.end(); ++it)
        {
            int d = *it;
            colorMap->AddRGBPoint(*it, cm[d].redF(), cm[d].greenF(), cm[d].blueF());
            colorMap->SetAnnotation(*it, std::to_string(d));
            cnt++;
        }
        const QColor DF_COLOR = QColor(255, 255, 255);
        if(hasNan)
        {
            colorMap->SetNanColor(DF_COLOR.redF(), DF_COLOR.greenF(), DF_COLOR.blueF());
            colorMap->AddRGBPoint(cnt, DF_COLOR.redF(), DF_COLOR.greenF(), DF_COLOR.blueF());
            colorMap->SetAnnotation(cnt, "NAN");
        }
        return {vd, colorMap};
    }
    return {nullptr, nullptr};
}

QString ReportVTK::ExportChartXY(const QVector<QPair<QVector<double>,QVector<double>>>& coordDatas,const QString& xName,const QString& yName,const QString& pngPath)
{
    auto table = vtkSmartPointer<vtkTable>::New();
    auto arrX = vtkSmartPointer<vtkDoubleArray>::New();
    auto arrY = vtkSmartPointer<vtkDoubleArray>::New();
    arrX->SetName(xName.toStdString().c_str());
    arrY->SetName(yName.toStdString().c_str());
    table->AddColumn(arrX);
    table->AddColumn(arrY);
    auto xData = coordDatas[0].first, yData = coordDatas[0].second;
    auto size = (int)xData.size();
    table->SetNumberOfRows(size);
    for(int i = 0; i < size; i++) {
        table->SetValue(i, 0, xData[i]);
        table->SetValue(i, 1, yData[i]);
    }

    auto chart = vtkSmartPointer<vtkChartXY>::New();
    chart->SetShowLegend(true);
    chart->SetZoomWithMouseWheel(true);
    chart->SetAdjustLowerBoundForLogPlot(true);
    chart->SetInteractive(true);
    chart->GetLegend()->SetInline(true);
    chart->GetAxis(1)->SetTitle(xName.toStdString());
    chart->GetAxis(0)->SetTitle(yName.toStdString());

    auto txtProp = chart->GetLegend()->GetLabelProperties();
    txtProp->SetFontFamily(VTK_FONT_FILE);
    txtProp->SetFontFile("fonts/simhei.ttf");
    txtProp = chart->GetAxis(0)->GetTitleProperties();
    txtProp->SetFontFamily(VTK_FONT_FILE);
    txtProp->SetFontFile("fonts/simhei.ttf");
    txtProp = chart->GetAxis(1)->GetTitleProperties();
    txtProp->SetFontFamily(VTK_FONT_FILE);
    txtProp->SetFontFile("fonts/simhei.ttf");

    vtkPlot* line = chart->AddPlot(vtkChart::LINE);
    line->SetInputData(table, 0, 1);
    line->SetColor(255, 0, 0);

    auto view = vtkSmartPointer<vtkContextView>::New();
    view->GetScene()->AddItem(chart);
    view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
    view->SetRenderWindow(m_rendererWindow);
    view->SetInteractor(m_interactor);

    m_rendererWindow->SetSize(400, 450);
    if(!m_opt->isInteract){
        m_rendererWindow->SetOffScreenRendering(true);
        m_rendererWindow->Render();
    }else{
        m_rendererWindow->Render();
        m_interactor->Initialize();
        m_interactor->Start();
    }

    WritePicture(pngPath);
    return "";
}

void ReportVTK::SetViewAction(int mode)
{
    vtkCamera* cam;
    cam = m_renderer->GetActiveCamera();
    switch(mode)
    {
    case Axo:
        cam->SetPosition(1, 1, 1);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(-1, 1, -1);
        break;
    case Front:
        cam->SetPosition(0, 0, 1);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 1, 0);
        break;
    case Back:
        cam->SetPosition(0, 0, -1);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 1, 0);
        break;
    case Left:
        cam->SetPosition(-1, 0, 0);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 1, 0);
        break;
    case Right:
        cam->SetPosition(1, 0, 0);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 1, 0);
        break;
    case Top:
        cam->SetPosition(0, 1, 0);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 0, -1);
        break;
    case Bottom:
        cam->SetPosition(0, -1, 0);
        cam->SetFocalPoint(0, 0, 0);
        cam->SetViewUp(0, 0, 1);
        break;
    case Fit:
        break;
    case Parallel:
        cam->SetParallelProjection(true);
        break;
    case Perspective:
        cam->SetParallelProjection(false);
        break;
    case RotateLeft:
        cam->OrthogonalizeViewUp();
        cam->Azimuth(90.0);
        break;
    case RotateRight:
        cam->OrthogonalizeViewUp();
        cam->Azimuth(-90.0);
        break;
    case RotateUp:
        cam->Pitch(90.0);
        cam->OrthogonalizeViewUp();
        break;
    case RotateDown:
        cam->Pitch(-90.0);
        cam->OrthogonalizeViewUp();
        break;
    case RotateCounter:
        cam->Roll(90.0);
        break;
    case RotateClock:
        cam->Roll(-90.0);
        break;
    }
    m_renderer->ResetCamera();
    m_rendererWindow->Render();
}

ReportVTK::ViewMode ReportVTK::Str2ViewMode(const QString &angle)
{
    auto t_angle = GFE::ToLower(angle.toStdString());
    if(t_angle == "default"){
        return ViewMode::Default;
    }else if(t_angle == "axo"){
        return ViewMode::Axo;
    }else if(t_angle == "front"){
        return ViewMode::Front;
    }else if(t_angle == "back"){
        return ViewMode::Back;
    }else if(t_angle == "left"){
        return ViewMode::Left;
    }else if(t_angle == "right"){
        return ViewMode::Right;
    }else if(t_angle == "top"){
        return ViewMode::Top;
    }else if(t_angle == "bottom"){
        return ViewMode::Bottom;
    }else if(t_angle == "fit"){
        return ViewMode::Fit;
    }else if(t_angle == "parallel"){
        return ViewMode::Parallel;
    }else if(t_angle == "perspective"){
        return ViewMode::Perspective;
    }else if(t_angle == "rotateLeft"){
        return ViewMode::RotateLeft;
    }else if(t_angle == "rotateRight"){
        return ViewMode::RotateRight;
    }else if(t_angle == "rotateUp"){
        return ViewMode::RotateUp;
    }else if(t_angle == "rotateDown"){
        return ViewMode::RotateDown;
    }else if(t_angle == "rotateCounter"){
        return ViewMode::RotateCounter;
    }else if(t_angle == "rotateClock"){
        return ViewMode::RotateClock;
    }
    return ViewMode::Unknow;
}

QString ReportVTK::HighLight(vtkSmartPointer<vtkExtractSelection> eExt, const QStringList& baseSetNames, const std::vector<GFE::data_t>& data)
{
    QString retInfo;

    auto esBase = SetsSelection(baseSetNames);
    if(!esBase){
        retInfo = "Warning: Can not select the base node or element sets!";
        return retInfo;
    }
    auto mapperBase = vtkSmartPointer<vtkDataSetMapper>::New();
    auto actorBase = vtkSmartPointer<vtkActor>::New();
    auto uGridBase = vtkSmartPointer<vtkUnstructuredGrid>::New();
    actorBase->SetMapper(mapperBase);
    uGridBase->ShallowCopy(esBase->GetOutput());
    mapperBase->SetInputData(uGridBase);
    auto color = ReportVTK::Color["Default"];
    actorBase->GetProperty()->SetColor(color.toStdVector().data());
    if(m_opt->isMesh){
        actorBase->GetProperty()->EdgeVisibilityOn();  // 显示边线
    }

    if(!eExt){
        retInfo = "Warning: Can not select the node or element sets!";
        return retInfo;
    }
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    auto property = actor->GetProperty();
    property->SetLineWidth(1);
    property->SetPointSize(5);
    property->SetColor(1, 0, 1);
    VTK_SP(vtkExtractEdges, eGlyph);
    eGlyph->SetInputConnection(eExt->GetOutputPort());
    eGlyph->Update();
    actor->GetMapper()->SetInputDataObject(eGlyph->GetOutput());

    auto rendererHL = vtkSmartPointer<vtkRenderer>::New();
    rendererHL->SetBackground(1.0, 1.0, 1.0);

    if(m_opt->isLabel){
        auto selectedUGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        selectedUGrid->ShallowCopy(eExt->GetOutput());


        auto vd = GFE_Utility::stdVecToVtkArray(data);
        if(m_opt->isCell){
            auto attr = selectedUGrid->GetCellData();
            vtkSmartPointer<vtkStringArray> strArray;
            strArray = StringArrayFilter(vd, attr->GetArray(ElementIdsName), "Maximum:");
            strArray->SetName("color_arr");
            attr->AddArray(strArray);
        }


        vtkSmartPointer<vtkCellCenters> center = vtkSmartPointer<vtkCellCenters>::New();
        center->SetInputData(selectedUGrid);

        vtkSmartPointer<vtkSelectVisiblePoints> visElm = vtkSmartPointer<vtkSelectVisiblePoints>::New();
        visElm->SetRenderer(m_renderer);
        visElm->SetInputConnection(center->GetOutputPort());
        visElm->SetTolerance(1);
        visElm->SelectInvisibleOff();

        vtkSmartPointer<vtkPointSetToLabelHierarchy> e2lLocal = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
        e2lLocal->SetInputConnection(visElm->GetOutputPort());
        e2lLocal->SetLabelArrayName("color_arr");

        vtkSmartPointer<vtkLabelPlacementMapper> labelMapperElmLocal = vtkSmartPointer<vtkLabelPlacementMapper>::New();
        labelMapperElmLocal->SetInputConnection(e2lLocal->GetOutputPort());
        labelMapperElmLocal->SetShapeToNone();
        labelMapperElmLocal->Update();

        vtkSmartPointer<vtkTextProperty> labelTextElm = vtkSmartPointer<vtkTextProperty>::New();
        labelTextElm->SetFontSize(18);
        labelTextElm->SetBold(false);
        labelTextElm->SetColor(0, 0, 0);
        labelTextElm->SetFrame(false);

        e2lLocal->SetTextProperty(labelTextElm);

        vtkSmartPointer<vtkActor2D> localElm = vtkSmartPointer<vtkActor2D>::New();

        localElm->SetMapper(labelMapperElmLocal);
        rendererHL->AddActor2D(localElm);

        localElm->GetMapper()->GetInputAlgorithm()->GetInputAlgorithm()->Update();
        localElm->VisibilityOn();

    }

    m_renderer->AddActor(actorBase);
    rendererHL->AddActor(actor);

    m_rendererWindow->SetNumberOfLayers(2);
    m_rendererWindow->AddRenderer(rendererHL);
    rendererHL->SetLayer(1);
    rendererHL->SetActiveCamera(m_renderer->GetActiveCamera());

    return retInfo;
}

QString ReportVTK::AddLabel()
{
    QString retInfo;

//    auto selectedUGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
//    selectedUGrid->ShallowCopy(eExt->GetOutput());

//    auto vd = GFE_Utility::stdVecToVtkArray(data);
//    if(m_opt->isCell){
//        auto attr = selectedUGrid->GetCellData();
//        vtkSmartPointer<vtkStringArray> strArray;
//        strArray = StringArrayFilter(vd, attr->GetArray(ElementIdsName), "Maximum:");
//        strArray->SetName("color_arr");
//        attr->AddArray(strArray);
//    }


//    vtkSmartPointer<vtkCellCenters> center = vtkSmartPointer<vtkCellCenters>::New();
//    center->SetInputData(selectedUGrid);

//    vtkSmartPointer<vtkSelectVisiblePoints> visElm = vtkSmartPointer<vtkSelectVisiblePoints>::New();
//    visElm->SetRenderer(m_renderer);
//    visElm->SetInputConnection(center->GetOutputPort());
//    visElm->SetTolerance(1);
//    visElm->SelectInvisibleOff();

//    vtkSmartPointer<vtkPointSetToLabelHierarchy> e2lLocal = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
//    e2lLocal->SetInputConnection(visElm->GetOutputPort());
//    e2lLocal->SetLabelArrayName("color_arr");

//    vtkSmartPointer<vtkLabelPlacementMapper> labelMapperElmLocal = vtkSmartPointer<vtkLabelPlacementMapper>::New();
//    labelMapperElmLocal->SetInputConnection(e2lLocal->GetOutputPort());
//    labelMapperElmLocal->SetShapeToNone();
//    labelMapperElmLocal->Update();

//    vtkSmartPointer<vtkTextProperty> labelTextElm = vtkSmartPointer<vtkTextProperty>::New();
//    labelTextElm->SetFontSize(18);
//    labelTextElm->SetBold(false);
//    labelTextElm->SetColor(0, 0, 0);
//    labelTextElm->SetFrame(false);

//    e2lLocal->SetTextProperty(labelTextElm);

//    vtkSmartPointer<vtkActor2D> localElm = vtkSmartPointer<vtkActor2D>::New();

//    localElm->SetMapper(labelMapperElmLocal);
//    rendererHL->AddActor2D(localElm);

//    localElm->GetMapper()->GetInputAlgorithm()->GetInputAlgorithm()->Update();
//    localElm->VisibilityOn();

    return retInfo;
}


QString ReportVTK::OffScreenRendering(const QString& pngPath, Option *opt, const QStringList& setNames,
                                      const QString& barTitle, const std::vector<float> &data, const QMap<int, QColor>& cm, const QString& mapperType,
                                      const QString& angle)
{
    QString retInfo;
    auto t_setsName = setNames;
    if(setNames.empty())t_setsName.push_back("");

    qDebug() << "Start OffScreenRendering at " + t_setsName[0];

    m_renderer->RemoveAllViewProps();
    retInfo = OptInit(opt);
    if(!retInfo.isEmpty())return retInfo;

    retInfo = Rendering(t_setsName, barTitle, data, mapperType, cm);
    if(!retInfo.isEmpty())return retInfo;

    retInfo = Start(angle);
    if(!retInfo.isEmpty())return retInfo;

    //创建过滤器，将渲染窗口中的图像数据转换为图像对象
    retInfo = WritePicture(pngPath);
    if(!retInfo.isEmpty())return retInfo;

    qDebug() << "OffScreenRendering finished success!";

    return "";
}

QString ReportVTK::OffScreenRendering(const QString &pngPath, Option *opt, const QStringList &setNames, const QStringList &baseSetNames, const std::vector<GFE::data_t>& data, const QString& extremeStr, const QString &angle)
{
    QString retInfo;
    m_renderer->RemoveAllViewProps();

    retInfo = OptInit(opt);
    if(!retInfo.isEmpty())return retInfo;

    auto t_baseSetNames = baseSetNames;
    if(baseSetNames.empty())t_baseSetNames.push_back("");

    if(!data.empty()){
        retInfo = GetOptimalValue(data, setNames);
        if(!retInfo.isEmpty())return retInfo;
    }

    if(m_opt->isHighLight){
        vtkSmartPointer<vtkExtractSelection> eExt;
        if(opt->isExtreme && extremeStr.toLower() == "max"){
            QVector<int> ids = {m_setMaxId};
            eExt = ElementSelection(ids);
        }else{
            eExt = SetsSelection(setNames);
        }

        retInfo = HighLight(eExt, t_baseSetNames, data);
        if(!retInfo.isEmpty())return retInfo;
    }

    retInfo = Start(angle);
    if(!retInfo.isEmpty())return retInfo;

    retInfo = WritePicture(pngPath);
    if(!retInfo.isEmpty())return retInfo;

    return retInfo;
}


QString ReportVTK::MaximumValueTag(const QString& variable, const QString &extremeStr)
{
    QString retInfo;
    int extremeId;
    auto t_extremeStr = GFE::ToLower(extremeStr.toStdString());
    double *startPoint, *endPoint;

    if(t_extremeStr == "max")extremeId = m_setMaxId;
    if(t_extremeStr == "min")extremeId = m_setMinId;

    if(m_opt->isCell){
        auto cell = m_polyDatas[variable]->GetCell(extremeId);
        GetCellCenter(cell, startPoint);
    }else{
        startPoint = m_polyDatas[variable]->GetPoint(extremeId);
    }

    auto windowSize = m_rendererWindow->GetSize();
    vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
    int rightTopXPos = windowSize[0] - 100, rightTopYPos = windowSize[1] - 50;
    textActor->SetInput(extremeStr.toStdString().c_str()); // 设置文本内容
    textActor->GetTextProperty()->SetColor(1.0, 0.0, 0.0); // 设置文本颜色为蓝色
    textActor->SetPosition(rightTopXPos, rightTopYPos);
    // 将文本演员添加到渲染器中
    m_renderer->AddActor2D(textActor);

    // 计算右上角的节点坐标
    double viewportPosition[2] = {1.0, 1.0}; // 视口坐标系中的位置（右上角）
    double viewportCoordinates[3];
    viewportCoordinates[0] = viewportPosition[0] * rightTopXPos;
    viewportCoordinates[1] = viewportPosition[1] * rightTopYPos;
    viewportCoordinates[2] = 0.0;
    // 将视口坐标转换为世界坐标
//    double startPoint[3];
    m_renderer->SetDisplayPoint(startPoint);
    m_renderer->DisplayToWorld();
    m_renderer->GetWorldPoint(startPoint);

    return retInfo;
}

QString ReportVTK::WritePicture(const QString& pngPath){
    //创建过滤器，将渲染窗口中的图像数据转换为图像对象
    auto windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(m_rendererWindow);
    windowToImageFilter->SetInputBufferTypeToRGBA();
    windowToImageFilter->ReadFrontBufferOff(); // 关闭读取前端缓冲区以避免闪烁
    windowToImageFilter->Update();

    vtkSmartPointer<vtkImageData> imageData = windowToImageFilter->GetOutput();
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();

    writer->SetFileName(pngPath.toStdString().c_str());
    writer->SetInputData(imageData);
    writer->Write();

    return "";
}

vtkSmartPointer<vtkIntArray> ReportVTK::IntArrayFilter(vtkDataArray *arr, vtkDataArray *ids, double fac)
{
    auto RET = vtkSmartPointer<vtkIntArray>::New();
    RET->SetName(arr->GetName());
    if(!ids) return RET;
    auto n = ids->GetNumberOfTuples();
    RET->Allocate(n);
    for(int i = 0; i < n; i++)
        RET->InsertNextValue(arr->GetTuple1(ids->GetTuple1(i)) * fac);
    return RET;
}

vtkSmartPointer<vtkFloatArray> ReportVTK::FloatArrayFilter(vtkDataArray *arr, vtkDataArray *ids, double fac)
{
    auto RET = vtkSmartPointer<vtkFloatArray>::New();
    RET->SetName(arr->GetName());
    if(!ids) return RET;
    auto n = ids->GetNumberOfTuples();
    RET->Allocate(n);
    for(int i = 0; i < n; i++)
        RET->InsertNextValue(arr->GetTuple1(ids->GetTuple1(i)) * fac);
    return RET;
}

vtkSmartPointer<vtkStringArray> ReportVTK::StringArrayFilter(vtkDataArray *arr, vtkDataArray *ids, std::string extremeStr, double fac)
{
    auto RET = vtkSmartPointer<vtkStringArray>::New();
    RET->SetName(arr->GetName());
    if(!ids) return RET;
    auto n = ids->GetNumberOfTuples();
    RET->Allocate(n);
    for(int i = 0; i < n; i++)
        RET->InsertNextValue(extremeStr + std::to_string(arr->GetTuple1(ids->GetTuple1(i))));
    return RET;
}

void ReportVTK::GetCellCenter(vtkCell *cell, double center[3])
{
    vtkPoints* points = cell->GetPoints();
    int numPoints = points->GetNumberOfPoints();

    center[0] = 0.0;
    center[1] = 0.0;
    center[2] = 0.0;

    for (int i = 0; i < numPoints; ++i) {
        double point[3];
        points->GetPoint(i, point);
        center[0] += point[0];
        center[1] += point[1];
        center[2] += point[2];
    }

    center[0] /= numPoints;
    center[1] /= numPoints;
    center[2] /= numPoints;
}

QString ReportVTK::GetOptimalValue(const std::vector<GFE::data_t>& data, const QStringList& setNames)
{
    QString retInfo;
    float rangeMax = -INFINITY, rangeMin = INFINITY, absMax = -INFINITY, absMin = INFINITY;
    int maxId = -1, minId = -1, absMaxId = -1, absMinId = -1;
    for(int i = 0;i < setNames.size();++i){
        auto setName = setNames[i];
        if(setName.isEmpty()){
            for(int j = 0;j < data.size();++j){
                auto d = data[j], absd = std::abs(d);
                if(std::isnan(d))continue;
                if(d > rangeMax){
                    rangeMax = d;
                    maxId = j;
                }
                if(d < rangeMin){
                    rangeMin = d;
                    minId = j;
                }
                if(absd > absMax){
                    absMax = absd;
                    absMaxId = j;
                }
                if(absd < absMin){
                    absMin = absd;
                    absMinId = j;
                }
            }
        }else{
            if(m_opt->isCell){
                auto elementSet = GFE::getElementSet(m_db, setName.toStdString());
                if(!elementSet){
                    continue;
                }
                for(const auto& elemId : elementSet->elements) {
                    auto d = data[elemId], absd = std::abs(d);
                    if(std::isnan(d))continue;
                    if(d > rangeMax){
                        rangeMax = d;
                        maxId = elemId;
                    }
                    if(d < rangeMin){
                        rangeMin = d;
                        minId = elemId;
                    }
                    if(absd > absMax){
                        absMax = absd;
                        absMaxId = elemId;
                    }
                    if(absd < absMin){
                        absMin = absd;
                        absMinId = elemId;
                    }
                }
            }else{
                auto nodes = GFE::getElementSetNodes(m_db, setName.toStdString());
                int idx = 0;
                for(int j = 0;j < data.size();++j){
                    if(j == nodes[idx]){
                        auto d = data[j], absd = std::abs(d);
                        if(std::isnan(d)){
                            idx++;
                            continue;
                        }
                        if(d > rangeMax){
                            rangeMax = d;
                            maxId = j;
                        }
                        if(d < rangeMin){
                            rangeMin = d;
                            minId = j;
                        }
                        if(absd > absMax){
                            absMax = absd;
                            absMaxId = j;
                        }
                        if(absd < absMin){
                            absMin = absd;
                            absMinId = j;
                        }
                        idx++;
                    }
                }
            }
        }
    }

    if(rangeMax == -INFINITY || rangeMin == INFINITY ||
        absMax == -INFINITY || absMin == INFINITY ||
        std::isnan(rangeMax) || std::isnan(rangeMin)){
        retInfo = "Warning: Color data is all NAN!";
        return retInfo;
    }
    m_setMax = rangeMax, m_setMin = rangeMin, m_setAbsMax = absMax, m_setAbsMin = absMin;
    m_setMaxId = maxId, m_setMinId = minId, m_setAbsMaxId = absMaxId, m_setAbsMinId = absMinId;
    return retInfo;
}

void ReportVTK::PaintArrow(double *startPoint, double *endPoint)
{
    // Set the background color.
    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    // std::array<unsigned char , 4> bkg{{26, 51, 77, 255}};
    // colors->SetColor("BkgColor", bkg.data());

    //创建圆锥体
    vtkSmartPointer<vtkCylinderSource> cylinderSource =
        vtkSmartPointer<vtkCylinderSource>::New();
    cylinderSource->SetResolution(15);
    cylinderSource->SetRadius(0.1);

    auto rng = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
    rng->SetSeed(8775070);

    //设置旋转坐标系
    double normalizedX[3], normalizedY[3], normalizedZ[3], *reverseX;
    //x轴，始终点方向
    vtkMath::Subtract(endPoint, startPoint, normalizedX);
    double length = vtkMath::Norm(normalizedX);
    vtkMath::Normalize(normalizedX);
    reverseX = normalizedX;
    vtkMath::MultiplyScalar(reverseX, -1);
    //z轴，任意方向
    double arbitrary[3];
    for (auto i = 0; i < 3; ++i)
    {
        rng->Next();
        arbitrary[i] = rng->GetRangeValue(-10, 10);
    }
    vtkMath::Cross(normalizedX, arbitrary, normalizedZ);
    vtkMath::Normalize(normalizedZ);
    //y轴
    vtkMath::Cross(normalizedZ, normalizedX, normalizedY);

    //创建方向余弦矩阵
    vtkSmartPointer<vtkMatrix4x4> matrix =
        vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity();
    for (unsigned int i = 0; i < 3; i++)
    {
        matrix->SetElement(i, 0, normalizedX[i]);
        matrix->SetElement(i, 1, normalizedY[i]);
        matrix->SetElement(i, 2, normalizedZ[i]);
    }

    //应用变换
    vtkSmartPointer<vtkTransform> transform =
        vtkSmartPointer<vtkTransform>::New();
    transform->Translate(startPoint);   // 设置起点
    transform->Concatenate(matrix);     // 设置余弦矩阵
    transform->RotateZ(-90.0);          // 将圆柱体的方向平行与X轴
    transform->Scale(1.0, length, 1.0); // 沿着高度向量缩放
    transform->Translate(0, .5, 0);     // 平移到起点

    //转换为polyData
    vtkSmartPointer<vtkTransformPolyDataFilter> transformPD =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformPD->SetTransform(transform);
    transformPD->SetInputConnection(cylinderSource->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
//是否将旋转坐标系应用到全局
#ifdef USER_MATRIX
    mapper->SetInputConnection(cylinderSource->GetOutputPort());
    actor->SetUserMatrix(transform->GetMatrix());
#else
    mapper->SetInputConnection(transformPD->GetOutputPort());
#endif
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d("Cyan").GetData());
    m_renderer->AddActor(actor);

    //创建圆锥体源对象
    vtkSmartPointer<vtkConeSource> coneSource = vtkSmartPointer<vtkConeSource>::New();
    coneSource->SetRadius(0.2);          // 圆锥体底部半径
    coneSource->SetHeight(0.4);          // 圆锥体高度
    coneSource->SetResolution(20);       // 圆锥体的分辨率
    coneSource->SetCenter(1,1,1);
    coneSource->SetDirection(reverseX);

    auto coneSourceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    coneSourceMapper->SetInputConnection(coneSource->GetOutputPort());
    auto coneSourceActor = vtkSmartPointer<vtkActor>::New();
    coneSourceActor->SetMapper(coneSourceMapper);
    coneSourceActor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
    m_renderer->AddActor(coneSourceActor);
}

QVector<int> ReportVTK::GetCellPointsId(int id)
{
    vtkSmartPointer<vtkCell> selectedCell = m_ugrid->GetCell(id);
    vtkSmartPointer<vtkIdList> pointIds = selectedCell->GetPointIds();
    QVector<int> rt;
    for (vtkIdType i = 0; i < pointIds->GetNumberOfIds(); ++i) {
        rt.push_back(pointIds->GetId(i));
    }
    return rt;
}
