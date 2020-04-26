#pragma once

#include"Shape.h"

class Rect:public Shape
{
public:
	Rect();
	~Rect();

	void Draw(QPainter &painter);
	int shape_type(void) { return 2; };
	int material_type(void) { return material; };
};

