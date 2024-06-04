#include "DlgReport.h"

#include "ui_DlgReport.h"
#include "ui_DlgReport_CombFactor.h"
#include "ui_DlgReport_Type.h"
#include "GFE_StrUtil.h"
#include "Utils/GFE_ComboBox.h"
#include "MemberPJ.h"
#include "GFE_FileInfo.h"
#include "GFE_Struct/GFE_AnaStep.h"
//#include "DlgFilter.h"
#include "Prog/GFE_ProgDlg_Simple.h"
//#include "Frame/Application.h"
//#include "GFE_API/include/Utility/GFE_StoreyShear.h"
//#include "MupFunc.h"
//#include "DamageLevel.h"
#include "gfe/GFE_GJDYReader.h"
#include <qfiledialog.h>

#include <QDebug>
#include <QFile>
#include <QDirIterator>
#include <QMessageBox>
#include <QMenu>
#include <QWidgetAction>
#include <QLockFile>
#include <QRegularExpressionMatch>

#include <unordered_set>

#define QD qDebug()<<"*********************"<<

#define BMWARN "Warning: No data at"
#define BMERR "Error: No data at "
#define DELETE_PTR(ptr) \
delete ptr; \
ptr = nullptr; \


DlgReport::DlgReport(QWidget* parent) :
    GFE_Dialog(parent),
    dlg_cf(new GFE_Dialog(parent)),
    dlg_t(new GFE_Dialog(parent)),
    ui(new Ui::DlgReport),
    ui_cf(new Ui::DlgReport_CombFactor),
    ui_t(new Ui::DlgReport_Type){

    ui->setupUi(this);
    ui_cf->setupUi(dlg_cf);
    ui_t->setupUi(dlg_t);

    Init();
}

DlgReport::~DlgReport()
{
    delete ui;
    delete ui_cf;
    delete ui_t;
    for(auto& t : m_dbsTables){
        delete t;
    }
    for(auto& t : m_cfTables){
        delete t;
    }
}

void DlgReport::Init()
{
    QFile qss(":/Res/QSS/DlgReport.qss");
    qss.open(QFile::ReadOnly);
    this->setStyleSheet(qss.readAll());
    qss.close();
    dlg_cf->setStyleSheet(this->styleSheet());
    dlg_t->setStyleSheet(this->styleSheet());
    ui->tbMainDir->setIcon(QPixmap(":Res/Images/1.0.0/open.png"));

    ui->sdr_limit->setText("1/550");
    QRegularExpressionValidator* validator2 = new QRegularExpressionValidator(QRegularExpression("^[0-9]+/[0-9]+$"), ui->sdr_limit);
    ui->sdr_limit->setValidator(validator2);
    ui->acr_limit->setText("0.85");
    ui->acr_limit->setValidator(new QDoubleValidator);

    connect(ui_t->btnNext, &QPushButton::clicked, this, [&](){
        dlg_t->hide();
        InitByReportType();
        this->show();
    });
    connect(ui->tbMainDir, &QToolButton::clicked, this, &DlgReport::OnMainDirBeSet);
    connect(ui_t->btnCancel,&QPushButton::clicked,this,[&](){
        dlg_t->hide();
    });

    QString guide = tr("Please Enter Layer Number\n"
                       "The number follows three kinds of input rules:\n"
                       "1. A single number.\n"
                       "   eg: \"10\" refers to the No.10 layer\n"
                       "2. Two numbers with a colon between them.\n"
                       "   eg: \"1:5\" refer to No.1 to No.5 layers.\n"
                       "3. Three numbers seperate by two colon.\n"
                       "   eg: \"1:5:2\" refer to No.1, No.3 and No.5 layer.\n"
                       "All the numbers must be integer greater than zero.\n"
                       "Any space, comma and line feed are accepted. But any other character is rejected.\n");
    ui->textEdit->setToolTip(guide);
    ui->textEdit_2->setToolTip(guide);

    auto MoveItem = [](QListWidget* from, QListWidget* to) {
        for(auto& item : from->selectedItems()) {
            auto row = from->row(item);
            to->addItem(from->takeItem(row));
        }
    };
    connect(ui->listTJ_Right1, &QPushButton::clicked, [&]{ MoveItem(ui->C, ui->listTJ_Wall); });
    connect(ui->listTJ_Right2, &QPushButton::clicked, [&]{ MoveItem(ui->listTJ_Beam, ui->listTJ_Slab); });
    connect(ui->listTJ_Left1, &QPushButton::clicked, [&]{ MoveItem(ui->listTJ_Wall, ui->C); });
    connect(ui->listTJ_Left2, &QPushButton::clicked, [&]{ MoveItem(ui->listTJ_Slab, ui->listTJ_Beam); });

    connect(ui->btnNext, &QPushButton::clicked, this, &DlgReport::ToDlgCF);
    connect(ui->btnCancel, &QPushButton::clicked,this, [this] {
        dlg_cf->hide();
        hide();
    });
    connect(ui_cf->btnPre, &QPushButton::clicked, this, &DlgReport::ToDlgReport);
    connect(ui_cf->btnOK, &QPushButton::clicked, this, &DlgReport::accept);
    connect(ui_cf->btnCancel, &QPushButton::clicked,this, [this]{
        dlg_cf->hide();
    });

    connect(ui->cbDBSType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DlgReport::DBSTypeChange_File);
    connect(ui_cf->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DlgReport::DBSTypeChange_CombFactor);

    ui->tabWidget->removeTab(1);
}

void DlgReport::ShowReportType()
{
//    Test();
    ui_t->typeList->clear();
    const auto& types = DlgReportUtil::ReportType()["Types"];
    ui_t->typeList->setSelectionMode(QAbstractItemView::SingleSelection);
    for(auto& type: types){
        QListWidgetItem* item = new QListWidgetItem(type);
        ui_t->typeList->addItem(item);
    }

    dlg_t->setWindowTitle(tr("Select report type"));
    dlg_t->show();
}

#include <vtk-8.2/vtkSmartPointer.h>
#include <vtk-8.2/vtkRenderer.h>
#include <vtk-8.2/vtkRenderWindow.h>
#include <vtk-8.2/vtkRenderWindowInteractor.h>
#include <vtk-8.2/vtkCubeSource.h>
#include <vtk-8.2/vtkPolyDataMapper.h>
#include <vtk-8.2/vtkActor.h>
#include <vtk-8.2/vtkProperty.h>

void DlgReport::Test(){
    auto db = GFE::open("C:/Users/GZYL-11/Desktop/高新园车站/Model-1-E2Yc/Model-1-E2Yc.db");
    ReportVTK::sPtr vtk(new ReportVTK(db));
    ReportVTK::Option opt;
    opt.isCell = true, opt.isHighLight = true, opt.isInteract = true, opt.isExtreme = true, opt.isLabel = true;
    std::vector<GFE::data_t> data(vtk->m_elementId2Label.size(), 0);
    data[11722] = 250;

    vtk->OffScreenRendering("", &opt, {""}, QStringList({"jiegou"}),data,"max");
}

void DlgReport::InitByReportType()
{
    auto& tree = ui->treeContent;
    tree->clear();

    std::function<QTreeWidgetItem*(const QString&)>
        Construct = [&Construct](const QString& text)
    {
        auto item = new QTreeWidgetItem;
        item->setText(0, text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);        // todo: 后续再ReportType里补上预设，不一定默认都是打开
        for(auto&& childTxt : DlgReportUtil::ReportType()[text])
        {
            auto childItem = Construct(childTxt);
            item->addChild(childItem);
        }
        return item;
    };
    // 控制树中父节点和子节点的级联关系
    connect(tree, &QTreeWidget::itemChanged, this, &DlgReport::TreeItemCheckStateChange);

    auto selectedType = ui_t->typeList->selectedItems();
    if(selectedType.empty())  {
        QMessageBox::critical(nullptr, tr("Error"), tr("Report type is not selected!"));
        return;
    }
    tree->addTopLevelItem(Construct(selectedType[0]->text()));

    QTreeWidgetItem* rootItem = tree->topLevelItem(0);
    m_reportType = rootItem->text(0);

    m_nDBS = 1;
    m_dbsTypes.clear();
    if(m_reportType == DlgReportUtil::tr(u8"Shock absorption")){
        m_nDBS = 2;
        for(int i = 0;i < m_nDBS;++i){
            m_caculates[i].reset(new GFECaculate);
            m_caculates[i]->m_opt.NoComb = true;
        }
        m_dbsTypes << tr("Frequent") << tr("Rare");
        ui->cbDBSTypeWidget->show();
        ui_cf->dbsWidget->show();
    }
    else {
        for(int i = 0;i < m_nDBS;++i){
            m_caculates[i].reset(new GFECaculate);
            m_caculates[i]->m_opt.NoComb = false;
        }
        m_dbsTypes << tr("Default");
        ui->cbDBSTypeWidget->hide();
        ui_cf->dbsWidget->hide();
    }

    if(m_reportType == DlgReportUtil::tr("Response displacement")) {
        ui->grpACR->hide();
        ui->pre_file_widget->show();
    }else{
        ui->pre_file_widget->hide();
    }


    // 因为ComboBox的addItems会触发currentIndexChanged，这几个表的初始化必须放在前面
    m_dbsMainDir.clear();
    m_dbsMainDir.resize(m_nDBS);
    m_dbsTables.clear();
    m_dbsTables.resize(m_nDBS);
    m_cfTables.clear();
    m_cfTables.resize(m_nDBS);

    ui->cbDBSType->clear();
    ui_cf->comboBox->clear();
    ui->cbDBSType->addItems(m_dbsTypes);
    ui_cf->comboBox->addItems(m_dbsTypes);

    tree->setHeaderHidden(true);
    tree->expandAll();
}

void DlgReport::TreeItemCheckStateChange(QTreeWidgetItem* item , int col)
{
    auto& tree = ui->treeContent;
    disconnect(tree, &QTreeWidget::itemChanged, this, &DlgReport::TreeItemCheckStateChange);
    auto parent = item->parent();
    bool isParent = !parent;
    auto checkState = (Qt::CheckState)item->data(col, Qt::CheckStateRole).toInt();
    if(isParent) {
        int nChild = item->childCount();
        for(int i = 0; i < nChild; i++)
            item->child(i)->setCheckState(0, checkState);
    }
    else {
        bool flag = true;
        Qt::CheckState flagState = checkState == Qt::Checked ? Qt::Unchecked :Qt::Checked;
        int nChild = parent->childCount();
        for(int i = 0; i < nChild; i++) {
            if(parent->child(i)->data(col, Qt::CheckStateRole).toInt() == flagState) {
                flag = false;
                break;
            }
        }
        if(flag) parent->setCheckState(0, checkState);
        else parent->setCheckState(0, Qt::PartiallyChecked);
    }
    connect(tree, &QTreeWidget::itemChanged, this, &DlgReport::TreeItemCheckStateChange);
}

void DlgReport::DBSTypeChange_File(int i)
{
    ui->lineEdit->setText(m_dbsMainDir[i]);

    // UI表数据缓存到内存表
    // 内存表的数据投射到UI表
    auto& table = ui->tableWidget;
    auto& tableDBSCur = m_dbsTables[i];
    auto& tableDBSLast = m_dbsTables[m_curDBSId];

    if(!tableDBSLast)
        tableDBSLast = new QTableWidget;
    // tableDBSCur为空时不创建，避免将空表复制给table

    CopyTable(table, tableDBSLast);
    if(i != m_curDBSId)
        CopyTable(tableDBSCur, table);
    m_curDBSId = i;
}

void DlgReport::DBSTypeChange_CombFactor(int i)
{
    // UI表数据缓存到内存表
    // 内存表的数据投射到UI表
    auto& table = ui_cf->tableWidget;
    auto& tableDBSCur = m_cfTables[i];
    auto& tableDBSLast = m_cfTables[m_curDBSId];

    CopyTable(table, tableDBSLast);
    if(i != m_curDBSId)
        CopyTable(tableDBSCur, table);
    m_curDBSId = i;
}

void DlgReport::CopyTable(QTableWidget* from, QTableWidget* to)
{
    if(!to)
        return;

    // from表是空指针，也要清空to表
    // 清除目标表格内容，不清除列表头
    to->clearContents();
    to->setRowCount(0);

    if(!from)
        return;

    // 设置目标表格的行数和列数
    auto nRow = from->rowCount();
    auto nCol = from->columnCount();
    to->setRowCount(nRow);
    to->setColumnCount(nCol);

    // 复制表头
    for(int i = 0; i < nCol; ++i)
        to->setHorizontalHeaderItem(i, new QTableWidgetItem(*from->horizontalHeaderItem(i)));

    for(int r = 0; r < nRow; ++r)
        for(int c = 0; c < nCol; ++c)
        {
            // 单独处理组合框逻辑
            auto cellWidget = from->cellWidget(r, c);
            if(cellWidget) {
                auto combo = qobject_cast<QComboBox*>(cellWidget);
                if(combo) {
                    auto comboNew = new QComboBox;
                    GFE_ComboBox::AlignCenter(comboNew);
                    for(int i = 0; i < combo->count(); i ++) {
                        comboNew->addItem(combo->itemText(i));
                        dynamic_cast<QStandardItemModel*>(comboNew->view()->model())->item(i)->setTextAlignment(Qt::AlignHCenter);
                    }
                    to->setCellWidget(r, c, comboNew);
                    comboNew->setCurrentIndex(combo->currentIndex());
                }
            }
            else {
                // 通用的复制逻辑
                auto item = from->item(r, c);
                if(item) {
                    to->setItem(r, c, new QTableWidgetItem(*item));
                }
            }
        }
}

void DlgReport::OnMainDirBeSet(){
    // 初始化
    ui->radioButton->setEnabled(false);
    ui->radioButton_2->setEnabled(false);
    ui->radioButton_3->setChecked(true);
    ui->radioButton_4->setEnabled(false);
    ui->radioButton_5->setEnabled(false);
    ui->radioButton_6->setChecked(true);

    auto sDir = SelectedDir();
    if(sDir.isEmpty())
        return;
    if(m_curDBSId == 0)m_sDir = sDir;
    ui->lineEdit->setText(sDir);
    QFileInfo fi(sDir);
    m_css[m_curDBSId].dbsName = fi.completeBaseName();
    m_css[m_curDBSId].dbsDir = sDir;

    using Str2 = QPair<QString, QString>;
    std::map<QString, std::vector<Str2>> allCase;
    int nCase = 0;

    //!搜索目录下的所有db文件和gjdy文件, 记录于allCase
    QDirIterator dirItr1(sDir, QStringList() << "*.db", QDir::Files, QDirIterator::Subdirectories),
                 dirItr2(sDir);
    while (dirItr1.hasNext()){
        auto dbPath = dirItr1.next();
        if(!dbPath.isEmpty()) {
            QFileInfo dbFile(dbPath);
            if(DlgReportUtil::IsParentChildPaths(sDir,DlgReportUtil::GetParentDirectory(dbPath))){
                allCase[dbFile.completeBaseName()].push_back({dbPath, ""});
            }
        }
    }
    while(dirItr2.hasNext()) {
        auto fileOrDir = dirItr2.next();
        QFileInfo finfo(fileOrDir);
        if(finfo.suffix() == "gjdy") {
            QFileInfo gjdyFile(fileOrDir);
            for(auto &it:allCase[gjdyFile.completeBaseName()]){
                if(it.second == ""){
                    it.second = fileOrDir;
                    nCase++;
                }
            }
        }
    }

    //! 更新输出控制页按钮enable状态
    if(!allCase.empty()){
        QString aDbPath;
        for(auto [caseName, str2s] : allCase){
            if(!str2s.empty()) {
                for(auto &it:str2s){
                    if(aDbPath.isEmpty()){
                        aDbPath = it.first;
                        break;
                    }
                }
                if(!aDbPath.isEmpty())break;
            }
        }
        //todo
        if(!aDbPath.isEmpty()) {
            auto db = GFE::open(aDbPath.toStdString());
            if(db) {
                auto baseCntStr = GFE::getParameter(db, "YJK Base Cnt");
                auto allCntStr = GFE::getParameter(db, "YJK Storey Cnt");
                bool ok = GFE::FromString(allCntStr, m_allStyCnt);
                ui->radioButton->setEnabled(ok);
                ui->radioButton->setChecked(ok);
                ui->radioButton_4->setEnabled(ok);
                ui->radioButton_4->setChecked(ok);
                ok = GFE::FromString(baseCntStr, m_baseCnt);
                if(ok && m_baseCnt > 0) {
                    ui->radioButton_2->setEnabled(ok);
                    ui->radioButton_5->setEnabled(ok);
                    ui->radioButton_2->setChecked(ok);
                    ui->radioButton_5->setChecked(ok);
                }
            }
        }
    }

    // if(m_dbsTables[m_curDBSId])m_dbsTables[m_curDBSId]->clear();
    // m_dbsTables[m_curDBSId] = new QTableWidget;
    // auto& table = m_dbsTables[m_curDBSId];
    auto& table = ui->tableWidget;
    table->clearContents();
    table->setRowCount(nCase);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Case")
                                                   << tr("DB Path") << tr("GJDY Path"));
    int iCase = 0;

    QStringList caseNames ,dbPathList, gjdyList;
    for(const auto& [caseName, str2s] : allCase){
        for(const auto& path2 : str2s){
            dbPathList.push_back(path2.first);
            gjdyList.push_back(path2.second);

            int idx = -1;
            // 猜测类型
            if(caseName.contains(u8"恒载")
                || caseName.contains(u8"地面堆载")
                || caseName.contains(u8"覆土荷载")
                || caseName.contains(u8"侧向土压力")
                || caseName.contains(u8"竖向地面堆载")
                || caseName.contains("Dead", Qt::CaseInsensitive))
                idx = 0;
            else if(caseName.left(2) == u8"活载"
                     || caseName.contains("Live", Qt::CaseInsensitive))
                idx = 1;
            else if(caseName.left(3) == u8"水压力")
                idx = 3;
            else
                idx = 2;
            auto combo = new QComboBox();
            GFE_ComboBox::AlignCenter(combo);
            combo->addItems({tr("Dead"), tr("Live"), tr("Dynamic"), tr("Ignore")});
            for(int j = 0; j < 4; j ++)
                dynamic_cast<QStandardItemModel*>(combo->view()->model())->item(j)->setTextAlignment(Qt::AlignHCenter);
            table->setCellWidget(iCase, 0, combo);
            combo->setCurrentIndex(idx);

            auto item = new QTableWidgetItem(caseName);     item->setTextAlignment(Qt::AlignCenter);
            table->setItem(iCase, 1, item);
            item = new QTableWidgetItem(path2.first);       item->setTextAlignment(Qt::AlignCenter);
            table->setItem(iCase, 2, item);
            item = new QTableWidgetItem(path2.second);      item->setTextAlignment(Qt::AlignCenter);
            table->setItem(iCase, 3, item);

            iCase++;
        }
    }


    if(nCase == 0){
        QMessageBox::critical(this, tr("Error"), tr("No GFE database, please select a new directory!"));
        return;
    }

    m_css[m_curDBSId].dbPathList = dbPathList;
    m_css[m_curDBSId].gjdyList = gjdyList;

    //todo
