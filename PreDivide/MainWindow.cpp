#include "MainWindow.h"
#include <QHBoxLayout>
#include <QDebug>


quint64 getDiskFreeSpace(QString driver)
{//return free mb
#if defined(Q_OS_WIN32)
	LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
	if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
	{
		qDebug() << "ERROR: Call to GetDiskFreeSpaceEx() failed.";
		return 0;
	}
	return (quint64)liTotalFreeBytes.QuadPart / 1024 / 1024;
#else
	int state;
	struct statvfs vfs;
	fsblkcnt_t block_size = 0;
	fsblkcnt_t free_size;

	state = statvfs("/", &vfs);
	if (state < 0) {
		printf("read statvfs error!!!\n");
	}
	block_size = vfs.f_bsize;
	free_size = vfs.f_bfree * block_size;
	return free_size / 1024 / 1024;
#endif
}

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
	auto d = getDiskFreeSpace("d:/");
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
