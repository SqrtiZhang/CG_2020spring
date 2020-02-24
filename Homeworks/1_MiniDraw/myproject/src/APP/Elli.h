#pragma once

#include "Shape.h"

class Elli :public Shape
{
public:
    Elli();
    ~Elli();

    void Draw(QPainter &painter);
};

