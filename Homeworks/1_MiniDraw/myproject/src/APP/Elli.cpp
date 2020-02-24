#include "Elli.h"

Elli::Elli()
{
}

Elli::~Elli()
{
}

void Elli::Draw(QPainter& painter)
{
    painter.setPen(QPen(color,width));
    painter.drawEllipse(start.x(), start.y(),
        end.x() - start.x(), end.y() - start.y());
}

