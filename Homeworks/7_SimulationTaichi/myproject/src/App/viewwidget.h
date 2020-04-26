#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include "ui_viewwidget.h"


#include "shape.h"
#include "Line.h"
#include "Rect.h"
#include "Elli.h"
#include "Poly.h"
#include "FreeHand.h"

#include <qevent.h>
#include <qpainter.h>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>

#include <vector>

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;
	bool stop_simulate;
	bool pause_simulate;
	QPoint start_point_;
    QPoint end_point_;
    QVector<QPoint> points;
	Shape::Type type_;
	Shape* shape_;
	std::vector<Shape*> shape_list_;
	std::vector<Shape*> shape_source_;
	std::vector<QPointF> point_;
	std::vector<long long>  point_color_;
	
    std::vector<Shape*> redo_shape_list;
    bool stop_polygon;

	float young_view;

private:
    QColor current_color;
    QString filename;
    int current_width;
	int current_Material;
	std::vector<float> current_vel;
    bool isundo;

	bool setSource;

public:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

public:
	void paintEvent(QPaintEvent*);

/*signals:
    void Change_Color(QColor);
    void Change_Width(int);
    void Change_Alpha(int);
*/
public slots:
	void setLine();
	void setRect();
    void setEllipse();
    void setPoly();
    void setFreeHand();

    void setColor(QColor c);
    void setWidth(int w);
    void setBackground();

    void Undo();
    void Redo();
    void Clear();

	void Sim_Start();
	void Sim_Stop() ;
	void Sim_Pause();
	void Sim_Source();
	void setMaterial(int m);
	void setYoung(QString s);
	void setVx(QString s);
	void setVy(QString s);
};

#endif // VIEWWIDGET_H