//    m_caculates->GetSDRInfo(dbPathList);
//    ui->layer_count->setText(QString::number(m_caculates->m_layer));
    auto db = GFE::open(dbPathList[0].toStdString());
    int dim = DlgReportUtil::Stoi(GFE::getParameter(db,"Model Dim"));
    //默认模型维度为3
    if(dim == 2){
        m_mp[m_curDBSId].dim = 2;
        m_vtks[m_curDBSId].reset(new ReportVTK(db, 1500, 1300));
    }else{
        m_mp[m_curDBSId].dim = 3;
        m_vtks[m_curDBSId].reset(new ReportVTK(db));
    }

    if(dim == 2){
        auto allPropName = GFE::getAllPropertyName(db);
        QStringList itemTextList;
        for(int i = 0; i < allPropName.size(); ++i){
            auto name = QString::fromStdString(allPropName[i]);
            m_propName2ID[name] = i;
            itemTextList.append(name);
        }

        ui->listTJ_Beam->clear();
        ui->C->clear();
        ui->listTJ_Slab->clear();
        ui->listTJ_Wall->clear();
        for(const auto& text : itemTextList)
        {
            if(text.contains("Beam"))
                ui->listTJ_Slab->addItem(text);
            else
                ui->C->addItem(text);
        }

        ui->tabWidget->insertTab(1, ui->tab_section, tr("Section Type"));
        ui->tabWidget->setCurrentIndex(1);
    }
    else if(ui->tabWidget->tabText(1) == tr("Section Type")){
        ui->tabWidget->removeTab(1);
    }

    m_dbsMainDir[m_curDBSId] = sDir;
    DBSTypeChange_File(m_curDBSId);
}

void DlgReport::ToDlgCF()
{
    DBSTypeChange_File(0);      // 当前UI表数据存入内存表，当前ID设为0，跟组合系数框同步

    // 检查是否所有table都不为空
    for(int i = 0; i < m_nDBS; ++i)
        if(m_dbsMainDir[i].isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("%1 is not set!").arg(m_dbsTypes[i]));
            return;
        }


    for(int i = 0;i < m_nDBS;++i) {
        QVector<QPair<QString, CaseType>> cases;
        auto& table = m_dbsTables[i];
        if(!table)continue;
        for(int r = 0; r < table->rowCount(); r++){
            m_css[i].caseTypeArr.push_back((CaseType)qobject_cast<QComboBox*>(table->cellWidget(r,0))->currentIndex());
            if(((QComboBox*)table->cellWidget(r,0))->currentIndex() != 3)
                cases.push_back(QPair<QString, CaseType>(table->item(r,1)->text(), (CaseType)qobject_cast<QComboBox*>(table->cellWidget(r,0))->currentIndex()));
        }
        QStringList allCaseKeys;
        for(auto& [name, type] : cases) allCaseKeys << name;
        auto defComb = GetDefaultCombination(cases);
        allCaseKeys << tr("Type");

        m_cfTables[i] = new QTableWidget;
        auto& table2 = m_cfTables[i];
        table2->clear();
        table2->setColumnCount(allCaseKeys.size());
        table2->setHorizontalHeaderLabels(allCaseKeys);
        table2->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table2->horizontalHeader()->setSectionResizeMode(allCaseKeys.size()-1, QHeaderView::ResizeToContents);

        auto nRow = defComb.size();
        table2->setRowCount(nRow);
        //        auto nCol = tableW->columnCount();
        for(int i = 0; i < nRow; i++) {
            auto factors = defComb[i].first;
            auto nCol = factors.size();
            for(int j = 0; j < nCol; j++) {
                auto item = new QTableWidgetItem(QString::number(factors[j]));
                item->setTextAlignment(Qt::AlignCenter);
                table2->setItem(i, j, item);
            }
            auto combo = new QComboBox;
            GFE_ComboBox::AlignCenter(combo);
            combo->addItems({tr("Basic"), tr("Standard"), tr("Frequent"), tr("Quasi-permanent")});
            for(int j = 0; j < 4; j ++)
                dynamic_cast<QStandardItemModel*>(combo->view()->model())->item(j)->setTextAlignment(Qt::AlignHCenter);
            table2->setCellWidget(i, nCol, combo);
            combo->setCurrentIndex(defComb[i].second);
        }
    }

    CopyTable(m_cfTables[0], ui_cf->tableWidget);

    this->hide();
    dlg_cf->show();
}

void DlgReport::ToDlgReport()
{
    dlg_cf->hide();
    this->show();
}

void DlgReport::accept(){
    for(int i = 0;i < m_nDBS;++i){
        auto& table = m_dbsTables[i];
        DBSTypeChange_CombFactor(i);
        if(!table)continue;
        int nRow = table->rowCount();
        QStringList dbPathList, gjdyList, caseNames;
        for(int r = 0; r < nRow; ++r)
        {
            if(((QComboBox*)table->cellWidget(r,0))->currentIndex() == 3)
                continue;
            caseNames << table->item(r, 1)->text();
            dbPathList << table->item(r, 2)->text();
            gjdyList << table->item(r, 3)->text();
        }
        int dynaCnt = 0;
        for(const auto& path : dbPathList)
        {
            auto db = GFE::open(path.toStdString(), false);
            if(GFE::Step::GetDynamic(db))
                dynaCnt++;
        }

        table = m_cfTables[i];

        nRow = table->rowCount();
        int nCol = table->columnCount();
        QList<QStringList> combList;
        for(int r = 0; r < nRow; ++r)
        {
            QStringList tmp;
            for(int c = 0; c < nCol - 1; ++c)
            {
                tmp << table->item(r, c)->text();
            }
            auto combo = qobject_cast<QComboBox*>(table->cellWidget(i, nCol - 1));
            tmp << QString::number(combo->currentIndex());
            combList << tmp;
        }
        dlg_cf->hide();

        QSet<int> sdr, acr;
        if(ui->radioButton_2->isChecked()) {
            for(int j = 1; j <= m_baseCnt; j++) sdr.insert(j);
            for(int j = 1; j <= m_baseCnt; j++) acr.insert(j);
        }
        else if (ui->radioButton->isChecked()){
            for(int j = 1; j <= m_allStyCnt; j++) sdr.insert(j);
            for(int j = 1; j <= m_allStyCnt; j++) acr.insert(j);
        }
//        else if(ui->radioButton_3->isChecked() && !DlgFilter::ParseText(ui->textEdit->toPlainText(), sdr))
//        {
//            QMessageBox::critical(this, tr("Error"), tr("The SDR Custom is invalid!"));
//            return;
//        }
        if(ui->radioButton_5->isChecked()) {
            for(int j = 1; j <= m_baseCnt; j++) sdr.insert(j);
            for(int j = 1; j <= m_baseCnt; j++) acr.insert(j);
        }
        else if (ui->radioButton_4->isChecked()){
            for(int j = 1; j <= m_allStyCnt; j++) sdr.insert(j);
            for(int j = 1; j <= m_allStyCnt; j++) acr.insert(j);
        }
//        else if(ui->radioButton_6->isChecked() && !DlgFilter::ParseText(ui->textEdit_2->toPlainText(), acr))
//        {
//            QMessageBox::critical(this, tr("Error"), tr("The ACR Custom is invalid!"));
//            return;
//        }

        // if(sdr.size() * dynaCnt > 20)
        // {
        //     QMessageBox::critical(this, tr("Error"), tr("The total number of SDR layer for all case is over 20. Please reduce the layer or the case."));
        //     return;
        // }

        auto lSdr = sdr.toList();
        auto lAcr = acr.toList();
        qSort(lSdr);
        qSort(lAcr);

        if(m_mp[i].dim == 2){
            for(int j = 0; j < ui->listTJ_Slab->count(); ++j)
                m_caculates[i]->m_vSlabProp.push_back(m_propName2ID[ui->listTJ_Slab->item(j)->text()]);
            for(int j = 0; j < ui->listTJ_Wall->count(); ++j)
                m_caculates[i]->m_vWallProp.push_back(m_propName2ID[ui->listTJ_Wall->item(j)->text()]);
        }

        m_css[i].caseNames = caseNames;
        m_css[i].sdrLayer = lSdr;
        m_css[i].acrLayer = lAcr;
        m_css[i].dbPathList = dbPathList;
        m_css[i].gjdyList = gjdyList;
        m_css[i].combList = combList;
        m_css[i].isSimplified = ui->checkSimp->isChecked();
    }

    OnGenerateReport();
}


QString DlgReport::WordTextInit(){
    QString retInfo, dbsDir = m_css[0].dbsDir;
    QDir outDir(m_css[0].dbsDir + u8"/Report"),tempDir(":Res/Report");

    if (!outDir.exists()) {
        if (!outDir.mkpath(".")) {
            retInfo =  tr("Error: Unable to create report folder");
            return retInfo;
        }
    }

    if (!tempDir.exists()&&!tempDir.mkpath(".")) {
        retInfo =  tr("Error: Unable to find template report folder");
        return retInfo;
    }

    //由于读取模板的路径必须为绝对路径，因此先创建临时文件夹并把/Res中的模板copy出来
    m_reportDir = outDir.path();
    if(!m_tDir.isValid())
    {
        return tr("Error: Cannot create temporary directory");
    }

    QString templatePath, tFile;
    if(m_reportType == DlgReportUtil::tr("Response displacement")){
        templatePath = ":Res/Report/ReactionDisp.docx";
        m_reportPath = m_reportDir + tr("/ReactDispReport.docx");
        tFile = m_tDir.path() + "/ReactionDisp.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Time history analysis")){
        templatePath = ":Res/Report/TimeHistory.docx";
        m_reportPath = m_reportDir + tr("/TimeHistoryReport.docx");
        tFile = m_tDir.path() + "/TimeHistory.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Time history(Elastic)")){
        templatePath = ":Res/Report/3DElasticTimeHistory.docx";
        m_reportPath = m_reportDir + tr("/TimeHistoryReport(Elastic).docx");
        tFile = m_tDir.path() + "/3DElasticTimeHistory.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Shock absorption")){
        templatePath = ":Res/Report/ShockAbsorptionIsolation.docx";
        m_reportPath = m_reportDir + tr("/ShockAbsorptionReport.docx");
        tFile = m_tDir.path() + "/ShockAbsorptionIsolation.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Shock absorption")){
        templatePath = ":Res/Report/ShockAbsorptionIsolation.docx";
        m_reportPath = m_reportDir + tr("/ShockAbsorptionReport.docx");
        tFile = m_tDir.path() + "/ShockAbsorptionIsolation.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Shock absorption")){
        templatePath = ":Res/Report/ShockAbsorptionIsolation.docx";
        m_reportPath = m_reportDir + tr("/ShockAbsorptionReport.docx");
        tFile = m_tDir.path() + "/ShockAbsorptionIsolation.docx";
    }else if(m_reportType == DlgReportUtil::tr(u8"Test")){
        templatePath = ":Res/Report/TestTemplate.docx";
        m_reportPath = m_reportDir + tr("/TestTemplate.docx");
        tFile = m_tDir.path() + "/TestTemplate.docx";
    }


    if(tFile.isEmpty()){
        return tr("Error: The temporary template does not exist!");
    }

    // 判断旧计算书是否为打开的状态

    QFile checkF(m_reportPath);
    checkF.close();
    if(checkF.exists())
    {
        QLockFile lf(checkF.fileName());
        if(!lf.tryLock())
        {
            return tr("Error(Retry): You must close the existed GFEReport document first!");
        }
        else
        {
            while(lf.isLocked())
                lf.unlock();
        }
    }

    QFile::copy(templatePath, tFile);
    m_word.reset(new ReportWord(tFile, m_reportPath));
    if(!m_word->m_word||!m_word->m_documents||!m_word->m_document){
        return tr("Error: Generate Report needs Microsoft Office Word or WPS Word!");
    }

    QString findText1 = "r_section";
    m_word->FindReplace(findText1, "1");
    QString findText2 = "r_dbsname";
    //todo
    m_word->FindReplace(findText2, m_css[0].dbsName);

    return retInfo;
}

QString DlgReport::ValidBMInit()
{
    QString retInfo;
    QAxObject *bookmarks = m_word->m_document->querySubObject("Bookmarks"); // 获取Bookmarks对象

    auto isCheckedBM = [&](QString &bmName)->bool{
        QStringList bms = bmName.split('_');
        int size = bms.size();
        QString bmNameNoDBS;
        bool flag = false;

        for(int i = 1;i < size;++i){
            bmNameNoDBS += bms[i];
            if(i != size - 1)bmNameNoDBS += '_';
        }

        std::function<void(QTreeWidgetItem*,QString)> TraverseItems = [&](QTreeWidgetItem* item,QString bmNameNoDBS)
        {
            // 遍历子节点
            const int rowCount = item->childCount();
            for (int i = 0; i < rowCount; ++i) {
                auto childItem = item->child(i);
                if (childItem->checkState(0) == Qt::Checked&&bmNameNoDBS == DlgReportUtil::Type2BMName()[childItem->text(0)]){
                    flag = true;
                    return;
                }
                TraverseItems(childItem,bmNameNoDBS);
            }
        };

        auto root = ui->treeContent->invisibleRootItem();
        TraverseItems(root,bmNameNoDBS);

        return flag;
    };

    if (bookmarks)
    {
        int count = bookmarks->dynamicCall("Count()").toInt(); // 获取书签数量
        qDebug() << "Template valid bookmarks: ";
        for (int i = 1; i <= count; ++i)
        {
            QAxObject *bookmark = bookmarks->querySubObject("Item(int)", i); // 获取书签对象
            QString bmName = bookmark->property("Name").toString(); // 获取书签名称

            if(ReportBookMark::isValid(bmName) && isCheckedBM(bmName)){
                qDebug() << bmName;
                m_bmList.append(ReportBookMark(bmName));
            }
            delete bookmark;
        }

        delete bookmarks;
    }
    if(m_bmList.empty()){
        retInfo = tr(u8"Warning: Template has no valid bookmark!");
    }
    return retInfo;
}

QString DlgReport::DBAndModelInit(){
    QString retInfo;
    for(int i = 0;i < m_nDBS;++i){
        if(m_css[i].dbPathList.empty()){
            return tr("Error: No GFE db chose!");
        }

        int nodeNum = -1;
        std::vector<GFE::id_t> elmSubType;
        //todo 可提升效率
        for(auto& dbPath : m_css[i].dbPathList){
            auto db = GFE::open(dbPath.toStdString(), false);
            if(nodeNum == -1)nodeNum = GFE::getNodeNum(db);
            else if(nodeNum != GFE::getNodeNum(db)){
                return tr("Error: All the selected db's model must be same!");
            }
            if(elmSubType.empty()){
                elmSubType = GFE::getAttribute(db, "ElementSubType");
            }
            else
            {
                auto tmp = GFE::getAttribute(db, "ElementSubType");
                int size = elmSubType.size();
                if(tmp.size() != elmSubType.size())
                    return tr("Errort: All the selected db's model must be same!");
                for(int j = 0; j < size; ++j)
                {
                    if(elmSubType[j] != tmp[j])
                        return tr("Error: All the selected db's model must be same!");
                }
            }
        }
        m_mp[i].nodeNum = nodeNum;
        m_mp[i].elemNum = elmSubType.size();
        m_mp[i].elmSubType = elmSubType;

        auto gjdyPath = m_css[i].gjdyList[0];
        QFile gjdyFile(gjdyPath);
        if(!gjdyFile.exists() || !gjdyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            return tr("Error: One certain GFE DB has no gjdy file");
        }

        //构件ID-构件单元
        QVector<QString> compNames;
        QString compName = "";

        while(!gjdyFile.atEnd()){
            QString line = gjdyFile.readLine();
            if(line.mid(0,4)=="Part"){
                compName = line.mid(6);
                compName.chop(1);
                compNames.push_back(compName);
                int compId = std::stoi(line.toStdString().substr(line.indexOf('-')+1));
                m_css[i].gjID2GjName[compId] = compName;
            }
            else{
                auto eleIDs = line.split(" ");
                for(auto &eleID:eleIDs){
                    bool ok;
                    int id = eleID.toInt(&ok);
                    if(ok){
                        m_css[i].compsElemID[compName].insert(id);
                    }
                }
            }
        }

        for(auto it = m_css[i].dbPathList.begin();it != m_css[i].dbPathList.end();){
            auto db = GFE::open(it->toStdString());
            auto t_it = it + 1;
            if(!db){
                m_css[i].dbPathList.erase(it);
            }else{
//                m_css[idx].dbList.push_back(db);
            }
            it = t_it;
        }
    }
    return retInfo;
}

QString DlgReport::CalculateInit()
{
    qDebug()<<"Start GFE Calculate ...";
    for(int i = 0;i < m_nDBS;++i){
        if(!m_caculates[i]->m_opt.NoComb)m_caculates[i]->GFECombPJCalculate(m_css[i]);
        if(!m_caculates[i]->m_opt.NoSDR)m_caculates[i]->GetSDRInfo(m_css[i].dbPathList);
    }
    qDebug()<<"GFE Calculate finished";
    return "";
}

void DlgReport::OnGenerateReport()
{
    if(m_nDBS == 0)
        return;
    m_plot.reset(new ReportPlot(m_tDir.path()));
    m_progDlg = new GFE_ProgDlg_Simple();
    m_progDlg->setWindowTitle(tr("Generating Report"));
    m_progDlg->Init(false);
    m_progDlg->autoClose = false;
    auto MainRoutine = [&] {
        qDebug()<<"Start generating report ...";

        QString retInfo;
        retInfo = GeneratorInit();
        if(!retInfo.isEmpty())
        {
            m_progDlg->Finish(InfoType(retInfo), retInfo);
        }
        retInfo = StartGenerate();
        if(!retInfo.isEmpty())
        {
            m_progDlg->Finish(InfoType(retInfo), retInfo);
        }
        retInfo = GeneratorFinish();
        if(!retInfo.isEmpty())
        {
            m_progDlg->Finish(InfoType(retInfo), retInfo);
        }
        m_progDlg->Finish(0, tr("Done"));
    };
//    MainRoutine();
    m_progDlg->Start(MainRoutine);
    if(m_progDlg->result) {
        QMessageBox mbox;
        mbox.setWindowTitle(tr("Export successfully"));
        mbox.setTextInteractionFlags(Qt::TextSelectableByMouse);
        mbox.setText(tr("The report has been exported to %1").arg(m_reportPath));
        mbox.exec();
    }
}

QString DlgReport::GeneratorInit()
{
    QString retInfo;

    retInfo = WordTextInit();
    if(!retInfo.isEmpty())return retInfo;

    retInfo = ValidBMInit();
    if(!retInfo.isEmpty())return retInfo;

    retInfo = DBAndModelInit();
    if(!retInfo.isEmpty())return retInfo;

    retInfo = CalculateInit();
    if(!retInfo.isEmpty())return retInfo;

    return retInfo;
}

QString DlgReport::StartGenerate()
{
    QString retInfo;
    for(auto& bm : m_bmList){
        switch (bm.m_insertType) {
        case ReportBookMark::Number:
            retInfo = ExportNumber(bm);
            break;
        case ReportBookMark::Table:
            retInfo = ExportTable(bm);
            break;
        case ReportBookMark::CurveChart:
            retInfo = ExportCurveChart(bm);
            break;
        case ReportBookMark::Picture:
            retInfo = ExportPicture(bm);
            break;
        case ReportBookMark::CloudChart:
            retInfo = ExportCloudChart(bm);
            break;
        default:
            break;
        }
        if(!retInfo.isEmpty()){
            m_progDlg->Finish(InfoType(retInfo),retInfo);
        }
    }
    return "";
}

