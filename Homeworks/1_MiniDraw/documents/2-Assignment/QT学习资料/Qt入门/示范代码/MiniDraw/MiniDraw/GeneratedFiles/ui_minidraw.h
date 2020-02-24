/********************************************************************************
** Form generated from reading UI file 'minidraw.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MINIDRAW_H
#define UI_MINIDRAW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MiniDrawClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MiniDrawClass)
    {
        if (MiniDrawClass->objectName().isEmpty())
            MiniDrawClass->setObjectName(QStringLiteral("MiniDrawClass"));
        MiniDrawClass->resize(600, 400);
        menuBar = new QMenuBar(MiniDrawClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        MiniDrawClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MiniDrawClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MiniDrawClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(MiniDrawClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MiniDrawClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MiniDrawClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MiniDrawClass->setStatusBar(statusBar);

        retranslateUi(MiniDrawClass);

        QMetaObject::connectSlotsByName(MiniDrawClass);
    } // setupUi

    void retranslateUi(QMainWindow *MiniDrawClass)
    {
        MiniDrawClass->setWindowTitle(QApplication::translate("MiniDrawClass", "MiniDraw", 0));
    } // retranslateUi

};

namespace Ui {
    class MiniDrawClass: public Ui_MiniDrawClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MINIDRAW_H
