#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <time.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "ChildWindow.h"
#include "scanline.h"
#include "PoissonEditGrad.h"
#include "PoissonEditMixGrad.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow *relatewindow)
{
    //image_ = new QImage();
    //image_backup_ = new QImage();

	draw_status_ = kNone;
	region_shape_ = kRectangle;
	paste_method_ = kCopy;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);
	rect_or_ellipse_ = QRect(point_start_,point_end_);
	polygon_.clear();
	poissonEdit_ = NULL;
	//time_count_ = 0;

	source_window_ = NULL;
}

ImageWidget::~ImageWidget(void)
{
	if (poissonEdit_)
		delete poissonEdit_;
	//delete source_window_;
	
	image_.release();
	image_backup_.release();

	for (int i = 0; i < image_backup_vec_.size(); i++)
		image_backup_vec_[i].release();
}

int ImageWidget::ImageWidth()
{
    return image_.cols;
}

int ImageWidget::ImageHeight()
{
    return image_.rows;
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
	ClearChosen();
	update();
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

void ImageWidget::set_region_shape(int shape)
{
	switch (shape)
	{
	case kRectangle:
		region_shape_ = kRectangle;
		break;
	case kEllipse:
		region_shape_ = kEllipse;
		break;
	case kPolygon:
		region_shape_ = kPolygon;
		break;
    case kFree:
        region_shape_=kFree;
	default:
		break;
	}
}

void ImageWidget::set_paste_method(int method)
{
	switch (method)
	{
	case kCopy:
		paste_method_ = kCopy;
		break;
	case kGrad:
		paste_method_ = kGrad;
		break;
	case kMixGrad:
		paste_method_ = kMixGrad;
		break;
	default:
		break;
	}
}

QImage ImageWidget::image()
{
    //return image_;
    QImage img((const unsigned char*)image_.data, image_.cols, image_.rows, image_.step,
               QImage::Format_RGB888);
    return img;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent *paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
    QRect rect = QRect( 0, 0, image_.cols, image_.rows);
    QImage qimg= QImage((unsigned char *)(image_.data), image_.cols, image_.rows, image_.step, QImage::Format_RGB888);
    painter.drawImage(rect, qimg);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	switch (region_shape_)
	{
	case kRectangle:
		painter.drawRect(point_start_.x(), point_start_.y(), 
			point_end_.x()-point_start_.x(), point_end_.y()-point_start_.y());
		break;
	case kEllipse:
		painter.drawEllipse(point_start_.x(), point_start_.y(), 
			point_end_.x()-point_start_.x(), point_end_.y()-point_start_.y());
		break;
	case kPolygon:
		if (is_choosing_)
		{
			painter.drawPolyline(polygon_);
			painter.drawLine(point_start_,point_end_);
		}
		else
			painter.drawPolygon(polygon_);
		break;
    case kFree:
        painter.drawPolyline(polygon_);
        if (!is_choosing_)
            painter.drawLine(point_start_,point_end_);
        break;
	default:
		break;
	}
	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent *mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChoose:
			is_choosing_ = true;
			switch (region_shape_)
			{
			case kRectangle:
			case kEllipse:
				point_start_ = point_end_ = mouseevent->pos();
				break;
			case kPolygon:
				if (polygon_.empty())
				{
					point_start_ = point_end_ = mouseevent->pos();
					polygon_<<point_start_;
				}
				else
				{
					point_end_ = mouseevent->pos();
					update();
				}
				break;
            case kFree:
                if (polygon_.empty())
                {
                    point_start_ = point_end_ = mouseevent->pos();
                    polygon_<<point_start_;
                }
                else
                {
                    point_end_ = mouseevent->pos();
                    update();
                }
                break;
			default:
                is_choosing_=false;
				break;
			}
			break;

		case kPaste:
			{
				is_pasting_ = true;
                is_choosing_=false;

				int xpos = mouseevent->pos().rx();
				int ypos = mouseevent->pos().ry();

				int xsourcepos = source_window_->imagewidget_->rect_or_ellipse_.left();
				int ysourcepos = source_window_->imagewidget_->rect_or_ellipse_.top();


				int w = source_window_->imagewidget_->rect_or_ellipse_.right()
					-source_window_->imagewidget_->rect_or_ellipse_.left()+1;
				int h = source_window_->imagewidget_->rect_or_ellipse_.bottom()
					-source_window_->imagewidget_->rect_or_ellipse_.top()+1;

				// Paste
                if ( (xpos+w<image_.cols) && (ypos+h<image_.rows))
				{
                    image_backup_vec_.push_back(image_backup_);
                    //image_backup_ = image_;
                    image_.copyTo(image_backup_);
					// Restore image
                   // image_ = image_backup_;
                    image_backup_.copyTo(image_);

					if (kCopy != paste_method_)
						poissoned_img_ = poissonEdit_->GetPoissonedImage(xpos,ypos);

					switch (paste_method_)
					{
					case kCopy:
                        for (int i=0; i<w; i++)
                            for (int j=0; j<h; j++)
                                if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
                                    //image_->setPixel(xpos+i, ypos+j, source_window_->imagewidget_->image()->pixel(xsourcepos+i, ysourcepos+j));
                                    image_.at<cv::Vec3b>(ypos+j,xpos+i)=source_window_->imagewidget_->image_.at<cv::Vec3b>(ysourcepos+j,xsourcepos+i);
						break;
					case kGrad:
					case kMixGrad:
						for (int i=0; i<w; i++)
							for (int j=0; j<h; j++)
                                if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j)){
                                    //image_->setPixel(xpos+i, ypos+j, poissoned_img_(xsourcepos+i,ysourcepos+j));
                                    QColor rgb_temp=QColor(poissoned_img_(xsourcepos+i,ysourcepos+j));

                                    image_.at<cv::Vec3b>( ypos+j,xpos+i)[0]=rgb_temp.red();
                                    image_.at<cv::Vec3b>( ypos+j,xpos+i)[1]=rgb_temp.green();
                                    image_.at<cv::Vec3b>( ypos+j,xpos+i)[2]=rgb_temp.blue();

                                }
                    break;
					default:
						break;
					}
				}
			}
			
			update();
			break;

		default:
			break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent *mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
        if(region_shape_==kFree)
        {
            if(is_choosing_)
            {
                polygon_.push_back(mouseevent->pos());
            }
        }
        else if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
		}
		break;

	case kPaste:
		if (is_pasting_)
		{
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			
			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->rect_or_ellipse_.left();
			int ysourcepos = source_window_->imagewidget_->rect_or_ellipse_.top();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->rect_or_ellipse_.right()
				-source_window_->imagewidget_->rect_or_ellipse_.left()+1;
			int h = source_window_->imagewidget_->rect_or_ellipse_.bottom()
				-source_window_->imagewidget_->rect_or_ellipse_.top()+1;

			// Paste
            if ( (xpos>0) && (ypos>0) && (xpos+w<image_.cols) && (ypos+h<image_.rows) )
			{
				// Restore image 
                //image_ =image_backup_;
                image_backup_.copyTo(image_);
                clock_t start,end;
                start=clock();
				if (kCopy != paste_method_)
					poissoned_img_ = poissonEdit_->GetPoissonedImage(xpos,ypos);
                end=clock();
                double endtime=(double)(end-start)/CLOCKS_PER_SEC;
                cout<<"Total time:"<<endtime<<endl;

				// Paste
				switch (paste_method_)
				{
				case kCopy:
					for (int i=0; i<w; i++)
						for (int j=0; j<h; j++)
							if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
                                //image_->setPixel(xpos+i, ypos+j, source_window_->imagewidget_->image()->pixel(xsourcepos+i, ysourcepos+j));
                                 image_.at<cv::Vec3b>(ypos+j,xpos+i)=source_window_->imagewidget_->image_.at<cv::Vec3b>(ysourcepos+j,xsourcepos+i);
                    break;
				case kGrad:
				case kMixGrad:
					for (int i=0; i<w; i++)
						for (int j=0; j<h; j++)
                            if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j)){
                                //image_->setPixel(xpos+i, ypos+j, poissoned_img_(xsourcepos+i,ysourcepos+j));
                                QColor rgb_temp=QColor(poissoned_img_(xsourcepos+i,ysourcepos+j));

                                image_.at<cv::Vec3b>( ypos+j,xpos+i)[0]=rgb_temp.red();
                                image_.at<cv::Vec3b>( ypos+j,xpos+i)[1]=rgb_temp.green();
                                image_.at<cv::Vec3b>( ypos+j,xpos+i)[2]=rgb_temp.blue();

                            }
					break;
				default:
					break;
				}
                clock_t end1=clock();
                double endtime1=(double)(end1-start)/CLOCKS_PER_SEC;
                cout<<"Total time2:"<<endtime1<<endl;
			}
		}

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			QPoint topLeft,bottomRight;
			switch (region_shape_)
			{
			case kRectangle:
			case kEllipse:
				topLeft.setX(min(point_start_.x(),point_end_.x()));
				topLeft.setY(min(point_start_.y(),point_end_.y()));
				bottomRight.setX(max(point_start_.x(),point_end_.x()));
				bottomRight.setY(max(point_start_.y(),point_end_.y()));
				rect_or_ellipse_ = QRect(topLeft,bottomRight);
				is_choosing_ = false;
				draw_status_ = kNone;
				find_inside_points();
				break;
			case kPolygon:
				polygon_<<point_end_;
				point_start_ = point_end_;
				break;
            case kFree:
                if(is_choosing_){
                    int top=1000000,bottom=-1,left=10000000,right=-1;


                            //point_start_ = point_end_ = mouseevent->pos();
                            polygon_.push_back(point_end_);
                            for (int i=0;i<polygon_.size();i++)
                            {
                                if (polygon_[i].x()<left)
                                    left = polygon_[i].x();
                                if(polygon_[i].x()>right)
                                    right = polygon_[i].x();
                                if(polygon_[i].y()>bottom)
                                    bottom = polygon_[i].y();
                                if(polygon_[i].y()<top)
                                    top = polygon_[i].y();
                            }
                            if (top < 0)
                                top =0;
                            if (bottom >= ImageHeight())
                                bottom = ImageHeight()-1;
                            if (left < 0)
                                left = 0;
                            if (right >= ImageWidth())
                                right = ImageWidth()-1;
                            rect_or_ellipse_ = QRect(QPoint(left,top),QPoint(right,bottom));
                            is_choosing_ = false;
                            draw_status_ = kNone;
                            find_inside_points();


                    //is_choosing_=false;
                 //   draw_status_==kNone;
                }
			default:
				break;
			}
		}
        break;
	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
		}
        break;
	default:
		break;
	}
	
	update();
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent *mouseevent)
{
	int top=1000000,bottom=-1,left=10000000,right=-1;
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			switch (region_shape_)
			{
            case kPolygon:
				point_start_ = point_end_ = mouseevent->pos();
				polygon_.push_back(point_end_);
				for (int i=0;i<polygon_.size();i++)
				{
					if (polygon_[i].x()<left)
						left = polygon_[i].x();
					if(polygon_[i].x()>right)
						right = polygon_[i].x();
					if(polygon_[i].y()>bottom)
						bottom = polygon_[i].y();
					if(polygon_[i].y()<top)
						top = polygon_[i].y();
				}
				if (top < 0)
					top =0;
				if (bottom >= ImageHeight())
					bottom = ImageHeight()-1;
				if (left < 0)
					left = 0;
				if (right >= ImageWidth())
					right = ImageWidth()-1;
				rect_or_ellipse_ = QRect(QPoint(left,top),QPoint(right,bottom));
				is_choosing_ = false;
				draw_status_ = kNone;
				find_inside_points();
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
        image_=cv::imread(filename.toLatin1().data());
        //image_=image_.t();
        //std::cout<<filename.toStdString()<<endl;
        //std::cout<<image_.rows<<endl;
        cv::cvtColor(image_,image_,cv::COLOR_BGR2RGB);
        //image_backup_=image_;
        image_.copyTo(image_backup_);
        //image_->load(filename);
        //*(image_backup_) = *(image_);
        inside_points_.resize(image_.cols,image_.rows);
        inside_points_ = Eigen::MatrixXi::Zero(image_.cols,image_.rows);
	}

//	setFixedSize(image_->width(), image_->height());
//	relate_window_->setWindowFlags(Qt::Dialog);
//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
//	relate_window_->setWindowFlags(Qt::SubWindow);
	
	//image_->invertPixels(QImage::InvertRgb);
	//*(image_) = image_->mirrored(true, true);
	//*(image_) = image_->rgbSwapped();
    cout<<"image size: "<<image_.cols<<' '<<image_.rows<<endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}	
    cv::Mat saveimg;
    cv::cvtColor(image_, saveimg, cv::COLOR_BGR2RGB);
    imwrite(filename.toLatin1().data(), saveimg);
    //image_->save(filename);
}

