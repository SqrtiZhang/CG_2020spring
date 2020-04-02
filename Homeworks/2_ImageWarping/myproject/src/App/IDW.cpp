#include "IDW.h"
#include <iostream>

IDW::IDW()
{
    mu_ = 2.0;

}
IDW::~IDW()
{

}


QMap<int, double> IDW::get_input_control_point_weight(QPoint input_point_)
{
    QMap<int, double> point_correspond_control_point_weight_map;
    double sum_weight_ = 0;

    for (int i = 0; i < p_.size(); i++)
    {
        double temp_ = 0;
        temp_ = 1 / this->get_distance(p_.at(i), input_point_);
        temp_ = qPow(temp_, mu_);
        sum_weight_ = sum_weight_ + temp_;
    }

    for (int i = 0; i < p_.size(); i++)
    {
        double temp_ = 0;
        temp_ = 1 / this->get_distance(p_.at(i), input_point_);
        temp_ = qPow(temp_, mu_);
        temp_ = temp_ / sum_weight_;
        point_correspond_control_point_weight_map.insert(i, temp_);
    }
    return point_correspond_control_point_weight_map;
}

QPoint IDW::get_output_point(QPoint input_point_)
{
    double offset_x_ = 0;
    double offset_y_ = 0;
    double x = 0, y = 0;
    QMap<int, double> tem_map_ = get_input_control_point_weight(input_point_);
    for (int i = 0; i < p_.size(); i++)
    {
        offset_x_ = q_.at(i).x() - p_.at(i).x();
        offset_y_ = q_.at(i).y() - p_.at(i).y();
        x += offset_x_*tem_map_.value(i);
        y += offset_y_*tem_map_.value(i);

    }

    input_point_.rx() = input_point_.x() + x;
    input_point_.ry() = input_point_.y() + y;
    return input_point_;
}

QImage IDW::get_image_deal_with_IDW(QImage src)
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

void IDW::set_mu_(double mu_)
{
    this->mu_ = mu_;
}

double IDW::get_mu_(void)
{
    return mu_;
}


vector<QPoint> IDW::get_p(void)
{
    return p_;
}


vector<QPoint> IDW::get_q(void)
{
    return q_;
}


