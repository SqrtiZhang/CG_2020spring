#include "PoissonEdit.h"


CPoissonEdit::CPoissonEdit(void)
	:source_imagewidget_(NULL),imagewidget_(NULL),source_rect_region_(NULL),source_inside_points_(NULL),
	inside_points_count_(0)
{
}


CPoissonEdit::~CPoissonEdit(void)
{
}

void CPoissonEdit::init(ImageWidget *src_imgwdg, ImageWidget *imgwdg)
{
	inside_points_count_ = 0;
	source_imagewidget_ = src_imgwdg;
	imagewidget_ = imgwdg;

	source_rect_region_ = &(source_imagewidget_->rect_or_ellipse_);
	source_inside_points_ = &(source_imagewidget_->inside_points_);

	poissoned_rgb_.resize(source_width(),source_height());
	coor_to_index_.resize(source_width(),source_height());
	index_to_coor_.clear();
	
	//对内点编号
	for (int i=0;i<source_width();i++)
		for (int j=0;j<source_height();j++)
			if (1 == (*source_inside_points_)(i,j))
			{
				coor_to_index_(i,j) = inside_points_count_++;
				index_to_coor_.push_back(QPoint(i,j));
			}
	//构建系数阵并分解
	if (inside_points_count_)
		buildAMat();
}

int CPoissonEdit::source_width()
{
	return source_imagewidget_->width();
}

int CPoissonEdit::source_height()
{
	return source_imagewidget_->height();
}

void CPoissonEdit::buildAMat()
{
	A_.resize(inside_points_count_,inside_points_count_);

	std::vector<Eigen::Triplet<double> > coefficients;
	for (int i=0;i<inside_points_count_;i++)
	{
		QPoint coor = index_to_coor_[i];
		int x = coor.x();
		int y = coor.y();
		int np = 4;
		if (0 == x || source_width()-1 == x)
			np--;
		if (0 == y || source_height()-1 == y)
			np--;
		coefficients.push_back(Eigen::Triplet<double>(i,i,np));

		if (x>0 && 1==(*source_inside_points_)(x-1,y))
			coefficients.push_back(Eigen::Triplet<double>(i,coor_to_index_(x-1,y),-1));
		if (x<source_width()-1 && 1==(*source_inside_points_)(x+1,y))
			coefficients.push_back(Eigen::Triplet<double>(i,coor_to_index_(x+1,y),-1));
		if (y>0 && 1==(*source_inside_points_)(x,y-1))
			coefficients.push_back(Eigen::Triplet<double>(i,coor_to_index_(x,y-1),-1));
		if (y<source_height()-1 && 1==(*source_inside_points_)(x,y+1))
			coefficients.push_back(Eigen::Triplet<double>(i,coor_to_index_(x,y+1),-1));
	}
	A_.setFromTriplets(coefficients.begin(),coefficients.end());
	LLT_.compute(A_);
}

Eigen::MatrixXi CPoissonEdit::GetPoissonedImage(int xpos, int ypos)
{
	if (!inside_points_count_)
		return poissoned_rgb_;
	buildbMat(xpos,ypos);
	Eigen::MatrixX3d X;
	X = LLT_.solve(b_);
	//解出的颜色不在[0,255]要设为临界值
	for (int i=0;i<inside_points_count_;i++)
	{
		QPoint coor = index_to_coor_[i];
		int r,g,b;
		r = (int)X(i,0);
		r = (r<0)?0:r;
		r = (r>255)?255:r;
		g = (int)X(i,1);
		g = (g<0)?0:g;
		g = (g>255)?255:g;
		b = (int)X(i,2);
		b = (b<0)?0:b;
		b = (b>255)?255:b;
		poissoned_rgb_(coor.x(),coor.y()) = qRgb(r,g,b);
	}
	return poissoned_rgb_;
}

void CPoissonEdit::buildbMat(int xpos, int ypos)
{
	QPoint delta[4] = {QPoint(-1,0),QPoint(1,0),QPoint(0,-1),QPoint(0,1)};
	b_ = Eigen::MatrixX3d::Zero(inside_points_count_,3);
	for (int i=0;i<inside_points_count_;i++)
	{
		QPoint coor = index_to_coor_[i];
		int x = coor.x();
		int y = coor.y();

		for (int j=0;j<4;j++)
		{
			QPoint q = coor + delta[j];
			if (q.x()>=0 && q.x()<source_width() && q.y()>=0 && q.y()<source_height())
			{
				b_(i,0) += v(coor,q,0,QPoint(xpos,ypos));
				b_(i,1) += v(coor,q,1,QPoint(xpos,ypos));
				b_(i,2) += v(coor,q,2,QPoint(xpos,ypos));
				if ( 2== (*source_inside_points_)(q.x(),q.y()))
				{
					b_(i,0) += qRed(imagewidget_->image()->pixel(q-source_rect_region_->topLeft()+QPoint(xpos,ypos)));
					b_(i,1) += qGreen(imagewidget_->image()->pixel(q-source_rect_region_->topLeft()+QPoint(xpos,ypos)));
					b_(i,2) += qBlue(imagewidget_->image()->pixel(q-source_rect_region_->topLeft()+QPoint(xpos,ypos)));
				}
			}
		}
	}
}