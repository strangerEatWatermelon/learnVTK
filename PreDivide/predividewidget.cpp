#include "predividewidget.h"
#include "ui_predividewidget.h"
#include <QFileDialog>
#include <QMessageBox>

#include <vtkImageActor.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageEllipsoidSource.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMask.h>
#include <vtkInteractorStyleImage.h>
#include <vtkJPEGWriter.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

PreDivideWidget::PreDivideWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreDivideWidget)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	rebuildList();
    connect(ui->m_preBtnDivide,SIGNAL(clicked()),this,SLOT(on_PreCut_clicked()));
    //m_3DViewWidget.setStyleSheet("background-color:rgb(110, 111, 102)");
    connect(ui->m_btnFile,SIGNAL(clicked()),this,SLOT(on_OpenFile_clicked()));
    this->setLayout(ui->verticalLayout);
    connect(ui->m_preBtnCut,SIGNAL(clicked()),this,SLOT(on_preBtnCut_clicked()));
    connect(ui->m_preSelect,SIGNAL(clicked()),this,SLOT(on_preBtnSelect_clicked()));
    connect(ui->m_CutBtnCuting,SIGNAL(clicked()),this,SLOT(on_CutBtnExport_clicked()));
    connect(ui->m_CutBtnRestore,SIGNAL(clicked()),this,SLOT(on_CutBtnRestore_clicked()));
	QHBoxLayout* hl = new QHBoxLayout(ui->wind1);
	hl->addWidget(&m_2DViewWidget[0]);
	QHBoxLayout* h2 = new QHBoxLayout(ui->wind2);
	h2->addWidget(&m_2DViewWidget[1]);
	QHBoxLayout* h3 = new QHBoxLayout(ui->wind3);
	h3->addWidget(&m_2DViewWidget[2]);
	connect(this, SIGNAL(showVolumeDataSignal()), this, SLOT(showVolumeDataSlot()),Qt::QueuedConnection);
	connect(ui->m_CutErase, SIGNAL(clicked()), this, SLOT(eraseBtn_clicked()));
	connect(ui->m_daub, SIGNAL(clicked()), this, SLOT(on_DuabBtn_clicked()));
	connect(ui->m_CheckLabelCount, SIGNAL(clicked()), this, SLOT(on_CheckLabelCountBtn_clicked()));
	connect(&m_3DViewWidget, SIGNAL(cutDataSignal(QList<QPointF> &)), this, SLOT(cutDataSlot(QList<QPointF> &)));

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	m_pContextMenu = new QMenu(this);
	m_pActionDel = new QAction(this);
	m_pActionDel->setText(QString(u8"删除"));
	m_pActionMerge = new QAction(this);
	m_pActionMerge->setText(QString(u8"合并"));
	m_pContextMenu->addAction(m_pActionMerge);
	m_pContextMenu->addAction(m_pActionDel);
	m_pSaveMenu = new QMenu(u8"保存为", m_pContextMenu);
	m_pActionSaveAsLeft = new QAction(u8"左侧");
	m_pActionSaveAsRight = new QAction(u8"右侧");
	m_pContextMenu->addMenu(m_pSaveMenu);
	m_pSaveMenu->addAction(m_pActionSaveAsLeft);
	m_pSaveMenu->addAction(m_pActionSaveAsRight);
	connect(m_pContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(SlotMenuClicked(QAction*)));
	connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(tableContexMenuRequested(const QPoint&)));
	ui->m_CutBtnRestore->setEnabled(false);
}

PreDivideWidget::~PreDivideWidget()
{
    delete ui;
	stopInteractor();
}

void PreDivideWidget::tableBtnClicked()
{
    QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	listUserData *d = dynamic_cast<listUserData*>(btn->userData(0));
    if(d->btnId == 1)
    {
        if(btn->text() == u8"显示")
        {
			btn->setText(u8"隐藏");
			if (volume.size() > 0)
			{
				compositeOpacity[d->labelId]->RemovePoint(d->labelId);
				compositeOpacity[d->labelId]->AddPoint(d->labelId, 0);
				volumeMulti->Update();
				m_labelVisible[d->labelId] = false;
			}
        }
        else {
			btn->setText(u8"显示");
			if (volume.size() > 0)
			{
				compositeOpacity[d->labelId]->RemovePoint(d->labelId);
				compositeOpacity[d->labelId]->AddPoint(d->labelId, 1);
				volumeMulti->Update();
				m_labelVisible[d->labelId] = true;
			}
        }
		m_3DViewWidget.renderWindow()->Render();
    }
	else if (d->btnId == 2)
	{
		if (color.size() == 0)
			return;
		QColor color_t = QColorDialog::getColor(Qt::white, this);
		if (color_t.isValid())
		{
			color[d->labelId]->RemovePoint(d->labelId);
			color[d->labelId]->AddRGBPoint(d->labelId, (double)color_t.red() / 255.0,
				(double)color_t.green() / 255.0, (double)color_t.blue() / 255.0);
			volumeMulti->Update();
			m_3DViewWidget.renderWindow()->Render();
			m_labelColor[d->labelId] = color_t;
			m_bt[d->labelId][1]->setStyleSheet(QString("background-color:rgb(%1, %2, %3)")
				.arg(QString::number(m_labelColor[d->labelId].red()),
					QString::number(m_labelColor[d->labelId].green()),
					QString::number(m_labelColor[d->labelId].blue())));
		}
	}
}

void PreDivideWidget::on_PreCut_clicked()
{
	ui->horizontalLayout_2->removeWidget(ui->wind1);
    ui->horizontalLayout_2->removeWidget(ui->wind2);
    ui->horizontalLayout_2->removeWidget(ui->wind3);
    ui->horizontalLayout_2->addWidget(&m_3DViewWidget);
    ui->wind1->hide();
    ui->wind2->hide();
    ui->wind3->hide();
	this->update();
	//必须等窗口创建成功后让qt信号机制自己去协调任务执行流程，showVolumeDataSignal是排队的非阻塞链接
	emit showVolumeDataSignal();
}