QString DlgReport::GeneratorFinish()
{
    QString retInfo;
    if(!m_word){
        return QObject::tr("Error(Retry): The program did not open the document. Please confirm whether the output document exists or has been closed?");
    }
    retInfo = m_word->Save();
    if(!retInfo.isEmpty())return retInfo;
    m_bmList.resize(0); //初始话有效书签列表
    return retInfo;
}

int DlgReport::InfoType(QString info)
{
    auto sl = info.split(":");
    if(sl[0] == "Error") return 1;
    if(sl[0] == "Warning") return 2;
    if(sl[0] == "Error(Retry)") return 4;
    return 0;
}

QString DlgReport::NewBMPrefix(int dbsId)
{
    return "New" + QString::number(dbsId) + "_";
}


QVector<QPair<QVector<double>, DlgReport::CombType> > DlgReport::GetDefaultCombination(const QVector<QPair<QString, CaseType> > &cases)
{
    QVector<QPair<QVector<double>, CombType>> result;
    using CbT = CombType;
    using CsT = CaseType;

    auto nCase = cases.size();
    QVector<int> caseTypeRecord(nCase);
    QVector<int> customCaseRecord;
    for(int i = 0; i < nCase; i++) {
        caseTypeRecord[i] = cases[i].second;
        if(caseTypeRecord[i] == CsT::Dynamic) {
            caseTypeRecord[i] = CsT::Ignore;
            customCaseRecord.push_back(i);
        }
    }

    auto nCustom = customCaseRecord.size();
    result.resize(8+6*nCustom);

    constexpr double heng1 = 1.0;
    constexpr double heng2 = 1.3;
    constexpr double huo1 = 0.5;
    constexpr double huo2 = 0.65;
    constexpr double huo3 = 1.5;
    constexpr double custom1 = 1.3;
    constexpr double custom2 = -1.3;

    int iResult = 0;
    auto Helper = [&caseTypeRecord, nCase](QPair<QVector<double>, CombType>& one, double heng, double huo, double custom, CombType ct) {
        one.second = ct;
        one.first = QVector<double>(nCase, 0);
        for(int j = 0; j < nCase; j++) {
            if(caseTypeRecord[j] == CsT::HengZai) one.first[j] = heng;
            else if(caseTypeRecord[j] == CsT::HuoZai) one.first[j] = huo;
            else if(caseTypeRecord[j] == CsT::Dynamic) one.first[j] = custom;
        }
    };
    auto Helper2 =[&](double heng, double huo, double custom, CombType ct) {
        for(auto i : customCaseRecord) {
            caseTypeRecord[i] = CsT::Dynamic;
            Helper(result[iResult++], heng, huo, custom, ct);
            caseTypeRecord[i] = CsT::Ignore;
        }
    };

    // 基本组合
    Helper(result[iResult++], heng1, huo1, 0, CbT::Basic);
    Helper2(heng1, huo1, custom1, CbT::Basic);
    Helper2(heng1, huo1, custom2, CbT::Basic);
    Helper(result[iResult++], heng1, huo3, 0, CbT::Basic);

    Helper(result[iResult++], heng2, huo2, 0, CbT::Basic);
    Helper2(heng2, huo2, custom1, CbT::Basic);
    Helper2(heng2, huo2, custom2, CbT::Basic);
    Helper(result[iResult++], heng2, huo3, 0, CbT::Basic);

    // 标准组合
    constexpr double custom3 = 1.0;
    constexpr double custom4 = -1.0;

    Helper(result[iResult++], heng1, huo1, 0, CbT::Standard);
    Helper2(heng1, huo1, custom3, CbT::Standard);
    Helper2(heng1, huo1, custom4, CbT::Standard);
    Helper(result[iResult++], heng1, 1.0, 0, CbT::Standard);

    // 频遇组合
    Helper(result[iResult++], heng1, 0.6, 0, CbT::Frequent);

    // 准永久组合
    Helper(result[iResult++], heng1, 0.5, 0, CbT::QuasiPermanent);

    return result;
}

QString DlgReport::ExportNumber(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_gfeType) {
    case ReportBookMark::Comb:
        ExportInterForceNumber(bm);
        break;
    case ReportBookMark::SDR:
        ExportSDRNumber(bm);
        break;
    case ReportBookMark::Model:
        ExportModelNumber(bm);
        break;
    case ReportBookMark::GFEType::Test:
        ExportTestNumber(bm);
    default:
        break;
    }
    return retInfo;
}

QString DlgReport::ExportInterForceNumber(const ReportBookMark &bm)
{
    QString retInfo;
    int dbsId = bm.m_dbsID;
    auto eleForce = m_caculates[dbsId]->m_pjResult.eleForce;

    switch (bm.m_param) {
    case ReportBookMark::AxialForce:{
        m_word->WriteText(bm.m_bmName,QString::number(m_caculates[dbsId]->m_maxSF1));
        break;
    }
    case ReportBookMark::ShearForce:{
        m_word->WriteText(bm.m_bmName,QString::number(m_caculates[dbsId]->m_maxSF2));
        break;
    }
    case ReportBookMark::Bending:{
        m_word->WriteText(bm.m_bmName,QString::number(m_caculates[dbsId]->m_maxSM1));
        break;
    }
    case ReportBookMark::ACR:{
        retInfo = ExportACRNumber(bm);
        break;
    }
    default:
        break;
    }
    return retInfo;
}

QString DlgReport::ExportSDRNumber(const ReportBookMark &bm)
{
    QString retInfo;
    QStringList dbPathList;
    double sdrMax = -INFINITY;
    int layerMax = -1;

    auto& cs = m_css[bm.m_dbsID];

    if(cs.acrLayer.empty()){
        retInfo = tr("NoData: Layer count is zero, no sdr infomation!");
        m_word->WriteText(bm.m_bmName, retInfo);
        return "";
    }

    bool hasDymaic = false;
    for(auto& dbPath : cs.dbPathList){
        auto db = GFE::open(dbPath.toStdString(), false);
        if(!GFE::Step::GetDynamic(db)){
            retInfo = tr("Warning: Can not find dynamic analysis result, which is necessary for extracting Storey Drift Ratio data");
            return retInfo;
        }
        hasDymaic = true;

        // 读取数据
        QFileInfo fi(dbPath);
        QString filePath;

        if(m_mp[bm.m_dbsID].dim == 2){
            filePath = fi.absolutePath() + u8"/XYData/层间位移角包络.txt";
        }else{
            QString dir;
            if(bm.m_extra[0]=="X")dir = "X";
            else if(bm.m_extra[0]=="Y") dir = "Y";
            filePath  = fi.absolutePath() + "/XYData/" + dir + u8"向层间位移角包络.txt";
        }

        m_caculates[bm.m_dbsID]->GetSDREnvelope(filePath, layerMax, sdrMax);
    }

    if(!hasDymaic){
        retInfo = tr("Warning: No dynamic step in all db!");
        return retInfo;
    }

    if(bm.m_param == ReportBookMark::Max){
        if(sdrMax != -INFINITY){
            int numerator, denominator;
            DlgReportUtil::FloatToReciprocal(sdrMax,numerator,denominator);
            auto strs = GFE::Split(ui->sdr_limit->text().toStdString(), "/");
            double sdrLimit = std::stod(strs[0]) / std::stod(strs[1]);
            if(sdrMax <= sdrLimit){
                m_word->WriteText(bm.m_bmName, QString::number(numerator) + "/" + QString::number(denominator) + u8"，层间位移角小于限值" + ui->sdr_limit->text());
            }else{
                m_word->WriteText(bm.m_bmName, QString::number(numerator) + "/" + QString::number(denominator) + u8"，层间位移角大于限值" + ui->sdr_limit->text());
            }
        }else{
            m_word->WriteText(bm.m_bmName, "No sdr infomation!");
        }
    }

    return retInfo;
}

QString DlgReport::ExportACRNumber(const ReportBookMark& bm){
    QString retInfo;

    //读gjdy文件
    int combId = 0; //todo

    auto gjdyPath = m_css[bm.m_dbsID].gjdyList[0];
    QFile gjdyFile(gjdyPath);
    if(!gjdyFile.exists() || !gjdyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        retInfo = tr("Warning: Open gjdy File failed");
        return retInfo;
    }

    //构件ID-构件单元
    std::unordered_map<std::string,std::unordered_set<int>> comps;
    std::vector<std::string> compIDs;
    std::string compID = "";

    while(!gjdyFile.atEnd()){
        QString line = gjdyFile.readLine();
        if(line.toStdString().substr(0,4)=="Part"){
            compID = line.toStdString().substr(6);
            compID.pop_back();
            compIDs.push_back(compID);
        }
        else{
            auto eleIDs = line.split(" ");
            for(auto &eleID:eleIDs){
                bool ok;
                int id = eleID.toInt(&ok);
                if(ok){
                    comps[compID].insert(id);
                }
            }
        }
    }

    std::unordered_map<int,std::unordered_set<int>> compsId;
    for(auto idstr:compIDs){
        int i,id;
        for(i=0;i<idstr.size();++i){
            if(idstr[i]=='-')break;
        }
        id = std::stoi(idstr.substr(i+1));
        compsId[id] = comps[idstr];
    }

    auto db = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString(),false);
    if(m_caculates[bm.m_dbsID]->m_pjResult.acr.empty()){
        retInfo = tr("NoData: No acr data!");
        m_word->WriteText(bm.m_bmName, retInfo);
        return "";
    }
    std::array<GFECaculate::AcrResult,2> data = m_caculates[bm.m_dbsID]->m_pjResult.acr[combId];
    std::vector<float> colorArr(GFE::getElementNum(db),NAN);

    for(int i=0;i<data.size();++i){
        for(auto it:data[i]){
            int id = it.first.second;//构件id
            double acr = it.second;//构件轴压比
            //        qDebug()<<it.first.first<<id<<acr;
            for(auto eleId:compsId[id]){
                colorArr[m_vtks[bm.m_dbsID]->m_elementLabel2Id[eleId]] = acr;
            }
        }
    }

    QStringList setsName;
    auto elemNames = GFE::getAllElementSetName(db);
    QString patternCol = "floor.*AllCol", patternWall = "floor.*AllWall";
    QRegularExpression regexCol(patternCol), regexWall(patternWall);
    for(auto& name:elemNames){
        auto t_name = QString::fromStdString(name);
        if(regexCol.match(t_name).hasMatch() ||
            regexWall.match(t_name).hasMatch()){
            setsName.append(t_name);
        }
    }

    ReportVTK::Option opt;
    opt.isCloud = true, opt.isCell = true;
    auto pngPath = m_tDir.path() + "/temp.png";
    retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt,setsName,"ACR Comb" + QString::number(combId + 1) ,colorArr);
    if(!retInfo.isEmpty()){
        return retInfo;
    }

    if(bm.m_extra[0] == "Max"){
        auto acrMax = m_vtks[bm.m_dbsID]->m_setMax;
        //todo
        double acrLimit = ui->acr_limit->text().toDouble();
        QString text;
        if(acrMax > acrLimit){
            text = QString::number(acrMax) + u8"，大于限值" + QString::number(acrLimit) + u8"，不符合规范要求";
        }else{
            text = QString::number(acrMax) + u8"，小于限值" + QString::number(acrLimit) + u8"，符合规范要求";
        }
        m_word->WriteText(bm.m_bmName, text);
    }

    return retInfo;
}

QString DlgReport::ExportModelNumber(const ReportBookMark &bm)
{
    QString retInfo;
    if(bm.m_param == ReportBookMark::Elem){
        m_word->WriteText(bm.m_bmName, QString::number(m_mp[bm.m_dbsID].elemNum));
    }

    if(bm.m_param == ReportBookMark::Node){
        m_word->WriteText(bm.m_bmName, QString::number(m_mp[bm.m_dbsID].nodeNum));
    }

    return retInfo;
}

QString DlgReport::ExportTestNumber(const ReportBookMark &bm)
{
    switch (bm.m_param) {
    case ReportBookMark::Param::Temp:
        ExportTestTempNumber(bm);
        break;
    default:
        break;
    }
    return "";
}

QString DlgReport::ExportTestTempNumber(const ReportBookMark &bm)
{
    m_word->WriteText(bm.m_bmName, "Test Number", "error");
    return "";
}


QString DlgReport::ExportCurveChart(const ReportBookMark& bm)
{
    QString retInfo;
    switch (bm.m_gfeType){
    case ReportBookMark::SDR:{
        if(m_css[bm.m_dbsID].sdrLayer.empty()){
            retInfo = tr("Warning: The number of floors is 0, no Storey information.");
            return retInfo;
        }
        retInfo = ExportStoreyCurveChart(bm);
        break;
    }
    case ReportBookMark::Storey:{
        if(m_css[bm.m_dbsID].sdrLayer.empty()){
            retInfo = tr("Warning: The number of floors is 0, no Storey information.");
            return retInfo;
        }
        retInfo = ExportStoreyShearForceCurveChart(bm);
        break;
    }
    case ReportBookMark::Vibration:{
        retInfo = ExportVibCurveChart(bm);
        break;
    }
    case ReportBookMark::Comb:{
        retInfo = ExportCombCurveChart(bm);
        break;
    }
    case ReportBookMark::AMP:{
        retInfo = ExportAMPCurveChart(bm);
        break;
    }
    case ReportBookMark::XYData:{
        retInfo = ExportXYDataCurveChart(bm);
        break;
    }
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportStoreyCurveChart(const ReportBookMark& bm)
{
    //提取层间位移角共用信息
    QString retInfo;
    QString xTitle,yTitle;
    QVector<QPair<QVector<double>,QVector<double>>> coordDatas;
    bool flag = false;//是否只输出一张折线图

    switch (bm.m_param) {
    case ReportBookMark::Envelope:{
        retInfo = ExportSDREnvelopeCurveChart(bm);
        break;
    }
    case ReportBookMark::Time:{
        retInfo = ExportSDRTimeCurveChart(bm,m_caculates[bm.m_dbsID]->m_sdrDir1,m_caculates[bm.m_dbsID]->m_sdrDir2);
        break;
    }
    case ReportBookMark::Displacement:{
        retInfo = ExportStoreyDispCurveChart(bm);
        break;
    }
    case ReportBookMark::UDiff:{
        retInfo = ExportSDRUDiffTimeCurveChart(bm);
        break;
    }
    default:
        break;
    }

    if(flag){
        if(m_mp[bm.m_dbsID].dim == 2){
            if(coordDatas.empty())return tr("Warning: No valid coordinate data");
            ReportVTK::Option opt;
            opt.isInteract = false;
            ReportVTK::sPtr vtk(new ReportVTK(&opt));
            auto pngPath = m_tDir.path() + "/" + bm.m_bmName + ".png";
            vtk->ExportChartXY(coordDatas,xTitle,yTitle,pngPath);
            auto size = ReportWord::Attributes["CurveChart"];
            m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
        }

        return "";
    }else{
        return retInfo;
    }

}

QString DlgReport::ExportSDREnvelopeCurveChart(const ReportBookMark& bm)
{
    QString retInfo;
    QStringList dbPathList;
    double sdrMax = -FLT_MAX;
    QVector<double> xDataMax,yDataMax;
    int type = 0, dbsId = bm.m_dbsID;
    QMap<QString, QStringList> vibDirects;
    QMap<QString, QStringList> sameVibDirectsDBs;

    if(m_mp[dbsId].dim == 2)type = 1;
    else if(m_mp[dbsId].dim == 3 && !bm.m_extra.empty())type = 2;
    else type = 3;

    int cnt = 1;
    QString lastBmName = bm.m_bmName;
    bool hasDynamic = false, hasSDRXYData = false;

    //三维弹性时程，X、Y向地震与层间位移方向匹配
    if(type == 2){
        bool hasVibLoad = false;
        QMap<QString, QString> dbPath2caseName;
        for(int i =0;i < m_css[bm.m_dbsID].dbPathList.size();++i){
            auto dbPath = m_css[bm.m_dbsID].dbPathList[i];
            auto caseName = m_css[bm.m_dbsID].caseNames[i];
            auto db = GFE::open(dbPath.toStdString(), false);
            if(!GFE::Step::GetDynamic(db)){
                continue;
            }
            hasDynamic = true;
            QString key;
            auto vibLoads = GFE::getAllVibLoad(db);
            if(!vibLoads.empty()){
                hasVibLoad = true;
            }
            for(auto& v : vibLoads){
                if(v.param["AmpX"] != ""){
                    vibDirects[dbPath].push_back("X");
                    key += "X";
                }
                if(v.param["AmpY"] != ""){
                    vibDirects[dbPath].push_back("Y");
                    key += "Y";
                }
                if(v.param["AmpZ"] != ""){
                    vibDirects[dbPath].push_back("Z");
                    key += "Z";
                }
            }
            dbPath2caseName[dbPath] = caseName;
            sameVibDirectsDBs[key].push_back(dbPath);
        }

        if(!hasVibLoad){
            retInfo = tr("NoData: The db composition has no vibration load!");
            m_word->WriteText(bm.m_bmName, retInfo);
            return "";
        }

        for(auto it = sameVibDirectsDBs.begin();it != sameVibDirectsDBs.end();++it){
            auto directions = it.key();
            for(auto& d : directions){
                QStringList filePaths,lineNames;
                QVector<QVector<double>> xDatas, yDatas;
                for(auto& dbPath : it.value()){
                    QFileInfo fi(dbPath);
                    auto name = fi.completeBaseName(), fullPath = fi.absolutePath();
                    filePaths.push_back(fullPath + "/XYData/" + d + u8"向层间位移角包络.txt");
                }

                for(int i = 0;i < filePaths.size();++i){
                    auto fp = filePaths[i];
                    QFile f(fp);
                    if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)){
                        continue;
                    }
                    QVector<double> xData,yData;
                    while(!f.atEnd())
                    {
                        QString line = f.readLine();
                        auto sp = line.split(",");
                        if(sp.size() < 2)
                            continue;
                        double x = sp[0].trimmed().toDouble(),
                            y = sp[1].trimmed().toDouble();
                        xData.push_back(x);
                        yData.push_back(y);
                    }

                    if(xData.empty()||yData.empty())continue;
                    hasSDRXYData = true;
                    xDatas.push_back(xData);
                    yDatas.push_back(yData);
                    lineNames.push_back(dbPath2caseName[it.value()[i]]);
                }


                QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR" + QString::number(cnt);
                QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR"  + QString::number(cnt) + "Title";
                QString chartTitleBmText =  QString(d) + "向地震下" + d + u8"向层间位移角包络";
                m_word->AddNewBM(lastBmName,chartBmName);
                m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");
                auto pngSize = ReportWord::Attributes["CurveChartSDR"];
                int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
                auto pngPath =  m_plot->PrintCurveChart(xDatas, yDatas, "SDR", "Layer", lineNames, 2, w, h);
                m_word->AddPicture(chartBmName, pngPath ,w ,h);
                lastBmName = chartTitleBmName;
                cnt++;
            }
        }
    }else{
        QVector<QVector<double>> xDatas, yDatas;
        QStringList lineNames, dbPathList;

        dbPathList = m_css[bm.m_dbsID].dbPathList;


        for(auto& dbPath:dbPathList){
            auto db = GFE::open(dbPath.toStdString(), false);
            if(!GFE::Step::GetDynamic(db)){
                continue;
            }
            hasDynamic = true;
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName(), fullPath = fi.absolutePath();

            QStringList filePaths;
            if(type == 1){
                filePaths.clear();
                filePaths.append(fullPath + u8"/XYData/层间位移角包络.txt");
            }
            if(type == 3){
                xDatas.clear(), yDatas.clear();
                lineNames.clear();
                filePaths.append(fullPath + u8"/XYData/X向层间位移角包络.txt");
                filePaths.append(fullPath + u8"/XYData/Y向层间位移角包络.txt");
            }

            for(auto& fp:filePaths){
                if(type == 3){
                    if(GFE::FileInfo::Name(fp.toStdString()).front() == 'X'){
                        lineNames.append(QString::fromStdString("Story drift envelope of X directiorn"));
                    }
                    if(GFE::FileInfo::Name(fp.toStdString()).front() == 'Y'){
                        lineNames.append(QString::fromStdString("Story drift envelope of Y directiorn"));
                    }
                }
                QFile f(fp);
                if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)){
                    continue;
                }
                QVector<double> xData,yData;
                while(!f.atEnd())
                {
                    QString line = f.readLine();
                    auto sp = line.split(",");
                    if(sp.size() < 2)
                        continue;
                    double x = sp[0].trimmed().toDouble(),
                        y = sp[1].trimmed().toDouble();
                    xData.push_back(x);
                    yData.push_back(y);
                    if(type == 1 && x > sdrMax){
                        sdrMax = x;
                        xDataMax = xData;
                        yDataMax = yData;
                    }
                }
                if(xData.empty()||yData.empty())continue;
                hasSDRXYData = true;

                if(type == 3){
                    xDatas.push_back(xData);
                    yDatas.push_back(yData);
                }
            }

            if(type == 3){
                QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR" + QString::number(cnt);
                QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR"  + QString::number(cnt) + "Title";
                QString chartTitleBmText = u8"图" + m_section + "-" + QString::number(cnt) + " " + name + u8"层间位移角包络";
                m_word->AddNewBM(lastBmName,chartBmName);
                m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

                auto pngSize = ReportWord::Attributes["CurveChartSDR"];
                int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
                auto pngPath =  m_plot->PrintCurveChart(xDatas, yDatas, "SDR", "Layer", lineNames, 2, w, h);
                m_word->AddPicture(chartBmName,pngPath, w, h);

                lastBmName = chartTitleBmName;
                cnt++;
            }
        }

        if(type == 1){
            xDatas.push_back(xDataMax);
            yDatas.push_back(yDataMax);
            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR" + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SDR"  + QString::number(cnt) + "Title";
            QString chartTitleBmText;
            if(type == 1){
                chartTitleBmText = u8"图" + m_section + "-" + QString::number(cnt) + u8"结构最大层间位移角包络";
            }

            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

            auto pngSize = ReportWord::Attributes["CurveChartSDR"];
            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
            auto pngPath =  m_plot->PrintCurveChart(xDatas, yDatas, "SDR", "Layer", {}, 2, w, h);
            m_word->AddPicture(chartBmName,pngPath, w, h);

            lastBmName = chartTitleBmName;
            cnt++;
        }
    }

    if(!hasDynamic){
        retInfo = tr("Warning: The db composition has no dynamic analysis step!");
        return retInfo;
    }

    if(!hasSDRXYData){
        retInfo = tr("Warning: The db composition has no SDR XYData!");
        return retInfo;
    }

    return retInfo;
}

