#pragma once
#include <QWidget>
#include <vector>
#include "ImageWidget.h"
#include <Eigen/Core>
#include <Eigen/SparseCholesky>

class QPoint;
class QRect;
class ImageWidget;

class CPoissonEdit
{
protected:
	ImageWidget		*source_imagewidget_;				//原图
	ImageWidget		*imagewidget_;						//新图
	QRect			*source_rect_region_;				//原图中选择区域的矩形外框
	Eigen::MatrixXi *source_inside_points_;				//原图内点标记矩阵
	Eigen::MatrixXi poissoned_rgb_;						//生成的新图像

	int inside_points_count_;							//原图内点数
	std::vector<QPoint> index_to_coor_;					//点号到坐标
	Eigen::MatrixXi coor_to_index_;						//坐标到点号

	Eigen::SparseMatrix<double> A_;						//方程系数阵
	Eigen::MatrixX3d b_;								//方程右端项
	Eigen::SimplicialLLT<Eigen::SparseMatrix<double> > LLT_;

	int source_width();
	int source_height();
	void buildAMat();									//构建系数阵
	void buildbMat(int, int);							//构建右端项
	virtual int v(QPoint, QPoint, int, QPoint) = 0;		//开放给子类构建不同的右端项

public:
	CPoissonEdit(void);
	virtual ~CPoissonEdit(void);

	void init(ImageWidget *, ImageWidget *);
	Eigen::MatrixXi GetPoissonedImage(int, int);		//求解并返回新图像
};

