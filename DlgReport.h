#pragma once

#include "DlgReportUtil.h"

#include <QTemporaryDir>
#include <QTableWidget>
#include "Utils/GFE_Dialog.h"

namespace Ui {
class DlgReport;
class DlgReport_CombFactor;
class DlgReport_Type;
}

class QAxObject;
class GFE_ProgDlg_Simple;
class QTreeWidgetItem;

class DlgReport : public GFE_Dialog{
    Q_OBJECT
public:
    using CombType = GFECaculate::PJInput::CombType;
    using CaseType = GFECaculate::PJInput::CaseType;

    explicit DlgReport(QWidget* parent = nullptr);
    ~DlgReport();

    void Init();

    //window
    void ShowReportType();
    void ToDlgCF();
    void ToDlgReport();
    virtual void accept() override;

    //helper
    void Test();
    int InfoType(QString info);
    QString NewBMPrefix(int dbsId);

    static QVector<QPair<QVector<double>, CombType>> GetDefaultCombination(const QVector<QPair<QString, CaseType>>&);

private:
    /* ui */
    //根据选择的报告类型，
    void InitByReportType();
    //反应未依法截面属性树型item转移函数
    void TreeItemCheckStateChange(QTreeWidgetItem*, int col);
    //选择DB组合主文件夹
    void OnMainDirBeSet();
    /* 多DB组合窗口 */
    //文件路径多DB组合表内存副本和窗口表之间的转移
    void DBSTypeChange_File(int i);
    //组合系数多DB组合表内存副本和窗口表之间的转移
    void DBSTypeChange_CombFactor(int i);
    //表内存副本和窗口表之间的复制
    void CopyTable(QTableWidget* from, QTableWidget* to);

    /* 报告程序初始化 */

    //报告word模板初始化
    QString WordTextInit();
    //有效书签列表初始化
    QString ValidBMInit();
    //DB参数初始化
    QString DBAndModelInit();
    //GFE计算初始化
    QString CalculateInit();
    //计算报告书初始化
    QString GeneratorInit();

    /* 生成报告开始 */

    //启动
    void OnGenerateReport();
    //读取有效书签，根据书签名在模板书签位置插入对应数据
    QString StartGenerate();
    //报告生成结束，回收资源
    QString GeneratorFinish();

    /* --- 向模板对应书签位置插入数值数据 --- */
    /* 插入数值 */
    QString ExportNumber(const ReportBookMark& bm);
    //内力数值
    QString ExportInterForceNumber(const ReportBookMark& bm);
    //层间位移角数值
    QString ExportSDRNumber(const ReportBookMark& bm);
    //轴力数值
    QString ExportACRNumber(const ReportBookMark& bm);
    //模型数值
    QString ExportModelNumber(const ReportBookMark& bm);

    QString ExportTestNumber(const ReportBookMark& bm);
    QString ExportTestTempNumber(const ReportBookMark& bm);



    /* --- 向模板对应书签位置插入折线图 --- */
    QString ExportCurveChart(const ReportBookMark& bm);

    /* 楼层层间相关曲线 */
    QString ExportStoreyCurveChart(const ReportBookMark& bm);
    //层间位移角包络曲线
    QString ExportSDREnvelopeCurveChart(const ReportBookMark& bm);
    //层间位移角时程曲线
    QString ExportSDRTimeCurveChart(const ReportBookMark& bm,const QString& dir1,const QString& dir2);
    //楼层位移曲线
    QString ExportStoreyDispCurveChart(const ReportBookMark& bm);
    //楼层位移时程曲线
    QString ExportSDRUDiffTimeCurveChart(const ReportBookMark& bm);
    //层间剪力
    QString ExportStoreyShearForceCurveChart(const ReportBookMark& bm);

    /* 地震动曲线 */
    QString ExportVibCurveChart(const ReportBookMark& bm);
    //地震动时程
    QString ExportVibTimeCurveChart(const ReportBookMark& bm);
    //E2地震动时程曲线
    QString ExportVibE2TimeCurveChart(const ReportBookMark& bm);
    //地震波按深度变化曲线
    QString ExportVibEERACurveChart(const ReportBookMark& bm);
    //反应谱
    QString ExportVibSpectrumCurveChart(const ReportBookMark& bm);
    //能量图
    QString ExportEnergyCurveChart(const ReportBookMark& bm);

    /* 幅值函数 */
    QString ExportAMPCurveChart(const ReportBookMark& bm);
    //地震波幅值曲线
    QString ExportAMPSeismWaveCurveChart(const ReportBookMark& bm);

    /* DB组合折线图 */
    QString ExportCombCurveChart(const ReportBookMark& bm);
    //轴压比散点图
    QString ExportCombACRCurveChart(const ReportBookMark& bm);

    /* XYData折线图 */
    QString ExportXYDataCurveChart(const ReportBookMark& bm);
    //指挥曲线
    QString ExportXYDataHystereticCurveChart(const ReportBookMark& bm);



    /* --- 向模板对应书签位置插入表格数据 --- */
    QString ExportTable(const ReportBookMark& bm);

    /* 材料参数表 */
    QString ExportMatTable(const ReportBookMark& bm);
    //墙、柱、梁、板材料表
    QString ExportMatLayerTable(const ReportBookMark& bm);

    /* 模型参数表 */
    QString ExportModelTable(const ReportBookMark& bm);
    //模型基本参数表
    QString ExportModelParamTable(const ReportBookMark& bm);

