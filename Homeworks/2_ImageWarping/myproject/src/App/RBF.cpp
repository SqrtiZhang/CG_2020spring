#include "RBF.h"
#include<iostream>
using namespace std;

RBF::RBF()
{
}

RBF::RBF(vector<QPoint>& p_, vector<QPoint>& q_, QPoint start,int u = -1)
{
    set_s(start);

    set_p(p_);
    set_q(q_);
    u_ = u;
    size_ = p_.size();
    d_ = new double[size_*size_];
    r_ = new double[size_];

    cal_distance_martix();
    calculate_ri();
    calculate_ai();
}

RBF::~RBF()
{
    delete d_;
    delete r_;
}


void RBF:: cal_distance_martix()
{
    for (size_t i = 0; i < size_; i++)
    {
        d_[i*size_ + i] = 0;
        for (size_t j = i + 1; j < size_; j++)
        {
            d_[i*size_ + j] = d_[j*size_ + i] = get_distance(p_[i], p_[j]);
        }
    }
}

void RBF::calculate_ri()
{
    double min;


    for (size_t i = 0; i < size_; i++)
    {
        min = INT_MAX;
        for (size_t j = 0; j < size_; j++)
        {
            if (min > d_[i*size_ + j] && i != j)
                min = d_[i*size_ + j];
        }
        r_[i] = min;
    }
}

void RBF::calculate_ai()
{
    Matrix<double, Dynamic, Dynamic> K(size_, size_);
    VectorXd Bx(size_);
    VectorXd By(size_);
    for (size_t i = 0; i < size_; i++)
    {
        Bx(i) = q_[i].x() - p_[i].x();
        By(i) = q_[i].y() - p_[i].y();
        for (size_t j = 0; j < size_; j++)
        {
            K(i, j) = pow(d_[j*size_ + i] + r_[j], u_);
        }
    }
    Ax_ = K.colPivHouseholderQr().solve(Bx);
    Ay_ = K.colPivHouseholderQr().solve(By);
}

QPoint RBF::get_output_point(QPoint p)
{
    double x = p.x(), y = p.y(), t;
    for (int i = 0; i < size_; i++)
    {
        t = pow(get_distance(p, p_[i]) + r_[i], u_);
        x += Ax_(i)*t;
        y += Ay_(i)*t;
    }
    return QPoint(x, y);
}


QImage RBF::get_image_deal_with_RBF(QImage src)
{
    QImage ouput_image_ = QImage(src.width(), src.height(), QImage::Format_ARGB32);
    QPoint output_point_;
    int **is_filled_;
    is_filled_ = new int*[src.height()];
    for (int j = 0; j < src.height(); j++)
        is_filled_[j] = new int[src.width()];

    for (int i = 0; i < src.height(); i++)
        for (int j = 0; j < src.width(); j++)
            is_filled_[i][j] = 0;

    for (int i = 0; i <src.width(); i++)
    {
        for (int j = 0; j <src.height(); j++)
        {
            //cout<<src.pixelColor(i,j).red()<<endl;
            output_point_ = this->get_output_point(QPoint(i, j));
            if (output_point_.x() > 0 && output_point_.y() > 0&& output_point_.x()<src.width() && output_point_.y() <src.height()) //提高效率的关键，必须写
            {
                is_filled_[output_point_.y()][output_point_.x()] = 1;
                ouput_image_.setPixelColor(output_point_.x(), output_point_.y(), src.pixelColor(QPoint(i, j)));
            }
        }
    }

    for (int i = 1; i < src.height() - 1; i++)
    {
        for (int j = 1; j < src.width() - 1; j++)
        {
            //std::cout<<j<<i<<std::endl;
            if (is_filled_[i][j] == 0)
            {
                if (is_filled_[i + 1][j] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j, i + 1)));
                else if (is_filled_[i - 1][j] != 0)
                    ouput_image_.setPixelColor(j, i , ouput_image_.pixelColor(QPoint(j, i - 1)));
                else if (is_filled_[i][j + 1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j + 1, i)));
                else if (is_filled_[i][j - 1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j - 1, i)));

                if (is_filled_[i + 1][j+1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j + 1, i + 1)));
                else if (is_filled_[i - 1][j-1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j-1, i - 1)));
                else if (is_filled_[i+1][j - 1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j - 1, i+1)));
                else if (is_filled_[i-1][j + 1] != 0)
                    ouput_image_.setPixelColor(j, i, ouput_image_.pixelColor(QPoint(j + 1, i-1)));
            }

        }
    }

	for (int j = 0; j < src.height(); j++)
		delete []is_filled_[j];
	delete []is_filled_;

    return ouput_image_;
}
