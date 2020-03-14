#pragma once
#include "PoissonEdit.h"

class CPoissonEditGrad :
	public CPoissonEdit
{
public:
	CPoissonEditGrad(void);
	~CPoissonEditGrad(void);

	int v(QPoint, QPoint, int, QPoint);
};

