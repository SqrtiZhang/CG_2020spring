#include "Warp.h"

Warp::Warp()
{

}

Warp::~Warp()
{

}

double Warp::get_distance(QPoint p1, QPoint p2)
{
    return qSqrt((p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y()));
}

void Warp::set_s(QPoint s)
{
    this->start_point=s;
}

void Warp::set_q(vector<QPoint> q_)
{
    for(int i=0;i<q_.size();i++)
    {
        QPoint temp;
        temp=q_[i];
        this->q_.push_back(QPoint(temp.x()-start_point.x(),temp.y()-start_point.y()));
    }

}

void Warp::set_p(vector<QPoint> p_)
{
    for(int i=0;i<p_.size();i++)
    {
        QPoint temp;
        temp=p_[i];
        this->p_.push_back(QPoint(temp.x()-start_point.x(),temp.y()-start_point.y()));
    }

}
