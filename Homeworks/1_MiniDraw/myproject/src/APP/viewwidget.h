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
	QPoint start_point_;
    QPoint end_point_;
    QVector<QPoint> points;
	Shape::Type type_;
	Shape* shape_;
	std::vector<Shape*> shape_list_;
    std::vector<Shape*> redo_shape_list;
    bool stop_polygon;

private:
    QColor current_color;
    QString filename;
    int current_width;
    bool isundo;

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

};

#endif // VIEWWIDGET_H