QString DlgReport::ExportSDRTimeCurveChart(const ReportBookMark &bm,const QString& dir1,const QString& dir2)
{
    int cnt = 1,picCnt = 1;
    bool hasSDR = false;
    bool allStatic = true;
    QString lastBmName = bm.m_bmName;

    auto helper = [&](const QString& dir,const QString& tag)
    {
        for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList)
        {
            auto db = GFE::open(dbPath.toStdString(), false);
            if(!GFE::Step::GetDynamic(db))
                continue;
            allStatic = false;
            // 读取数据
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName();
            for(auto it : m_css[bm.m_dbsID].sdrLayer)
            {
                QFile f;
                if(dir == ""){
                    f.setFileName(fi.absolutePath() + "/XYData/" + QString::number(it) + u8"层位移角时程.txt");
                }else{
                    f.setFileName(fi.absolutePath() + "/XYData/" + QString::number(it) + u8"层" + dir + u8"向位移角时程.txt");
                }
                if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;
                QVector<double> xData, yData;
                while(!f.atEnd())
                {
                    QString line = f.readLine();
                    auto sp = line.split(",");
                    if(sp.size() < 2)
                        continue;
                    xData.push_back(sp[0].trimmed().toDouble());
                    yData.push_back(sp[1].trimmed().toDouble());
                }
                f.close();
                if(xData.empty() || yData.empty())
                    continue;
                hasSDR = true;
                QString chartName = "Chart_DriftRatio" + QString::number(cnt) + tag;
                QString titleName = "Chart_DriftRatio" + QString::number(cnt) + tag + '_' + "Title";
                QString titleText;
                if(dir == ""){
                    titleText = u8"图" + m_section + "-" + QString::number(picCnt++) + " " + name + u8"第" +
                                QString::number(it) + u8"层层间位移角时程曲线" +  "\r";
                }else{
                    titleText = u8"图" + m_section + "-" + QString::number(picCnt) + " " + name + u8"第" +
                                QString::number(it) + u8"层" + dir + u8"向层间位移角时程曲线" + "\r";
                    picCnt += 2;
                }

                QString xTitle = u8"时间(s)",yTitle = u8"层间位移角";
                m_word->AddNewBM(lastBmName,chartName);
                m_word->AddNewBM(chartName,titleName,titleText,"title");
                m_word->AddChart(chartName,{{xData,yData}},xTitle,yTitle);
                lastBmName = titleName;
                cnt++;
            }
        }
    };

    if(m_mp[bm.m_dbsID].dim == 2){
        helper("","");
    }
    else if(m_mp[bm.m_dbsID].dim == 3){
        int oriPicCnt = picCnt;
        helper(dir1, "");
        cnt = 1;
        picCnt = oriPicCnt + 1;
        helper(dir2, "_2");
    }

    if(!hasSDR && !allStatic){
        m_word->WriteText(bm.m_bmName, u8"未找到层间位移角文件");
    }

    return "";
}

QString DlgReport::ExportStoreyDispCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

    bool hasDynamic = false;
    int cnt = 1;
    QString lastBmName = bm.m_bmName;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;
        auto elemNames = GFE::getAllElementSetName(db);
        QStringList setsName;
        QString pattern = "^floor\\d+-All$";
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();

        QRegularExpression regex(pattern);
        for(auto& eleName:elemNames){
            auto t_name = QString::fromStdString(eleName);
            if(regex.match(t_name).hasMatch()){
                setsName.append(t_name);
            }
        }

        int frame = (int)GFE::FO::GetFrame(db,true).size() - 1;

        QStringList directions = {"X","Y"};
        for(int i = 0;i < 2;++i){
            QString lname = name + " direction of " + directions[i];\
            QVector<double> xData,yData;
            for(auto& s:setsName){
                //todo
                yData.push_back(s.mid(5,1).toDouble());

                std::vector<float> data;
                auto FOName = "U U" + std::to_string(i + 1);

                data = m_caculates[bm.m_dbsID]->GetEnvelopeData(db, QString::fromStdString(FOName), "Max");

                auto elemsId = GFE::getElementSet(db,s.toStdString())->elements;
                auto elems = GFE::getElement(db,std::move(elemsId));
                std::unordered_set<int> nodes;
                for(auto& e:elems){
                    nodes.insert(e.nodes.begin(),e.nodes.end());
                }

                double maxDisp = -INFINITY;
                for(int i = 0;i < data.size();++i){
                    if(std::isnan(data[i]))continue;
                    if(nodes.find(i) != nodes.end() && std::abs(data[i]) > maxDisp){
                        maxDisp = std::abs(data[i]);
                    }
                }
                xData.push_back(maxDisp);
            }

            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismSpectrum" + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismSpectrum"  + QString::number(cnt) + "Title";
            QString chartTitleBmText = u8"图" + m_section + "-" + QString::number(cnt) + " " + name + " " + directions[i] + u8"向楼层位移";
            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

            auto pngSize = ReportWord::Attributes["CurveChart"];
            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
            auto pngPath =  m_plot->PrintCurveChart({xData}, {yData}, "Storey displacement", "Layer", {lname}, 0, w, h);
            m_word->AddPicture(chartBmName, pngPath, w, h);

            lastBmName = chartTitleBmName;
            cnt++;
        }


    }

    if(!hasDynamic){
        retInfo = tr("Warning: Databases has no dynamic step!");
        return retInfo;
    }


    return retInfo;
}

QString DlgReport::ExportSDRUDiffTimeCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

    int cnt = 1;
    bool hasDynamic = false, hasVibLoad = false;
    QString lastBmName = bm.m_bmName;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;
        // 读取数据
        QFileInfo fi(dbPath);
        auto dbName = fi.completeBaseName();

        auto vibLoads = GFE::getAllVibLoad(db);
        if(vibLoads.empty())continue;
        hasVibLoad = true;

        QStringList directions;
        for(auto& v : vibLoads){
            if(v.param["AmpX"] != ""){
                directions.push_back("X");
            }
            if(v.param["AmpY"] != ""){
                directions.push_back("Y");
            }
            if(v.param["AmpZ"] != ""){
                directions.push_back("Z");
            }
        }

        for(auto& direction : directions){
            QFile envf,nodef;
            envf.setFileName(fi.absolutePath() + "/XYData/" + direction + u8"向层间位移角包络.txt");
            if(!envf.exists() || !envf.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            nodef.setFileName(fi.absolutePath() + "/XYData/" + direction + u8"向层间位移角最大节点.txt");
            if(!nodef.exists() || !nodef.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            double maxSDR = -INFINITY, maxLayer = -1;
            QString nodeStr, FOName;
            if(direction == "X")FOName = "U U1";
            if(direction == "Y")FOName = "U U2";
            if(direction == "Z")FOName = "U U3";
            while(!envf.atEnd())
            {
                QString line = envf.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                double sdr =  sp[0].trimmed().toDouble();
                int layer = sp[1].trimmed().toInt();
                if(sdr > maxSDR){
                    maxSDR = sdr, maxLayer = layer;
                }
            }

            if(maxSDR == -INFINITY)continue;
            while(!nodef.atEnd())
            {
                QString line = nodef.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                QString layerStr = sp[0].trimmed();
                int layer = layerStr.split(' ')[1].trimmed().toInt();
                if(layer == maxLayer){
                    nodeStr = sp[1].split(':')[1];
                    break;
                }
            }
            if(nodeStr.isEmpty())continue;
            nodeStr.chop(1);

            std::vector<int> nodes = {m_vtks[bm.m_dbsID]->m_nodeLabel2Id[std::stoi(GFE::Split(GFE::Trim(nodeStr.toStdString()), " ")[0])],
                                      m_vtks[bm.m_dbsID]->m_nodeLabel2Id[std::stoi(GFE::Split(GFE::Trim(nodeStr.toStdString()), " ")[1])]};


            auto frames = GFE::FO::GetFrame(db);
            std::vector<int> frameIds;
            for(int i = 0;i < frames.size();++i){
                frameIds.push_back(i);
            }

            auto U = GFE::FO::GetData(db, FOName.toStdString(), false, frameIds, nodes);
            auto foTime = GFE::FO::GetFrame(db, true);
            QVector<double> xData, yData;
            for(int i = 0;i < U.size();++i){
                xData.push_back(foTime[i]);
                yData.push_back(U[i].data[0] - U[i].data[1]);
            }

            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_UDiffTime" + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_UDiffTime"  + QString::number(cnt) + "Title";
            QString chartTitleBmText = dbName + " " + direction + u8"向最大层间位移角对应节点位移时程差";
            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

            auto pngSize = ReportWord::Attributes["CurveChart2"];
            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
            auto pngPath =  m_plot->PrintCurveChart({xData}, {yData}, "time", "U diff", {nodeStr}, 0, w, h);
            m_word->AddPicture(chartBmName, pngPath,w ,h);

            lastBmName = chartTitleBmName;
            cnt++;
        }
    }

    if(!hasVibLoad){
        retInfo = tr("Warning: No vibLoads in all dbs");
        return retInfo;
    }

    if(!hasDynamic){
        retInfo = tr("Warning: No dynamic step in all dbs");
        return retInfo;
    }

    return retInfo;
}

QString DlgReport::ExportStoreyShearForceCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

//    GFE_StoreyShear shearCalc;

//    int cnt = 1;
//    QString lastBmName = bm.m_bmName;
//    std::vector<std::string> elsetNames;
////    auto db1 = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString());
//    auto db1 = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString());
//    auto elemSetNames = GFE::getAllElementSetName(db1);
//    QString pattern = "StoryShear-floor.*";
//    for(auto &name: elemSetNames){
//        if(DlgReportUtil::IsMatch(QString::fromStdString(name), pattern)){
//            elsetNames.push_back(name);
//        }
//    }

//    for(auto &dbPath: m_css[bm.m_dbsID].dbPathList){
//        auto db = GFE::open(dbPath.toStdString(), false);
//        if(!GFE::Step::GetDynamic(db))continue;
//        QFileInfo fi(dbPath);
//        auto name = fi.completeBaseName();
//        QStringList directions = {"X", "Y"};

//        int modelDim = 3;       // 模型维度
//        int heightDir = 2;      // 高度方向
//        if(auto p = GFE::getParameter(db, "ModelDim"); !p.empty())
//            GFE::FromString(p, modelDim);
//        if(auto p = GFE::getParameter(db, "HeightDir"); !p.empty())
//            GFE::FromString(p, heightDir);

//        // if(!shearCalc.Perform(db, elsetNames, heightDir, modelDim))
//        // {
//        //     QString errStr;
//        //     switch(shearCalc.GetLastError()) {
//        //     case 1: errStr = tr("Database not found"); break;
//        //     case 2: errStr = tr("NFORC not found"); break;
//        //     }
//        // }else{
//        //     std::vector<double> storydata;
//        //     auto x_envelope = shearCalc.GetEnvelopeA();
//        //     auto y_envelope = shearCalc.GetEnvelopeB();
//        //     {
//        //         auto tmp = shearCalc.GetStorey();
//        //         storydata = std::vector<double>(tmp.begin(), tmp.end());
//        //     }
//        //     QVector<double> xData, yData;
//        //     if(directions[i] == "X"){
//        //         xData = QVector<double>::fromStdVector(x_envelope);
//        //         yData = QVector<double>::fromStdVector(storydata);
//        //     }else{
//        //         xData = QVector<double>::fromStdVector(y_envelope);
//        //         yData = QVector<double>::fromStdVector(storydata);
//        //     }

//        //     QString xTitle = directions[i] + u8"-Shear(KN)", yTitle = u8"Layer";
//        //     QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_StoreyShear" + QString::number(cnt);
//        //     QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_StoreyShear"  + QString::number(cnt) + "Title";
//        //     QString chartTitleBmText = name + " " + directions[i]  + u8"向层间剪力";
//        //     m_word->AddNewBM(lastBmName,chartBmName);
//        //     m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

//        //     auto pngSize = ReportWord::Attributes["CurveChartSDR"];
//        //     int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
//        //     auto pngPath = m_plot->PrintCurveChart({xData} ,{yData} ,xTitle, yTitle,{}, 1, w, h);
//        //     m_word->AddPicture(chartBmName, pngPath, w ,h);

//        //     lastBmName = chartTitleBmName;
//        //     cnt++;
//        // }
//    }

    return retInfo;
}

QString DlgReport::ExportEnergyCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
    QHash<QString,QString> energys = {
        {"veldamp",tr(u8"Velocity damping dissipation")},//速度型阻尼耗能
        {"dispdamp",tr(u8"Displacement damping dissipation")},//位移型阻尼耗能
        {"allpd",tr(u8"Plastic dissipation")},//塑性耗能
        {"allvd",tr(u8"Plastic dissipation")},//粘滞耗能
        {"damp",tr(u8"Damping dissipation")},//阻尼耗能
        {"allse",tr(u8"Strain energy")},//应变能
        {"allke",tr(u8"Kinetic energy")},//动能
        {"etotal", tr(u8"Total energy")},//总能量

    };

    bool hasDynamic = false;
    int cnt = 1;
    auto lastBmName = bm.m_bmName;
    //读能量数据文件
    for(auto &dbPath: m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString(), false);
        //必须为动力分析
        if(!GFE::Step::GetDynamic(db))continue;
        hasDynamic = true;
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();
        QString filePath;

        //读取数据
        filePath = fi.absolutePath() + "/" + name.split(".")[0] + ".BAEnH";

        QFile f(filePath);
        if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)){
            return tr("Warning: No energy data file!");
        }

        QStringList title;
        QVector<QVector<double>> data;

        while(!f.atEnd()){
            QString line = f.readLine();
            line.remove('\n');
            auto sp = line.split(",");
            if(sp.size() < 2)
                continue;
            if(title.empty()){
                for(auto& s:sp){
                    auto t = s.trimmed();
                    title.append(t);
                }
                continue;
            }
            QVector<double> d;
            for(auto& s:sp){
                double x = s.trimmed().toDouble();
                d.push_back(x);
            }
            data.push_back(d);
        }
        if(data.empty()){
            return tr("Warning: No energy data!");
        }
        QVector<double> xData;
        QVector<QVector<double>> yDatas;
        QHash<QString,QVector<double>> hashData;
        QString xName;
        QStringList areaNames;
        for(int j = 0;j < data[0].size();++j){
            if(title[j].toLower() != "time" &&
                !energys.contains(title[j].toLower())){
                continue;
            }
            QVector<double> yData;
            for(int i = 0;i < data.size();++i){
                auto d = data[i][j];
                if(j == 0){
                    xData.push_back(d);
                }else{
                    yData.push_back(d);
                }
            }
            if(!yData.empty()){
                hashData[title[j].toLower()] = yData;
            }
        }

        QStringList vars = {{"veldamp"},{"dispdamp"},{"damp"},{"allse"},{"allke"},{"etotal"}};

        for(auto& v:vars){
            if(v == "damp"){
                QVector<double> yData;
                for(int i = 0;i < hashData["allpd"].size() && i < hashData["allvd"].size(); ++i){
                    yData.push_back(hashData["allpd"][i] + hashData["allvd"][i]);
                }
                yDatas.push_back(yData);
            }else{
                yDatas.push_back(hashData[v]);
            }

            areaNames.push_back(energys[v]);
        }
        xName = title[0];

        double dampRatio = 0.05,
            dispDamp = hashData["dispdamp"].back()/yDatas[2].back()*dampRatio,
            velDamp = hashData["veldamp"].back()/yDatas[2].back()*dampRatio,
            titolDamo = dispDamp + velDamp + dampRatio,
            elastoplasticity = hashData["allpd"].back()/hashData["allvd"].back()*dampRatio;

        for(int i = 1;i < yDatas.size();++i){
            for(int j = 0;j < yDatas[i].size();++j){
                yDatas[i][j] = yDatas[i - 1][j] + yDatas[i][j];
            }
        }

        auto pngSize = ReportWord::Attributes["CurveChart2"];
        int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();

        auto pngPath = m_plot->PrintEnergyCurveChart({xData},yDatas,"Time","Energy",areaNames,0,w,h);
        QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_Energy" + QString::number(cnt);
        QString chartTextBmName = chartBmName += "Text";
        QString chartBmText = u8"\n图" + m_section + "-" + QString::number(cnt) + " " + name + u8" 能量图";
        QString infoText = u8"结构初始阻尼比: " + QString::number(dampRatio * 100) + "%\n" +
                           u8"位移型阻尼器：" + QString::number(dispDamp * 100) + "%\n" +
                           u8"速度型阻尼器：" + QString::number(velDamp * 100) + "%\n" +
                           u8"结构弹塑性：" + QString::number(elastoplasticity * 100) + "%\n" +
                           u8"总等效阻尼比：" + QString::number(titolDamo * 100) + "%\n";


        chartBmText += "\n" + infoText;

        m_word->AddNewBM(lastBmName,chartBmName);
        m_word->AddNewBM(chartBmName,chartTextBmName,chartBmText,"title");
        m_word->AddPicture(chartBmName, pngPath, w, h);

        lastBmName = chartTextBmName;
        ++cnt;
    }

    if(!hasDynamic){
        retInfo = tr("Wanirng: There is no dynamic analysis step for any db!");
        return retInfo;
    }

    return retInfo;
}

