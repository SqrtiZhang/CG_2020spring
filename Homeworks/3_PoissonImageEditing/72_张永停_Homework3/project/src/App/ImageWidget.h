#pragma once
#include <QWidget>
#include <Eigen/Dense>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


class ChildWindow;
class CPoissonEdit;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
class QRect;
class QPolygon;
QT_END_NAMESPACE

enum PasteMethods
{
    kCopy,
    kGrad,
    kMixGrad
};

enum RegionShape
{
    kRectangle,
    kEllipse,
    kPolygon,
    kFree
};

enum DrawStatus
{
	kChoose, 
	kPaste, 
	kNone
};

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(ChildWindow *relatewindow);
	~ImageWidget(void);

    int ImageWidth();
    int ImageHeight();
	void set_draw_status_to_choose();
	void set_draw_status_to_paste();
	void set_region_shape(int);
	void set_paste_method(int);
    QImage image();
	void set_source_window(ChildWindow* childwindow);
    void find_inside_points();
    void find_boundary();
	void test_inside_points();
    void matrix_predecomp();
    void ClearChosen();

protected:
	void paintEvent(QPaintEvent *paintevent);
	void mousePressEvent(QMouseEvent *mouseevent);
	void mouseMoveEvent(QMouseEvent *mouseevent);
	void mouseReleaseEvent(QMouseEvent *mouseevent);
	void mouseDoubleClickEvent(QMouseEvent *mouseevent);

public slots:
	// File IO
	void Open(QString filename);								// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal=false, bool vertical=true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
    void Restore();												// Restore image to origin

public:
	QPoint						point_start_;					// Left top point of rectangle region
	QPoint						point_end_;						// Right bottom point of rectangle region
    QRect						rect_or_ellipse_;
    QPolygon					polygon_;
    Eigen::MatrixXi				inside_points_;
    CPoissonEdit				*poissonEdit_;
    Eigen::MatrixXi				poissoned_img_;

private:
    //QImage						*image_;						// image
    //QImage						*image_backup_;
    cv::Mat                     image_;
    cv::Mat                     image_backup_;
    std::vector<cv::Mat>		image_backup_vec_;


	// Pointer of child window
	ChildWindow					*source_window_;				// Source child window

	// Signs
	DrawStatus					draw_status_;					// Enum type of draw status
    RegionShape					region_shape_;
    PasteMethods				paste_method_;
	bool						is_choosing_;
	bool						is_pasting_;
};