void ImageWidget::Invert()
{
    for (int i=0; i<image_.rows; i++)
	{
        for (int j=0; j<image_.cols; j++)
		{
            //QRgb color = image_->pixel(i, j);
            //image_->setPixel(i, j, qRgb(255-qRed(color), 255-qGreen(color), 255-qBlue(color)) );
             //image_.at<cv::Vec3b>(i, j)=poissoned_img_(xsourcepos+i, ysourcepos+j);
            image_.at<cv::Vec3b>(i, j)[0]=255-image_.at<cv::Vec3b>(i, j)[0];
            image_.at<cv::Vec3b>(i, j)[1]=255-image_.at<cv::Vec3b>(i, j)[1];
            image_.at<cv::Vec3b>(i, j)[2]=255-image_.at<cv::Vec3b>(i, j)[2];
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
    //QImage image_tmp(*(image_));
    cv::Mat image_tmp;
    //int width = image_.cols;
    //int height = image_.rows;
    image_.copyTo(image_tmp);
    int height = image_.cols;
    int width = image_.rows;

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
                    image_.at<cv::Vec3b>(i, j)=image_tmp.at<cv::Vec3b>(width-1-i, height-1-j);
                    //image_->setPixel(i, j, image_tmp.pixel(width-1-i, height-1-j));
				}
			}
		} 
		else
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
                    //image_->setPixel(i, j, image_tmp.pixel(i, height-1-j));
                    image_.at<cv::Vec3b>(i, j)=image_tmp.at<cv::Vec3b>(i, height-1-j);
				}
			}
		}
		
	}
	else
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
                    image_.at<cv::Vec3b>(i, j)=image_tmp.at<cv::Vec3b>(width-1-i, j);
                    //image_->setPixel(i, j, image_tmp.pixel(width-1-i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
    for (int i=0; i<image_.rows; i++)
	{
        for (int j=0; j<image_.cols; j++)
		{
            //QRgb color = image_->pixel(i, j);

            int gray_value = (image_.at<cv::Vec3b>(i,j)[0]+image_.at<cv::Vec3b>(i,j)[1]+image_.at<cv::Vec3b>(i,j)[2])/3;
            //image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
            image_.at<cv::Vec3b>(i,j)[0]=gray_value;
            image_.at<cv::Vec3b>(i,j)[1]=gray_value;
            image_.at<cv::Vec3b>(i,j)[2]=gray_value;

		}
	}

	update();
}