QString DlgReport::ExportVibCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_param) {
    case ReportBookMark::Time:
        retInfo = ExportVibTimeCurveChart(bm);
        break;
    case ReportBookMark::EERA:
        retInfo = ExportVibEERACurveChart(bm);
        break;
    case ReportBookMark::E2X:
        retInfo = ExportVibE2TimeCurveChart(bm);
        break;
    case ReportBookMark::E2Y:
        retInfo = ExportVibE2TimeCurveChart(bm);
        break;
    case ReportBookMark::Energy:
        retInfo = ExportEnergyCurveChart(bm);
        break;
    case ReportBookMark::Spectrum:
        retInfo = ExportVibSpectrumCurveChart(bm);
        break;
    default:
        break;
    }
    return retInfo;
}

QString DlgReport::ExportVibTimeCurveChart(const ReportBookMark &bm)
{
    int cnt = 1,picCnt = 1;
    QString lastBmName = bm.m_bmName;
    bool hasAcce = false;
    for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString(), false);
        auto vibs = GFE::getAllVibLoad(db);
        if(vibs.empty())
            continue;
        auto vib = vibs[0];
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();
        //子标题
        hasAcce = false;
        for(const QString& dir : {"X", "Y", "Z"}){
            // 读取数据
            auto ampName = vib.param["Amp" + dir.toStdString()];
            auto amp = GFE::getFunction(db, ampName);

            if(amp == nullptr)
            {
                if(dir == "Z" && hasAcce == false)
                    break;
                continue;
            }
            QVector<double> xData, yData;
            int size = amp->values.size();
            for(int i = 0; i < size; ++i)
            {
                if(i % 2 == 0)
                    xData.push_back(amp->values[i]);
                else
                    yData.push_back(amp->values[i]);
            }
            if(xData.empty() || yData.empty())
                continue;
            hasAcce = true;
            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_Acce" + dir + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_" + dir + QString::number(cnt) + "Title";
            QString chartTitleBmText = u8"图" + m_section + "-" + QString::number(picCnt++) + " "
                                       + name + "(" + dir + u8"向)\r";
            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");
            m_word->AddChart(chartBmName,{{xData,yData}},u8"时间(s)",u8"加速度(m/s2)");
            lastBmName = chartTitleBmName;
            cnt++;
        }
    }
    if(!hasAcce){
        m_word->WriteText(bm.m_bmName,u8"未找到地震动时程数据文件");
    }
    return "";
}

QString DlgReport::ExportVibE2TimeCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
    QStringList dbPathList;

    bool hasE2 = false;
    int cnt = 1;
    bool hasDynamic = false, hasVibLoad = false;
    QString lastBmName = bm.m_bmName;
    for(auto& dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;
        auto vibLoads = GFE::getAllVibLoad(db);
        if(vibLoads.empty())continue;
        hasVibLoad = true;

        QFileInfo fi(dbPath);
        auto dbName = fi.completeBaseName();

        std::map<std::string, std::string> ampNames;

        for(auto& v : vibLoads){
            if(v.param["AmpX"] != ""){
                ampNames["X"] = v.param["AmpX"];
            }
            if(v.param["AmpY"] != ""){
                ampNames["Y"] = v.param["AmpY"];
            }
            if(v.param["AmpZ"] != ""){
                ampNames["Z"] = v.param["AmpZ"];
            }
        }

        for(auto& it : ampNames){
            auto amp = GFE::getFunction(db, it.second);
            if(!amp)continue;
            QVector<double> xData, yData;
            int size = amp->values.size();
            for(int i = 0; i < size; ++i)
            {
                if(i % 2 == 0)
                    xData.push_back(amp->values[i]);
                else
                    yData.push_back(amp->values[i]);
            }
            if(xData.empty() || yData.empty())
                continue;

            hasE2 = true;
            QString chartTitleText = dbName + " " + QString::fromStdString(it.first) + u8"向地震波";
            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_E2_TimeHistory_" + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_E2_TimeHistory" + QString::number(cnt);
            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleText,"title");

            auto pngSize = ReportWord::Attributes["CurveChart2"];
            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
            auto pngPath = m_plot->PrintCurveChart({xData}, {yData}, "Time(s)", "Acceleration(m/s^2)", {"Time history of ground motion"}, 0, w, h);
            m_word->AddPicture(chartBmName, pngPath, w, h);

            lastBmName = chartTitleBmName;
            cnt++;
        }
    }

    if(!hasVibLoad){
        retInfo = tr("NoData: The db composition has no vibration load!");
        m_word->WriteText(bm.m_bmName, retInfo);
        return "";
    }

    if(!hasDynamic){
        retInfo = tr("Warning: No dynamic step in all dbs");
        return retInfo;
    }

    if(!hasE2){
        retInfo = tr("Warning: There is no E2 earthquake history data in the model");
        return retInfo;
    }

    return retInfo;
}

QString DlgReport::ExportVibEERACurveChart(const ReportBookMark &bm)
{
    int cnt = 1,picCnt = 1;
    QString lastBmName = bm.m_bmName;
    bool hasEERA = false;

    auto helper = [&](const QString& bmPre, const QString& filePre, const QString& axisTitle, const QString& dbPath)
    {
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();
        for(const QString& dir : {"X", "Y", "Z"})
        {
            QFileInfo fi(dbPath);
            QFile f(fi.absolutePath() + "/EERA/" + filePre + dir + u8"向地震.txt");
            if(!f.exists())
                continue;
            if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            f.readLine();
            QVector<double> xData, yData;
            while(!f.atEnd())
            {
                QString line = f.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                xData.push_back(sp[0].trimmed().toDouble());
                yData.push_back(-1 * sp[1].trimmed().toDouble());
            }
            f.close();
            if(xData.empty() || yData.empty())
                continue;
            hasEERA = true;
            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_" + bmPre + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_" + bmPre + QString::number(cnt) + "Title";
            QString chartTitleText = u8"图" + m_section + "-" + QString::number(picCnt++) +
                                     " " + name + axisTitle + u8"随深度变化曲线\r";

            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleText,"title");
            m_word->AddChart(chartBmName,{{xData,yData}},axisTitle,u8"深度(m)");

            lastBmName = chartTitleBmName;
            cnt++;
        }
    };

    for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList){
        hasEERA = false;
        helper("DampRatio", u8"阻尼比_", u8"阻尼比", dbPath);
        if(!hasEERA){
            QString bmName = "Chart_DampRatio" + QString::number(cnt);
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName();
            m_word->AddNewBM(lastBmName,bmName,u8"在" + name + u8"目录中未找到阻尼比文件数据");
            lastBmName = bmName;
        }
        hasEERA = false;
        helper("ShearStress",u8"最大应力_", u8"剪应力峰值", dbPath);
        if(!hasEERA){
            QString bmName = "Chart_ShearStress" + QString::number(cnt);
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName();
            m_word->AddNewBM(lastBmName,bmName,u8"在" + name + u8"目录中未找到最大应力文件数据");
            lastBmName = bmName;
        }
        hasEERA = false;
        helper("ShearStrain", u8"最大应变_", u8"剪应变峰值", dbPath);
        if(!hasEERA){
            QString bmName = "Chart_ShearStrain" + QString::number(cnt);
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName();
            m_word->AddNewBM(lastBmName,bmName,u8"在" + name + u8"目录中未找到最大应变数据");
            lastBmName = bmName;
        }
    }

    return "";
}

QString DlgReport::ExportAMPCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::SeismWave:
        retInfo = ExportAMPSeismWaveCurveChart(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportAMPSeismWaveCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

    int cnt = 1;
    QString lastBmName = bm.m_bmName;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString(), false);
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();
        QVector<std::shared_ptr<GFE::RegularBC>> rbcs;
        auto bcs = GFE::BC::All<GFE::RegularBC>(db);
        for(auto& bc : bcs){
            if(!bc->func_name.empty() && bc->TypeStr() == "GRAVITY"){
                rbcs.push_back(bc);
            }
        }

        for(auto& rbc : rbcs){
            std::shared_ptr<GFE::Function> amp;
            double multix = rbc->values[0],
                multiy = rbc->values[1],
                multiz = rbc->values[2];
            amp = GFE::getFunction(db,rbc->func_name);
            if(!amp || rbc->values.size() < 3)continue;
            if(multix == 0 && multiy == 0 && multiz == 0){
                continue;
            }

            double multi = 0;
            QString direction;
            if(multix != 0){
                multi = multix;
                direction = "X";
            }
            if(multiy != 0){
                multi = multiy;
                direction = "Y";
            }
            if(multiz != 0){
                multi = multiz;
                direction = "Z";
            }

            QVector<double> xData, yData;
            int size = amp->values.size();
            for(int i = 0; i < size; ++i)
            {
                if(i % 2 == 0)
                    xData.push_back(amp->values[i]);
                else{
                    if(multi != 0){
                        yData.push_back(amp->values[i] * multi);
                    }else{
                        yData.push_back(amp->values[i]);
                    }
                }

            }
            if(xData.empty() || yData.empty())
                continue;

            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismWave" + QString::number(cnt);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismWave"  + QString::number(cnt) + "Title";
            QString chartTitleBmText = name + " " + direction + u8" 向地震波曲线";
            m_word->AddNewBM(lastBmName, chartBmName);
            m_word->AddNewBM(chartBmName, chartTitleBmName, chartTitleBmText, "title");

            auto pngSize = ReportWord::Attributes["CurveChart1"];
            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
            auto pngPath = m_plot->PrintCurveChart({xData}, {yData},"Time(s)","Acceleration(m/s^2)",{QString::fromStdString(rbc->func_name)}, 0, w, h);
            m_word->AddPicture(chartBmName, pngPath, w ,h);
            lastBmName = chartTitleBmName;
            cnt++;

        }
    }

    return retInfo;
}

QString DlgReport::ExportCombCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_param) {
    case ReportBookMark::ACR:
        retInfo = ExportCombACRCurveChart(bm);
        break;
    default:
        break;
    }
    return retInfo;
}

QString DlgReport::ExportCombACRCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
    QString lastBmName = bm.m_bmName;
    int picCnt = 1;
    if(m_css[bm.m_dbsID].acrLayer.empty()){
        m_word->WriteText(bm.m_bmName,u8"未指定写出轴压比的楼层");
        return tr("Warning: Floors with written axial pressure ratios not specified!");
    }
    m_acrFit = "";
    auto acrRes = m_caculates[bm.m_dbsID]->m_pjResult.acr;
    bool hasACR = false;
    auto helper = [&](const QString& type, int idx, double limit, const QString& cnType)
    {
//        int cnt = 1;
        int size = std::min(25, (int)acrRes.size());

        QVector<QVector<double>> xDatas, yDatas;
        xDatas.resize(size);
        yDatas.resize(size);
        QVector<std::tuple<int, int, int>> check;
        for(int i = 0; i < size; ++i)
        {
            auto res = acrRes[i][idx];
            for(const auto& [pair, val] : res)
            {
                xDatas[i].push_back(val);
                yDatas[i].push_back(pair.first);
                if(val > limit)
                    check.push_back({i + 1, pair.first, pair.second});
            }
            if(xDatas[i].empty()||yDatas[i].empty())continue;
            hasACR = true;
            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_" + type + "Acr" + QString::number(i + 1);
            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_" + type + "Acr" + QString::number(i + 1) + "Title";
            QString chartTitleText = u8"图" + m_section + "-" + QString::number(picCnt++) + " " +
                                     u8" 组合" + QString::number(i + 1) +
                                     cnType + u8"轴压比散点图\r";
            m_word->AddNewBM(lastBmName,chartBmName);
            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleText,"title");
            m_word->AddChart(chartBmName,{{xDatas[i],yDatas[i]}}, u8"轴压比", u8"楼层", -4169);
            lastBmName = chartTitleBmName;
        }

        // 限值检查
        if(check.empty())
        {
            QString fitBmName = NewBMPrefix(bm.m_dbsID) + "ACR" + type + "Fit2";
            auto fit = cnType + u8"轴压比满足规范要求；\r";
            m_word->AddNewBM(lastBmName,fitBmName,fit);
            m_acrFit += fit;
            lastBmName = fitBmName;
        }else{
            QString fitTableTitleBmName = NewBMPrefix(bm.m_dbsID) + "ACR" + type + "Fit_Table_Title";
            QString fitTableTitleText = u8"表" + m_section + "-1 " + type + u8"轴压比不满足规范要求信息表";
            QString fitTableBmName = NewBMPrefix(bm.m_dbsID) + "ACR_" + type + "_Fit_Table";
            QStringList header = {u8"组合",u8"楼层",u8"构件编号",u8"类型",u8"组合",u8"楼层",u8"构件编号",u8"类型"};
            QVector<QStringList> info;
            for(int i = 0;i < check.size();i+=2){
                QStringList sl;
                auto c1 = check[i];
                sl.append({QString::number(std::get<0>(c1)),QString::number(std::get<1>(c1)),QString::number(std::get<2>(c1)),cnType});
                if(i+1<check.size()){
                    auto c2 = check[i+1];
                    sl.append({QString::number(std::get<0>(c2)),QString::number(std::get<1>(c2)),QString::number(std::get<2>(c2)),cnType});
                }
                info.push_back(sl);
            }

            m_word->AddNewBM(lastBmName,fitTableTitleBmName,fitTableTitleText,"title");
            m_word->AddNewBM(fitTableTitleBmName,fitTableBmName);
            m_word->AddTable(fitTableBmName,header,info);

            lastBmName = "ACR" + type + "Fit2";
        }
    };
    if(bm.m_extra[0] == "Col"){
        hasACR = false;
        helper("Col", 0, 0.75, u8"柱");
        if(!hasACR)m_word->WriteText(bm.m_bmName,u8"未找到柱轴压比数据");
        m_acrFit.chop(1);
        m_acrFit += u8"，";
    }
    if(bm.m_extra[0] == "Wall"){
        hasACR = false;
        helper("Wall", 1, 0.6, u8"墙");
        if(!hasACR)m_word->WriteText(bm.m_bmName,u8"未找到墙轴压比数据");
    }

    return retInfo;
}

