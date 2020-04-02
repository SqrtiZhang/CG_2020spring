#ifndef WARP_H
#define WARP_H
#include <vector>
#include <QtMath>
#include <QPoint>
using namespace std;

class Warp
{
public:
    Warp();
    virtual ~Warp();

    double get_distance(QPoint p, QPoint q);
    void set_s(QPoint s);

    void set_p(vector<QPoint> p_);
    void set_q(vector<QPoint> q_);

protected:
    vector<QPoint> p_;
    vector<QPoint> q_;
    QPoint start_point;
};

#endif // WARP_H
