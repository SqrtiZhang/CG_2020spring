#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class ChildWindow;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
class QComboBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	// File IO
//	void New();
	void Open();								// Open an existing file
	void Save();								// Save image to file
	void SaveAs();
	ChildWindow *CreateChildWindow();
	void SetActiveSubWindow(QWidget* window);

	// Image Processing
	void Invert();								// Invert each pixel's rgb value
	void Mirror();								// Mirror image vertically or horizontally
	void GrayScale();							// Turn image to gray-scale map
	void Restore();								// Restore image to origin

	void TestInsidePoints();

    void ChooseRect();
    void ChooseElli();
    void ChoosePoly();
    void ChooseFree();

    void ChooseCopy();
    void ChoosePoi();
    void ChooseMixPoi();

private:
	void CreateActions();
	void CreateMenus();
	void CreateToolBars();
	void CreateStatusBar();

	QMdiSubWindow *FindChild(const QString &filename);
    ChildWindow* GetChildWindow();
	
private:
	Ui::MainWindowClass ui;

	QMenu						*menu_file_;
	QMenu						*menu_edit_;
    QMenu                       *menu_paint_;
    QMenu                       *menu_paste_;
	QMenu						*menu_help_;
	QToolBar					*toolbar_file_;
	QToolBar					*toolbar_edit_;
    QToolBar					*toolbar_paint_;
    QToolBar					*toolbar_paste_;

	QAction						*action_open_;
	QAction						*action_save_;
	QAction						*action_saveas_;

	QAction						*action_invert_;
	QAction						*action_mirror_;
	QAction						*action_gray_;
	QAction						*action_restore_;

    QComboBox					*combobox_shape_;
	QAction						*action_choose_;
    QComboBox					*combobox_paste_;
    //QAction						*action_copy_;
	QAction						*action_paste_;	

    QAction                     *action_rect_;
    QAction                     *action_elli_;
    QAction                     *action_poly_;
    QAction                     *action_free_;

    QAction                     *action_copy_;
    QAction                     *action_poi_;
    QAction                     *action_mixpoi_;



	QMdiArea					*mdi_area_;
	QSignalMapper				*window_mapper_;

	ChildWindow					*child_source_;
};

#endif // MAINWINDOW_H