QString DlgReport::ExportXYDataCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Hysteretic:
        ExportXYDataHystereticCurveChart(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportXYDataHystereticCurveChart(const ReportBookMark &bm)
{
    QString retInfo;
//    int dbsId = bm.m_dbsID;
//    auto cs = m_css[dbsId];

//    int cnt = 1;
//    bool hasDynamic = false;
//    QString lastBmName = bm.m_bmName;
//    for(int idx = 0;idx < cs.dbPathList.size();++idx){
//        auto dbPath = cs.dbPathList[idx];
//        auto db = GFE::open(dbPath.toStdString());
//        if(!GFE::Step::GetDynamic(db))continue;
//        hasDynamic = true;

//        auto caseName = cs.caseNames[idx];

//        //todo 选取那个单元
//        std::vector<int> elems = {m_vtks[dbsId]->m_elementLabel2Id[8520]};

//        auto frames = GFE::FO::GetFrame(db);
//        std::vector<int> frameIds;
//        for(int i = 0;i < frames.size();++i){
//            frameIds.push_back(i);
//        }

//        //todo 如何确定方向
//        auto seData = GFE::FO::GetData(db, "SE SE1", true, frameIds, {elems});
//        auto sfData = GFE::FO::GetData(db, "SF SF1", true, frameIds, {elems});
//        auto foTime = GFE::FO::GetFrame(db, true);

//        int n = foTime.size();
//        std::shared_ptr<mup::xy::Combine> combine(new mup::xy::Combine);
//        mup::ptr_val_type ret(new mup::Value), args[2];
//        mup::Value *seXYData = new mup::Value(n, 2, 0),
//            *sfXYData = new mup::Value(n, 2, 0);

//        for(int i = 0;i < n;++i){
//            seXYData->At(i, 0) = foTime[i];
//            seXYData->At(i, 1) = seData[i].data[0];
//            sfXYData->At(i, 0) = foTime[i];
//            sfXYData->At(i, 1) = sfData[i].data[0];
//            QD seData[i].data[0] << sfData[i].data[0];
//        }

//        args[0].Reset(seXYData);
//        args[1].Reset(sfXYData);

//        combine->Eval(ret, args, 2);

//        QVector<double> retXData,retYData;
//        auto& hysteretic = ret.Get()->At(0);
//        int r = hysteretic.GetRows();

//        for(int i = 0;i < r;++i){
//            retXData.push_back(hysteretic.At(i, 0).GetFloat());
//            QD i << hysteretic.At(i, 0).GetFloat();
////            retYData.push_back(hysteretic.At(i, 1).GetFloat());
////            QD i << hysteretic.At(i, 1).GetFloat();
//        }

//        QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_Hysteretic" + QString::number(cnt);
//        QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_Hysteretic"  + QString::number(cnt) + "Title";
//        QString chartTitleBmText = caseName + u8" 滞回曲线";
//        m_word->AddNewBM(lastBmName,chartBmName);
//        m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

//        auto pngSize = ReportWord::Attributes["CurveChart1"];
//        int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
//        auto pngPath = m_plot->PrintCurveChart({retXData}, {retYData}, "Strain", "Stress",{}, 0, w, h);
//        m_word->AddPicture(chartBmName, pngPath, w ,h);

//        lastBmName = chartTitleBmName;
//        cnt++;
//    }

//    if(!hasDynamic){
//        retInfo = tr("Warning: No dynamic step in all dbs");
//        return retInfo;
//    }

    return retInfo;
}

QString DlgReport::ExportVibSpectrumCurveChart(const ReportBookMark &bm)
{
    QString retInfo;

//    int cnt = 1;
//    QString lastBmName = bm.m_bmName;
//    bool hasDynamic = false;
//    for(auto &dbPath : m_css[bm.m_dbsID].dbPathList){
//        auto db = GFE::open(dbPath.toStdString());
//        if(!GFE::Step::GetDynamic(db))continue;
//        hasDynamic = true;
//        QFileInfo fi(dbPath);
//        auto name = fi.completeBaseName();
//        QVector<std::shared_ptr<GFE::RegularBC>> rbcs;
//        auto bcs = GFE::BC::All<GFE::RegularBC>(db);
//        for(auto& bc : bcs){
//            if(!bc->func_name.empty() && bc->TypeStr() == "GRAVITY"){
//                rbcs.push_back(bc);
//            }
//        }

//        for(auto& rbc : rbcs){
//            std::shared_ptr<GFE::Function> amp;
//            double multix = rbc->values[0],
//                   multiy = rbc->values[1],
//                   multiz = rbc->values[2];
//            amp = GFE::getFunction(db,rbc->func_name);
//            if(!amp || rbc->values.size() < 3)continue;
//            if(multix == 0 && multiy == 0 && multiz == 0){
//                continue;
//            }

//            double multi = 0;
//            QString direction;
//            if(multix != 0){
//                multi = multix;
//                direction = "X";
//            }
//            if(multiy != 0){
//                multi = multiy;
//                direction = "Y";
//            }
//            if(multiz != 0){
//                multi = multiz;
//                direction = "Z";
//            }

//            QVector<double> xData, yData;
//            int size = amp->values.size();
//            for(int i = 0; i < size; ++i)
//            {
//                if(i % 2 == 0)
//                    xData.push_back(amp->values[i]);
//                else{
//                    if(multi != 0){
//                        yData.push_back(amp->values[i] * multi);
//                    }else{
//                        yData.push_back(amp->values[i]);
//                    }
//                }

//            }
//            if(xData.empty() || yData.empty())
//                continue;

//            std::shared_ptr<mup::xy::RespSpectrum> rs(new mup::xy::RespSpectrum);
//            mup::ptr_val_type ret(new mup::Value), args[4];
//            mup::Value *xyData = new mup::Value(xData.size(),2,0),
//                       *znb =  new mup::Value(0.05) ,
//                       *step = new mup::Value(0.02),
//                       *end = new mup::Value(12);

//            for(int i = 0;i < xData.size(); ++i){
//                xyData->At(i, 0) = xData[i];
//                xyData->At(i, 1) = yData[i];
//            }

//            args[0].Reset(xyData);
//            args[1].Reset(znb);
//            args[2].Reset(step);
//            args[3].Reset(end);

//            rs->Eval(ret, args, 4);

//            QVector<double> retXData,retYData;
//            QVector<QVector<double>> xDatas, yDatas;

//            auto& aT = ret.Get()->At(1);
//            int r = aT.GetRows();

//            for(int i = 0;i < r;++i){
//                retXData.push_back(aT.At(i, 0).GetFloat());
//                retYData.push_back(aT.At(i, 1).GetFloat());
//            }


//            xDatas.push_back(retXData);
//            yDatas.push_back(retYData);

//            QString chartBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismSpectrum" + QString::number(cnt);
//            QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + "Chart_SeismSpectrum"  + QString::number(cnt) + "Title";
//            QString chartTitleBmText = name + " " + direction + u8"向地震动谱";
//            m_word->AddNewBM(lastBmName,chartBmName);
//            m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");

//            auto pngSize = ReportWord::Attributes["CurveChart1"];
//            int w = pngSize["width"].toInt(), h = pngSize["height"].toInt();
//            auto pngPath = m_plot->PrintCurveChart(xDatas,yDatas,"Time","Amax",{QString::fromStdString(rbc->func_name)}, 0, w, h);
//            m_word->AddPicture(chartBmName, pngPath, w ,h);

//            lastBmName = chartTitleBmName;
//            cnt++;
//        }
//    }

//    if(!hasDynamic){
//        retInfo = tr("Warning: Databases has no dynamic step!");
//        return retInfo;
//    }

    return retInfo;
}

QString DlgReport::ExportCloudChart(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_gfeType) {
    case ReportBookMark::Comb:
        retInfo = ExportCombCloudChart(bm);
        break;
    case ReportBookMark::Vibration:
        retInfo = ExportVibrationCloudChart(bm);
        break;
    case ReportBookMark::Damage:
        retInfo = ExportDamageCloudChart(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportCombCloudChart(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_param) {
    case ReportBookMark::AxialForce:{
        QString maxSF1Comb = m_caculates[bm.m_dbsID]->m_maxSF1Comb;
        auto eleForce = m_caculates[bm.m_dbsID]->m_pjResult.eleForce[std::pair<std::string,std::string>{"SF1",maxSF1Comb.toStdString()}];
        retInfo = ExportCombInterForceCloudChart(bm, "SF1 " + maxSF1Comb ,eleForce);
        break;
    }
    case ReportBookMark::ShearForce:{
        QString maxSF2Comb = m_caculates[bm.m_dbsID]->m_maxSF2Comb;
        auto eleForce = m_caculates[bm.m_dbsID]->m_pjResult.eleForce[std::pair<std::string,std::string>{"SF2",maxSF2Comb.toStdString()}];
        retInfo = ExportCombInterForceCloudChart(bm, "SF2 " + maxSF2Comb,eleForce);
        break;
    }
    case ReportBookMark::Bending:{
        QString maxSM1Comb = m_caculates[bm.m_dbsID]->m_maxSM1Comb;
        auto eleForce = m_caculates[bm.m_dbsID]->m_pjResult.eleForce[std::pair<std::string,std::string>{"SM1",maxSM1Comb.toStdString()}];
        retInfo = ExportCombInterForceCloudChart(bm, "SM1 " + maxSM1Comb,eleForce);
        break;
    }
    case ReportBookMark::ACR:{
        retInfo = ExportCombACRCloudChart(bm);
        break;
    }
    default:
        break;
    }

    return "";
}

QString DlgReport::ExportCombInterForceCloudChart(const ReportBookMark &bm, const QString &forceName, std::vector<std::pair<int, double> > &eleForce)
{
    QString retInfo;
    auto db = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString(),false);

    for(auto &ef:eleForce){
        ef.first = m_vtks[bm.m_dbsID]->m_elementLabel2Id[ef.first];
    }
    std::vector<float> colorArr(GFE::getElementNum(db));

    for(auto it:eleForce){
        colorArr[it.first] = (float)it.second;
    }

    ReportVTK::Option opt;
    auto pngPath = m_tDir.path() + "/temp.png";
    if(m_mp[bm.m_dbsID].dim == 2){
        opt.isCloud = true,
        opt.isLabel = true,
        opt.isCell = true;
        opt.isZoom = true;
        opt.zoomFactor = 1.3;
        retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, {}, forceName, colorArr);
    }else{
        opt.isCloud = true,
        opt.isCell = true;
        opt.isZoom = true;
        opt.zoomFactor = 1.5;
        retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, {"jiegou"}, forceName, colorArr);
    }

    if(!retInfo.isEmpty()){
        return retInfo;
    }

    if(m_mp[bm.m_dbsID].dim == 2){
        auto size = ReportWord::Attributes["CloudChart2D"];
        int height = size["height"].toInt(), width = size["width"].toInt();
        m_word->AddPicture(bm.m_bmName, pngPath, width, height);
    }else{
        auto size = ReportWord::Attributes["CloudChart3D"];
        int height = size["height"].toInt(), width = size["width"].toInt();
        m_word->AddPicture(bm.m_bmName, pngPath, width, height);
    }

    return retInfo;
}

QString DlgReport::ExportCombACRCloudChart(const ReportBookMark &bm)
{
    QString retInfo;
    //读gjdy文件
    int combId = 0; //todo

    auto gjdyPath = m_css[bm.m_dbsID].gjdyList[0];
    QFile gjdyFile(gjdyPath);
    if(!gjdyFile.exists() || !gjdyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        retInfo = tr("Warning: Open gjdy File failed");
        return retInfo;
    }

    //构件ID-构件单元
    std::unordered_map<std::string,std::unordered_set<int>> comps;
    std::vector<std::string> compIDs;
    std::string compID = "";

    while(!gjdyFile.atEnd()){
        QString line = gjdyFile.readLine();
        if(line.toStdString().substr(0,4)=="Part"){
            compID = line.toStdString().substr(6);
            compID.pop_back();
            compIDs.push_back(compID);
        }
        else{
            auto eleIDs = line.split(" ");
            for(auto &eleID:eleIDs){
                bool ok;
                int id = eleID.toInt(&ok);
                if(ok){
                    comps[compID].insert(id);
                }
            }
        }
    }

    std::unordered_map<int,std::unordered_set<int>> compsId;
    for(auto idstr:compIDs){
        int i,id;
        for(i=0;i<idstr.size();++i){
            if(idstr[i]=='-')break;
        }
        id = std::stoi(idstr.substr(i+1));
        compsId[id] = comps[idstr];
    }

    auto db = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString(),false);
    if(m_caculates[bm.m_dbsID]->m_pjResult.acr.empty()){
        retInfo = tr("NoData: No acr data!");
        m_word->WriteText(bm.m_bmName, retInfo);
        return "";
    }
    std::array<GFECaculate::AcrResult,2> data = m_caculates[bm.m_dbsID]->m_pjResult.acr[combId];
    std::vector<float> colorArr(GFE::getElementNum(db),NAN);

    for(int i=0;i<data.size();++i){
        for(auto it:data[i]){
            int id = it.first.second;//构件id
            double acr = it.second;//构件轴压比
            //        qDebug()<<it.first.first<<id<<acr;
            for(auto eleId:compsId[id]){
                colorArr[m_vtks[bm.m_dbsID]->m_elementLabel2Id[eleId]] = acr;
            }
        }
    }

    QVector<QStringList> setsNames;
    QVector<QStringList> patternsVec;
    QVector<QStringList> compsNames = {{"col"}, {"wall"}};
    auto elemNames = GFE::getAllElementSetName(db);
    for(auto& comps : compsNames){
        QStringList setsName;
        for(auto& comp : comps){
            QString pattern;
            if(comp == "col")pattern = "^floor.*AllCol$";
            if(comp == "wall")pattern = "^floor.*AllWall$";
            if(comp == "beam")pattern = "^floor.*AllBeam$";
            if(comp == "slab")pattern = "^floor.*AllSlab$";
            QRegularExpression regex(pattern);
            for(auto& name:elemNames){
                auto t_name = QString::fromStdString(name);
                if(regex.match(t_name).hasMatch()){
                    setsName.push_back(t_name);
                }
            }
        }
        setsNames.push_back(setsName);
    }

    QString lastBmName = bm.m_bmName;
    int cnt = 1;
    for(int i = 0;i < compsNames.size();++i){
        auto setsName = setsNames[i];
        auto setsKey = DlgReportUtil::ConnectStrs(compsNames[i], "_");
        ReportVTK::Option opt;
        opt.isCloud = true, opt.isCell = true, opt.isZoom = true, opt.zoomFactor = 1.5;
        auto pngPath = m_tDir.path() + "/temp.png";
        retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, setsName, "ACR Comb" + QString::number(combId + 1) ,colorArr);
        if(!retInfo.isEmpty()){
            return retInfo;
        }

        QString chartBmName = NewBMPrefix(bm.m_dbsID) + bm.m_bmName + "_CloudChart_ACR" + QString::number(cnt);
        QString chartTitleBmName = NewBMPrefix(bm.m_dbsID) + bm.m_bmName + "_CloudChart_ACR_Title" + QString::number(cnt);
        QString chartTitleBmText;
        if(setsKey == "col")chartTitleBmText += u8"柱轴压比云图";
        if(setsKey == "wall")chartTitleBmText += u8"墙轴压比云图";

        m_word->AddNewBM(lastBmName,chartBmName);
        m_word->AddNewBM(chartBmName,chartTitleBmName,chartTitleBmText,"title");
        auto size = ReportWord::Attributes["CloudChart3D"];
        int w = size["width"].toInt(), h = size["height"].toInt();
        m_word->AddPicture(chartBmName, pngPath, w, h);
        lastBmName = chartTitleBmName;
        cnt++;
    }

    return retInfo;
}

QString DlgReport::ExportVibrationCloudChart(const ReportBookMark &bm)
{

    QString retInfo;
    QString FOName, pattern, replaceText;
    QStringList dbPathList;
    int method = 0;

    int picCnt = 1;
    QString lastbmName = bm.m_bmName;
    bool hasDynamic = false, hasVibLoad = false, hasDirection = false;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;
        // 读取数据
        QFileInfo fi(dbPath);
        auto dbName = fi.completeBaseName();

        auto vibLoads = GFE::getAllVibLoad(db);
        if(vibLoads.empty())continue;
        hasVibLoad = true;

        QStringList directions;
        for(auto& v : vibLoads){
            if(v.param["AmpX"] != ""){
                directions.push_back("X");
                replaceText += u8"X、";
            }
            if(v.param["AmpY"] != ""){
                directions.push_back("Y");
                replaceText += u8"Y、";
            }
            if(v.param["AmpZ"] != ""){
                directions.push_back("Z");
                replaceText += u8"Y、";
            }
        }

        if(directions.empty())continue;
        hasDirection = true;

        if(bm.m_extra[0] == "Max")method = 2;
        for(auto& direction : directions){
            if(direction == "X"){
                FOName = "U U1";
            }
            if(direction == "Y"){
                FOName = "U U2";
            }
            std::vector<GFE::data_t> data;
            auto eoData = GFE::EO::GetData(db,method,FOName.toStdString());
            if(eoData){
                data = eoData->data;
            }else{
                data = m_caculates[bm.m_dbsID]->GetEnvelopeData(db,FOName,"Max");
            }

            if(data.empty()){
                continue;
            }

            ReportVTK::Option opt;
            opt.isCloud = true, opt.isZoom = true, opt.zoomFactor = 1.5;
            //todo
            auto pngPath = m_tDir.path() + "/temp.png";
            retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, {"jiegou"}, FOName, data);
            if(!retInfo.isEmpty()){
                return retInfo;
            }

            QString chartBmName,chartTitleBmName,chartTitleText;
            if(bm.m_param == ReportBookMark::E2X){
                chartBmName = "CloudChart_E2X" + QString::number(picCnt);
                chartTitleBmName = "CloudChart_E2X_Title" + QString::number(picCnt);
            }else if(bm.m_param == ReportBookMark::E2Y){
                chartBmName = "CloudChart_E2Y" + QString::number(picCnt);
                chartTitleBmName = "CloudChart_E2Y_Title" + QString::number(picCnt);
            }
            chartTitleText = direction + u8"向地震波(" + QString::fromStdString(GFE::FileInfo::Name(dbPath.toStdString())) + u8")最大位移包络云图" ;
            m_word->AddNewBM(lastbmName, chartBmName);
            m_word->AddNewBM(chartBmName, chartTitleBmName, chartTitleText, "title");
            auto size = ReportWord::Attributes["CloudChart3D"];
            m_word->AddPicture(chartBmName,pngPath,size["width"].toInt(),size["height"].toInt());

            lastbmName = chartTitleBmName;
            picCnt++;
        }
    }

    if(!hasVibLoad || !hasDirection){
        retInfo = tr("Warning: No vibLoads in all dbs");
        return retInfo;
    }

    if(!hasDynamic){
        retInfo = tr("Warning: No dynamic step in all dbs");
        return retInfo;
    }

    return retInfo;
}

QString DlgReport::ExportDamageCloudChart(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Damagec:
        ExportDamageCCloudChart(bm);
        break;
    case ReportBookMark::Level:
        ExportDamageLevelCloudChart(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportDamageCCloudChart(const ReportBookMark &bm)
{
    QString retInfo;

    bool hasDynamic = false;
    QString lastBmName = bm.m_bmName;
    int cnt = 1;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }

        hasDynamic = true;

        auto elemNames = GFE::getAllElementSetName(db);

        QString pattern;
        if(bm.m_extra[0] == "Wall")pattern = "^floor.*AllWall$";
        if(bm.m_extra[0] == "Col")pattern = "^floor.*AllCol$";
        if(bm.m_extra[0] == "Beam")pattern = "^floor.*AllBeam$";

        QStringList setsName;
        QRegularExpression regex(pattern);
        for(auto& name:elemNames){
            auto t_name = QString::fromStdString(name);
            if(regex.match(t_name).hasMatch()){
                setsName.append(t_name);
            }
        }

        //todo 获取最后一帧的数据
        QString FOName = "DAMAGEC DAMAGEC";
        int frame = (int)GFE::FO::GetFrame(db,true).size() - 1;
        auto data = GFE::FO::GetData(db,FOName.toStdString(),true,frame);

        if(data.empty()){
            retInfo = tr("Warning: No damagec cloud chart data");
            m_word->WriteText(bm.m_bmName, retInfo);
            continue;
        }

        ReportVTK::Option opt;
        opt.isCloud = true, opt.isCell = true;

        auto pngPath = m_tDir.path() + "/temp.png";
        retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, setsName, FOName, data);
        if(!retInfo.isEmpty()){
            return retInfo;
        }

        QString chartBmName = NewBMPrefix(bm.m_dbsID) + bm.m_bmName + "_Chart" + QString::number(cnt);
        QString titleBmName = NewBMPrefix(bm.m_dbsID) + bm.m_bmName + "_Title" + QString::number(cnt);
        QString titleText;

        titleText += QString::fromStdString(GFE::FileInfo::Name(dbPath.toStdString()));
        if(bm.m_extra[0] == "Wall")titleText += u8"剪力墙";
        if(bm.m_extra[0] == "Col")titleText += u8"框架柱";
        if(bm.m_extra[0] == "Beam")titleText += u8"框架梁";
        titleText += u8"损伤";

        m_word->AddNewBM(lastBmName, chartBmName);
        m_word->AddNewBM(chartBmName, titleBmName, titleText, "title");
        auto size = ReportWord::Attributes["CloudChart3D"];
        m_word->AddPicture(chartBmName, pngPath, size["width"].toInt(), size["height"].toInt());
        lastBmName = titleBmName;
        cnt++;
    }

    if(!hasDynamic){
        retInfo = tr("No dynamic step in db combination");
    }

    return retInfo;
}

QString DlgReport::ExportDamageLevelCloudChart(const ReportBookMark &bm)
{
    QString retInfo;
    QStringList setsName;

//    bool hasDynamic = false, evalFailed = true;
//    for(int i = 0;i < m_css[bm.m_dbsID].dbPathList.size();++i){
//        auto dbPath = m_css[bm.m_dbsID].dbPathList[i];
//        auto db = GFE::open(dbPath.toStdString());

//        if(!GFE::Step::GetDynamic(db)){
//            continue;
//        }

//        hasDynamic = true;
//        auto elemNames = GFE::getAllElementSetName(db);

//        QString pattern;
//        if(bm.m_extra[0] == "Wall")pattern = "^floor.*AllWall$";
//        if(bm.m_extra[0] == "Col")pattern = "^floor.*AllCol$";
//        if(bm.m_extra[0] == "Beam")pattern = "^floor.*AllBeam$";

//        QRegularExpression regex(pattern);
//        for(auto& name:elemNames){
//            auto t_name = QString::fromStdString(name);
//            if(regex.match(t_name).hasMatch()){
//                setsName.append(t_name);
//            }
//        }

//        QString FOName = "DAMAGELEVEL Element";

//        std::array<std::array<double, 6>, 6> limit = {{
//            {0,0,1,3,6,12},
//            {0,0.1,0.2,0.4,0.6,0.8},
//            {0,0.3,1,1,1,1},
//            {0,0,1,3,6,12},
//            {0,0.1,0.2,0.4,0.6,0.8},
//            {0,0.3,1,1,1,1},
//        }};

//        QMap<int, QColor> cm = {
//            {0, QColor(0,0,200)},
//            {1, QColor(0,147,200)},
//            {2, QColor(50,225,0)},
//            {3, QColor(224,240,0)},
//            {4, QColor(240,84,0)},
//            {5, QColor(235,18,18)}
//        };

//        std::shared_ptr<DamageLevel> dl(new DamageLevel);
//        dl->SetDB(db);
//        dl->SetGjdy(m_css[bm.m_dbsID].gjdyList[i].toStdString());
//        dl->SetLimit(limit);
//        dl->SetColorMap(cm);
//        if(!dl->PrepareData() || !dl->Compute())continue;

//        evalFailed = false;
//        dl->AddScalar();

//        int frame = (int)GFE::FO::GetFrame(db,true).size() - 1;
//        auto data = dl->GetData(frame, 0);
//        std::set<float> dataSet(data.begin(), data.end());
//        QVector<QColor> colors;
//        QVector<double> labels;

//        if(data.empty()){
//            retInfo = tr("Warning: No damage level cloud chart data");
//            m_word->WriteText(bm.m_bmName, retInfo);
//            continue;
//        }

//        for(auto& d : dataSet){
//            colors.push_back(cm[d]);
//            labels.push_back(d);
//        }

//        ReportVTK::Option opt;
//        opt.isCloud = true, opt.isCell = true, opt.isDiscrete = true;
//        auto pngPath = m_tDir.path() + "/temp.png";
//        retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, setsName, FOName, data, cm);
//        if(!retInfo.isEmpty()){
//            return retInfo;
//        }

//        QString titleBmName = bm.m_bmName + "_Title";
//        QString titleText;
//        titleText += QString::fromStdString(GFE::FileInfo::Name(dbPath.toStdString()));
//        if(bm.m_extra[0] == "Wall")titleText += u8"剪力墙";
//        if(bm.m_extra[0] == "Col")titleText += u8"框架柱";
//        if(bm.m_extra[0] == "Beam")titleText += u8"框架梁";
//        titleText += u8"损伤等级";
//        m_word->AddNewBM(bm.m_bmName, titleBmName, titleText, "title");

//        auto size = ReportWord::Attributes["CloudChart3D"];
//        m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
//    }

//    if(evalFailed){
//        retInfo = tr("Warning: Performance evaluation failed");
//        return retInfo;
//    }

//    if(!hasDynamic){
//        retInfo = tr("Warning: No dynamic step in db combination");
//        return retInfo;
//    }

    return retInfo;
}