void PreDivideWidget::on_OpenFile_clicked()
{
	QString filePath = QFileDialog::getExistingDirectory(this, u8"请选择DICOM文件路径");
	if (filePath.isEmpty())
		return;
	typedef signed short    PixelType;
	const unsigned int      Dimension = 3;
	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
	typedef itk::Image< PixelType, Dimension >         ImageType;
	using ReaderType = itk::ImageSeriesReader< ImageType >;
	using ImageIOType = itk::GDCMImageIO;
	ReaderType::Pointer reader = ReaderType::New();
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	reader->SetImageIO(dicomIO);
	//获取读取序列名称
	using NamesGeneratorType = itk::GDCMSeriesFileNames;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->SetDirectory(filePath.toStdString());
	using SeriesIdContainer = std::vector< std::string >;
	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
	auto seriesItr = seriesUID.begin();
	auto seriesEnd = seriesUID.end();
	using FileNamesContainer = std::vector< std::string >;
	FileNamesContainer fileNames;
	std::string seriesIdentifier;
	while (seriesItr != seriesEnd)
	{
		seriesIdentifier = seriesItr->c_str();
		fileNames = nameGenerator->GetFileNames(seriesIdentifier);
		++seriesItr;
	}
	//读取序列
	reader->SetFileNames(fileNames);
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
		return;
	}
	ImageType::SizeType imgSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
	cout << "read done！Original size: " << imgSize << endl;

	typedef itk::ImageToVTKImageFilter< ImageType> itkTovtkFilterType;
	itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
	itkTovtkImageFilter->SetInput(reader->GetOutput());
	itkTovtkImageFilter->Update();

	m_vtkImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	m_vtkImageFlip->SetInputData(itkTovtkImageFilter->GetOutput());
	m_vtkImageFlip->SetFilteredAxes(1);
	m_vtkImageFlip->Update();

	m_vtkImageFlip->GetOutput()->GetExtent(extent);
	m_vtkImageFlip->GetOutput()->GetSpacing(spacing);
	m_vtkImageFlip->GetOutput()->GetOrigin(origin);
	m_vtkImageFlip->GetOutput()->GetDimensions(dimensions);

	double center[3];
	center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
	center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

	static double axialElements[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	static double coronalElements[16] = {
		1, 0, 0, 0,
		0, 0, 1, 0,
		0,-1, 0, 0,
		0, 0, 0, 1 
	};
	static double sagittalElements[16] = {
		0, 0,-1, 0,
		1, 0, 0, 0,
		0,-1, 0, 0,
		0, 0, 0, 1 
	};

	vtkSmartPointer<vtkMatrix4x4> resliceAxes =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceAxes->DeepCopy(axialElements);
	resliceAxes->SetElement(0, 3, center[0]);
	resliceAxes->SetElement(1, 3, center[1]);
	resliceAxes->SetElement(2, 3, center[2]);

	reslice1 = vtkSmartPointer<vtkImageReslice>::New();
	reslice1->SetInputConnection(m_vtkImageFlip->GetOutputPort());
	reslice1->SetOutputDimensionality(2);
	reslice1->SetResliceAxes(resliceAxes);
	reslice1->SetInterpolationModeToLinear();


	vtkSmartPointer<vtkMatrix4x4> resliceSagittal =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceSagittal->DeepCopy(sagittalElements);
	resliceSagittal->SetElement(0, 3, center[0]);
	resliceSagittal->SetElement(1, 3, center[1]);
	resliceSagittal->SetElement(2, 3, center[2]);
	

	reslice2 = vtkSmartPointer<vtkImageReslice>::New();
	reslice2->SetInputConnection(m_vtkImageFlip->GetOutputPort());
	reslice2->SetOutputDimensionality(2);
	reslice2->SetResliceAxes(resliceSagittal);
	reslice2->SetInterpolationModeToLinear();


	vtkSmartPointer<vtkMatrix4x4> resliceCoronal =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceCoronal->DeepCopy(coronalElements);
	resliceCoronal->SetElement(0, 3, center[0]);
	resliceCoronal->SetElement(1, 3, center[1]);
	resliceCoronal->SetElement(2, 3, center[2]);

	reslice3 = vtkSmartPointer<vtkImageReslice>::New();
	reslice3->SetInputConnection(m_vtkImageFlip->GetOutputPort());
	reslice3->SetOutputDimensionality(2);
	reslice3->SetResliceAxes(resliceCoronal);
	reslice3->SetInterpolationModeToLinear();


	vtkSmartPointer<vtkLookupTable> colorTable =
		vtkSmartPointer<vtkLookupTable>::New();
	colorTable->SetRange(0, 1000);
	colorTable->SetValueRange(0.0, 1.0);
	colorTable->SetSaturationRange(0.0, 0.0);
	colorTable->SetRampToLinear();
	colorTable->Build();

	vtkSmartPointer<vtkImageMapToColors> colorMap1 =
		vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap1->SetLookupTable(colorTable);
	colorMap1->SetInputConnection(reslice1->GetOutputPort());
	colorMap1->Update();

	imgActor1 = vtkSmartPointer<vtkImageActor>::New();
	imgActor1->SetInputData(colorMap1->GetOutput());

	vtkSmartPointer<vtkImageMapToColors> colorMap2 =
		vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap2->SetLookupTable(colorTable);
	colorMap2->SetInputConnection(reslice2->GetOutputPort());
	colorMap2->Update();

	imgActor2 = vtkSmartPointer<vtkImageActor>::New();
	imgActor2->SetInputData(colorMap2->GetOutput());

	vtkSmartPointer<vtkImageMapToColors> colorMap3 =
		vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap3->SetLookupTable(colorTable);
	colorMap3->SetInputConnection(reslice3->GetOutputPort());
	colorMap3->Update();

	imgActor3 = vtkSmartPointer<vtkImageActor>::New();
	imgActor3->SetInputData(colorMap3->GetOutput());

	m_2DViewWidget[0].m_renderer->AddActor(imgActor1);
	m_2DViewWidget[1].m_renderer->AddActor(imgActor2);
	m_2DViewWidget[2].m_renderer->AddActor(imgActor3);
	m_2DViewWidget[0].renderWindow()->AddRenderer(m_2DViewWidget[0].m_renderer);
	m_2DViewWidget[1].renderWindow()->AddRenderer(m_2DViewWidget[1].m_renderer);
	m_2DViewWidget[2].renderWindow()->AddRenderer(m_2DViewWidget[2].m_renderer);
	m_2DViewWidget[0].renderWindow()->Render();
	m_2DViewWidget[1].renderWindow()->Render();
	m_2DViewWidget[2].renderWindow()->Render();

	renderWindowInteractor1 = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor2 = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor3 = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	imagestyle1 = vtkSmartPointer<vtkInteractorStyleImage>::New();
	imagestyle2 = vtkSmartPointer<vtkInteractorStyleImage>::New();
	imagestyle3 = vtkSmartPointer<vtkInteractorStyleImage>::New();

	renderWindowInteractor1->SetInteractorStyle(imagestyle1);
	renderWindowInteractor1->SetRenderWindow(m_2DViewWidget[0].renderWindow());
	renderWindowInteractor1->Initialize();
	renderWindowInteractor2->SetInteractorStyle(imagestyle2);
	renderWindowInteractor2->SetRenderWindow(m_2DViewWidget[1].renderWindow());
	renderWindowInteractor2->Initialize();
	renderWindowInteractor3->SetInteractorStyle(imagestyle3);
	renderWindowInteractor3->SetRenderWindow(m_2DViewWidget[2].renderWindow());
	renderWindowInteractor3->Initialize();

	vtkSmartPointer<vtkImageInteractionCallback> callback1 =
		vtkSmartPointer<vtkImageInteractionCallback>::New();
	callback1->SetImageReslice(reslice1);
	callback1->SetInteractor(renderWindowInteractor1);
	callback1->SetImageMapToColors(colorMap1);
	vtkSmartPointer<vtkImageInteractionCallback> callback2 =
		vtkSmartPointer<vtkImageInteractionCallback>::New();
	callback2->SetImageReslice(reslice2);
	callback2->SetInteractor(renderWindowInteractor2);
	callback2->SetImageMapToColors(colorMap2);
	vtkSmartPointer<vtkImageInteractionCallback> callback3 =
		vtkSmartPointer<vtkImageInteractionCallback>::New();
	callback3->SetImageReslice(reslice3);
	callback3->SetInteractor(renderWindowInteractor3);
	callback3->SetImageMapToColors(colorMap3);

	//imagestyle1->AddObserver(vtkCommand::MouseMoveEvent, callback1);
	//imagestyle1->AddObserver(vtkCommand::LeftButtonPressEvent, callback1);
	//imagestyle1->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback1);
	//imagestyle2->AddObserver(vtkCommand::MouseMoveEvent, callback2);
	//imagestyle2->AddObserver(vtkCommand::LeftButtonPressEvent, callback2);
	//imagestyle2->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback2);
	//imagestyle3->AddObserver(vtkCommand::MouseMoveEvent, callback3);
	//imagestyle3->AddObserver(vtkCommand::LeftButtonPressEvent, callback3);
	//imagestyle3->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback3);
	imagestyle1->AddObserver(vtkCommand::MouseWheelForwardEvent, callback1);
	imagestyle1->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback1);
	imagestyle2->AddObserver(vtkCommand::MouseWheelForwardEvent, callback2);
	imagestyle2->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback2);
	imagestyle3->AddObserver(vtkCommand::MouseWheelForwardEvent, callback3);
	imagestyle3->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback3);

	renderWindowInteractor1->Start();
	renderWindowInteractor2->Start();
	renderWindowInteractor3->Start();
}