void ImageWidget::Restore()
{
    //image_ = image_backup_;
    image_backup_.copyTo(image_);
	if (!image_backup_vec_.empty())
	{
        //*image_backup_ = image_backup_vec_.back();
        image_backup_vec_.back().copyTo(image_backup_);
		image_backup_vec_.pop_back();
	}
	point_start_ = point_end_ = QPoint(0, 0);
	rect_or_ellipse_ = QRect(point_start_,point_end_);
	polygon_.clear();
	update();
}

void ImageWidget::find_inside_points()
{
	int a,b,c;
	CScanLine scanline;
	switch (region_shape_)
	{
	case kRectangle:
		for (int i=rect_or_ellipse_.left();i<=rect_or_ellipse_.right();i++)
			for (int j=rect_or_ellipse_.top();j<=rect_or_ellipse_.bottom();j++)
				inside_points_(i,j) = 1;
		break;
	case kEllipse:
		a = (rect_or_ellipse_.right()-rect_or_ellipse_.left())/2;
		b = (rect_or_ellipse_.bottom()-rect_or_ellipse_.top())/2;
		c = (int)sqrt(double(b*b-a*a));
		for (int i=rect_or_ellipse_.left();i<=rect_or_ellipse_.right();i++)
			for (int j=rect_or_ellipse_.top();j<=rect_or_ellipse_.bottom();j++)
			{
				int x = i-(rect_or_ellipse_.left()+a);
				int y = j-(rect_or_ellipse_.top()+b);
				if (x*x/double(a*a)+y*y/double(b*b)<=1.0)
					inside_points_(i,j) = 1;
			}
		break;
	case kPolygon:
		scanline.init(ImageWidth(),ImageHeight(),polygon_);
		inside_points_ = scanline.insideMatrix();
		break;
    case kFree:
        scanline.init(ImageWidth(),ImageHeight(),polygon_);
        inside_points_ = scanline.insideMatrix();
        break;
	default:
		break;
	}
	find_boundary();
}

