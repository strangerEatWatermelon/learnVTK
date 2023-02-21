#include "MainWindow.h"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	w = new PreDivideWidget(this);
	w->show();
	//w->hide();
	j = new jointWidget(this);
	//j->show();
	j->hide();

	QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->addWidget(w);
	//horizontalLayout->addWidget(j);
	centralWidget()->setLayout(horizontalLayout);
}

MainWindow::~MainWindow()
{
	delete w;
	delete j;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	w->stopInteractor();
}
