#ifndef DLGREPORTUTIL_H
#define DLGREPORTUTIL_H

#include <QString>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QStandardItem>

#include <ActiveQt/QAxBase>
#include <ActiveQt/QAxObject>

#include <vtk-8.2/vtkSelection.h>
#include <vtk-8.2/vtkRenderer.h>
#include <vtk-8.2/vtkRenderWindow.h>
#include <vtk-8.2/vtkRenderWindowInteractor.h>
#include <vtk-8.2/vtkUnstructuredGrid.h>
#include <vtk-8.2/vtkLookupTable.h>
#include <vtk-8.2/vtkScalarBarWidget.h>
#include <vtk-8.2/vtkDataSetMapper.h>
#include <vtk-8.2/vtkIntArray.h>
#include <vtk-8.2/vtkPolyData.h>
#include <vtk-8.2/vtkExtractSelection.h>

#include "GFE_API.h"

class DlgReportUtil : public QObject
{
    Q_OBJECT
public:
    DlgReportUtil();
    ~DlgReportUtil();

    struct Fraction {
        int numerator;
        int denominator;
    };

    static const QMap<QString,QStringList>& ReportType();
    static const QHash<QString,QString>& Type2BMName();

    //String
    static QString GenerateRandomString(int length);
    static bool IsMatch(QString input, QString pattern);
    static int Stoi(const std::string& str);
    static bool IsParentChildPaths(const QString& parentPath, const QString& childPath);
    static QString GetParentDirectory(const QString& path);

    //Math
    static void FloatToReciprocal(double number, int& numerator, int& denominator);
    static std::string FloatToReciprocal(double number);
    template <class T>
    static bool IsEqual(T a,T b) {
        double epsilon = 1e-8;
        return std::abs(a - b) < epsilon;
    }
    template <class T>
    static std::vector<T> VectorUnion(const std::vector<T>& v1, const std::vector<T>& v2) {
        std::vector<T> result;
        result.reserve(v1.size() + v2.size());

        result.insert(result.end(), v1.begin(), v1.end());
        result.insert(result.end(), v2.begin(), v2.end());

        std::sort(result.begin(), result.end());

        result.erase(std::unique(result.begin(), result.end(),IsEqual<T>), result.end());

        return result;
    }
    static bool IsFloat(const std::string& str);
    static QString ConnectStrs(const QStringList& strs, const QString& delimiter);
private:

};


class ReportBookMark
{
public:
    typedef enum InsertType{
        UNKNOWInsertType = -1,
        Number,
        Text,
        Table,
        CurveChart,
        Picture,
        CloudChart,
    }InsertType;

    typedef enum GFEType{
        UNKNOWGFEType = -1,
        Test,
        Damage,//损伤
        Model,//模型信息
        Property,//截面属性
        SDR,    //层间位移角
        Storey,  //层间信息
        Floor, //楼面信息
        SoilDyna,//土动信息
        Soil, //土层信息
        AMP,//幅值函数
        Vibration,//地震场地反应信息
        Comb, //组合信息
        Comp, //构件信息
        Material, //材料信息
        XYData, //xyData
    }GFEType;

    typedef enum Param{
        UNKNOWParam = -1,
        ACR,//轴压比
        Attribute, //属性
        E2X,//X向E2地震
        E2Y,//Y向E2地震
        EERA,//地震场地反应分析结果
        ElemSet,//单元集
        Elem, //模型单元信息
        Energy, //能量
        PJ,//配筋信息
        SeismWave,//地震波
        Time,//时程
        Max,//最大值
        Node,//模型节点信息
        DLQ,//地连墙
        Jiegou,
        List, //列表信息
        MatParam, //土动参数信息
        SoilInfo,
        Acce,
        Frame,
        InterForce,
        ShearForce,
        Overall,
        AxialForce,
        Bending,
        Dim2,
        Dim3,
        Envelope,//包络信息
        LastFrame,//最后一帧
        Wall,//墙
        Col,//柱
        Beam,//梁

        Damagec,
        Level,//等级

        Spectrum,//反应谱
        Case, //工况

        Displacement, //位移
        Acceleration, //加速度

        Layer, //土层、楼层

        UDiff, //位移时程差
        HighLight, //高亮
        Hysteretic, //滞回曲线

        Temp,
    }Param;

