#pragma once

#include "shape.h"

class FreeHand:public Shape
{
public:
    FreeHand();
    ~FreeHand();

    void Draw(QPainter& painter);
	int shape_type(void) { return 5; };
};
