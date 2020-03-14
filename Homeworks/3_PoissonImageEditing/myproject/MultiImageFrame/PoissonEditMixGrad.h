#pragma once
#include "PoissonEdit.h"

class CPoissonEditMixGrad :
	public CPoissonEdit
{
public:
	CPoissonEditMixGrad(void);
	~CPoissonEditMixGrad(void);

	int v(QPoint, QPoint, int, QPoint);
};