void PreDivideWidget::on_preBtnCut_clicked()
{
	if (waitThread.isRunning())
	{
		QMessageBox::information(this, u8"提示", u8"导出正在进行中");
		return;
	}
	if (boxWidget1)
	{
		vtkNew<vtkExtractVOI>  extract_voi;
		extract_voi->SetInputData(m_vtkImageFlip->GetOutput());
		//bounds（XMin，XMax, YMin, YMax, ZMin, ZMax）
		auto bounds1 = dynamic_cast<vtkBoxRepresentation*>(boxWidget1->GetRepresentation())->GetBounds();
		auto bounds2 = dynamic_cast<vtkBoxRepresentation*>(boxWidget2->GetRepresentation())->GetBounds();
		auto bounds3 = dynamic_cast<vtkBoxRepresentation*>(boxWidget3->GetRepresentation())->GetBounds();
		double b1[6], b2[6], b3[6];
		memcpy(b1, bounds1, sizeof(double) * 6);
		memcpy(b2, bounds2, sizeof(double) * 6);
		memcpy(b3, bounds3, sizeof(double) * 6);
		double origin1[3], origin2[3], origin3[3];
		reslice1->GetOutput()->GetOrigin(origin1);
		reslice2->GetOutput()->GetOrigin(origin2);
		reslice3->GetOutput()->GetOrigin(origin3);
		
		int extractXMin = (bounds3[0] - origin3[0]) / spacing[0];
		int extractXMax = (bounds3[1] - origin3[0]) / spacing[0];
		int extractYMin = (bounds1[2] - origin1[1]) / spacing[1];
		int extractYMax = (bounds1[3] - origin1[1]) / spacing[1];
		int extractZMin = (bounds2[2] - origin2[1]) / spacing[2];
		int extractZMax = (bounds2[3] - origin2[1]) / spacing[2];
		if (extractXMin < 0)
			extractXMin = 0;
		if (extractYMin < 0)
			extractYMin = 0;
		if (extractZMin < 0)
			extractZMin = 0;
		if (extractXMin > dimensions[0])
			extractXMin = dimensions[0];
		if (extractYMin > dimensions[1])
			extractYMin = dimensions[1];
		if (extractZMin > dimensions[2])
			extractZMin = dimensions[2];

		extract_voi->SetVOI(extractXMin, extractXMax, extractYMin, extractYMax, extractZMin, extractZMax);
		extract_voi->SetSampleRate(1, 1, 1);
		extract_voi->Update();
		QString path = QDir::tempPath();
		path += "/test.nii.gz";
		vtkNew<vtkNIFTIImageWriter> writer;
		writer->SetFileName(path.toStdString().c_str());
		writer->SetInputData(extract_voi->GetOutput());
		writer->Write();
		//调用算法
		//QFile::remove(QDir::currentPath() + "/test.nii.gz");
		QString program = "E:/nnunet_seg/dist/nnunet_seg_qt/nnunet_seg.exe";
		QString arg = "-i " + path + " -o " + QCoreApplication::applicationDirPath() + "/aa.nii.gz"
			+ " -m ct --keep";
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = L"open";
		sei.lpFile = (LPCWSTR)program.unicode();
		sei.lpParameters = (LPCWSTR)arg.unicode();
		sei.nShow = SW_SHOWDEFAULT;
		ShellExecuteEx(&sei);
		waitThread.setHandle(sei.hProcess);
		disconnect(&waitThread, SIGNAL(programClosed()), this, SLOT(programClosedSlot()));
		connect(&waitThread, SIGNAL(programClosed()), this, SLOT(programClosedSlot()));
		waitThread.start();
	}
}

