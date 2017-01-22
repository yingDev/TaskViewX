/********************************************************************************
** Form generated from reading UI file 'TaskViewX.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TASKVIEWX_H
#define UI_TASKVIEWX_H

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

class Ui_TaskViewXClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *TaskViewXClass)
    {
        if (TaskViewXClass->objectName().isEmpty())
            TaskViewXClass->setObjectName(QStringLiteral("TaskViewXClass"));
        TaskViewXClass->resize(600, 400);
        menuBar = new QMenuBar(TaskViewXClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        TaskViewXClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(TaskViewXClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        TaskViewXClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(TaskViewXClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        TaskViewXClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(TaskViewXClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        TaskViewXClass->setStatusBar(statusBar);

        retranslateUi(TaskViewXClass);

        QMetaObject::connectSlotsByName(TaskViewXClass);
    } // setupUi

    void retranslateUi(QMainWindow *TaskViewXClass)
    {
        TaskViewXClass->setWindowTitle(QApplication::translate("TaskViewXClass", "TaskViewX", 0));
    } // retranslateUi

};

namespace Ui {
    class TaskViewXClass: public Ui_TaskViewXClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TASKVIEWX_H
