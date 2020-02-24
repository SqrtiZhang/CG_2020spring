#include "Poly.h"

Poly::Poly()
{
}

Poly::~Poly()
{
}



void Poly::Draw(QPainter& painter)
{
    QPolygon polygon(points_.size());
    for(int i = 0; i < points_.size(); ++i)
    {
        polygon.setPoint(i, *points_[i]);
    }
    painter.setPen(QPen(color,width));
    painter.drawPolygon(polygon);
}

void Poly::Push(QPoint *p)
{
    points_.push_back(p);
}

QPoint *Poly::top()
{
    return points_.back();
}

