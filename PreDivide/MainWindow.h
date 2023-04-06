#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "predividewidget.h"
#include "jointwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *event);
private:
    Ui::MainWindowClass ui;
	PreDivideWidget* w;
	jointWidget* j;
};
