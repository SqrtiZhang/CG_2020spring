#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();
    ptr_tmp = new QImage();
    current_point_=-1;
    is_input_=false;
}


ImageWidget::~ImageWidget(void)
{

    delete ptr_tmp;
    delete ptr_image_;
    delete ptr_image_backup_;

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
	QRect rect = QRect( (width()-ptr_image_->width())/2, (height()-ptr_image_->height())/2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_); 

    if(is_input_)
    {
        QPen pen;

        pen.setColor("red");
        pen.setWidth(2);
        painter.setPen(pen);

        for(int i=0;i<p_.size();i++)
            painter.drawLine(p_[i],q_[i]);
    }

	painter.end();

    update();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}

    std::cout<<"image size: "<<ptr_image_->width()<<' '<<ptr_image_->height()<<std::endl;
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

	ptr_image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255-qRed(color), 255-qGreen(color), 255-qBlue(color)) );
		}
	}

	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, height-1-j));
				}
			}
		} 
		else
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height-1-j));
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
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color)+qGreen(color)+qBlue(color))/3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	update();
}

void ImageWidget::mousePressEvent(QMouseEvent * event)
{
    if (Qt::LeftButton == event->button()&&is_input_)
    {
        current_point_++;
        //std::cout<<event->pos().x()<<" "<<event->pos().y()<<endl;
        p_.push_back(event->pos());
        q_.push_back(event->pos());
    }
    *ptr_tmp=*ptr_image_;
}

void ImageWidget::mouseMoveEvent(QMouseEvent * event)
{
    if(is_input_)
    {
        q_[current_point_]=event->pos();
    }

}

void ImageWidget::mouseReleaseEvent(QMouseEvent * event)
{
    return;
}

void ImageWidget::Input()
{
    is_input_=true;
    p_.clear();
    q_.clear();

    p_.push_back(QPoint((width()-ptr_image_->width())/2,(height()-ptr_image_->height())/2));
    q_.push_back(QPoint((width()-ptr_image_->width())/2,(height()-ptr_image_->height())/2));

    p_.push_back(QPoint((width()-ptr_image_->width())/2,(height()+ptr_image_->height())/2));
    q_.push_back(QPoint((width()-ptr_image_->width())/2,(height()+ptr_image_->height())/2));

    p_.push_back(QPoint((width()+ptr_image_->width())/2,(height()-ptr_image_->height())/2));
    q_.push_back(QPoint((width()+ptr_image_->width())/2,(height()-ptr_image_->height())/2));

    p_.push_back(QPoint((width()+ptr_image_->width())/2,(height()+ptr_image_->height())/2));
    q_.push_back(QPoint((width()+ptr_image_->width())/2,(height()+ptr_image_->height())/2));

    current_point_=3;
}

void ImageWidget::DoIDW()
{
    is_input_=false;
    IDW idw = IDW();
    idw.set_s(QPoint((width()-ptr_image_->width())/2,(height()-ptr_image_->height())/2));
    idw.set_p(p_);
    idw.set_q(q_);
    *ptr_tmp=idw.get_image_deal_with_IDW(*ptr_image_);
    *(ptr_image_)=*(ptr_tmp);
    update();
}

void ImageWidget::DoRBF()
{
    is_input_=false;

    RBF rbf=RBF(p_,q_,QPoint((width()-ptr_image_->width())/2,(height()-ptr_image_->height())/2),-1);
    *ptr_tmp=rbf.get_image_deal_with_RBF(*ptr_image_);
    *(ptr_image_)=*(ptr_tmp);

    update();
}
