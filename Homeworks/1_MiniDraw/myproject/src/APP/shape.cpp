#include "shape.h"

Shape::Shape()
{
    start=QPoint(0,0);
    end=QPoint(0,0);
    color=Qt::black;
    width=1;
}

Shape::~Shape()
{
}

void Shape::set_start(QPoint s)
{
	start = s;
}

void Shape::set_end(QPoint e)
{
	end = e;
}

void Shape::set_line(QColor line_color, int line_width)
{
    color=line_color;
    width=line_width;
}

QPoint Shape::get_start()
{
    return start;
}

QPoint Shape::get_end()
{
    return end;
}