void PreDivideWidget::on_preBtnSelect_clicked()
{
	if (boxWidget1 || !imgActor1)
		return;
	vtkNew<vtkBoxCallback> boxCallback;

	boxWidget1 = vtkSmartPointer<vtkBoxWidget2>::New();
	boxWidget1->SetInteractor(renderWindowInteractor1);
	boxWidget1->GetRepresentation()->SetPlaceFactor(1);
	boxWidget1->GetRepresentation()->PlaceWidget(imgActor1->GetBounds());
	boxWidget1->SetRotationEnabled(false);
	boxWidget1->AddObserver(vtkCommand::InteractionEvent, boxCallback);
	boxWidget1->On();
	boxCallback->box1 = boxWidget1;

	boxWidget2 = vtkSmartPointer<vtkBoxWidget2>::New();
	boxWidget2->SetInteractor(renderWindowInteractor2);
	boxWidget2->GetRepresentation()->SetPlaceFactor(1);
	boxWidget2->GetRepresentation()->PlaceWidget(imgActor2->GetBounds());
	boxWidget2->SetRotationEnabled(false);
	boxWidget2->AddObserver(vtkCommand::InteractionEvent, boxCallback);
	boxWidget2->On();
	boxCallback->box2 = boxWidget2;

	boxWidget3 = vtkSmartPointer<vtkBoxWidget2>::New();
	boxWidget3->SetInteractor(renderWindowInteractor3);
	boxWidget3->GetRepresentation()->SetPlaceFactor(1);
	boxWidget3->GetRepresentation()->PlaceWidget(imgActor3->GetBounds());
	boxWidget3->SetRotationEnabled(false);
	boxWidget3->AddObserver(vtkCommand::InteractionEvent, boxCallback);
	boxWidget3->On();
	boxCallback->box3 = boxWidget3;

	m_2DViewWidget[0].renderWindow()->Render();
	m_2DViewWidget[1].renderWindow()->Render();
	m_2DViewWidget[2].renderWindow()->Render();
	renderWindowInteractor1->Render();
	renderWindowInteractor2->Render();
	renderWindowInteractor3->Render();
}

void PreDivideWidget::on_CutBtnExport_clicked()
{
	vtkNew<vtkImageData> save_data;
	save_data->DeepCopy(empty_data);
	for (int i = 1; i <= m_MaxDataDimension; ++i)
	{
		vtkSmartPointer<vtkImageLogic> maskFilter =
			vtkSmartPointer<vtkImageLogic>::New();
		maskFilter->SetInputData(0, save_data);
		maskFilter->SetInputData(1, imagedata[i]);
		maskFilter->SetOperationToOr();
		maskFilter->Update();
		save_data->DeepCopy(maskFilter->GetOutput());
	}

	QString path;
	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
	path = QCoreApplication::applicationDirPath() + "/test.stl";
	auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(save_data);
	for (int i = 1; i <= m_MaxDataDimension; ++i)
	{
		marchingCubes->SetValue(i, i);
	}
	auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInputConnection(marchingCubes->GetOutputPort());
	stlWriter->SetFileName(path.toStdString().c_str());
	stlWriter->SetInputConnection(triangleFilter->GetOutputPort());
	stlWriter->SetFileTypeToBinary();
	stlWriter->Write();
	QMessageBox::information(this, u8"提示", u8"导出完成");
}

void PreDivideWidget::on_CutBtnRestore_clicked()
{
	ui->m_CutBtnRestore->setEnabled(false);
	for (auto i = m_restoreData.begin(); i != m_restoreData.end(); ++i)
	{
		imagedata[i->first]->DeepCopy(i->second);
	}
	volumeMulti->Update();
	m_3DViewWidget.m_renderer->RemoveVolume(volumeMulti);
	m_3DViewWidget.m_renderer->AddVolume(volumeMulti);
	m_3DViewWidget.renderWindow()->Render();
	m_3DViewWidget.renderWindow()->GetInteractor()->Render();
	for (int k = 0; k < m_removedRow.size(); ++k)
		ui->tableWidget->setRowHidden(m_removedRow[k], false);
}

