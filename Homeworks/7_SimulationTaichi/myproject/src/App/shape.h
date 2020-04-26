#pragma once

#include <QtGui>

class Shape
{
public:
	Shape();
	virtual ~Shape();
    virtual void Draw(QPainter &paint)=0;
	virtual int shape_type() { return NULL; };
	virtual int material_type() { return NULL; };
    void set_start(QPoint s);
    void set_end(QPoint e);
    void set_line(QColor line_color,int line_width);
	void set_material(int m) { material = m; };
	void set_v(float x, float y) { v = QPoint(x, y); };
    QPoint get_start();
    QPoint get_end();
	QPoint get_v() { return v; };
	QColor get_color() { return color; };

public:
	enum Type
	{
		kDefault = 0,
		kLine=1,
		kRect=2,
        kEllipse=3,
        kPoly=4,
        kFreeHand=5,
	};
	
protected:
	QPoint start;
	QPoint end;
    QColor color;
	QPoint v;
    int width;
	int material;  
};

