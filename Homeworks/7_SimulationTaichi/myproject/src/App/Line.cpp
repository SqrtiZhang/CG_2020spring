#include "Line.h"

Line::Line()
{
}

Line::~Line()
{
}

void Line::Draw(QPainter& painter)
{
    painter.setPen(QPen(color,width));
    painter.drawLine(start, end);
}
