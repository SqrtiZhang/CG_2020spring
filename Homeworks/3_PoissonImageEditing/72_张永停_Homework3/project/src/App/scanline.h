#pragma once
#include <QWidget>
#include <vector>
#include <list>
#include <Eigen/Dense>

class QPolygon;

typedef struct
{
	int ymax;
	double dx;
	double x;
}EDGE;

class CScanLine
{
	int width_,height_;
	Eigen::MatrixXi inside_points_;
	QPolygon polygon_;
	int top_,bottom_,left_,right_;

    std::vector<std::list<EDGE> >	net_;
    std::list<EDGE>					aet_;
    std::vector<double>				intersects_;

    void constructNet();
    void deleteOldEDGEs(int);
    void transCurrentEDGEs();
    void insertNewEDGEs(int);
    static bool edgeCompareByX(EDGE, EDGE);
    void calcIntersects(int);
    void markInsidePoints(int);

public:
	CScanLine(void);
	~CScanLine(void);

	void init(int,int,QPolygon);
    Eigen::MatrixXi insideMatrix();
};