void ImageWidget::find_boundary()
{
	for (int i=0;i<ImageWidth();i++)
		for (int j=0;j<ImageHeight();j++)
			if (0 == inside_points_(i,j))
			{
				if (i+1<ImageWidth() && 1 == inside_points_(i+1,j))
					inside_points_(i,j) = 2;
				else if (i>0 && 1 == inside_points_(i-1,j))
					inside_points_(i,j) = 2;
				else if (j+1<ImageHeight() && 1 == inside_points_(i,j+1))
					inside_points_(i,j) = 2;
				else if (j>0 && 1 == inside_points_(i,j-1))
					inside_points_(i,j) = 2;
			}
}

void ImageWidget::test_inside_points()
{
    for (int i=0; i<image_.cols; i++)
	{
        for (int j=0; j<image_.rows; j++)
		{
            //QRgb color;
			if (1 == inside_points_(i,j))
            {
                image_.at<cv::Vec3b>(i,j)[0]=255;
                image_.at<cv::Vec3b>(i,j)[1]=255;
                image_.at<cv::Vec3b>(i,j)[2]=255;
            }
                //color = qRgb(255,255,255);
			else
            {
                image_.at<cv::Vec3b>(i,j)[0]=0;
                image_.at<cv::Vec3b>(i,j)[1]=0;
                image_.at<cv::Vec3b>(i,j)[2]=0;
            }
                //color = qRgb(0,0,0);
            //image_->setPixel(i, j, color);

		}
	}
	update();
}

void ImageWidget::matrix_predecomp()
{
	if (poissonEdit_)
		delete poissonEdit_;

	switch (paste_method_)
	{
	case kGrad:
		poissonEdit_ = new CPoissonEditGrad;
		break;
	case kMixGrad:
		poissonEdit_ = new CPoissonEditMixGrad;
		break;
	default:
		break;
	}
	poissonEdit_->init(source_window_->imagewidget_,this);
}

void ImageWidget::ClearChosen()
{
	point_start_ = point_end_ = QPoint(0,0);
	polygon_.clear();
	inside_points_ = Eigen::MatrixXi::Zero(ImageWidth(),ImageHeight());
}