void PreDivideWidget::showVolumeDataSlot()
{
	QString path = QCoreApplication::applicationDirPath() + "/aa.nii.gz";
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(path.toStdString().c_str());
	reader->Update();

	image_data = reader->GetOutput();
	volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	m_MaxDataDimension = 0;
	empty_data = vtkSmartPointer<vtkImageData>::New();
	empty_data->DeepCopy(image_data);
	vtkIdType count = image_data->GetNumberOfPoints();
	unsigned char* p = (unsigned char*)empty_data->GetScalarPointer();
	for (vtkIdType i = 0; i < count; ++i)
	{
		if (int(*p) > m_MaxDataDimension)
			m_MaxDataDimension = int(*p);
		*p = 0;
		++p;
	}

	volumeProperty.clear();
	color.clear();
	auto colorNames = QColor::colorNames();
	for (int i = 1; i <= m_MaxDataDimension; ++i)
	{
		compositeOpacity[i] = vtkSmartPointer<vtkPiecewiseFunction>::New();
		compositeOpacity[i]->AddPoint(0, 0);
		volumeProperty[i] = vtkSmartPointer<vtkVolumeProperty>::New();
		volumeProperty[i]->SetInterpolationTypeToLinear();
		volumeProperty[i]->ShadeOn();
		volumeProperty[i]->SetAmbient(0.4);
		volumeProperty[i]->SetDiffuse(0.6);
		volumeProperty[i]->SetSpecular(0.2);
		volumeProperty[i]->SetScalarOpacity(compositeOpacity[i]);
		color[i] = vtkSmartPointer<vtkColorTransferFunction>::New();
		compositeOpacity[i]->AddPoint(i, 1);
		m_labelColor[i] = colorNames.at(i);
		color[i]->AddRGBPoint(0, 0, 0, 0);
		color[i]->AddRGBPoint(i, double(m_labelColor[i].red()) / 255.0,
			double(m_labelColor[i].green()) / 255.0, 
			double(m_labelColor[i].blue()) / 255.0);
		color[i]->AddRGBPoint(m_MaxDataDimension + 1, 1, 1, 1);
		volumeProperty[i]->SetColor(color[i]);
		compositeOpacity[i]->AddPoint(m_MaxDataDimension + 1, 0);
	}

	for (int i = 1; i <= m_MaxDataDimension; ++i)
	{
		volume[i] = vtkSmartPointer<vtkVolume>::New();
		volume[i]->SetProperty(volumeProperty[i]);
		imagedata[i] = vtkSmartPointer<vtkImageData>::New();
		imagedata[i]->DeepCopy(empty_data);
		m_pDataPointer[i] = (unsigned char*)imagedata[i]->GetScalarPointer();
	}

	unsigned char data_t = 0;
	auto src_pData = (unsigned char*)image_data->GetScalarPointer();
	for (vtkIdType i = 0; i < count; ++i)
	{
		data_t = src_pData[i];
		if (data_t > 0)
			m_pDataPointer[data_t][i] = data_t;
	}

	volumeMulti = vtkSmartPointer<vtkMultiVolume>::New();
	volumeMulti->SetMapper(volumeMapper);
	for (int i = 1; i <= m_MaxDataDimension; ++i)
	{
		volumeMapper->SetInputDataObject(i - 1, imagedata[i]);
		volumeMulti->SetVolume(volume[i], i - 1);
	}

	m_3DViewWidget.renderWindow()->AddRenderer(m_3DViewWidget.m_renderer);
	iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(m_3DViewWidget.renderWindow());

	m_3DViewWidget.m_renderer->AddVolume(volumeMulti);
	m_3DViewWidget.m_renderer->ResetCamera();

	m_3dViewStyle = vtkSmartPointer<MyvtkInteractorStyle>::New();
	iren->SetInteractorStyle(m_3dViewStyle);
	m_3DViewWidget.m_renderer->ResetCameraClippingRange();

	m_3DViewWidget.renderWindow()->Render();
	m_WorldPointPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
	iren->SetPicker(m_WorldPointPicker);
	m_3DViewWidget.renderWindow()->GetInteractor()->SetPicker(m_WorldPointPicker);
	rebuildList();

	iren->Initialize();
	iren->Start();
}

void PreDivideWidget::eraseBtn_clicked()
{
	if (m_curSelWorkType == toCut)
	{
		m_curSelWorkType = nothingToDo;
		if (m_3dViewStyle)
			m_3dViewStyle->interactive_ = true;
		m_3DViewWidget.m_bDrawRandomArea = false;
		return;
	}
	if (m_3dViewStyle)
		m_3dViewStyle->interactive_ = false;
	m_3DViewWidget.m_bDrawRandomArea = true;
	m_curSelWorkType = toCut;
}

void PreDivideWidget::programClosedSlot()
{
	QMessageBox::information(this, u8"提示", u8"截取完成");
}

