#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TSS_App.h"

class TSS_App : public QMainWindow
{
    Q_OBJECT

public:
    TSS_App(QWidget *parent = nullptr);
    ~TSS_App();

private:
    Ui::TSS_AppClass ui;
};

