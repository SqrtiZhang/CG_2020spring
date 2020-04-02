#include "ScanLine.h"


CScanLine::CScanLine(void)
	:width_(0),height_(0),top_(0),bottom_(0),left_(0),right_(0)
{
}


CScanLine::~CScanLine(void)
{
}


void CScanLine::init(int width, int height, QPolygon polygon)
{
	width_ = width;
	height_ = height;
	polygon_ = polygon;
	inside_points_ = Eigen::MatrixXi::Zero(width,height);

	top_ = 100000000;
	bottom_ = -1;
	left_ = 10000000;
	right_ = -1;
	for (QPolygon::iterator iter=polygon_.begin();iter!=polygon_.end();iter++)
	{
		if (iter->x() < left_)
			left_ = iter->x();
		if (iter->x() > right_)
			right_ = iter->x();
		if (iter->y() < top_)
			top_ = iter->y();
		if (iter->y() > bottom_)
			bottom_ = iter->y();
	}
}

Eigen::MatrixXi CScanLine::insideMatrix()
{
	constructNet();
	for (int i=top_;i<=bottom_;i++)
	{
		deleteOldEDGEs(i);
		transCurrentEDGEs();
		insertNewEDGEs(i);
		aet_.sort(edgeCompareByX);
		calcIntersects(i);
		markInsidePoints(i);
	}
	return inside_points_;
}

void CScanLine::constructNet()
{
	net_.resize(bottom_-top_+1);
	int count = polygon_.size()-1;
	for (int i=0;i<count;i++)
	{
		EDGE e;
		int low,high;
		QPoint p[2];

		p[0] = polygon_[i];
		p[1] = polygon_[(i+1)%count];
		if(p[0].y()<p[1].y())
		{
			e.x = p[0].x();
			e.ymax = p[1].y();
			e.dx = (p[1].x()-p[0].x())/double(p[1].y()-p[0].y());
			net_[p[0].y()-top_].insert(net_[p[0].y()-top_].end(),e);
		}
		else if (p[0].y()>p[1].y())
		{
			e.x = p[1].x();
			e.ymax = p[0].y();
			e.dx = (p[1].x()-p[0].x())/double(p[1].y()-p[0].y());
			net_[p[1].y()-top_].insert(net_[p[1].y()-top_].end(),e);
		}
		else
		{
			e.x = p[0].x();
			e.ymax = p[0].y();
			e.dx = 100000000;
			net_[p[0].y()-top_].insert(net_[p[0].y()-top_].end(),e);
			e.x = p[1].x();
			e.ymax = p[1].y();
			e.dx = 100000000;
			net_[p[1].y()-top_].insert(net_[p[1].y()-top_].end(),e);
		}
	}
}

void CScanLine::deleteOldEDGEs(int height)
{
	std::list<EDGE>::iterator iter=aet_.begin();
	while (iter != aet_.end())
	{
		if (iter->ymax < height)
		{
			iter = aet_.erase(iter);
		}else
			iter++;

	}
}

void CScanLine::transCurrentEDGEs()
{
	std::list<EDGE>::iterator iter=aet_.begin();
	while (iter != aet_.end())
	{
		iter->x += iter->dx;
		iter++;
	}
}

void CScanLine::insertNewEDGEs(int height)
{
	aet_.insert(aet_.end(),net_[height-top_].begin(),net_[height-top_].end());
}

bool CScanLine::edgeCompareByX(EDGE e1, EDGE e2)
{
	return (e1.x < e2.x);
}

void CScanLine::calcIntersects(int height)
{
	intersects_.clear();
	if (aet_.empty())
		return;
	std::list<EDGE>::iterator iter=aet_.begin(),iter2;
	while (iter != aet_.end())
	{
		iter2 = iter;
		iter2++;
		if (aet_.end() == iter2)
		{
			intersects_.push_back(iter->x);
			iter++;
		}
        else if(fabs(iter->x - iter2->x)>2.0)
		{
			intersects_.push_back(iter->x);
			iter++;
		}
		else
		{
			if ((iter->ymax == height) && (iter2->ymax == height))
			{
				intersects_.push_back(iter->x);
				intersects_.push_back(iter->x);
			}
            else if((iter->ymax > height) && (iter2->ymax > height))
			{
			}
			else
			{
                intersects_.push_back(iter->x);
			}
			iter++;
			iter++;

		}
	}
	std::sort(intersects_.begin(),intersects_.end());
}

void CScanLine::markInsidePoints(int height)
{
    int status = 0;
	std::vector<int> intPts;

	for(int i=0;i<intersects_.size();i++)
	{
		if (intPts.empty())
			intPts.push_back((int)intersects_[i]);
		else if((int)intersects_[i] == intPts.back())
		{
			intPts.pop_back();
		}
		else
			intPts.push_back((int)intersects_[i]);
	}
	intPts.push_back(100000000);
	int index = 0;
	for(int i=left_-1;i<=right_;i++)
	{
		if (i == intPts[index])
		{
            status = !status;
			index++;
		}
		if (status)
			if (i>0 && i<width_ && height>0 && height<height_)
				inside_points_(i,height) = 1;
	}
}