    typedef std::shared_ptr<ReportBookMark> sPtr;
    ReportBookMark();
    ReportBookMark(QString bmName);
    ~ReportBookMark();

public:
    static InsertType StrToInsertType(const std::string& str);
    static GFEType StrToGFEType(const std::string& str);
    static Param StrToParam(const std::string& str);

    ReportBookMark GetBM(QString bmName);
    static bool isValid(QString bmName);
    static qint32 GetDBSID(QString bmName);
    static const QHash<InsertType, QHash<GFEType, QSet<Param>>>& ValidBMs();
public:
    QString m_bmName;
    qint32 m_dbsID;
    InsertType m_insertType;
    GFEType m_gfeType;
    Param m_param;
    QVector<QString> m_extra;
private:
};

class ReportWord{
public:
    typedef std::shared_ptr<ReportWord> sPtr;
    ReportWord(QString wordPath,QString m_outPath);
    ~ReportWord();

    static QHash<QString,QHash<QString,QString>> Attributes;

    QString Save();

    /*书签*/
    QString AddNewBM(const QString& oldBmName,const QString& newBmName,const QString &text = "",const QString& textType = "");
    QAxObject* GetBM(QString bmName);

    /*文字*/
    QString WriteText(const QString &bmName, const QString &text,const QString& type = "");
    QString FindReplace(QString findText,QString replaceText);

    /*表格*/
    QAxObject* GetTable(QString bmName);
    QString AddTable(QString bmName,const QStringList& header,const QVector<QStringList>& data);//有表头
    QString AddTable(QString bmName,const QVector<QStringList>& data);//无表头
    QString TableAddRow(QAxObject* table, QStringList info);
    QString TableAddCol(QAxObject *table, QStringList info);
    QString TableMergeCol(QAxObject *table,int colNum, int rowStart = INT_MAX,int rowEnd = INT_MIN);
    QString TableMergeRow(QAxObject *table,int rowNum);
    QString TableWriteCell(QAxObject *table,int row,int col,const QString& text);

    /*图片*/
    QString AddPicture(QString bmName, const QString &pngPath, int width = 280, int height = 280);

    /*折线图*/
    QString AddChart(QString bmName, QVector<QPair<QVector<double>,QVector<double>>> coordDatas, const QString &xName, const QString &yName,
                     int type = 75, const QString& xType = "",const QString& yType = "");
    QString AddChart(QString bmName, std::vector<std::pair<std::vector<double>,std::vector<double>>> coordDatas, const QString &xName, const QString &yName,
                     int type = 75, const QString& xType = "",const QString& yType = "");

public:
    QString m_outPath;
    std::shared_ptr<QAxObject> m_word;
    std::shared_ptr<QAxObject> m_documents;
    std::shared_ptr<QAxObject> m_document;
};

class GFECaculate{
public:
    typedef std::shared_ptr<GFECaculate> sPtr;

    GFECaculate();
    ~GFECaculate();

public:
    struct Option{
        bool NoComb = false;//是否不组合计算
        bool NoSDR = false; //是否不计算层间信息
    };

    /*GFE计算*/
    //反应位移法和三维时程分析组合计算
    struct CombStruct{
        int dbsId;//DB组合ID
        QString dbsName; //DB组合名
        QString dbsDir;  //DB组合目录
        QStringList dbPathList; //DB路径列表
        QStringList gjdyList;   //gjdy列表
        QStringList caseNames;  //工况名
        bool isSimplified;      //是否简化
        QList<int> acrLayer, sdrLayer; //轴力楼层和位移角楼层
        QList<QStringList> combList;   //组合列表
        std::vector<int> caseTypeArr;  //工况类型数组
        QVector<std::shared_ptr<GFE::DB>> dbList;  //DB列表
        QHash<qint32,QString> gjID2GjName; //构件ID转构件名称
        QHash<QString,QSet<int>> compsElemID; //构件单元组成
    };

    // 用于组合内力、轴压比、配筋计算的数据结构以及函数（CalcPJ）
    using AcrResult = std::map<std::pair<int, int>, double>;
    using Str2 = std::pair<std::string, std::string>;
    struct PJResult {
        std::vector<std::array<AcrResult, 2>>  acr;
        std::map<Str2, std::vector<std::pair<int, double>>> eleForce;
        QVector<QStringList> gjPj;
    };
    struct PJInput {
        enum CombType{
            Basic,
            Standard,
            Frequent,
            QuasiPermanent
        };
        enum CaseType {
            HengZai,
            HuoZai,
            Dynamic,
            Ignore
        };
        std::vector<Str2> files;        // {db路径，gjdy路径}
        std::vector<std::pair<std::vector<double>, int>> combFactor;            // {组合系数，组合类型}
        std::vector<int> acrStorey;                                             // 需要计算轴压比的楼层
        bool isSimplified;                                                      // 是否采用简化计算
        std::vector<int> caseType;                                              // 各个工况的类型（恒载，活载，动力）；用于判断是否为拟静力工况