QString DlgReport::ExportTable(const ReportBookMark &bm)
{
    QString retinfo;
    switch (bm.m_gfeType) {
    case ReportBookMark::Comp:
        retinfo = ExportCompTable(bm);
        break;
    case ReportBookMark::SoilDyna:
        retinfo = ExportSoilDynaTable(bm);
        break;
    case ReportBookMark::Property:
        retinfo = ExportPropTable(bm);
        break;
    case ReportBookMark::Comb:{
        if(bm.m_param == ReportBookMark::List){
            retinfo = ExportCombListTable(bm);
        }
        break;
    }
    case ReportBookMark::Model:{
        retinfo = ExportModelTable(bm);
        break;
    }
    case ReportBookMark::SDR:{
        retinfo = ExportSDRTable(bm);
        break;
    }
    case ReportBookMark::Vibration:{
        if(bm.m_param == ReportBookMark::Case){
            retinfo = ExportVibCasesTable(bm);
        }
        break;
    }
    case ReportBookMark::Floor:{
        retinfo = ExportFloorTable(bm);
        break;
    }
    case ReportBookMark::Soil:{
        retinfo = ExportSoilTable(bm);
        break;
    }

    case ReportBookMark::Material:{
        retinfo = ExportMatTable(bm);
        break;
    }

    default:
        break;
    }
    return retinfo;
}

QString DlgReport::ExportMatTable(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Layer:
        ExportMatLayerTable(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportMatLayerTable(const ReportBookMark &bm)
{
    QString retInfo;
    QVector<QStringList> infos;
    QStringList header = {u8"楼层", u8"梁", u8"板", u8"墙", u8"柱"};

    auto dbsId = bm.m_dbsID;
    auto cs = m_css[dbsId];

    //toask
    bool hasDynamic = false;
    std::shared_ptr<GFE::DB> db;
    QString gjdyPath;

    for(int i = 0;i < cs.dbPathList.size();++i){
        auto dbPath = cs.dbPathList[i];
        gjdyPath = cs.gjdyList[i];
        db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db))continue;
        hasDynamic = true;
        break;
    }
    auto gjReader = std::make_shared<GFE_GJDYReader>();
    gjReader->SetPath(gjdyPath.toStdWString());
    gjReader->Read();
    auto elem2Part = gjReader->GetElementPart();

    auto elementMat = GFE::getAttribute(db, "ElementMaterial");

    QString pattern = "^C.*Mat";
    QRegularExpression regex(pattern);

    for(int i = 0;i < cs.acrLayer.size();++i){
        QStringList info;
        auto l = cs.acrLayer[i];
        info << "Floor" + QString::number(l);
        info << "-" << "-" << "-" << "-";

        QString setName;
        setName = "floor" + QString::number(l) +"-All";
        auto elemSet = GFE::getElementSet(db, setName.toStdString());
        if(!elemSet)continue;
        for(auto& elemLabel : elemSet->elements){
            auto elemId = m_vtks[dbsId]->m_elementLabel2Id[elemLabel];
            auto partType = gjReader->GetPartType(elem2Part[elemId]);
            auto matName = QString::fromStdString(GFE::getMaterialName(db, elementMat[elemId]));
            QString strength;
            if(regex.match(matName).hasMatch()){
                QRegularExpression regex2("\\d+$");
                QRegularExpressionMatch match;
                match = regex2.match(matName);
                strength = "C" + match.captured(0);
            }else{
                strength = matName;
            }
            if(partType == "Beam" && info[1] == "-"){
                info[1] = strength;
            }
            if(partType == "Slab" && info[2] == "-"){
                info[2] = strength;
            }
            if(partType == "Wall" && info[3] == "-"){
                info[3] = strength;
            }
            if(partType == "Col" && info[4] == "-"){
                info[4] = strength;
            }
            if(info[1] != "-" && info[2] != "-" &&
               info[3] != "-" && info[4] != "-"){
                break;
            }
        }
        infos.push_back(info);
    }

    if(!hasDynamic){
        retInfo = tr("Warning: No dynamic step in db combination");
        return retInfo;
    }

    m_word->AddTable(bm.m_bmName, header, infos);

    return retInfo;
}

QString DlgReport::ExportModelTable(const ReportBookMark &bm)
{
    QString retInfo;
    switch (bm.m_param) {
    case ReportBookMark::Attribute:
        retInfo = ExportModelParamTable(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportModelParamTable(const ReportBookMark &bm)
{
    QString retInfo;
    QMap<QString, int> subTypeCnt;
    for(auto st : m_mp[bm.m_dbsID].elmSubType)
    {
        QString name = GFE::getElementSubTypeName(st).data();
        if(subTypeCnt.find(name) == subTypeCnt.end())
            subTypeCnt[name] = 1;
        else
            subTypeCnt[name]++;

    }
    int elmCnt = 0;
    bool isFirst = true;
    QVector<QStringList> info;
    for(auto it = subTypeCnt.begin(); it != subTypeCnt.end(); ++it){
        if(isFirst)
        {
            info.push_back({u8"单元类别", it.key(), QString::number(it.value())});
            isFirst = false;
        }
        else
        {
            info.push_back({"", it.key(), QString::number(it.value())});
        }
        elmCnt++;
    }
    m_word->AddTable(bm.m_bmName,info);
    // 合并“单元类别”单元格
    auto table = m_word->GetTable(bm.m_bmName);

    for(int i = 0; i < elmCnt - 1; ++i)
    {
        auto cell1 = table->querySubObject("Cell(int, int)", 5, 1);
        auto cell2 = table->querySubObject("Cell(int, int)", i + 6, 1);
        cell1->dynamicCall("Merge(QVariant)", cell2->asVariant());
        DELETE_PTR(cell1)
        DELETE_PTR(cell2)
    }
    return retInfo;
}

QString DlgReport::ExportSoilTable(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Layer:
        ExportSoilLayerTable(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportSoilDynaTable(const ReportBookMark &bm)
{
    QString retInfo;
    QString tableTitleBmName = NewBMPrefix(bm.m_dbsID) + bm.m_bmName + "_Title";;
    QString tableTitleText;
    QStringList header;
    QVector<QStringList> data;
    std::vector<GFE::SoilLayer> soils;
    std::shared_ptr<GFE::DB> db = nullptr;
    for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList)
    {
        db = GFE::open(dbPath.toStdString(), false);
        if(soils.empty())
            soils = GFE::getAllSoil(db);
        if(!soils.empty())
            break;
    }
    if(soils.empty())
    {
        m_word->WriteText(bm.m_bmName, u8"未找到土动信息");
        return "";
    }
    auto soil = soils[0];
    QSet<QString> mats;
    for(const auto& m : soil.layer_mat)
        mats.insert(m.data());

    switch (bm.m_param) {
    case ReportBookMark::MatParam:{
        tableTitleText = u8"表" + m_section + u8"-1 土动参数信息";
        header = QStringList({u8"材料名称",u8"弹性模量E(MPa)",u8"泊松比v",u8"密度(kg/m3)",u8"阻尼系数α	",u8"阻尼系数β"});
        data = ExportSoilDynaMatParamTable(bm,db,mats);
        break;
    }
    case ReportBookMark::SoilInfo:{
        tableTitleText = u8"表" + m_section + u8"-2 土层分布信息";
        header = QStringList({u8"序号",u8"土层名",u8"厚度(m)"});
        data = ExportSoilDynaSoilInfoTable(bm,soil);
        break;
    }
    default:
        break;
    }

    if(data.empty()||header.empty())return BMWARN + bm.m_bmName;

    m_word->AddNewBM(bm.m_bmName,tableTitleBmName,tableTitleText,"title");
    retInfo = m_word->AddTable(bm.m_bmName,header,data);

    return retInfo;
}

QVector<QStringList> DlgReport::ExportSoilDynaMatParamTable(const ReportBookMark &bm,std::shared_ptr<GFE::DB> db, const QSet<QString> &mats)
{
    QVector<QStringList> data;
    for(const auto& it : mats)
    {
        auto mat = GFE::getMaterial(db, it.toStdString());
        QStringList matInfo = {it, "Null", "Null", "Null", "0", "0"};
        auto ela = mat->GetElastic();
        if(ela)
        {
            matInfo[1] = QString::number(ela->params[0]);
            matInfo[2] = QString::number(ela->params[1]);
        }
        auto den = mat->GetDensity();
        if(den)
            matInfo[3] = QString::number(den->params[0]);
        auto damp = mat->GetDamping();
        if(damp)
        {
            matInfo[4] = QString::number(damp->params[0]);
            matInfo[5] = QString::number(damp->params[1]);
        }
        data.push_back(matInfo);
    }

    return data;
}

QVector<QStringList> DlgReport::ExportSoilDynaSoilInfoTable(const ReportBookMark &bm, const GFE::SoilLayer &soil)
{
    QVector<QStringList> data;

    for(int i = 0; i < soil.n_layer; ++i)
    {
        QStringList soilInfo = {QString::number(i + 1)};
        soilInfo << soil.layer_mat[i].data();
        soilInfo << QString::number(soil.layer_thickness[i]);
        data.push_back(soilInfo);
    }

    return data;
}

QString DlgReport::ExportSoilLayerTable(const ReportBookMark &bm)
{
    QString retInfo;
    QVector<QStringList> infos;

    std::shared_ptr<GFE::DB> db;
    for(auto& dbPath : m_css[bm.m_dbsID].dbPathList){
        db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db))continue;
        break;
    }

    auto soils = GFE::getAllSoil(db);

    QStringList header = {u8"土层名称", u8"土层厚度(m)", u8"弹性模量E(kPa)", u8"泊松比v", u8"密度(kg/m^3)"};

    if(soils.empty()){
        m_word->WriteText(bm.m_bmName, tr("NoData: DB has no soil!"));
        return "";
    }

    for(auto& s : soils){
        for(int i = 0;i < s.layer_mat.size();++i){
            QStringList info;
            auto lm = s.layer_mat[i];
            auto lt = s.layer_thickness[i];
            info << QString::fromStdString(lm) << QString::number(lt);
            auto m = GFE::getMaterial(db, lm);
            GFE::Material2 m2(*m);
            auto ps = m2.GetPara_SSA();
            info << QString::number(ps.E) << QString::number(ps.V) << QString::number(ps.Rou * 10);
            infos.push_back(info);
        }
    }

    m_word->AddTable(bm.m_bmName, header, infos);

    return retInfo;
}


QString DlgReport::ExportCompTable(const ReportBookMark& bm)
{
    QStringList header;
    QVector<QStringList> data;

    switch (bm.m_param) {
    case ReportBookMark::InterForce:{
        header = QStringList({u8"构件ID",u8"轴力（kN/m）",u8"剪力（kN/m）",u8"弯矩（kN·m/m）"});
        data = ExportCompInterForceTable(bm,m_caculates[bm.m_dbsID]->m_pjResult.eleForce);
        if(header.empty()||data.empty())return tr("Error:can not get comp inter force table!");
        break;
    }
    default:
        break;
    }

    m_word->AddTable(bm.m_bmName,header,data);

    return "";
}

QVector<QStringList> DlgReport::ExportCompInterForceTable(const ReportBookMark &bm, const std::map<std::pair<std::string, std::string>, std::vector<std::pair<int, double>>> &eleForce)
{
    auto gjdyPath = m_css[bm.m_dbsID].gjdyList[0];
    QFile gjdyFile(gjdyPath);
    if(!gjdyFile.exists() || !gjdyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        return {};
    }
    //构件ID-构件单元
    std::unordered_map<std::string,std::unordered_set<int>> comps;
    std::vector<std::string> compIDs;
    std::string combID = "";

    while(!gjdyFile.atEnd()){
        QString line = gjdyFile.readLine();
        if(line.toStdString().substr(0,4) == "Part"){
            combID = line.toStdString().substr(6);
            combID.pop_back();
            compIDs.push_back(combID);
        }
        else{
            auto eleIDs = line.split(" ");
            for(auto &eleID:eleIDs){
                bool ok;
                int id = eleID.toInt(&ok);
                if(ok){
                    comps[combID].insert(id);
                }
            }
        }
    }

    QVector<QStringList> gjdyInfo;

    //    std::vector<QStringList> gjdyInfo({{u8"构件ID",u8"轴力",u8"剪力",u8"弯矩"}});
    for(auto &compID:compIDs){
        auto compEles = comps[compID];

        //构件轴力最大值
        double axialForceMax = -FLT_MAX;
        //构件剪力最大值
        double shearForceMax = -FLT_MAX;
        //构件弯矩最大值
        double bendingMax = -FLT_MAX;
        auto eleAxialForce = eleForce.at({"SF1",m_caculates[bm.m_dbsID]->m_maxSF1Comb.toStdString()});
        auto eleShearForce = eleForce.at({"SF2",m_caculates[bm.m_dbsID]->m_maxSF2Comb.toStdString()});
        auto eleBending = eleForce.at({"SM1",m_caculates[bm.m_dbsID]->m_maxSM1Comb.toStdString()});
        std::unordered_map<int, double> eleAxialForceHash(eleAxialForce.begin(), eleAxialForce.end());
        std::unordered_map<int, double> eleShearForceHash(eleShearForce.begin(), eleShearForce.end());
        std::unordered_map<int, double> eleBendingHash(eleBending.begin(), eleBending.end());
        for(auto id : compEles){
            if(eleAxialForceHash.find(id)!=eleAxialForceHash.end()){
                axialForceMax = std::max(axialForceMax,abs(eleAxialForceHash[id]));
            }
            if(eleShearForceHash.find(id)!=eleShearForceHash.end()){
                shearForceMax = std::max(shearForceMax,abs(eleShearForceHash[id]));
            }
            if(eleBendingHash.find(id)!=eleBendingHash.end()){
                bendingMax = std::max(bendingMax,abs(eleBendingHash[id]));
            }
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << axialForceMax;
        QString axialForceMaxStr = QString::fromStdString(oss.str());
        oss.str("");
        oss << std::fixed << std::setprecision(0) << shearForceMax;
        QString shearForceMaxStr = QString::fromStdString(oss.str());
        oss.str("");
        oss << std::fixed << std::setprecision(0) << bendingMax;
        QString bendingMaxStr = QString::fromStdString(oss.str());
        oss.str("");

        gjdyInfo.push_back({QString::fromStdString(compID),axialForceMaxStr,shearForceMaxStr,bendingMaxStr});
    }

    return gjdyInfo;
}

QString DlgReport::ExportPropTable(const ReportBookMark &bm)
{
    QString retInfo;

    QStringList header;
    QVector<QStringList> data;
    switch (bm.m_param) {
    case ReportBookMark::PJ:{
        header = QStringList({u8"断面名称",u8"结构构件",u8"构件厚度\n（mm）",u8"包络弯矩\n（kN·m/m）",u8"最大配筋面积\n(mm²/m)",u8"配筋率\n(%)"});
        data = ExportPropPJTable(bm);
        break;
    }
    default:
        break;
    }

    if(data.empty())return BMWARN + bm.m_bmName;
    m_word->AddTable(bm.m_bmName,header,data);
    auto table = m_word->GetTable(bm.m_bmName);
    m_word->TableMergeCol(table,1,2);
    m_word->TableWriteCell(table, 2, 1, m_css[bm.m_dbsID].dbsName);
    DELETE_PTR(table);
    return retInfo;
}

QVector<QStringList> DlgReport::ExportPropPJTable(const ReportBookMark &bm)
{
    QVector<QStringList> info;
    auto data = m_caculates[bm.m_dbsID]->m_pjResult.gjPj;
    for(auto &d:data){
        d[0] = m_css[bm.m_dbsID].gjID2GjName[std::stoi(d[0].toStdString())];
    }

    for(auto &d:data){
        QStringList sl;
        sl.append("");
        sl.append(d);
        info.push_back(sl);
    }
    return info;
}

QString DlgReport::ExportCombListTable(const ReportBookMark &bm)
{
    QString retInfo;
    int combNum = 4;
    QList<QStringList> basicList, standList, freqList, quasiList;

    for(auto &sl : m_css[bm.m_dbsID].combList)
    {
        int size = sl.size();
        int type = sl[size - 1].toInt();
        QStringList t_sl;
        for(int i = 0;i < size;++i){
            if(type == CombType::Basic){
                t_sl.push_back(sl[i]);
            }else{
                if(i < size - 1){
                    t_sl.push_back(sl[i]);
                }
            }

        }

        switch(type)
        {
        case CombType::Basic:
            basicList.push_back(t_sl);
            break;
        case CombType::Standard:
            standList.push_back(t_sl);
            break;
        case CombType::Frequent:
            freqList.push_back(t_sl);
            break;
        case CombType::QuasiPermanent:
            quasiList.push_back(t_sl);
            break;
        }
    }


    // 工况名
    QStringList caseList;
    for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList)
    {
        QFileInfo fi(dbPath);
        caseList << fi.completeBaseName();
    }

    // 填表格信息
    auto helper = [&](const QString& prefix, const QString& cnPrefix, QList<QStringList> data)
    {
        if(!data.empty()){
            QString titleBmName = NewBMPrefix(bm.m_dbsID) + prefix + "_Comb_Title";
            QString tableBmName = NewBMPrefix(bm.m_dbsID) +"Table_" + prefix + "_Comb";
            QStringList header;
            QVector<QStringList> info;

            header.append(u8"序号\\工况");
            header.append(caseList);

            int cnt = 1;

            for(auto &comb : data)
            {

                QStringList toAdd;
                toAdd << QString::number(cnt++);
                toAdd.append(comb);
                info.push_back(toAdd);
            }

            m_word->AddNewBM(bm.m_bmName, titleBmName, cnPrefix, "title");                                                                                             m_word->AddNewBM(titleBmName,tableBmName);
            m_word->AddNewBM(titleBmName,tableBmName);
            m_word->AddTable(tableBmName,header,info);
            combNum--;
        }
    };
    //notice 在一个书签添加多个表格时需要逆序添加
    helper("Freq", u8"频遇组合", freqList);
    helper("Quasi", u8"准永久组合", quasiList);
    helper("Stand", u8"标准组合", standList);
    helper("Basic", u8"基本组合", basicList);
    return retInfo;
}

QString DlgReport::ExportVibCasesTable(const ReportBookMark &bm)
{
    QString retInfo;

    QStringList header = {u8"工况",u8"起始时间(s)",u8"终止时间(s)",
                          u8"主方向(X)加速度(m/s^2)",u8"次方向(Y)加速度(m/s^2)",u8"竖直方向(Z)加速度(m/s^2)"};
    QVector<QStringList> infos;

    bool hasDynamic = false;
    QMap<QString, bool> directions = {{"X",false},{"Y",false},{"Z",false}};
    for(auto& dbPath : m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db))continue;
        hasDynamic = true;
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();

        QVector<std::shared_ptr<GFE::RegularBC>> rbcs;
        auto bcs = GFE::BC::All<GFE::RegularBC>(db);
        for(auto& bc : bcs){
            if(!bc->func_name.empty() && bc->TypeStr() == "GRAVITY"){
                rbcs.push_back(bc);
            }
        }

        QMap<QString, std::shared_ptr<GFE::Function>> amps = {
            {"X",nullptr},{"Y",nullptr},{"Z",nullptr}
        };
        QMap<QString, double> multis;
        for(auto& rbc : rbcs){
            std::shared_ptr<GFE::Function> amp;
            double multix = rbc->values[0],
                multiy = rbc->values[1],
                multiz = rbc->values[2];
            amp = GFE::getFunction(db,rbc->func_name);
            if(!amp || rbc->values.size() < 3)continue;
            if(multix == 0 && multiy == 0 && multiz == 0){
                continue;
            }
            QString direction;
            if(multix != 0){
                directions["X"] = true;
                amps["X"] = amp;
                multis["X"] = multix;
            }
            if(multiy != 0){
                directions["Y"] = true;
                amps["Y"] = amp;
                multis["Y"] = multiy;
            }
            if(multiz != 0){
                directions["Z"] = true;
                amps["Z"] = amp;
                multis["Z"] = multiy;
            }
        }

        QStringList info;
        for(int i = 0;i < 6;++i)info << "";
        double start, end;
        for(auto it = amps.begin();it != amps.end();++it){
            auto amp = it.value();
            if(it.value() && directions[it.key()]){
                QVector<double> xData, yData;
                int size = amp->values.size();
                auto multi = multis[it.key()];
                for(int i = 0; i < size; ++i)
                {
                    if(i % 2 == 0)
                        xData.push_back(amp->values[i]);
                    else{
                        if(multi != 0){
                            yData.push_back(amp->values[i] * multi);
                        }else{
                            yData.push_back(amp->values[i]);
                        }
                    }
                }
                if(xData.empty() || yData.empty())
                    continue;
                start = xData.front(),end = xData.back();
                double Amax = *std::max_element(yData.begin(),yData.end());
                info[3 + it.key().toStdString()[0] - 'X'] = QString::number(Amax);
            }else{
                info[3 + it.key().toStdString()[0] - 'X'] = "/";
            }
        }

        info[0] = name;
        info[1] = QString::number(start);
        info[2] = QString::number(end);

        infos.push_back(info);
    }

    if(!hasDynamic){
        retInfo = tr("Warning: Databases has no dynamic step!");
        return retInfo;
    }

    if(infos.empty()){
        retInfo = tr("Warning: No seism motion acceleration time history information!");
        return retInfo;
    }

    m_word->AddTable(bm.m_bmName, header, infos);

    return retInfo;
}

