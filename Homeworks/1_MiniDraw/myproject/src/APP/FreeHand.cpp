#include "FreeHand.h"

FreeHand::FreeHand()
{
}

FreeHand::~FreeHand()
{
}

void FreeHand::Draw(QPainter &painter)
{
    painter.setPen(QPen(color,width));
    painter.drawLine(start,end);
}