    /* 土体参数表 */
    QString ExportSoilTable(const ReportBookMark& bm);
    //土体材料参数
    QString ExportSoilLayerTable(const ReportBookMark& bm);

    /* 土动信息表 */
    QString ExportSoilDynaTable(const ReportBookMark& bm);
    //土动参数信息表
    QVector<QStringList> ExportSoilDynaMatParamTable(const ReportBookMark& bm,std::shared_ptr<GFE::DB> db,const QSet<QString>& mats);
    //土层分布信息表
    QVector<QStringList> ExportSoilDynaSoilInfoTable(const ReportBookMark& bm,const GFE::SoilLayer& soil);

    /* 构件信息表 */
    QString ExportCompTable(const ReportBookMark& bm);
    // 构件内力表
    QVector<QStringList> ExportCompInterForceTable(const ReportBookMark& bm,const std::map<std::pair<std::string, std::string>, std::vector<std::pair<int, double>>>& eleForce);

    /* 截面属性信息表 */
    QString ExportPropTable(const ReportBookMark& bm);
    //截面配筋表
    QVector<QStringList> ExportPropPJTable(const ReportBookMark& bm);

    //工况组合信息表
    QString ExportCombListTable(const ReportBookMark& bm);

    //地震工况信息表
    QString ExportVibCasesTable(const ReportBookMark& bm);

    /* 层间位移角信息表 */
    QString ExportSDRTable(const ReportBookMark& bm);
    //层间位移角包络表
    QString ExportSDREnvelopeTable(const ReportBookMark& bm,const int& layer,const int& denoLimit,const QString& dir1,const QString& dir2);
    //E2地震下层间位移角包络表
    QString ExportSDRE2Table(const ReportBookMark& bm);
    //楼层位移表
    QString ExportSDRDispTable(const ReportBookMark& bm);

    /* 楼面信息表 */
    QString ExportFloorTable(const ReportBookMark& bm);
    //楼层加速度表
    QString ExportFloorAccelerationTable(const ReportBookMark& bm);



    /* --- 向模板对应书签位置插入图片数据 --- */
    QString ExportPicture(const ReportBookMark& bm);

    /*插入模型图片*/
    QString ExportModelPicture(const ReportBookMark& bm);
    //模型单元集图片
    QString ExportModelElemSetPicture(const ReportBookMark& bm);
    //模型高亮图片
    QString ExportModelHighLightPicture(const ReportBookMark& bm);



    /* --- 向模板对应书签位置插入云图数据 --- */
    QString ExportCloudChart(const ReportBookMark& bm);

    /* 组合数据云图 */
    QString ExportCombCloudChart(const ReportBookMark& bm);
    //组合内力云图
    QString ExportCombInterForceCloudChart(const ReportBookMark& bm, const QString &forceName,std::vector<std::pair<int,double>> &eleForce);
    //组合轴压比云图
    QString ExportCombACRCloudChart(const ReportBookMark& bm);

    //地震作用下位移云图
    QString ExportVibrationCloudChart(const ReportBookMark& bm);

    /* 损伤相关云图 */
    QString ExportDamageCloudChart(const ReportBookMark &bm);
    //墙、柱、梁损伤云图
    QString ExportDamageCCloudChart(const ReportBookMark &bm);
    //损伤等级云图
    QString ExportDamageLevelCloudChart(const ReportBookMark &bm);

private:
    /* 窗口 */
    GFE_Dialog* dlg_cf; //组合系数窗口
    GFE_Dialog* dlg_t; //报告类型选择列表窗口
    Ui::DlgReport* ui; //生成报告主窗口ui
    Ui::DlgReport_CombFactor* ui_cf; //组合系数窗口ui
    Ui::DlgReport_Type *ui_t; //报告类型选择窗口ui
    QVector<QTableWidget*> m_dbsTables, m_cfTables; //DB组合表、组合系数表
    QVector<QString> m_dbsMainDir;   // DB组合主目录

    /* 报告 */
    QString m_reportType; //报告类型

    /* GFE DB信息 */
    int m_nDBS = 0; //DB组合数
    int m_baseCnt = 0; //地下室层数
    int m_allStyCnt = 0; //所有楼层数
    QStringList m_dbsTypes; //DB组合类型
    int m_curDBSId = 0; //当前DB组合id
    //todo
    QMap<QString, int> m_propName2ID; //截面属性名映射构件ID
    QMap<int, GFECaculate::CombStruct> m_css; //DB组合结构
    //todo
    QString m_section = "1", m_acrFit = "", m_sdr1Fit = "", m_sdr2Fit = "";

    /* 模型信息 */
    struct ModelParam{
        qint32 dim; //模型维度
        qint32 nodeNum; //节点数
        qint32 elemNum;//单元数
        std::vector<GFE::id_t> elmSubType; //???
    };
    QMap<int, ModelParam> m_mp;//模型信息

    /* 文件路径  */
    QString m_sDir; //DB组合
    QString m_reportDir;  //报告的文件夹
    QString m_reportPath; //输出报告doc的路径
    QTemporaryDir m_tDir; //临时目录路径

    /* 接口 */
    ReportWord::sPtr m_word; //word接口
    QVector<ReportBookMark> m_bmList; //有效书签列表
    QMap<int, ReportVTK::sPtr> m_vtks; //VTK渲染器，每个DB组合ID对应一个VTK
    QMap<int, GFECaculate::sPtr> m_caculates; //GFE计算
    GFE_ProgDlg_Simple* m_progDlg; //进度条
    ReportPlot::sPtr m_plot; //python matplotlib折线图
};