QString DlgReport::ExportSDRTable(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Envelope:
        retInfo = ExportSDREnvelopeTable(bm,m_caculates[bm.m_dbsID]->m_layer,m_caculates[bm.m_dbsID]->m_denoLimit,
                                         m_caculates[bm.m_dbsID]->m_sdrDir1,m_caculates[bm.m_dbsID]->m_sdrDir2);
        break;
    case ReportBookMark::E2X:
        retInfo = ExportSDRE2Table(bm);
        break;
    case ReportBookMark::E2Y:
        retInfo = ExportSDRE2Table(bm);
        break;
    case ReportBookMark::Displacement:
        retInfo = ExportSDRDispTable(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportSDREnvelopeTable(const ReportBookMark &bm,const int& layer,const int& denoLimit,const QString& dir1,const QString& dir2)
{
    QString retInfo;
    QString lastBmName = bm.m_bmName;
    int cnt = 1;//todo
    bool check = true;

    QString sumBmName = NewBMPrefix(bm.m_dbsID) + "SDR_Envelope_Sum";
    m_word->AddNewBM(lastBmName,sumBmName,u8"本工程楼层层数为" + QString::number(layer) +
                     u8"，因此位移角限值应为1/" + QString::number(denoLimit) +
                     u8"。");
    lastBmName = sumBmName;

    if(m_mp[bm.m_dbsID].dim == 2){
        QString tableTitleBmName = NewBMPrefix(bm.m_dbsID) + "Table_DriftRatio1Title";
        QString tableTitleText = u8"表" + m_section + '-' + QString::number(cnt++) + u8" 地震作用下的最大位移角";
        QString tableBmName = NewBMPrefix(bm.m_dbsID) + "Table_DriftRatio1";
        QVector<QStringList> info;
        for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList){
            auto db = GFE::open(dbPath.toStdString(), false);
            QFileInfo fi(dbPath);
            auto name = fi.completeBaseName();
            QFile f(fi.absolutePath() + QString::fromStdWString(u8"/XYData/层间位移角包络.txt"_cp));
            if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            QVector<double> env;
            f.readLine();
            while(!f.atEnd())
            {
                QString line = f.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                env.push_back(sp[0].trimmed().toDouble());
            }
            f.close();
            if(env.empty())
                continue;
            for(auto it : m_css[bm.m_dbsID].acrLayer)
            {
                if(it > env.size())
                    break;
                long long deno = 1 / env[it - 1];
                if(abs(deno) < denoLimit)
                    check = false;
                info.push_back({name, QString::number(it), "1/" + QString::number(deno), "1/" + QString::number(denoLimit)});
            }
        }

        QString fitBmName = NewBMPrefix(bm.m_dbsID) + "Drift1Fit";
        QString fitText;
        QStringList header = {u8"工况",u8"层号",u8"层间位移角",u8"限值"};
        fitText = u8"位移角";
        if(!check)
            fitText += u8"不";
        fitText += u8"满足规范要求；";
        m_sdr1Fit = fitText;
        m_sdr2Fit = "";

        m_word->AddNewBM(lastBmName,tableTitleBmName,tableTitleText,"title");
        m_word->AddNewBM(tableTitleBmName,tableBmName);
        m_word->AddNewBM(tableBmName,fitBmName,fitText);
        m_word->AddTable(tableBmName,header,info);

    }
    else if(m_mp[bm.m_dbsID].dim == 3){
        bool check = false;
        auto helper = [&](const QString& dir, int num){
            check = true;
            QVector<QStringList> info;
            for(const auto& dbPath : m_css[bm.m_dbsID].dbPathList)
            {
                auto db = GFE::open(dbPath.toStdString(), false);
                QFileInfo fi(dbPath);
                auto name = fi.completeBaseName();
                QFile f(fi.absolutePath() + "/XYData/" + dir + QString::fromStdWString(u8"向层间位移角包络.txt"_cp));
                if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;
                QVector<double> env;
                f.readLine();
                while(!f.atEnd())
                {
                    QString line = f.readLine();
                    auto sp = line.split(",");
                    if(sp.size() < 2)
                        continue;
                    env.push_back(sp[0].trimmed().toDouble());
                }
                f.close();
                if(env.empty())
                    continue;
                for(auto &it : m_css[bm.m_dbsID].sdrLayer)
                {
                    if(it > env.size())
                        break;
                    if(env[it - 1] > 1)
                    {
                        check = false;
                        info.push_back({name, QString::number(it), QString::number(env[it - 1]), "1/" + QString::number(denoLimit)});
                    }
                    else if(env[it - 1] == 0)
                    {
                        info.push_back({name, QString::number(it), QString::number(env[it - 1]), "1/" + QString::number(denoLimit)});
                    }
                    else
                    {
                        long long deno = 1 / env[it - 1];
                        if(abs(deno) < denoLimit)
                            check = false;
                        info.push_back({name, QString::number(it), "1/" + QString::number(deno), "1/" + QString::number(denoLimit)});
                    }
                }
            }
            QString tableTitleBmName =  NewBMPrefix(bm.m_dbsID) + "Table_DriftRatio" + dir + "Title";
            QString tableTitleText = u8"表" + QString::number(num) + "-"
                                     + u8" 地震作用下的" + dir + u8"向最大位移角";
            QString tableBmName = NewBMPrefix(bm.m_dbsID) + "Table_DriftRatio" + dir;
            QString fitBmName = "Drift" + dir + "Fit";
            QString fitText = dir + u8"向位移角";
            QStringList header = {u8"工况",u8"层号",u8"层间位移角",u8"限值"};

            if(!check)
                fitText += u8"不";
            fitText += u8"满足规范要求；";
            if(num == 1){
                //todo
                m_sdr1Fit = fitText;
            }else{
                m_sdr2Fit = fitText;
            }

            m_word->AddNewBM(lastBmName,tableTitleBmName,tableTitleText,"title");
            m_word->AddNewBM(tableTitleBmName,tableBmName);
            m_word->AddNewBM(tableBmName,fitBmName,fitText);
            m_word->AddTable(tableBmName,header,info);
            lastBmName = fitBmName;
        };


        helper(dir1,1);
        helper(dir2,2);
    }

    return retInfo;
}

QString DlgReport::ExportSDRE2Table(const ReportBookMark &bm)
{
    QString retInfo;
    QStringList dbPathList;
    QStringList header = {u8"地震波作用",u8"最大层间位移角数值",u8"最大层间位移角对应节点编号"};
    QVector<QStringList> infos;

    int cnt = 1;
    bool hasDynamic = false, hasVibLoad = false;
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;
        // 读取数据
        QFileInfo fi(dbPath);
        auto dbName = fi.completeBaseName();

        auto vibLoads = GFE::getAllVibLoad(db);
        if(vibLoads.empty())continue;
        hasVibLoad = true;

        QStringList directions;
        for(auto& v : vibLoads){
            if(v.param["AmpX"] != ""){
                directions.push_back("X");
            }
            if(v.param["AmpY"] != ""){
                directions.push_back("Y");
            }
            if(v.param["AmpZ"] != ""){
                directions.push_back("Z");
            }
        }

        for(auto& direction : directions){
            QFile envf,nodef;
            envf.setFileName(fi.absolutePath() + "/XYData/" + direction + u8"向层间位移角包络.txt");
            if(!envf.exists() || !envf.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            nodef.setFileName(fi.absolutePath() + "/XYData/" + direction + u8"向层间位移角最大节点.txt");
            if(!nodef.exists() || !nodef.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;

            double maxSDR = -INFINITY, maxLayer = -1;
            QString nodeStr;
            while(!envf.atEnd())
            {
                QString line = envf.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                double sdr =  sp[0].trimmed().toDouble();
                int layer = sp[1].trimmed().toInt();
                if(sdr > maxSDR){
                    maxSDR = sdr, maxLayer = layer;
                }
            }

            if(maxSDR == -INFINITY)continue;
            while(!nodef.atEnd())
            {
                QString line = nodef.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                QString layerStr = sp[0].trimmed();
                int layer = layerStr.split(' ')[1].trimmed().toInt();
                if(layer == maxLayer){
                    nodeStr = sp[1].split(':')[1];
                    break;
                }
            }
            if(nodeStr.isEmpty())continue;

            QStringList info;
            info.push_back(dbName + " " + direction +  u8"向地震波");
            int nume = -1,deno = -1;
            DlgReportUtil::FloatToReciprocal(maxSDR,nume,deno);
            info.push_back(QString::number(nume) + '/' + QString::number(deno));
            info.push_back(nodeStr);
            infos.push_back(info);
            ++cnt;
        }


    }

    if(!hasVibLoad){
        retInfo = tr("Warning: No vibLoads in all dbs");
        return retInfo;
    }

    if(!hasDynamic){
        retInfo = tr("Warning: No dynamic step in all dbs");
        return retInfo;
    }

    if(infos.isEmpty()){
        retInfo = tr("Warning: No statistical information about story drift!");
        return retInfo;
    }

    m_word->AddTable(bm.m_bmName,header,infos);

    return retInfo;
}

QString DlgReport::ExportSDRDispTable(const ReportBookMark &bm)
{
    QString retInfo;

    QStringList header = {u8"工况", u8"主方向", u8"类型", u8"最大顶点位移(m)", u8"最大层间位移角", u8"最大层间位移角对应层号"};
    QVector<QStringList> infos;

    for(auto &dbPath : m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString(), false);
        if(!GFE::Step::GetDynamic(db)){
            qDebug()<<"Can not get dynamic!";
            continue;
        }
        QFileInfo fi(dbPath);
        auto name = fi.completeBaseName();

        QStringList filePaths, info;
        filePaths.append(fi.absolutePath() + u8"/XYData/X向层间位移角包络.txt");
        filePaths.append(fi.absolutePath() + u8"/XYData/Y向层间位移角包络.txt");

        double sdrMax = -INFINITY;
        int sdrMaxLayer = -1;

        for(auto& fp:filePaths){
            QFile f(fp);
            if(!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)){
                QString warnInfo = u8"Warning: Can not open SDR XYData file: " + fp;
                m_progDlg->Finish(InfoType(warnInfo), warnInfo.toUtf8().constData());
                continue;
            }

            while(!f.atEnd())
            {
                QString line = f.readLine();
                auto sp = line.split(",");
                if(sp.size() < 2)
                    continue;
                double x = sp[0].trimmed().toDouble(),
                    y = sp[1].trimmed().toInt();
                if(x > sdrMax){
                    sdrMax = x;
                    sdrMaxLayer = y;
                }
            }
        }

        //todo 主方向用自由度去判断?类型？最大顶点位移
        QString direction, type = u8"弹塑性";
        std::shared_ptr<GFE::RegularBC> rbc;
        QString bcName;
        auto rbcx = GFE::BC::Find<GFE::RegularBC>(db,"EARTHX");
        auto rbcy = GFE::BC::Find<GFE::RegularBC>(db,"EARTHY");
        auto rbcz = GFE::BC::Find<GFE::RegularBC>(db,"EARTHZ");
        if(rbcx){
            rbc = rbcx;
            bcName = "EARTHX";
        }else if(rbcy){
            rbc = rbcy;
            bcName = "EARTHY";
        }else if(rbcz){
            rbc = rbcz;
            bcName = "EARTHZ";
        }
        else continue;

        if(bcName == "EARTHX"){
            direction = "X";
        }else if(bcName == "EARTHY"){
            direction = "Y";
        }else{
            direction = "Z";
        }

        double maxDisp = 0.0;

        info << name << direction << type;
        if(sdrMax != -INFINITY){
            info << QString::number(maxDisp) << QString::number(sdrMax) << QString::number(sdrMaxLayer);
        }else{
            info << "/" << "/";
        }

        infos.push_back(info);
    }

    if(infos.empty()){
        retInfo = tr("Warning: There is no displacement table data under ground motion!");
        return retInfo;
    }

    m_word->AddTable(bm.m_bmName,header,infos);

    return retInfo;
}

QString DlgReport::ExportFloorTable(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_param) {
    case ReportBookMark::Acceleration:
        retInfo = ExportFloorAccelerationTable(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportFloorAccelerationTable(const ReportBookMark &bm)
{
    QString retInfo;

    bool hasDynamic = false;

    QStringList header = {u8"楼层",u8"质心X坐标(m)",u8"质心Y坐标(m)",u8"质心Z坐标",
                          u8"节点编号",u8"X向绝对加速度(m/s^2)",u8"X向绝对加速度(m/s^2)"};
    for(auto &dbPath:m_css[bm.m_dbsID].dbPathList){
        auto db = GFE::open(dbPath.toStdString());
        if(!GFE::Step::GetDynamic(db)){
            continue;
        }
        hasDynamic = true;

        //todo 质心

    }

    return retInfo;
}

QString DlgReport::ExportPicture(const ReportBookMark &bm)
{
    QString retInfo;

    switch (bm.m_gfeType) {
    case ReportBookMark::Model:
        retInfo = ExportModelPicture(bm);
        break;
    default:
        break;
    }

    return retInfo;
}

QString DlgReport::ExportModelPicture(const ReportBookMark &bm)
{
    QString retInfo;
    QString pngPath;

    switch (bm.m_param) {
    case ReportBookMark::ElemSet:
        retInfo = ExportModelElemSetPicture(bm);
        break;
    case ReportBookMark::HighLight:
        retInfo = ExportModelHighLightPicture(bm);
        break;
    default:
        break;
    }
    return retInfo;
}

QString DlgReport::ExportModelElemSetPicture(const ReportBookMark &bm)
{
    QString retInfo;
    ReportVTK::Option opt;
    auto extra = bm.m_extra[0];
    QStringList setsName;
    auto db = GFE::open(m_css[bm.m_dbsID].dbPathList[0].toStdString());

    if(extra == "Jiegou"){
        setsName.append("jiegou");
        opt.isZoom = true;
        opt.zoomFactor = 1.8;
    }else if(extra == "JiegouMesh"){
        setsName.append("jiegou");
        opt.isMesh = true;
        opt.isZoom = true;
        opt.zoomFactor = 5.0;
    }else if(extra == "DLQ"){
        //todo
        setsName.append("DLQ-jiegou");
        if(!m_vtks[bm.m_dbsID]->ElementSelection("DLQ-jiegou")){
            retInfo = tr("NoData: The elementSet DLQ-jiegou do not exist!");
            m_word->WriteText(bm.m_bmName, retInfo);
            return "";
        }
        opt.isMesh = true;
        opt.isZoom = true;
        opt.zoomFactor = 1.8;
    }

    auto pngPath = m_tDir.path() + "/temp.png";
    retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, setsName);
    if(!retInfo.isEmpty()){
        return retInfo;
    }

    if(m_mp[bm.m_dbsID].dim == 2){
        auto size = ReportWord::Attributes["ModelPicture2D"];
        m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
    }else{
        auto size = ReportWord::Attributes["ModelPicture3D"];
        m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
    }
    return retInfo;
}

QString DlgReport::ExportModelHighLightPicture(const ReportBookMark &bm)
{
    QString retInfo;

    QStringList setNames, baseSetNames;
    if(bm.m_extra[0] == "Jiegou"){
        setNames.push_back("jiegou");
    }

    ReportVTK::Option opt;
    opt.isCell = true, opt.isHighLight = true, opt.isMesh = true, opt.isZoom = true,opt.zoomFactor = 1.8;
    auto pngPath = m_tDir.path() + "/temp.png";
    retInfo = m_vtks[bm.m_dbsID]->OffScreenRendering(pngPath, &opt, setNames, baseSetNames);
    if(!retInfo.isEmpty()){
        return retInfo;
    }
    if(m_mp[bm.m_dbsID].dim == 2){
        auto size = ReportWord::Attributes["ModelPicture2D"];
        m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
    }else{
        auto size = ReportWord::Attributes["ModelPicture3D"];
        m_word->AddPicture(bm.m_bmName,pngPath,size["width"].toInt(),size["height"].toInt());
    }

    return retInfo;
}




