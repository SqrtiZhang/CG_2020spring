#ifndef MINIDRAW_H
#define MINIDRAW_H

#include <ui_minidraw.h>
#include <viewwidget.h>

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>

#include <QLabel>
#include <QAction>
#include <QComboBox>
#include <QToolBar>
#include <QMenu>
#include <QLineEdit>
#include <QMenuBar>
#include <QDial>

class MiniDraw : public QMainWindow
{
	Q_OBJECT

public:
	MiniDraw(QWidget *parent = 0);
	~MiniDraw();

    QMenu *pMenu,*p2Menu,*p3Menu,*simMenu;
    QToolBar *pToolBar,*p2ToolBar,*simToolBar;
	QAction *Action_About;
	QAction *Action_Line;
	QAction *Action_Rect;
    QAction *Action_Ellipse;
    QAction *Action_Poly;
    QAction *Action_FreeHand;
    QAction *Action_Color;
    QAction *Action_Width;
    QAction *Action_Open;
    QAction *Action_Undo;
    QAction *Action_Redo;
    QAction *Action_Clear;
	QAction* Action_Sim;
	QAction* Action_Stop;
	QAction* Action_Source;
	QAction* Action_Pause;
	QLineEdit* Velocityx;
	QLineEdit* Velocityy;
	QComboBox* Material;
	QLineEdit* Change_Young;

	void Creat_Menu();
	void Creat_ToolBar();
    void Creat_Action();
	void AboutBox();
    void color_slot();
    void width_slot();

signals:
    void change_color(QColor);
    void change_width(int);

private:
	Ui::MiniDrawClass ui;
	ViewWidget*	view_widget_;


private:
    QLabel *width;
};

#endif // MINIDRAW_H
