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
	m_pAddMenu = new QMenu(u8"添加", m_pContextMenu);
	m_pActionAddLabel1n = new QAction(u8"骶骨");
	m_pActionAddLabel2n = new QAction(u8"左髂骨");
	m_pActionAddLabel3n = new QAction(u8"右髂骨");
	m_pAddMenu->addAction(m_pActionAddLabel1n);
	m_pAddMenu->addAction(m_pActionAddLabel2n);
	m_pAddMenu->addAction(m_pActionAddLabel3n);
	m_pContextMenu->addMenu(m_pAddMenu);
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
	if (isLoadDicom)
		return;
	QString filePath = QFileDialog::getExistingDirectory(this, u8"请选择DICOM文件路径");
	if (filePath.isEmpty())
		return;
	
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
	typedef itk::ImageToVTKImageFilter<ImageType>       ConnectorType;
	ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(reader->GetOutput());
	connector->Update();
	vtkSmartPointer<vtkImageFlip> flip = vtkSmartPointer<vtkImageFlip>::New();
	flip->SetInputData(connector->GetOutput());
	flip->SetFilteredAxes(1);
	flip->Update();

	if (!dicom_data)
		dicom_data = vtkSmartPointer<vtkImageData>::New();
	dicom_data->DeepCopy(flip->GetOutput());
	dicom_data->GetExtent(extent);
	dicom_data->GetSpacing(spacing);
	dicom_data->GetOrigin(origin);
	dicom_data->GetDimensions(dimensions);
	
	double center[3];
	center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
	center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
	ImageType::DirectionType direction = reader->GetOutput()->GetDirection();
	m_direction[0] = direction[0][0];
	m_direction[1] = direction[1][1];
	m_direction[2] = direction[2][2];
	double axialElements[16] = {
		1 * m_direction[0], 0, 0, 0,
		0, 1 * m_direction[1], 0, 0,
		0, 0, -1 * m_direction[2], 0,
		0, 0, 0, 1
	};
	double coronalElements[16] = {
		1 * m_direction[1], 0, 0, 0,
		0, 0, -1 * m_direction[1], 0,
		0,1 * m_direction[2], 0, 0,
		0, 0, 0, 1 
	};
	double sagittalElements[16] = {
		0, 0,1 * m_direction[1], 0,
		-1 * m_direction[1], 0, 0, 0,
		0,1 * m_direction[2], 0, 0,
		0, 0, 0, 1 
	};

	vtkSmartPointer<vtkMatrix4x4> resliceAxes =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceAxes->DeepCopy(axialElements);
	resliceAxes->SetElement(0, 3, center[0]);
	resliceAxes->SetElement(1, 3, center[1]);
	resliceAxes->SetElement(2, 3, center[2]);

	vtkSmartPointer<vtkMatrix4x4> resliceSagittal =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceSagittal->DeepCopy(sagittalElements);
	resliceSagittal->SetElement(0, 3, center[0]);
	resliceSagittal->SetElement(1, 3, center[1]);
	resliceSagittal->SetElement(2, 3, center[2]);

	vtkSmartPointer<vtkMatrix4x4> resliceCoronal =
		vtkSmartPointer<vtkMatrix4x4>::New();
	resliceCoronal->DeepCopy(coronalElements);
	resliceCoronal->SetElement(0, 3, center[0]);
	resliceCoronal->SetElement(1, 3, center[1]);
	resliceCoronal->SetElement(2, 3, center[2]);
	
	
	reslice1 = vtkSmartPointer<vtkImageReslice>::New();
	reslice1->SetInputData(dicom_data);
	reslice1->SetOutputDimensionality(2);
	reslice1->SetResliceAxes(resliceAxes);
	reslice1->SetInterpolationModeToLinear();

	reslice2 = vtkSmartPointer<vtkImageReslice>::New();
	reslice2->SetInputData(dicom_data);
	reslice2->SetOutputDimensionality(2);
	reslice2->SetResliceAxes(resliceCoronal);
	reslice2->SetInterpolationModeToLinear();

	reslice3 = vtkSmartPointer<vtkImageReslice>::New();
	reslice3->SetInputData(dicom_data);
	reslice3->SetOutputDimensionality(2);
	reslice3->SetResliceAxes(resliceSagittal);
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

	m_2DViewWidget[0].m_renderer->RemoveAllViewProps();
	m_2DViewWidget[1].m_renderer->RemoveAllViewProps();
	m_2DViewWidget[2].m_renderer->RemoveAllViewProps();
	m_2DViewWidget[0].m_renderer->AddActor(imgActor1);
	m_2DViewWidget[1].m_renderer->AddActor(imgActor2);
	m_2DViewWidget[2].m_renderer->AddActor(imgActor3);
	m_2DViewWidget[0].renderWindow()->RemoveRenderer(m_2DViewWidget[0].m_renderer);
	m_2DViewWidget[1].renderWindow()->RemoveRenderer(m_2DViewWidget[1].m_renderer);
	m_2DViewWidget[2].renderWindow()->RemoveRenderer(m_2DViewWidget[2].m_renderer);
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

	imagestyle1->AddObserver(vtkCommand::MouseWheelForwardEvent, callback1);
	imagestyle1->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback1);
	imagestyle2->AddObserver(vtkCommand::MouseWheelForwardEvent, callback2);
	imagestyle2->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback2);
	imagestyle3->AddObserver(vtkCommand::MouseWheelForwardEvent, callback3);
	imagestyle3->AddObserver(vtkCommand::MouseWheelBackwardEvent, callback3);
	isLoadDicom = true;
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
		//bounds（XMin，XMax, YMin, YMax, ZMin, ZMax）
		auto bounds1 = dynamic_cast<vtkBoxRepresentation*>(boxWidget1->GetRepresentation())->GetBounds();
		//auto bounds2 = dynamic_cast<vtkBoxRepresentation*>(boxWidget2->GetRepresentation())->GetBounds();
		//auto bounds3 = dynamic_cast<vtkBoxRepresentation*>(boxWidget3->GetRepresentation())->GetBounds();
		auto bounds2 = dynamic_cast<vtkBoxRepresentation*>(boxWidget3->GetRepresentation())->GetBounds();
		auto bounds3 = dynamic_cast<vtkBoxRepresentation*>(boxWidget2->GetRepresentation())->GetBounds();
		double b1[6], b2[6], b3[6];
		memcpy(b1, bounds1, sizeof(double) * 6);
		memcpy(b2, bounds2, sizeof(double) * 6);
		memcpy(b3, bounds3, sizeof(double) * 6);
		double origin1[3], origin2[3], origin3[3];
		reslice1->GetOutput()->GetOrigin(origin1);
		//reslice2->GetOutput()->GetOrigin(origin2);
		//reslice3->GetOutput()->GetOrigin(origin3);
		reslice2->GetOutput()->GetOrigin(origin3);
		reslice3->GetOutput()->GetOrigin(origin2);
		
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
		if (extractXMax < 0)
			extractXMax = 0;
		if (extractYMax < 0)
			extractYMax = 0;
		if (extractZMax < 0)
			extractZMax = 0;
		if (extractXMax > dimensions[0])
			extractXMax = dimensions[0];
		if (extractYMax > dimensions[1])
			extractYMax = dimensions[1];
		if (extractZMax > dimensions[2])
			extractZMax = dimensions[2];

		QString path;
		path = QCoreApplication::applicationDirPath() + "/output";
		QDir dir(path);
		if (!dir.exists())
		{
			dir.mkdir(path);
		}
		path += "/cropped.nii.gz";
		ImageType::SizeType lowerSize = { extractXMin, extractYMin, extractZMin };
		ImageType::SizeType upperSize = { dimensions[0] - extractXMax, 
			dimensions[1] - extractYMax, dimensions[2] - extractZMax };
		if (m_direction[0] < 0)
		{
			lowerSize[0] = dimensions[0] - extractXMax;
			upperSize[0] = extractXMin;
		}
		if (m_direction[1] > 0)
		{
			lowerSize[1] = dimensions[1] - extractYMax;
			upperSize[1] = extractYMin;
		}
		if (m_direction[2] < 0)
		{
			lowerSize[2] = dimensions[2] - extractZMax;
			upperSize[2] = extractZMin;
		}
		cropFilter->SetLowerBoundaryCropSize(lowerSize);
		cropFilter->SetUpperBoundaryCropSize(upperSize);
		cropFilter->SetInput(reader->GetOutput());
		try
		{
			cropFilter->Update();
		}
		catch (const itk::ExceptionObject& err)
		{
			string str = err.what();
			std::cerr << str << std::endl;
			return;
		}
		using WriterType = itk::ImageFileWriter<itk::Image<PixelType, 3>>;
		itk::NiftiImageIO::Pointer niiIO = itk::NiftiImageIO::New();
		WriterType::Pointer writer = WriterType::New();
		writer->SetInput(cropFilter->GetOutput());
		writer->SetFileName(path.toLocal8Bit().toStdString());
		writer->SetImageIO(niiIO);
		try
		{
			writer->Update();
		}
		catch (const itk::ExceptionObject& err)
		{
			string str = err.what();
			std::cerr << str << std::endl;
			return;
		}

		//调用算法
		QString program = "E:/nnunet_seg/dist/nnunet_seg_qt/nnunet_pelfrac.exe";
		QString arg = "-i " + path + " -o " + QCoreApplication::applicationDirPath() + "/aa.nii.gz";
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
	memcpy(boxCallback->m_direction, m_direction, sizeof(double) * 3);

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
	boxCallback->box3 = boxWidget2;

	boxWidget3 = vtkSmartPointer<vtkBoxWidget2>::New();
	boxWidget3->SetInteractor(renderWindowInteractor3);
	boxWidget3->GetRepresentation()->SetPlaceFactor(1);
	boxWidget3->GetRepresentation()->PlaceWidget(imgActor3->GetBounds());
	boxWidget3->SetRotationEnabled(false);
	boxWidget3->AddObserver(vtkCommand::InteractionEvent, boxCallback);
	boxWidget3->On();
	boxCallback->box2 = boxWidget3;

	m_2DViewWidget[0].renderWindow()->Render();
	m_2DViewWidget[1].renderWindow()->Render();
	m_2DViewWidget[2].renderWindow()->Render();
	renderWindowInteractor1->Render();
	renderWindowInteractor2->Render();
	renderWindowInteractor3->Render();
}

