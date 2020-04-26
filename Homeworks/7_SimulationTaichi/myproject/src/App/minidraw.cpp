#include "minidraw.h"
#include <QColorDialog>
#include <QInputDialog>

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

    setStyleSheet("padding:10px 4px;");
	view_widget_ = new ViewWidget();

    setCentralWidget(view_widget_);
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();

}

void MiniDraw::Creat_Action()
{
    Action_About = new QAction(QIcon("../icon/about.png"),tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

    Action_Line = new QAction(QIcon("../icon/line.png"),tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

    Action_Rect = new QAction(QIcon("../icon/rect.png"),tr("&Rect"), this);
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);

    Action_Ellipse = new QAction(QIcon("../icon/elli.png"),tr("&Ellipse"), this);
    connect(Action_Ellipse, &QAction::triggered, view_widget_, &ViewWidget::setEllipse);

    Action_Poly = new QAction(QIcon("../icon/poly.png"),tr("&Polygon"), this);
    connect(Action_Poly, &QAction::triggered, view_widget_, &ViewWidget::setPoly);

    Action_FreeHand = new QAction(QIcon("../icon/hand.png"),tr("&FreeHand"), this);
    connect(Action_FreeHand, &QAction::triggered, view_widget_, &ViewWidget::setFreeHand);

    Action_Color = new QAction(QIcon("../icon/color.png"),tr("&Color"), this);
    connect(Action_Color, &QAction::triggered,this,&MiniDraw::color_slot);
    connect(this,&MiniDraw::change_color,view_widget_,&ViewWidget::setColor);

    Action_Width = new QAction(QIcon("../icon/width.png"),tr("&Width"),this);
    connect(Action_Width, &QAction::triggered,this,&MiniDraw::width_slot);
    connect(this,&MiniDraw::change_width,view_widget_,&ViewWidget::setWidth);

    Action_Open = new QAction(QIcon("../icon/open.png"),tr("&Insert Picture"),this);
    connect(Action_Open, &QAction::triggered, view_widget_, &ViewWidget::setBackground);

    Action_Undo = new QAction(QIcon("../icon/undo.png"),tr("&Undo"),this);
    connect(Action_Undo ,&QAction::triggered, view_widget_, &ViewWidget::Undo);

    Action_Redo = new QAction(QIcon("../icon/redo.png"),tr("&Redo"),this);
    connect(Action_Redo, &QAction::triggered, view_widget_, &ViewWidget::Redo);

    Action_Clear= new QAction(QIcon("../icon/clear.png"),tr("&Clear"),this);
    connect(Action_Clear, &QAction::triggered, view_widget_, &ViewWidget::Clear);

	Action_Sim = new QAction(QIcon("../icon/sim.png"), tr("&Simulate"), this);
	connect(Action_Sim, &QAction::triggered, view_widget_, &ViewWidget::Sim_Start);

	Action_Stop= new QAction(QIcon("../icon/stop.png"), tr("&Stop Simulate"), this);
	connect(Action_Stop, &QAction::triggered, view_widget_, &ViewWidget::Sim_Stop);

	Action_Pause = new QAction(QIcon("../icon/pause.png"), tr("&Pause Simulate"), this);
	connect(Action_Pause, &QAction::triggered, view_widget_, &ViewWidget::Sim_Pause);


	Action_Source = new QAction(QIcon("../icon/emit.png"), tr("&Simulate Source"), this);
	connect(Action_Source, &QAction::triggered, view_widget_, &ViewWidget::Sim_Source);

	Material = new QComboBox(this);
	Material->addItem(tr("Fluid"));
	Material->addItem(tr("Jelly"));
	Material->addItem(tr("Snow"));
	Material->addItem(tr("Wood"));
	connect(Material, SIGNAL(currentIndexChanged(int)), view_widget_,SLOT(setMaterial(int)));

	Change_Young = new QLineEdit(this);
	connect(Change_Young, SIGNAL(textEdited(QString)), view_widget_, SLOT(setYoung(QString)));
	Change_Young->setPlaceholderText("Young's Modulus(1e3-1e4)");

	Velocityx = new QLineEdit(this);
	connect(Velocityx, SIGNAL(textEdited(QString)), view_widget_, SLOT(setVx(QString)));
	Velocityx->setPlaceholderText("Velocity x");

	Velocityy = new QLineEdit(this);
	connect(Velocityy, SIGNAL(textEdited(QString)), view_widget_, SLOT(setVy(QString)));
	Velocityy->setPlaceholderText("Velocity y");

}

void MiniDraw::Creat_ToolBar()
{
    pToolBar = addToolBar(tr("&Draw"));
    //pToolBar->addAction(Action_Line);
    pToolBar->addAction(Action_Rect);
    pToolBar->addAction(Action_Ellipse);
    //pToolBar->addAction(Action_Poly);
    //pToolBar->addAction(Action_FreeHand);
    pToolBar->addAction(Action_Color);
    pToolBar->addAction(Action_Width);

    p2ToolBar=addToolBar(tr("&Tool"));
    p2ToolBar->addAction(Action_Open);
    p2ToolBar->addAction(Action_Undo);
    p2ToolBar->addAction(Action_Redo);
    p2ToolBar->addAction(Action_Clear);
    p2ToolBar->addAction(Action_About);

	simToolBar = addToolBar(tr("&Simulate"));
	simToolBar->addAction(Action_Sim);
	simToolBar->addAction(Action_Pause);
	simToolBar->addAction(Action_Stop);
	simToolBar->addAction(Action_Source);
	//QLabel* lbl = new QLabel(tr("Material"));
	//simToolBar->addWidget(lbl);
	simToolBar->addWidget(Material);
	simToolBar->addWidget(Change_Young);
	simToolBar->addWidget(Velocityx);
	simToolBar->addWidget(Velocityy);

}

void MiniDraw::Creat_Menu()
{
    p2Menu=menuBar()->addMenu(tr("&File"));
    p2Menu->addAction(Action_Open);
    p2Menu->addAction(Action_About);

    pMenu = menuBar()->addMenu(tr("&Figure"));
   // pMenu->addAction(Action_Line);
    pMenu->addAction(Action_Rect);
    pMenu->addAction(Action_Ellipse);
    //pMenu->addAction(Action_Poly);
    //pMenu->addAction(Action_FreeHand);

    p3Menu=menuBar()->addMenu(tr("&Edit"));
    p3Menu->addAction(Action_Undo);
    p3Menu->addAction(Action_Redo);
    p3Menu->addAction(Action_Clear);

	simMenu=menuBar()->addMenu(tr("&Simulate"));
	simMenu->addAction(Action_Sim);
	simMenu->addAction(Action_Pause);
	simMenu->addAction(Action_Stop);
}

void MiniDraw::color_slot()
{
    QColor c=QColorDialog::getColor(Qt::black);
    emit change_color(c);
}

void MiniDraw::width_slot()
{
    int w= QInputDialog::getInt(this, QString("Set Width"), QString("Input width"), true, 1, 20, 1);
    emit change_width(w);
}

void MiniDraw::AboutBox()
{
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}



MiniDraw::~MiniDraw()
{

}
