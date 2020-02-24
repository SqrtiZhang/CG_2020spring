#include "viewwidget.h"

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	draw_status_ = false;
	shape_ = NULL;
	type_ = Shape::kDefault;
    current_color=Qt::black;
    current_width=1;
    stop_polygon=true;
    isundo=false;
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setLine()
{
    type_ = Shape::kLine;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setEllipse()
{
    type_ = Shape::kEllipse;
}

void ViewWidget::setPoly()
{
    type_=Shape::kPoly;
}

void ViewWidget::setFreeHand()
{
    type_=Shape::kFreeHand;
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
    isundo=false;

    if(Qt::RightButton==event->button())
    {
        stop_polygon=true;
        return;
    }

	if (Qt::LeftButton == event->button())
    {
        if(type_ ==Shape:: kPoly)
        {
            if(stop_polygon==true||shape_list_.empty())
            {
                shape_ = new Poly;
                Poly *tmp=dynamic_cast<Poly *>(shape_);
                QPoint *p = new QPoint(event->pos());
                tmp->Push(p);
                QPoint *p2 = new QPoint(event->pos() + QPoint(0, 1));
                tmp->Push(p2);
                shape_list_.push_back(shape_);
                shape_->set_line(current_color,current_width);
                stop_polygon = false;
            }
            else
            {
                if(type_ !=Shape:: kPoly)
                {
                    stop_polygon=true;
                    return;
                }
                QPoint *p = new QPoint(event->pos());
                dynamic_cast<Poly *>(shape_)->Push(p);
            }
            if (shape_ != NULL)
            {
                draw_status_ = true;
                start_point_ = end_point_ = event->pos();
                shape_->set_start(start_point_);
                shape_->set_end(end_point_);
            }
            update();
            return;
         }

        stop_polygon=true;

        switch (type_)
        {
        case Shape::kLine:
            shape_ = new Line();
            break;
        case Shape::kDefault:
            break;
        case Shape::kRect:
            shape_ = new Rect();
            break;
        case Shape::kEllipse:
            shape_ = new Elli();
            break;
        case Shape::kFreeHand:{
            shape_=new FreeHand();
            Shape *zero=new FreeHand;
            zero->set_start(QPoint(0,0));
            zero->set_end(QPoint(0,0));
            shape_list_.push_back(zero);
            break;}
        default:
            break;
        }

        if (shape_ != NULL)
        {
            draw_status_ = true;
            start_point_ = end_point_ = event->pos();
            shape_->set_start(start_point_);
            shape_->set_end(end_point_);
            shape_->set_line(current_color,current_width);
        }
        update();
   }

}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{

	if (draw_status_ && shape_ != NULL)
	{
        if(type_ == Shape::kPoly)
        {
            Poly *tmp = dynamic_cast<Poly *>(shape_list_.back());
            QPoint *p = tmp->top();
            p->setX(event->x());
            p->setY(event->y());
            update();
            return;
        }
        end_point_ = event->pos();
        shape_->set_end(end_point_);
        if(type_==Shape::kFreeHand)
        {
            shape_=new Line();
            shape_->set_line(current_color,current_width);
            shape_list_.push_back(shape_);
            shape_->set_start(event->pos());
            shape_->set_end(event->pos());
        }
        update();
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(type_==Shape::kFreeHand)
    {
        Shape *zero=new FreeHand;
        zero->set_start(QPoint(0,0));
        zero->set_end(QPoint(0,0));
        shape_list_.push_back(zero);
        return;
    }
    if (shape_ != NULL)
    {
        draw_status_ = false;
        if(type_!=Shape::kPoly)
        {
            shape_list_.push_back(shape_);
            shape_ = NULL;
        }

    }
}

void ViewWidget::paintEvent(QPaintEvent*)
{
    this->setAutoFillBackground(true);
    QPalette pal;
    if(!filename.isEmpty())
        pal.setBrush(QPalette::Window,QBrush(QPixmap(filename)));
    else
        pal.setColor(QPalette::Window,Qt::white);
    setPalette(pal);

	QPainter painter(this);

	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}

    if (shape_ != NULL && !isundo) {
		shape_->Draw(painter);
	}

	update();
}


void ViewWidget::setColor(QColor c)
{
    current_color=c;
}

void ViewWidget::setWidth(int w)
{
    current_width = w % 20;
}

void ViewWidget::setBackground()
{
    filename=QFileDialog::getOpenFileName(this,"打开文件",QDir::currentPath());
}

void ViewWidget::Undo()
{
	stop_polygon = true;
    if(!shape_list_.empty())
    {
        isundo=true;

        if(typeid(*shape_list_.back())==typeid(FreeHand))
        {
            redo_shape_list.push_back(shape_list_.back());
            shape_list_.pop_back();

            while(!shape_list_.empty())
            {
                if(shape_list_.back()->get_start()==QPoint(0,0) &&
                        shape_list_.back()->get_end()==QPoint(0,0))
                {
                    redo_shape_list.push_back(shape_list_.back());
                    shape_list_.pop_back();
                    break;
                }

                redo_shape_list.push_back(shape_list_.back());
                shape_list_.pop_back();
            }
        }
        else
        {
            redo_shape_list.push_back(shape_list_.back());
            shape_list_.pop_back();
        }
    }
    update();
}

void ViewWidget::Redo()
{
	stop_polygon = true;
    if(!redo_shape_list.empty())
    {
        if(typeid(*redo_shape_list.back())==typeid(FreeHand))
        {
            shape_list_.push_back(redo_shape_list.back());
            redo_shape_list.pop_back();

            while(!redo_shape_list.empty())
            {
                if(redo_shape_list.back()->get_start()==QPoint(0,0) &&
                        redo_shape_list.back()->get_end()==QPoint(0,0))
                {
                    shape_list_.push_back(redo_shape_list.back());
                    redo_shape_list.pop_back();
                    break;
                }
                shape_list_.push_back(redo_shape_list.back());
                redo_shape_list.pop_back();
            }
        }
        else
        {
            shape_list_.push_back(redo_shape_list.back());
            redo_shape_list.pop_back();
        }
    }
    update();
}

void ViewWidget::Clear()
{
    current_color=Qt::black;
    current_width=1;
    shape_list_.clear();
    redo_shape_list.clear();
    stop_polygon=true;
    filename="";
    update();
}