void PreDivideWidget::on_CutBtnExport_clicked()
{
	/*
	vtkNew<vtkImageData> save_data;
	save_data->DeepCopy(empty_data);
	for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
	{
		vtkSmartPointer<vtkImageLogic> maskFilter =
			vtkSmartPointer<vtkImageLogic>::New();
		maskFilter->SetInputData(0, save_data);
		maskFilter->SetInputData(1, imagedata[idNumber]);
		maskFilter->SetOperationToOr();
		maskFilter->Update();
		save_data->DeepCopy(maskFilter->GetOutput());
	}

	QString path;
	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
	path = QCoreApplication::applicationDirPath() + "/test.stl";
	auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(save_data);
	marchingCubes->SetValue(0, 1);
	auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	smoother->SetInputConnection(marchingCubes->GetOutputPort());
	smoother->SetNumberOfIterations(20);
	smoother->NonManifoldSmoothingOn();
	smoother->NormalizeCoordinatesOff();
	smoother->GenerateErrorScalarsOn();
	smoother->Update();
	stlWriter->SetFileName(path.toStdString().c_str());
	stlWriter->SetInputConnection(smoother->GetOutputPort());
	stlWriter->SetFileTypeToBinary();
	stlWriter->Write();
	*/
	using ImageIoType = itk::NiftiImageIO;
	using ReaderType = itk::ImageFileReader<ImageType>;
	ReaderType::Pointer ItkReader = ReaderType::New();
	ImageIoType::Pointer niftiIO = ImageIoType::New();
	ItkReader->SetImageIO(niftiIO);
	QString ai_runedpath = QCoreApplication::applicationDirPath() + "/aa.nii.gz";
	ItkReader->SetFileName(ai_runedpath.toLocal8Bit().toStdString());
	ItkReader->Update();
	ImageType::Pointer image = ItkReader->GetOutput();
	auto md = image->GetDirection();
	auto mo = image->GetOrigin();
	auto ms = image->GetSpacing();
	//itk图像坐标系转XYZ坐标系
	vtkNew<vtkMatrix4x4> matrix;
	matrix->Identity();
	//vtk图像坐标系转XYZ坐标系
	vtkNew<vtkMatrix4x4> matrix2;
	matrix2->Identity();
	vtkNew<vtkMatrix4x4> mXYZcorrect;
	for (int i = 0; i < 3; ++i)
	{
		for (int t = 0; t < 3; ++t)
		{
			matrix->SetElement(i, t, md[i][t] * ms[t]);
			matrix2->SetElement(i, t, abs(md[i][t]) * image_data->GetSpacing()[t]);
		}
		matrix->SetElement(i, 3, mo[i]);
		matrix2->SetElement(i, 3, image_data->GetOrigin()[i]);
	}
	matrix->Modified();
	matrix2->Modified();
	matrix2->Invert();
	//vtk模型转换到itk模型，矩阵
	vtkMatrix4x4::Multiply4x4(matrix, matrix2, mXYZcorrect);
	matrix->Modified();
	matrix2->Modified();
	vtkNew<vtkTransform> transform;
	transform->SetMatrix(mXYZcorrect);
	transform->Update();

	for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
	{
		QString path;
		path = QCoreApplication::applicationDirPath() + "/output";
		QDir dir(path);
		if (!dir.exists())
		{
			dir.mkdir(path);
		}
		path += "/" + QString::number(*i) + ".stl";
		vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
		auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
		marchingCubes->SetInputData(imagedata[*i]);
		marchingCubes->SetValue(0, 1);
		auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
		smoother->SetInputConnection(marchingCubes->GetOutputPort());
		smoother->SetNumberOfIterations(10);
		smoother->NonManifoldSmoothingOn();
		smoother->NormalizeCoordinatesOff();
		smoother->GenerateErrorScalarsOn();
		smoother->Update();
		vtkNew<vtkTransformPolyDataFilter> tranFilter;
		tranFilter->SetInputConnection(smoother->GetOutputPort());
		tranFilter->SetTransform(transform);
		tranFilter->Update();
		stlWriter->SetFileName(path.toStdString().c_str());
		stlWriter->SetInputConnection(tranFilter->GetOutputPort());
		stlWriter->SetFileTypeToBinary();
		stlWriter->Write();
	}
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
	for (auto i = m_removedComboList.begin(); i != m_removedComboList.end(); ++i)
	{
		ui->m_daubValue->addItem(QString::number(*i));
	}
}

