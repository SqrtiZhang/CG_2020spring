#pragma once
#ifndef IDW_H
#define IDW_H

#include <QPoint>
#include <vector>

#include <QDebug>
#include <QImage>
#include "Warp.h"

using namespace std;

class IDW:public Warp
{
public:
    IDW();
    ~IDW();
public:
    QMap<int, double> get_input_control_point_weight( QPoint input_point_);
    QPoint get_output_point( QPoint input_point_);
    QImage get_image_deal_with_IDW( QImage src);

    void set_mu_(double mu_);
    double get_mu_(void);

    vector<QPoint> get_p(void);
    vector<QPoint> get_q(void);


private:
    double mu_;
};
#endif