void PreDivideWidget::cutDataSlot(QList<QPointF>& l)
{
	saveRestoreData();
	//循环设置0，这样慢
	//QPolygonF p(l.toVector());
	//p.append(p[0]);
	//CutingImagedata(image_data, volume, m_3DViewWidget.m_renderer, p, true);
	if (m_curSelWorkType == toDaub)
	{
		QPolygonF p(l.toVector());
		p.append(p[0]);
		duabArea(p);
	}

	//用mask
	if (m_curSelWorkType == toCut)
	{
		vtkSmartPointer<vtkCoordinate> coor_transfer = vtkSmartPointer<vtkCoordinate>::New();
		coor_transfer->SetCoordinateSystemToDisplay();
		QList<MyPosData> worldPos;
		QList<MyPosData> worldPosIn;
		auto camera = m_3DViewWidget.m_renderer->GetActiveCamera();
		int dis = camera->GetDistance() * 2;
		for (int i = 0; i < l.size(); ++i)
		{
			coor_transfer->SetValue(l[i].x(), m_3DViewWidget.height() - l[i].y(), 0);
			auto world_point = coor_transfer->GetComputedWorldValue(m_3DViewWidget.m_renderer);
			MyPosData t;
			memcpy(t.d, world_point, sizeof(double) * 3);
			worldPos.append(t);

			auto cp = camera->GetPosition();
			double vView[3];
			vView[0] = t.d[0] - cp[0];
			vView[1] = t.d[1] - cp[1];
			vView[2] = t.d[2] - cp[2];
			vtkMath::Normalize(vView);

			MyPosData t2;
			t2.d[0] = t.d[0] + dis * vView[0];
			t2.d[1] = t.d[1] + dis * vView[1];
			t2.d[2] = t.d[2] + dis * vView[2];
			worldPosIn.append(t2);
		}
		vtkSmartPointer<vtkPolyData> geometry = vtkSmartPointer<vtkPolyData>::New();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		for (size_t i = 0; i < worldPos.size(); ++i)
		{
			points->InsertPoint(2 * i, worldPos[i].d);
			points->InsertPoint(2 * i + 1, worldPosIn[i].d);
		}

		vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkCellArray> strips = vtkSmartPointer<vtkCellArray>::New();

		// 前平面
		polys->InsertNextCell(worldPos.size());
		for (int i = 0; i < worldPos.size(); ++i) {
			polys->InsertCellPoint(2 * i);
		}

		// 后平面
		polys->InsertNextCell(worldPos.size());
		for (int i = 0; i < worldPos.size(); ++i) {
			polys->InsertCellPoint(2 * i + 1);
		}

		// 中间柱面
		int sizeVec = worldPos.size() * 2;
		strips->InsertNextCell(sizeVec + 2);
		for (int i = 0; i < sizeVec; i++)
			strips->InsertCellPoint(i);

		strips->InsertCellPoint(0);
		strips->InsertCellPoint(1);
		geometry->SetPoints(points);
		geometry->SetPolys(polys);
		geometry->SetStrips(strips);

		//polygonal data --> image stencil:
		vtkSmartPointer<vtkPolyDataToImageStencil> pdtoImageStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
		pdtoImageStencil->SetInputData(geometry);
		pdtoImageStencil->SetOutputOrigin(image_data->GetOrigin());
		pdtoImageStencil->SetOutputSpacing(image_data->GetSpacing());
		pdtoImageStencil->SetOutputWholeExtent(image_data->GetExtent());
		pdtoImageStencil->Update();

		vtkSmartPointer<vtkImageStencilToImage> imageStencilToImage = vtkSmartPointer<vtkImageStencilToImage>::New();
		imageStencilToImage->SetInputConnection(pdtoImageStencil->GetOutputPort());
		imageStencilToImage->SetOutputScalarType(VTK_UNSIGNED_CHAR);
		imageStencilToImage->SetInsideValue(0);
		imageStencilToImage->SetOutsideValue(m_curOutValue);
		imageStencilToImage->Update();
		vtkSmartPointer<vtkImageMask> maskFilter =
			vtkSmartPointer<vtkImageMask>::New();
		maskFilter->SetMaskedOutputValue(0, 0, 0);
		for (int i = 1; i <= m_MaxDataDimension; ++i)
		{
			if (m_labelVisible[i])
			{
				maskFilter->SetInputData(0, imagedata[i]);
				maskFilter->SetInputData(1, imageStencilToImage->GetOutput());
				maskFilter->Update();
				imagedata[i]->DeepCopy(maskFilter->GetOutput());
			}
		}
	}

	volumeMulti->Update();
	m_3DViewWidget.m_renderer->RemoveVolume(volumeMulti);
	m_3DViewWidget.m_renderer->AddVolume(volumeMulti);
	m_3dViewStyle->interactive_ = true;
	m_3DViewWidget.m_bDrawRandomArea = false;
	m_curSelWorkType = nothingToDo;
	m_3DViewWidget.renderWindow()->Render();
	m_3DViewWidget.renderWindow()->GetInteractor()->Render();
	l.clear();
}

void PreDivideWidget::SlotMenuClicked(QAction * act)
{
	auto sel = ui->tableWidget->selectionModel()->selectedIndexes();
	set<int> sel_index;
	for (int i = 0; i < sel.size(); ++i)
	{
		auto r = sel[i].row();
		sel_index.insert(r);
	}
	if (sel_index.size() == 0)
		return;
	if (act == m_pActionDel)
	{
		m_removedRow.clear();
		saveRestoreData();
		for (auto i = sel_index.begin(); i != sel_index.end(); ++i)
		{
			vtkIdType count = imagedata[*i + 1]->GetNumberOfPoints();
			auto p = (unsigned char*)imagedata[*i + 1]->GetScalarPointer();
			for (vtkIdType t = 0; t < count; ++t)
			{
				if (*p != 0)
					*p = 0;
				++p;
			}
			ui->tableWidget->setRowHidden(*i, true);
			m_removedRow.push_back(*i);
		}
	}
	else if (act == m_pActionMerge)
	{
		if (sel_index.size() <= 1)
			return;
		unsigned char tarValue = *sel_index.begin() + 1;
		auto pSrc = (unsigned char*)imagedata[*sel_index.begin() + 1]->GetScalarPointer();
		m_removedRow.clear();
		saveRestoreData();
		unsigned char zeroValue = 0;
		vector<unsigned char*> pData;
		for (auto i = ++sel_index.begin(); i != sel_index.end(); ++i)
		{
			pData.push_back((unsigned char*)imagedata[*i + 1]->GetScalarPointer());
		}
		for (size_t pIndex = 0; pIndex < image_data->GetNumberOfPoints(); ++pIndex)
		{
			bool b = false;
			for (int i = 0; i < pData.size(); ++i)
			{
				if ((*(pData[i])) != zeroValue)
				{
					b = true;
				}
				++pData[i];
			}
			if (b)
				*pSrc = tarValue;
			++pSrc;
		}
		for (auto i = ++sel_index.begin(); i != sel_index.end(); ++i)
		{
			imagedata[*i + 1]->DeepCopy(empty_data);
			ui->tableWidget->setRowHidden(*i, true);
			m_removedRow.push_back(*i);
		}
		ui->tableWidget->clearSelection();

	}
	else if (act == m_pActionSaveAsLeft || act == m_pActionSaveAsLeft)
	{
		QString path;
		vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
		if (act == m_pActionSaveAsLeft)
			path = QCoreApplication::applicationDirPath() + "/left.stl";
		else
			path = QCoreApplication::applicationDirPath() + "/right.stl";
		auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
		marchingCubes->SetInputData(imagedata[*sel_index.begin() + 1]);
		for (int i = 1; i <= m_MaxDataDimension; ++i)
		{
			marchingCubes->SetValue(i, i);
		}
		auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
		triangleFilter->SetInputConnection(marchingCubes->GetOutputPort());
		stlWriter->SetFileName(path.toStdString().c_str());
		stlWriter->SetInputConnection(triangleFilter->GetOutputPort());
		stlWriter->SetFileTypeToBinary();
		stlWriter->Write();
		QMessageBox::information(this, u8"提示", u8"保存完成");
		return;
	}
	volumeMulti->Update();
	m_3DViewWidget.m_renderer->RemoveVolume(volumeMulti);
	m_3DViewWidget.m_renderer->AddVolume(volumeMulti);
	m_3DViewWidget.renderWindow()->Render();
}