void PreDivideWidget::showVolumeDataSlot()
{
	if (isShowNii)
		return;
	m_restoreData.clear();
	m_removedRow.clear();
	QString path = QCoreApplication::applicationDirPath() + "/aa.nii.gz";
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(path.toStdString().c_str());
	reader->Update();

	if (!image_data)
		image_data = vtkSmartPointer<vtkImageData>::New();
	image_data->DeepCopy(reader->GetOutput());
	volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	m_MaxDataDimension.clear();
	m_iMaxDataDimension = 0;
	empty_data = vtkSmartPointer<vtkImageData>::New();
	empty_data->DeepCopy(image_data);
	vtkIdType count = image_data->GetNumberOfPoints();
	unsigned char* p = (unsigned char*)empty_data->GetScalarPointer();
	for (vtkIdType i = 0; i < count; ++i)
	{
		if (int(*p) != 0)
			m_MaxDataDimension.insert(int(*p));
		if (int(*p) > m_iMaxDataDimension)
			m_iMaxDataDimension = int(*p);
		*p = 0;
		++p;
	}

	volumeProperty.clear();
	color.clear();
	volumeMulti = vtkSmartPointer<vtkMultiVolume>::New();
	volumeMulti->SetMapper(volumeMapper);
	for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
	{
		addEmptyVolume(*i);
	}

	unsigned char data_t = 0;
	auto src_pData = (unsigned char*)image_data->GetScalarPointer();
	for (vtkIdType i = 0; i < count; ++i)
	{
		data_t = src_pData[i];
		if (data_t > 0)
			m_pDataPointer[data_t][i] = data_t;
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
	isShowNii = true;
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

	if (m_curSelWorkType == toDaub)
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
		for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
		{
			if (m_labelVisible[*i])
			{
				maskFilter->SetInputData(0, imagedata[*i]);
				maskFilter->SetInputData(1, imageStencilToImage->GetOutput());
				maskFilter->Update();
				imagedata[*i]->DeepCopy(maskFilter->GetOutput());
			}
		}
		vtkIdType count = image_data->GetNumberOfPoints();
		unsigned char* p_c = (unsigned char*)imagedata[ui->m_daubValue->currentText().toInt()]->GetScalarPointer();
		unsigned char value = (unsigned char)ui->m_daubValue->currentText().toInt();
		for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
		{
			if (m_labelVisible[*i])
			{
				unsigned char* p = (unsigned char*)m_restoreData[*i]->GetScalarPointer();
				unsigned char* p2 = (unsigned char*)imagedata[*i]->GetScalarPointer();
				for (vtkIdType t = 0; t < count; ++t)
				{
					if (int(p[t]) != int(p2[t]))
						p_c[t] = value;
				}
			}
		}
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
		for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
		{
			if (m_labelVisible[*i])
			{
				maskFilter->SetInputData(0, imagedata[*i]);
				maskFilter->SetInputData(1, imageStencilToImage->GetOutput());
				maskFilter->Update();
				imagedata[*i]->DeepCopy(maskFilter->GetOutput());
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
	if (act == m_pActionAddLabel1n ||
		act == m_pActionAddLabel2n ||
		act == m_pActionAddLabel3n)
	{
		if (act == m_pActionAddLabel1n)
		{
			int t_id = 1, t_max = 10;
			for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
			{
				if (*i > t_id && *i < t_max)
				{
					t_id = *i;
				}
			}
			if (m_MaxDataDimension.find(t_id) != m_MaxDataDimension.end())
			{
				++t_id;
			}
			if (m_MaxDataDimension.find(t_id) == m_MaxDataDimension.end()
				&& t_id < t_max)
			{
				m_MaxDataDimension.insert(t_id);
				addEmptyVolume(t_id);
				addOneRowToMaskList(t_id);
			}
		}
		else if (act == m_pActionAddLabel2n)
		{
			int t_id = 11, t_max = 20;
			for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
			{
				if (*i > t_id && *i < t_max)
				{
					t_id = *i;
				}
			}
			if (m_MaxDataDimension.find(t_id) != m_MaxDataDimension.end())
			{
				++t_id;
				if (m_MaxDataDimension.find(t_id) == m_MaxDataDimension.end()
					&& t_id < t_max)
				{
					m_MaxDataDimension.insert(t_id);
					addEmptyVolume(t_id);
					addOneRowToMaskList(t_id);
				}
			}
		}
		else if (act == m_pActionAddLabel3n)
		{
			int t_id = 21, t_max = 30;
			for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
			{
				if (*i > t_id && *i < t_max)
				{
					t_id = *i;
				}
			}
			if (m_MaxDataDimension.find(t_id) != m_MaxDataDimension.end())
			{
				++t_id;
				if (m_MaxDataDimension.find(t_id) == m_MaxDataDimension.end()
					&& t_id < t_max)
				{
					m_MaxDataDimension.insert(t_id);
					addEmptyVolume(t_id);
					addOneRowToMaskList(t_id);
				}
			}
		}
		return;
	}
	auto sel = ui->tableWidget->selectionModel()->selectedIndexes();
	set<int> sel_index;
	set<int> sel_row;
	QString str("btnShow");
	for (int i = 0; i < sel.size(); ++i)
	{
		auto r = sel[i].row();
		sel_row.insert(r);
		auto w = ui->tableWidget->cellWidget(r, 1);
		auto c = w->findChild<QPushButton*>(str);
		if (c)
		{
			listUserData *d = dynamic_cast<listUserData*>(c->userData(0));
			sel_index.insert(d->labelId);
		}
	}
	if (act == m_pActionDel)
	{
		saveRestoreData();
		auto r = sel_row.begin();
		for (auto i = sel_index.begin(); i != sel_index.end(); ++i,++r)
		{
			imagedata[*i]->DeepCopy(empty_data);
			ui->tableWidget->setRowHidden(*r, true);
			m_removedRow.push_back(*r);
			m_MaxDataDimension.erase(*i);
			m_removedComboList.push_back(*i);
		}
		ui->m_daubValue->clear();
		for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
		{
			ui->m_daubValue->addItem(QString::number(*i));
		}
		ui->m_daubValue->setCurrentIndex(0);
	}
	else if (act == m_pActionMerge)
	{
		if (sel_index.size() <= 1)
			return;
		unsigned char tarValue = *sel_index.begin();
		auto pSrc = (unsigned char*)imagedata[*sel_index.begin()]->GetScalarPointer();
		saveRestoreData();
		unsigned char zeroValue = 0;
		vector<unsigned char*> pData;
		for (auto i = ++sel_index.begin(); i != sel_index.end(); ++i)
		{
			pData.push_back((unsigned char*)imagedata[*i]->GetScalarPointer());
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
		auto r = ++sel_row.begin();
		for (auto i = ++sel_index.begin(); i != sel_index.end(); ++i,++r)
		{
			imagedata[*i]->DeepCopy(empty_data);
			ui->tableWidget->setRowHidden(*r, true);
			m_removedRow.push_back(*r);
			m_MaxDataDimension.erase(*i);
			m_removedComboList.push_back(*i);
		}
		ui->m_daubValue->clear();
		for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
		{
			ui->m_daubValue->addItem(QString::number(*i));
		}
		ui->m_daubValue->setCurrentIndex(0);
		ui->tableWidget->clearSelection();

	}
	else if (act == m_pActionSaveAsLeft || act == m_pActionSaveAsRight)
	{
		QString path;
		path = QCoreApplication::applicationDirPath() + "/output";
		QDir dir(path);
		if (!dir.exists())
		{
			dir.mkdir(path);
		}
		vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
		if (act == m_pActionSaveAsLeft)
			path += "/left.stl";
		else
			path += "/right.stl";
		auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
		marchingCubes->SetInputData(imagedata[*sel_index.begin()]);
		marchingCubes->SetValue(0, 1);
		auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
		smoother->SetInputConnection(marchingCubes->GetOutputPort());
		smoother->SetNumberOfIterations(10);
		smoother->NonManifoldSmoothingOn();
		smoother->NormalizeCoordinatesOff();
		smoother->GenerateErrorScalarsOn();
		smoother->Update();
		using ImageIoType = itk::NiftiImageIO;
		using ReaderType = itk::ImageFileReader<ImageType>;
		ReaderType::Pointer ItkReader = ReaderType::New();
		ImageIoType::Pointer niftiIO = ImageIoType::New();
		ItkReader->SetImageIO(niftiIO);
		QString ai_runedpath = QCoreApplication::applicationDirPath() + "/aa.nii.gz";
		ItkReader->SetFileName(ai_runedpath.toLocal8Bit().toStdString());
		ItkReader->Update();
		ImageType::Pointer image = ItkReader->GetOutput();
		auto md = image->GetDirection();
		auto mo = image->GetOrigin();
		auto ms = image->GetSpacing();
		//itk图像坐标系转XYZ坐标系
		vtkNew<vtkMatrix4x4> matrix;
		matrix->Identity();
		//vtk图像坐标系转XYZ坐标系
		vtkNew<vtkMatrix4x4> matrix2;
		matrix2->Identity();
		vtkNew<vtkMatrix4x4> mXYZcorrect;
		for (int i = 0; i < 3; ++i)
		{
			for (int t = 0; t < 3; ++t)
			{
				matrix->SetElement(i, t, md[i][t] * ms[t]);
				matrix2->SetElement(i, t, abs(md[i][t]) * image_data->GetSpacing()[t]);
			}
			matrix->SetElement(i, 3, mo[i]);
			matrix2->SetElement(i, 3, image_data->GetOrigin()[i]);
		}
		matrix->Modified();
		matrix2->Modified();
		matrix2->Invert();
		//vtk模型转换到itk模型，矩阵
		vtkMatrix4x4::Multiply4x4(matrix, matrix2, mXYZcorrect);
		matrix->Modified();
		matrix2->Modified();
		vtkNew<vtkTransform> transform;
		transform->SetMatrix(mXYZcorrect);
		transform->Update();
		vtkNew<vtkTransformPolyDataFilter> tranFilter;
		tranFilter->SetInputConnection(smoother->GetOutputPort());
		tranFilter->SetTransform(transform);
		tranFilter->Update();
		stlWriter->SetFileName(path.toStdString().c_str());
		stlWriter->SetInputConnection(tranFilter->GetOutputPort());
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
	if (m_iMaxDataDimension > 0)
		ui->m_CutBtnRestore->setEnabled(true);
	m_removedRow.clear();
	m_removedComboList.clear();
}

void PreDivideWidget::rebuildList()
{
	ui->tableWidget->clear();
	ui->m_daubValue->clear();
	ui->tableWidget->setColumnCount(3);
	for (auto i = m_MaxDataDimension.begin(); i != m_MaxDataDimension.end(); ++i)
	{
		addOneRowToMaskList(*i);
	}
	ui->m_daubValue->setCurrentIndex(0);
}

void PreDivideWidget::addEmptyVolume(int idNumber)
{
	auto colorNames = QColor::colorNames();
	compositeOpacity[idNumber] = vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity[idNumber]->AddPoint(0, 0);
	volumeProperty[idNumber] = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty[idNumber]->SetInterpolationTypeToLinear();
	volumeProperty[idNumber]->ShadeOn();
	volumeProperty[idNumber]->SetAmbient(0.4);
	volumeProperty[idNumber]->SetDiffuse(0.6);
	volumeProperty[idNumber]->SetSpecular(0.2);
	volumeProperty[idNumber]->SetScalarOpacity(compositeOpacity[idNumber]);
	color[idNumber] = vtkSmartPointer<vtkColorTransferFunction>::New();
	compositeOpacity[idNumber]->AddPoint(idNumber, 1);
	if (idNumber > (colorNames.size() - 1))
		m_labelColor[idNumber] = colorNames.at(idNumber % (colorNames.size() - 1));
	else
		m_labelColor[idNumber] = colorNames.at(idNumber);
	color[idNumber]->AddRGBPoint(0, 0, 0, 0);
	color[idNumber]->AddRGBPoint(idNumber, double(m_labelColor[idNumber].red()) / 255.0,
		double(m_labelColor[idNumber].green()) / 255.0,
		double(m_labelColor[idNumber].blue()) / 255.0);
	if (m_iMaxDataDimension < idNumber)
		m_iMaxDataDimension = idNumber;
	color[idNumber]->AddRGBPoint(m_iMaxDataDimension + 1, 1, 1, 1);
	volumeProperty[idNumber]->SetColor(color[idNumber]);
	compositeOpacity[idNumber]->AddPoint(m_iMaxDataDimension + 1, 0);
	volume[idNumber] = vtkSmartPointer<vtkVolume>::New();
	volume[idNumber]->SetProperty(volumeProperty[idNumber]);
	imagedata[idNumber] = vtkSmartPointer<vtkImageData>::New();
	imagedata[idNumber]->DeepCopy(empty_data);
	m_pDataPointer[idNumber] = (unsigned char*)imagedata[idNumber]->GetScalarPointer();
	static int index_t = 0;
	volumeMapper->SetInputDataObject(index_t, imagedata[idNumber]);
	volumeMulti->SetVolume(volume[idNumber], index_t);
	++index_t;
}

void PreDivideWidget::addOneRowToMaskList(int idNumber)
{
	if (m_bt.find(idNumber) == m_bt.end())
	{
		QPushButton* pb1 = new QPushButton;
		QPushButton* pb2 = new QPushButton;
		pb1->setObjectName(QString("btnShow"));
		m_bt[idNumber].push_back(pb1);
		m_bt[idNumber].push_back(pb2);
	}
	else
	{
		m_bt[idNumber][0]->setText(u8"显示");
	}
	m_labelVisible[idNumber] = true;
	ui->tableWidget->setRowCount(m_MaxDataDimension.size());
	static int index_t = 0;
	if (index_t < 4)
		ui->tableWidget->setColumnWidth(index_t - 1, 60);
	ui->tableWidget->setRowHeight(index_t, 50);
	m_labelVisible[idNumber] = true;
	for (int t = 0; t < 3; ++t) {
		if (t == 0)
		{
			QLabel *label = new QLabel();
			QWidget* w = new QWidget(this);
			QHBoxLayout* hl = new QHBoxLayout(w);
			hl->setContentsMargins(0, 0, 0, 0);
			w->setLayout(hl);
			hl->addWidget(label);
			ui->tableWidget->setCellWidget(index_t, t, w);
			label->setText(QString(u8"label ") + QString::number(idNumber));
			label->setStyleSheet("color:rgb(255, 255, 255)");
			continue;
		}
		QWidget* w = new QWidget(this);
		QHBoxLayout* hl = new QHBoxLayout(w);
		hl->setContentsMargins(0, 0, 0, 0);
		w->setLayout(hl);
		hl->addWidget(m_bt[idNumber][t - 1]);
		ui->tableWidget->setCellWidget(index_t, t, w);
		connect(m_bt[idNumber][t - 1], SIGNAL(clicked()), this, SLOT(tableBtnClicked()));
		m_bt[idNumber][t - 1]->setMaximumWidth(50);
		m_bt[idNumber][t - 1]->setStyleSheet("background-color:rgb(110, 111, 102)");
		if (t == 1)
			m_bt[idNumber][t - 1]->setText(u8"显示");
		else if (t == 2)
		{
			m_bt[idNumber][t - 1]->setText(u8"颜色");
			m_bt[idNumber][t - 1]->setStyleSheet(QString("background-color:rgb(%1, %2, %3)")
				.arg(QString::number(m_labelColor[idNumber].red()),
					QString::number(m_labelColor[idNumber].green()),
					QString::number(m_labelColor[idNumber].blue())));
		}
		listUserData *d = new listUserData;
		d->btnId = t;
		d->labelId = idNumber;
		m_bt[idNumber][t - 1]->setUserData(0, d);
		m_bt[idNumber][t - 1]->setParent(w);
	}
	ui->m_daubValue->addItem(QString::number(idNumber));
	++index_t;
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

bool PreDivideWidget::DeleteDirectory(const QString &path)
{
	if (path.isEmpty())
	{
		return false;
	}

	QDir dir(path);
	if (!dir.exists())
	{
		return true;
	}

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	foreach(QFileInfo fi, fileList)
	{
		if (fi.isFile())
		{
			fi.dir().remove(fi.fileName());
		}
		else
		{
			DeleteDirectory(fi.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath());
}