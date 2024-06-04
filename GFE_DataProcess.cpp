#include "GFE_DataProcess.h"
#include <Mgr/Substruction/OCC_SoilMgr.h>
#include <Mgr/Model/OCC_MaterialMgr.h>

namespace GFE_DataProcess {
SeismicSiteResponse::SeismicSiteResponse(std::shared_ptr<GFE::DB> db)
    :m_db(db)
{
}


QString SeismicSiteResponse::Solve(const GFE::VibLoad &vib_load, Method method, double top_depth, double bot_depth, int direct, QString var)
{
    QString retInfo;
    m_method = method;
    m_cur_vib_load = vib_load;
    auto& vib_param = vib_load.param;

    //拟动力分析
    if(m_method == PseudoDyna){
        if(std::isnan(top_depth) || std::isnan(bot_depth)){
            return tr("Error: Please specify the top depth and the bottom depth!");
        }

        double total_depth = 0;

        auto soil_name = GFE::getSoil(m_db, vib_param.at("Soil"));
        auto tmp = soil_name->layer_thickness;
        GFE::FloatAcc(tmp.data(), tmp.size());
        total_depth = tmp.back();

        if(bot_depth < 0
            || bot_depth > total_depth
            || top_depth < 0
            || top_depth > total_depth
            || bot_depth < top_depth){
            return tr("Error: Please specify valid top depth and bottom depth!");
        }

        int depth_direct = std::stoi(vib_param.at("PWave"));
        if(depth_direct == 2) {
            return "Error: Invalid soil depth direction";
        }
    }

    //不进行调幅

    std::array<std::shared_ptr<ERASolver>, 3> solvers;
    bool calcIERA = false;
    if(m_method == PseudoDyna){
        calcIERA = true;
    }

    int amp_direc = -1;
    if(!vib_param.at("AmpX").empty()){
        amp_direc = 0;
    }else if(!vib_param.at("AmpY").empty()){
        amp_direc = 1;
    }else if(!vib_param.at("AmpZ").empty()){
        amp_direc = 2;
    }

    if(amp_direc < 0){
        return "Error: Invalid amp direction";
    }

    while(true){
        auto eera = std::make_shared<EERASolver>();

        //todo
        QString retInfo = SetSolverParam(eera, amp_direc);
        if(!retInfo.isEmpty()){
            return retInfo;
        }

        //Computing frequency-domain ERA
        eera->Calc();
        if(!eera->Result.ErrStr.empty()) {
            return QString::fromStdString(eera->Result.ErrStr);
        }

        if(calcIERA) {
            //Computing time-domain ERA
            auto iera = std::make_shared<IERASolver>();
            iera->Input_.EERA = eera;
            iera->Calc();
            solvers[amp_direc] = iera;
        }
        else
            solvers[amp_direc] = eera;
    }

    if(m_method == PseudoDyna){
        Setup(solvers, std::pair<double,double>{top_depth, bot_depth}, direct, var);
    }

    return retInfo;
}

QString SeismicSiteResponse::SetSolverParam(const std::shared_ptr<EERASolver> &solver, int dir, double amp_scale)
{
    Q_ASSERT(solver != nullptr);

    solver->Param.N = 4096;
    solver->Param.MaxIter = 100;
    solver->Param.Tol = 1e-2;
    solver->Param.Rr = 0.5;
    solver->Param.SubLayerHeight = 1;
    solver->Param.TimeInterval = 0.02;
    solver->Param.DefKsi = NAN;
    solver->Param.DampConvOrder = 1;
    solver->Param.DampScale = 1;

    auto& vib_param = m_cur_vib_load.param;

    // Input Loc
    std::string input_loc = vib_param.at("Input Loc");

    //基岩露头
    if(input_loc == "Outcrop"){
        solver->Input.IsOutcrop = true;
    }
    //基岩处
    if(input_loc == "Inside bedrock"){
        solver->Input.IsOutcrop = false;
        solver->Input.A_Layer = -1;
    }
    //地表
    if(input_loc == "Shallow ground"){
        solver->Input.IsOutcrop = false;
        solver->Input.A_Layer = 1;
    }

    //todo
    // 土
    {
//        OCC_AmpMgr::Get()
        auto soil_name = vib_param.at("Soil");
        auto soil = GFE::getSoil(m_db, soil_name);

        for(const auto& mat_name : soil->layer_mat) {
            auto mat = GFE::getMaterial(m_db, mat_name);
            solver->Input.Mats.push_back(std::move(*mat));
        }

        //! 基岩层材料
        {
            auto br_mat = GFE::getMaterial(m_db, soil->bedrockMat);
            solver->Input.BrMat = std::move(*br_mat);
        }

        solver->Input.Depths = soil->layer_thickness;

        // P波 or S波
        solver->Param.WaveType = (std::stoi(vib_param.at("PWave")) == dir);
    }

    // amplitude * scale
    auto ScaleAmp = [](GFE::Function& amp, double scale) {
        auto n = amp.values.size() / 2;
        for(size_t i = 1; i < n ; i+=2)
            amp.values[i] *= scale;     
    };

    // amplitude * scale
    std::string amp_name = vib_param.at("Amp" + ('X' + dir));
    if(!amp_name.empty()) {
        auto func = GFE::getFunction(m_db, amp_name);
        solver->Input.A = *func;
        if(!isnan(amp_scale))
            ScaleAmp(solver->Input.A, amp_scale);
    }


    return "";
}

void SeismicSiteResponse::Setup(const sp3<ERASolver>& era, const std::optional<std::pair<double, double>>& ar, int direct ,QString var)
{
    //! 判断输入类型
    sp3<EERASolver> eera;
    sp3<IERASolver> iera;
    int type = 0;       // bit_0 eera; bit_1 iera;
    for(int i = 0; i < 3; i++) {
        if(!era[i]) continue;
        auto dp = std::dynamic_pointer_cast<EERASolver>(era[i]);
        if(!dp) break;
        eera[i] = dp;
        type |= 1;
    }
    for(int i = 0; i < 3; i++) {
        if(!era[i]) continue;
        auto dp = std::dynamic_pointer_cast<IERASolver>(era[i]);
        if(!dp) break;
        iera[i] = dp;
        type |= (1<<1);
        if(dp->Input_.EERA) {
            eera[i] = dp->Input_.EERA;
            type |= 1;
        }
    }

    if(type == 0) return;
    constexpr const char* XYZ[3] = {"X Dir", "Y Dir", "Z Dir"};
    //! 反应加速度
    if(ar){
        //频域法
        if(type & 1) {
            for(int i = 0; i < 3; i++) {
                if(!era[i] || era[i]->GetBaseOutput()->IsEmpty)
                    continue;
                m_AR_EERA[i]= std::make_shared<ARSolver>();
                m_AR_EERA[i]->Input_.StrcTopD = ar->first;
                m_AR_EERA[i]->Input_.StrcBotD = ar->second;
                m_AR_EERA[i]->Input_.eraSolver = eera[i];
                m_AR_EERA[i]->Calc();
            }
        }
        //时域法
        if(type & 2) {
            for(int i = 0; i < 3; i++) {
                if(!era[i] || era[i]->GetBaseOutput()->IsEmpty)
                    continue;
                m_AR_IERA[i]= std::make_shared<ARSolver>();
                m_AR_IERA[i]->Input_.StrcTopD = ar->first;
                m_AR_IERA[i]->Input_.StrcBotD = ar->second;
                m_AR_IERA[i]->Input_.eraSolver = iera[i];
                m_AR_IERA[i]->Calc();
            }
        }

        auto [x, y] = GetXYData(ar->first, ar->second, direct, var);
    }
}

QPair<QVector<double>, QVector<double>> SeismicSiteResponse::GetXYData(double top_depth, double bot_depth, int direct, QString var)
{
    bool cEERA = true;
    bool cIERA = false;

    QString yAxis;

    if(cEERA){
        yAxis = "EERA";
    }else if(cIERA){
        yAxis = "IERA";
    }

    bool convToCoord = true;
    double topCoord = top_depth;

    std::shared_ptr<const ARSolver> ar;
    ar = m_AR_EERA[direct];

    if(!ar) return {};
    const GFE::Function2* amp;
    if(var == tr("U")) amp = &ar->Output_.U;
    else if(var == tr("V")) amp = &ar->Output_.V;
    else if(var == tr("A")) amp = &ar->Output_.A;
    else if(var == tr("S")) amp = &ar->Output_.S;
    else if(var == tr("A(calc by S)")) amp = &ar->Output_.A_S;
    else return {};

    QVector<double> x, y;
    auto size = (int)amp->DataArr().size()/2;
    x.resize(size);
    y.resize(size);

    for(int i = 0; i < size; i++) {
        // xy轴调换
        x[i] = amp->DataArr()[2*i+1];
        auto depth = amp->DataArr()[2*i];
        if(convToCoord)
            y[i] = topCoord-depth;
        else
            y[i] = depth;
    }

    return {x, y};
}
}