void PreDivideWidget::tableContexMenuRequested(const QPoint & pos)
{
	m_pContextMenu->exec(QCursor::pos());
}

void PreDivideWidget::on_DuabBtn_clicked()
{
	if (ui->m_daubValue->currentText().isEmpty())
		return;
	if (m_curSelWorkType == toDaub)
	{
		m_curSelWorkType = nothingToDo;
		if (m_3dViewStyle)
			m_3dViewStyle->interactive_ = true;
		m_3DViewWidget.m_bDrawRandomArea = false;
		return;
	}
	if (m_3dViewStyle)
		m_3dViewStyle->interactive_ = false;
	m_3DViewWidget.m_bDrawRandomArea = true;
	m_curSelWorkType = toDaub;
}

void PreDivideWidget::on_CheckLabelCountBtn_clicked()
{
	vector<int> shownRow;
	for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
	{
		if (!ui->tableWidget->isRowHidden(i))
		{
			shownRow.push_back(i);
		}
	}
	if (shownRow.size() == 2)
	{
		ui->group1->setEnabled(false);
		ui->group2->setEnabled(false);
		ui->group3->setEnabled(false);
	}
	else
	{
		QMessageBox::information(this, u8"提示", u8"当前mask总数不为2");
	}
}

void PreDivideWidget::saveRestoreData()
{
	for (auto i = imagedata.begin(); i != imagedata.end(); ++i)
	{
		m_restoreData[i->first] = vtkSmartPointer<vtkImageData>::New();
		m_restoreData[i->first]->DeepCopy(i->second);
	}
	if (m_MaxDataDimension > 0)
		ui->m_CutBtnRestore->setEnabled(true);
}

void PreDivideWidget::duabArea(const QPolygonF & polygon)
{
	int img_dims[3];
	double img_spacing[3];
	double img_origian[3];
	image_data->GetDimensions(img_dims);
	image_data->GetSpacing(img_spacing);
	image_data->GetOrigin(img_origian);
	vtkNew<vtkCoordinate> corrdinate;
	corrdinate->SetCoordinateSystemToWorld();
	unsigned char value = (unsigned char)ui->m_daubValue->currentText().toInt();
	for (int k = 0; k < img_dims[2]; ++k) {
		for (int i = 0; i < img_dims[0]; ++i) {
			for (int j = 0; j < img_dims[1]; ++j) {
				double word_pos[3];
				word_pos[0] = i * img_spacing[0] + img_origian[0];
				word_pos[1] = j * img_spacing[1] + img_origian[1];
				word_pos[2] = k * img_spacing[2] + img_origian[2];
				corrdinate->SetValue(word_pos);
				double *display_pos = corrdinate->GetComputedDoubleDisplayValue(m_3DViewWidget.m_renderer);
				QPointF q_display_pos(display_pos[0], m_3DViewWidget.height() - display_pos[1]);
				if (polygon.containsPoint(q_display_pos, Qt::OddEvenFill)) {
					for (size_t index = 1; index <= m_MaxDataDimension; ++index)
					{
						if (!m_labelVisible[index])
							continue;
						auto pPixel = static_cast<unsigned char *>(imagedata[index]->GetScalarPointer(i, j, k));
						if (*pPixel != 0)
							*pPixel = value;
					}
				}
			}
		}
	}
}

void PreDivideWidget::rebuildList()
{
	int size = m_bt.size();
	for (int i = max(1, size); i <= m_MaxDataDimension; ++i)
	{
		if (m_bt.find(i) == m_bt.end())
		{
			QPushButton* pb1 = new QPushButton;
			QPushButton* pb2 = new QPushButton;
			m_bt[i].push_back(pb1);
			m_bt[i].push_back(pb2);
		}
		else
		{
			m_bt[i][0]->setText(u8"显示");
		}
	}
	ui->tableWidget->clear();
	ui->tableWidget->setRowCount(m_MaxDataDimension);
	ui->tableWidget->setColumnCount(3);
	ui->m_daubValue->clear();
	for (int i = 1; i <= m_MaxDataDimension; ++i) {
		if (i < 4)
			ui->tableWidget->setColumnWidth(i - 1, 60);
		ui->tableWidget->setRowHeight(i - 1, 50);
		m_labelVisible[i] = true;
		for (int t = 0; t < 3; ++t) {
			if (t == 0)
			{
				QLabel *label = new QLabel();
				QWidget* w = new QWidget(this);
				QHBoxLayout* hl = new QHBoxLayout(w);
				hl->setContentsMargins(0, 0, 0, 0);
				w->setLayout(hl);
				hl->addWidget(label);
				ui->tableWidget->setCellWidget(i - 1, t, w);
				label->setText(QString(u8"label ") + QString::number(i));
				label->setStyleSheet("color:rgb(255, 255, 255)");
				if (i == 0)
					break;
				continue;
			}
			QWidget* w = new QWidget(this);
			QHBoxLayout* hl = new QHBoxLayout(w);
			hl->setContentsMargins(0, 0, 0, 0);
			w->setLayout(hl);
			hl->addWidget(m_bt[i][t - 1]);
			ui->tableWidget->setCellWidget(i - 1, t, w);
			connect(m_bt[i][t - 1], SIGNAL(clicked()), this, SLOT(tableBtnClicked()));
			m_bt[i][t - 1]->setMaximumWidth(50);
			m_bt[i][t - 1]->setStyleSheet("background-color:rgb(110, 111, 102)");
			if(t == 1)
				m_bt[i][t - 1]->setText(u8"显示");
			else if (t == 2)
			{
				m_bt[i][t - 1]->setText(u8"颜色");
				m_bt[i][t - 1]->setStyleSheet(QString("background-color:rgb(%1, %2, %3)")
					.arg(QString::number(m_labelColor[i].red()), 
						QString::number(m_labelColor[i].green()),
						QString::number(m_labelColor[i].blue())));
			}
			listUserData *d = new listUserData;
			d->btnId = t;
			d->labelId = i;
			m_bt[i][t - 1]->setUserData(0, d);
		}
		ui->m_daubValue->addItem(QString::number(i));
	}
	ui->m_daubValue->setCurrentIndex(0);
}