        // 用于二维模型的配筋计算
        std::vector<int> slabProp;                                           // 属于板的截面属性ID
        std::vector<int> wallProp;                                           // 属于墙的截面属性ID
    };
    static PJResult CalcPJ(const PJInput& input);
    //计算组合配筋和内力结果
    QString GFECombPJCalculate(CombStruct& cs);

    //层间位移角计算
    void GetSDRInfo(QStringList dbPathList);

    //获取层间位移角包络
    void GetSDREnvelope(QString sdrPath, int& layerMax, double& sdrMax, QVector<int>&& layer = {}, QVector<double>&& sdr = {});

    //取场输出包络类型帧
    int GetEnvelopeFrame(std::shared_ptr<GFE::DB> db, QString var, QString frameStr);

    //自定义计算包络数据
    std::vector<GFE::data_t> GetEnvelopeData(std::shared_ptr<GFE::DB> db,const QString& variable,const QString& frameStr);

    //获取DB的弹塑性类型
    static int GetElastoplasticity(std::shared_ptr<GFE::DB> db);

public:
    PJResult m_pjResult;
    QString m_maxSF1Comb, m_maxSF2Comb, m_maxSM1Comb;
    GFE::data_t m_maxSF1 = 0,m_maxSF2 = 0,m_maxSM1 = 0;
    int m_dim = 2,m_layer = 0,m_denoLimit = 0;
    QString m_sdrDir1,m_sdrDir2;
    std::vector<int> m_vSlabProp,m_vWallProp; //二维模型，按板配筋的梁和柱构件的属性，property_[i]的集合
    Option m_opt;
};

class ReportPlot{
public:
    typedef std::shared_ptr<ReportPlot> sPtr;
    ReportPlot(const QString& outputDir);
    ~ReportPlot();

    static void PyRun(const std::string &runStr);

    /**
     * @brief 绘制能量图
     * @param type=0:所有折线横坐标轴相同;type=1:所有折线纵坐标轴相同;
     * @return 返回能量图保存路径
     */
    QString PrintEnergyCurveChart(const QVector<QVector<double>>& xDatas,const QVector<QVector<double>>& yDatas,
                                  const QString& xAxisName, const QString& yAxisName,
                                  const QStringList& areaNames, int type, int pngWidth, int pngHeight);

    /**
     * @brief 绘制折线图
     * @param type=0:所有折线横坐标轴相同;type=1:所有折线纵坐标轴相同;
     * @return 返回折线图保存路径
     */
    QString PrintCurveChart(const QVector<QVector<double>>& xDatas,const QVector<QVector<double>>& yDatas,
                            const QString& xAxisName, const QString& yAxisName,
                            const QStringList& lineNames, int type = 0,
                            int pngWidth = 1920, int pngHeight = 1080);

    QString m_outputDir; //折线图保存目录
    QString m_curExePath; //python解释器执行路径
    QStringList m_colors; //颜色列表

private:

    /* 将C++字符串或容器转换为python执行语句 */
    std::string keyWords2ArgsStr(std::unordered_map<std::string,std::string>& keyWords);
    std::string vector2PyStr(const std::vector<double>& vec, std::string name);
    std::string strVec2PyStr(const std::vector<std::string>& strVec, std::string name);
    std::string list2PyStr(std::vector<std::string> strs, std::string name);
    std::string plotStr(std::string xName,std::string yName,std::unordered_map<std::string,std::string>& keyWords);
    std::string fillBetweenStr(std::string xName,std::string y1Name, std::string y2Name,std::unordered_map<std::string,std::string>& keyWords);
    std::string patchStr(std::unordered_map<std::string,std::string>& keyWords);

    int m_cnt = 0; //已输出图片总数
};


class ReportVTK{
public:
    //    friend class CustomCommand;
    typedef std::shared_ptr<ReportVTK> sPtr;

    static QMap<QString,QVector<double>> Color;

