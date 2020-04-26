#pragma once

#include "shape.h"

#include <QVector>
#include <QPolygon>
#include <QPoint>

class Poly :public Shape
{
public:
    Poly();
    ~Poly();

   QPoint *top();
   void Draw(QPainter &painter);
   void Push(QPoint *);
   int shape_type(void) { return 4; };

private:
    QVector<QPoint *> points_;
};