QWidget* PreDivideWidget::get2d3dView()
{
    return ui->m_2d3dView;
}

QWidget* PreDivideWidget::getCtrlView()
{
    return ui->widget_2;
}

void PreDivideWidget::stopInteractor()
{
	if (iren)
	{
		iren->TerminateApp();
		iren->Disable();
	}
	if (renderWindowInteractor1)
	{
		renderWindowInteractor1->TerminateApp();
		renderWindowInteractor2->TerminateApp();
		renderWindowInteractor3->TerminateApp();
		renderWindowInteractor1->Disable();
		renderWindowInteractor2->Disable();
		renderWindowInteractor3->Disable();
	}
}

void PreDivideWidget::CutingImagedata(vtkSmartPointer<vtkImageData> image_data,
	vtkSmartPointer<vtkVolume> volume,
	vtkSmartPointer<vtkRenderer> renderer,
	const QPolygonF &polygon, bool cutInside)
{
	int img_dims[3];
	double img_spacing[3];
	double img_origian[3];
	image_data->GetDimensions(img_dims);
	image_data->GetSpacing(img_spacing);
	image_data->GetOrigin(img_origian);
	vtkNew<vtkCoordinate> corrdinate;
	corrdinate->SetCoordinateSystemToWorld();
	// 不用vtkImageIterator，迭代器没办法获得空间位置
	// 只是个demo，直接认为vtkImageData 数据是 unsigned char 保存的。如果是其他记得要改。
	// 只是个demo，直接认为bround是从0，0，0开始的，实际工程记得校验。
	for (int k = 0; k < img_dims[2]; ++k) {
		for (int i = 0; i < img_dims[0]; ++i) {
			for (int j = 0; j < img_dims[1]; ++j) {
				double word_pos[3];
				word_pos[0] = i * img_spacing[0] + img_origian[0];
				word_pos[1] = j * img_spacing[1] + img_origian[1];
				word_pos[2] = k * img_spacing[2] + img_origian[2];
				corrdinate->SetValue(word_pos);
				double *display_pos = corrdinate->GetComputedDoubleDisplayValue(renderer);
				QPointF q_display_pos(display_pos[0], m_3DViewWidget.height() - display_pos[1]);
				if (cutInside
					&& polygon.containsPoint(q_display_pos, Qt::OddEvenFill)) {
					auto pPixel = static_cast<unsigned char *>(image_data->GetScalarPointer(i, j, k));
					*pPixel = 0;
				}
				else if (!cutInside
					&& !polygon.containsPoint(q_display_pos.toPoint(), Qt::OddEvenFill)) {
					auto pPixel = static_cast<unsigned char *>(image_data->GetScalarPointer(i, j, k));
					*pPixel = 0;
				}
			}
		}
	}
	volume->Update();
	renderer->RemoveVolume(volume);
	renderer->AddVolume(volume);
}


threadWaitAIexit::threadWaitAIexit()
{
}

threadWaitAIexit::~threadWaitAIexit()
{
}

void threadWaitAIexit::setHandle(HANDLE h)
{
	m_handle = h;
}

void threadWaitAIexit::run()
{
	WaitForSingleObject(m_handle, INFINITE);
	CloseHandle(m_handle);
	emit programClosed();

}

void QVTKRenderWindow::paintGL()
{
	QVTKOpenGLNativeWidget::paintGL();
	if (m_bDrawRandomArea) {
		QPainter painter(this);
		DrawArea(cutting_points_, painter);
	}
}

void QVTKRenderWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && m_bDrawRandomArea) {
		cutting_points_.clear();
	}
	QVTKOpenGLNativeWidget::mousePressEvent(event);
}

void QVTKRenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && m_bDrawRandomArea) {
		if (cutting_points_.size() > 1)
			emit cutDataSignal(cutting_points_);
		update();
	}
	QVTKOpenGLNativeWidget::mouseReleaseEvent(event);
}

void QVTKRenderWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (m_bDrawRandomArea && event->buttons() & Qt::LeftButton) {
		cutting_points_.append(event->pos());
		update();
	}
	QVTKOpenGLNativeWidget::mouseMoveEvent(event);
}

void QVTKRenderWindow::DrawArea(QList<QPointF> &pf, QPainter &painter)
{
	if (pf.size() < 1) {
		return;
	}

	QPainterPath path(pf[0]);
	for (int i = 1; i < pf.size(); ++i) {
		path.lineTo(pf[i]);
	}

	QPen pen;
	pen.setColor(Qt::green);
	painter.setPen(pen);
	painter.setBrush(QBrush(Qt::green, Qt::Dense4Pattern));
	painter.drawPath(path);
	painter.drawLine(pf[0], pf.last());
}