    typedef struct Option{
        Option(){
            isMesh = false;
            isCloud = false;
            isLabel = false;
            isZoom = false;
            zoomFactor = 1.0;
            isCell = false;
            isInteract = false;
            isDiscrete = false;
        }

        bool isMesh;  //是否展示划分网格
        bool isCloud; //是否是云图
        bool isLabel; //是否在图上显示标签
        bool isZoom; //是否放大
        double zoomFactor; //缩放倍数
        bool isCell;  //是否显示单元信息
        bool isInteract; //是否不显示交互窗口
        bool isDiscrete; //颜色数据是否离散
        bool isHighLight; //是否高亮
        bool isExtreme; //是否最值标记
    }Option;

    typedef enum ViewMode
    {
        Unknow,
        Default,
        Axo,
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom,
        Fit,
        Parallel,
        Perspective,
        RotateLeft,
        RotateRight,
        RotateUp,
        RotateDown,
        RotateCounter,
        RotateClock,
    }ViewMode;

    /**
     * @brief ReportVTK构造
     * @param db：GFE DB
     * @param width：渲染窗口宽度
     * @param height：渲染窗口高度
     */
    ReportVTK(std::shared_ptr<GFE::DB> db, int width = 1920, int height = 1080);
    /**
     * @brief ReportVTK构造，使用VTK导出折线图用
     * @param opt：选项
     */
    ReportVTK(Option *opt);
    ~ReportVTK();

    //初始化
    QString Init();
    /**
     * @brief 调整相机，设置全局坐标轴
     * @param angle：相机视角
     */
    QString Start(const QString& angle = "default");
    //加载模型
    QString LoadModel();
    //选项初始化
    QString OptInit(Option *opt);

    /**
     * @brief vtkRenderer渲染
     * @param barTitle：色标带标题
     * @param data：标量数据
     * @param mapperType：映射方式，origin-原始， banded-平滑
     * @param cm：离散数据颜色映射
     */
    QString Rendering(const QStringList& setNames = {},
                      const QString& barTitle = "", const std::vector<float> &data = {}, const QString& mapperType = "banded",
                      const QMap<int, QColor>& cm = {});

    /**
     * @brief 向单元或点设置标量数据
     * @param type：等于0，连续性；等于1，离散型
     * @param data：离散型标量数据
     * @param cm：离散数据颜色映射
     */
    QString SetScalar(int type, const std::vector<float> &data = {}, const QMap<int, QColor>& cm = {});

    /**
     * @brief 获取标量数据的lookuptable
     * @param type：等于0，连续性；等于1，离散型
     * @param data：离散型标量数据
     * @param cm：离散数据颜色映射
     */
    std::pair<vtkSmartPointer<vtkDataArray>,vtkSmartPointer<vtkScalarsToColors>> GetScalar(int type, const std::vector<float> &data = {}, const QMap<int, QColor>& cm = {});

    /**
     * @brief 调整相机视角
     * @param mode：相机视角
     */
    void SetViewAction(int mode);

    /**
     * @brief 根据单元集名返回含有属于该单元集所有ID的vtkSelection
     * @param elementSetName：单元集名
     * @return 含有属于该单元集所有ID的vtkSelection
     */
    vtkSmartPointer<vtkSelection> ElementSelection(const QString& elementSetName = "");

    /**
     * @brief 根据单元ID返回m_uGrid中含有这些单元的vtkExtractSelection
     * @param elemIds：单元ID数组
     * @return m_uGrid中含有这些单元的vtkExtractSelection
     */
    vtkSmartPointer<vtkExtractSelection> ElementSelection(const QVector<int>& elemIds);

    /**
     * @brief 根据单元集名列表返回m_uGrid中含有这些单元的vtkExtractSelection
     * @param elemIds：单元ID数组
     * @return m_uGrid中含有这些单元的vtkExtractSelection
     */
    vtkSmartPointer<vtkExtractSelection> SetsSelection(const QStringList& setNames);

    /**
     * @brief 高亮单元集
     * @param eExt：m_uGrid中含有指定单元的vtkExtractSelection
     * @param baseSetNames：背景单元集
     * @param data：标量数据
     */
    QString HighLight(vtkSmartPointer<vtkExtractSelection> eExt,
                      const QStringList& baseSetNames = {},
                      const std::vector<GFE::data_t>& data = {});

    //添加标签
    QString AddLabel();

