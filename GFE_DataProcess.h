#ifndef GFE_DATAPROCESS_H
#define GFE_DATAPROCESS_H

#include <GFE_API.h>
#include <Vibration/ERASolver.h>
#include <Vibration/EERASolver.h>
#include <Vibration/IERASolver.h>
#include <Vibration/ARSolver.h>

#include <QString>
#include <QObject>
#include <QSet>

namespace GFE_DataProcess {

class SeismicSiteResponse: public QObject
{
    Q_OBJECT
public:
    enum Method {
        TimeHist_EqLinear,  //时程分析，等效线性化
        TimeHist_Nonlinear, //时程分析，非线性
        PseudoDyna, //拟动力分析
    };

    SeismicSiteResponse(std::shared_ptr<GFE::DB> db);

    /**
     * @param method 方法
     * @param top_depth 顶点埋深
     * @param bot_depth 底点埋深
     * @return
     */
    QString Solve(const GFE::VibLoad& vib_load, Method method, double top_depth, double bot_depth, int direct, QString var);
    QString SetSolverParam(const std::shared_ptr<EERASolver>& solver, int dir, double amp_scale = NAN);
    void Setup(const sp3<ERASolver>& era, const std::optional<std::pair<double, double>>& ar, int direct, QString var);
    QPair<QVector<double>, QVector<double>> GetXYData(double top_depth, double bot_depth, int direct, QString var);

private:
    std::shared_ptr<GFE::DB> m_db;
    Method m_method;
    GFE::VibLoad m_cur_vib_load;
    sp3<ARSolver> m_AR_EERA, m_AR_IERA;
};

}



#endif // GFE_DATAPROCESS_H
