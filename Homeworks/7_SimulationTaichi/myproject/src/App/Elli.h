#pragma once

#include "Shape.h"

class Elli :public Shape
{
public:
    Elli();
    ~Elli();

    void Draw(QPainter &painter);
	int shape_type() { return 3; };
	int material_type(void) { return material; };
};

