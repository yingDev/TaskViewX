#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TaskViewX.h"

class TaskViewX : public QMainWindow
{
    Q_OBJECT

public:
    TaskViewX(QWidget *parent = Q_NULLPTR);

private:
    Ui::TaskViewXClass ui;
};