    /**
     * @brief VTK渲染并输出图片
     * @param pngPath：图片输出路径
     * @param opt：选项
     * @param setNames：单元集名列表
     * @param barTitle：色标带标题
     * @param data：标量数据
     * @param mapperType：映射方式，origin-原始， banded-平滑
     * @param cm：离散数据颜色映射
     * @param angle；相机视角
     */
    QString OffScreenRendering(const QString& pngPath, Option *opt, const QStringList& setNames = {},
                               const QString& barTitle = "", const std::vector<float> &data = {},
                               const QMap<int, QColor>& cm = {}, const QString& mapperType = "banded",
                               const QString& angle = "default");

    /**
     * @brief VTK渲染并输出图片(含高亮时背景单元集）
     * @param pngPath：图片输出路径
     * @param opt：选项
     * @param setNames：单元集名列表
     * @param baseSetNames：背景单元集名
     * @param barTitle：色标带标题
     * @param data：标量数据
     * @param mapperType：映射方式，origin-原始， banded-平滑
     * @param cm：离散数据颜色映射
     * @param angle；相机视角
     */
    QString OffScreenRendering(const QString& pngPath, Option *opt, const QStringList& setNames,
                               const QStringList& baseSetNames, const std::vector<GFE::data_t>& data = {},
                               const QString& extremeStr = "", const QString& angle = "default");

    QString ExportChartXY(const QVector<QPair<QVector<double>,QVector<double>>>& coordDatas,const QString& xName,const QString& yName,const QString& pngPath);


    //最值标记
    QString MaximumValueTag(const QString& variable, const QString& extremeStr);

    //helper
    QString WritePicture(const QString& pngPath);
    static vtkSmartPointer<vtkIntArray> IntArrayFilter(vtkDataArray* arr, vtkDataArray* ids, double fac = 1);
    static vtkSmartPointer<vtkFloatArray> FloatArrayFilter(vtkDataArray* arr, vtkDataArray* ids, double fac = 1);
    static vtkSmartPointer<vtkStringArray> StringArrayFilter(vtkDataArray* arr, vtkDataArray* ids, std::string extremeStr, double fac = 1);
    static ViewMode Str2ViewMode(const QString& angle);
    void GetCellCenter(vtkCell* cell, double center[3]);
    QString GetOptimalValue(const std::vector<GFE::data_t>& data, const QStringList& setNames);

    //painter
    void PaintArrow(double *startPoint, double *endPoint);
    QVector<int> GetCellPointsId(int id);

    template<typename T>
    vtkSmartPointer<vtkAOSDataArrayTemplate<T>> StdVecToVtkArray(const std::vector<T>& _v) {
        auto RET = vtkSmartPointer<vtkAOSDataArrayTemplate<T>>::New();
        RET->Allocate(_v.size());
        for(const auto& i : _v)
            RET->InsertNextTuple1(i);
        return RET;
    }
public:
    std::vector<int> m_nodeLabel2Id, m_elementLabel2Id, m_nodeId2Label, m_elementId2Label;
    float m_setMax = -INFINITY, m_setMin = INFINITY, m_setAbsMax = 0, m_setAbsMin = INFINITY;
    int m_setMaxId = -1, m_setMinId = -1, m_setAbsMinId = -1, m_setAbsMaxId = -1;

private:
    vtkSmartPointer<vtkActor> m_curActor;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkRenderWindow> m_rendererWindow;

    vtkSmartPointer<vtkUnstructuredGrid> m_ugrid;
    vtkSmartPointer<vtkLookupTable> m_lookupTable;
    vtkSmartPointer<vtkScalarBarWidget> m_scalarBarWgt;
    vtkSmartPointer<vtkScalarBarRepresentation> m_scalarBarRep;
    QHash<QString, vtkSmartPointer<vtkPolyData>> m_polyDatas;

    std::shared_ptr<GFE::DB> m_db;
    int m_dim = -1;
    int m_nNode = 0, m_nElem = 0;
    Option *m_opt;
    float m_colorMax = -FLT_MAX,m_colorMin = FLT_MAX;

    static constexpr char NodeIdsName[] = "point_ids";
    static constexpr char NodeLabelsName[] = "inp_point_ids";
    static constexpr char ElementIdsName[] = "cell_ids";
    static constexpr char ElementLabelsName[] = "inp_cell_ids";
    static constexpr char ElementFaceIdsName[] = "eface_ids";
};

#endif // DLGREPORTUTIL_H
