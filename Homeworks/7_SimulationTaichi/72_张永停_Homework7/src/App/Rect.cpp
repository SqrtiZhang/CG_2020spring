#include "Rect.h"

Rect::Rect()
{
}

Rect::~Rect()
{
}

void Rect::Draw(QPainter& painter)
{
    painter.setPen(QPen(color,width));
    painter.drawRect(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());

}

