#include "predividewidget.h"
#include "ui_predividewidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>

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
    ui->tableWidget->setRowCount(3);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    for (int i=0;i<3;++i) {
        ui->tableWidget->setColumnWidth(i,60);
        ui->tableWidget->setRowHeight(i,50);
    }
    for (int i = 0;i<9;++i) {
        QWidget* w = new QWidget(this);
        QHBoxLayout* hl = new QHBoxLayout(w);
        hl->setContentsMargins(0,0,0,0);
        w->setLayout(hl);
        hl->addWidget(&m_bt[i]);
        ui->tableWidget->setCellWidget(i/3,i%3,w);
        connect(&m_bt[i],SIGNAL(clicked()),this,SLOT(tableBtnClicked()));
        m_bt[i].setMaximumWidth(50);
        m_bt[i].setStyleSheet("background-color:rgb(110, 111, 102)");
    }
    m_bt[0].setText(u8"左侧");
    m_bt[1].setText(u8"显示");
    m_bt[2].setText(u8"颜色");
    m_bt[3].setText(u8"右侧");
    m_bt[4].setText(u8"显示");
    m_bt[5].setText(u8"颜色");
    m_bt[6].setText(u8"骶髂");
    m_bt[7].setText(u8"显示");
    m_bt[8].setText(u8"颜色");
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
	connect(&m_3DViewWidget, SIGNAL(cutDataSignal(QList<QPointF> &)), this, SLOT(cutDataSlot(QList<QPointF> &)));
}

PreDivideWidget::~PreDivideWidget()
{
    delete ui;
	stopInteractor();
}

void PreDivideWidget::tableBtnClicked()
{
    QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
    for (int i = 0;i<9;++i) {
        if(btn == &m_bt[i])
        {
            if(i % 3 == 1)
            {
                if(m_bt[i].text() == u8"显示")
                {
                    m_bt[i].setText(u8"隐藏");
                }
                else {
                    m_bt[i].setText(u8"显示");
                }
                switch (i/3) {
                case 0:
                    if(ui->wind1->isVisible())
                        ui->wind1->hide();
                    else
                        ui->wind1->show();
                break;
                case 1:
                    if(ui->wind2->isVisible())
                        ui->wind2->hide();
                    else
                        ui->wind2->show();
                break;
                case 2:
                    if(ui->wind3->isVisible())
                        ui->wind3->hide();
                    else
                        ui->wind3->show();
                break;
                default:
                    break;
                }
            }
			else if (i % 3 == 2)
			{
				//if (!imgActor1)
				//	return;
				QColor color = QColorDialog::getColor(Qt::white, this);
				if (color.isValid())
				{
					switch (i / 3) {
					case 0:
						imgActor1->GetProperty()->SetBackingColor((double)color.red()/255.0,
							(double)color.green() / 255.0, (double)color.blue() / 255.0);
						//m_2DViewWidget[0].m_renderer->SetBackground((double)color.red() / 255.0,
						//	(double)color.green() / 255.0, (double)color.blue() / 255.0);
						m_2DViewWidget[0].renderWindow()->Render();
						break;
					case 1:
						imgActor2->GetProperty()->SetBackingColor((double)color.red() / 255.0,
							(double)color.green() / 255.0, (double)color.blue() / 255.0);
						//m_2DViewWidget[1].m_renderer->SetBackground((double)color.red() / 255.0,
						//	(double)color.green() / 255.0, (double)color.blue() / 255.0);
						m_2DViewWidget[1].renderWindow()->Render();
						break;
					case 2:
						imgActor3->GetProperty()->SetBackingColor((double)color.red() / 255.0,
							(double)color.green() / 255.0, (double)color.blue() / 255.0);
						//m_2DViewWidget[2].m_renderer->SetBackground((double)color.red() / 255.0,
						//	(double)color.green() / 255.0, (double)color.blue() / 255.0);
						m_2DViewWidget[2].renderWindow()->Render();
						break;
					default:
						break;
					}
				}
			}
            break;
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
	//if (boxWidget1)
	{
		//vtkNew<vtkExtractVOI>  extract_voi;
		//extract_voi->SetInputData(m_vtkImageFlip->GetOutput());
		////bounds（XMin，XMax, YMin, YMax, ZMin, ZMax）
		//auto bounds1 = dynamic_cast<vtkBoxRepresentation*>(boxWidget1->GetRepresentation())->GetBounds();
		//auto bounds2 = dynamic_cast<vtkBoxRepresentation*>(boxWidget2->GetRepresentation())->GetBounds();
		//auto bounds3 = dynamic_cast<vtkBoxRepresentation*>(boxWidget3->GetRepresentation())->GetBounds();
		//double b1[6], b2[6], b3[6];
		//memcpy(b1, bounds1, sizeof(double) * 6);
		//memcpy(b2, bounds2, sizeof(double) * 6);
		//memcpy(b3, bounds3, sizeof(double) * 6);
		//double origin1[3], origin2[3], origin3[3];
		//reslice1->GetOutput()->GetOrigin(origin1);
		//reslice2->GetOutput()->GetOrigin(origin2);
		//reslice3->GetOutput()->GetOrigin(origin3);
		//
		//int extractXMin = (bounds3[0] - origin3[0]) / spacing[0];
		//int extractXMax = (bounds3[1] - origin3[0]) / spacing[0];
		//int extractYMin = (bounds1[2] - origin1[1]) / spacing[1];
		//int extractYMax = (bounds1[3] - origin1[1]) / spacing[1];
		//int extractZMin = (bounds2[2] - origin2[1]) / spacing[2];
		//int extractZMax = (bounds2[3] - origin2[1]) / spacing[2];
		//if (extractXMin < 0)
		//	extractXMin = 0;
		//if (extractYMin < 0)
		//	extractYMin = 0;
		//if (extractZMin < 0)
		//	extractZMin = 0;
		//if (extractXMin > dimensions[0])
		//	extractXMin = dimensions[0];
		//if (extractYMin > dimensions[1])
		//	extractYMin = dimensions[1];
		//if (extractZMin > dimensions[2])
		//	extractZMin = dimensions[2];

		//extract_voi->SetVOI(extractXMin, extractXMax, extractYMin, extractYMax, extractZMin, extractZMax);
		//extract_voi->SetSampleRate(1, 1, 1);
		//extract_voi->Update();
		QString path = QDir::tempPath();
		path += "/test.nii.gz";
		//vtkNew<vtkNIFTIImageWriter> writer;
		//writer->SetFileName(path.toStdString().c_str());
		//writer->SetInputData(extract_voi->GetOutput());
		//writer->Write();
		//调用算法
		//QFile::remove(QDir::currentPath() + "/test.nii.gz");
		QString program = "E:/nnunet_seg/dist/nnunet_seg_qt/nnunet_seg.exe";
		QString arg = "-i " + path + " -o " + QDir::currentPath() + "/111.nii.gz"
			+ " -m ct --keep";
		QStringList arguments;
		arguments << arg;
		//myProcess.setArguments(arguments);
		//myProcess.start("cmd.exe");
		//myProcess.write(arg.toUtf8());
		//if(myProcess.isOpen())
		//	myProcess.waitForFinished();
		//1
		//ShellExecute(NULL, L"open", (LPCWSTR)program.unicode(), (LPCWSTR)arg.unicode(), NULL, SW_SHOWNORMAL);
		//QMessageBox::information(this, u8"提示", u8"截取完成");
		//2
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = L"open";
		sei.lpFile = (LPCWSTR)program.unicode();
		sei.lpParameters = (LPCWSTR)arg.unicode();
		sei.nShow = SW_SHOWDEFAULT;
		ShellExecuteEx(&sei);
		t.setHandle(sei.hProcess);
		connect(&t, SIGNAL(programClosed()), this, SLOT(programClosedSlot()));
		t.start();
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
	QString path = QDir::tempPath();
	path += "/test.nii.gz";
	vtkNew<vtkNIFTIImageReader> reader;
	reader->SetFileName(path.toStdString().c_str());
	reader->Update();
	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
	path = QDir::tempPath() + "/test.stl";
	auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(reader->GetOutput());
	marchingCubes->SetValue(0, 300);
	auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInputConnection(marchingCubes->GetOutputPort());
	stlWriter->SetFileName(path.toStdString().c_str());
	stlWriter->SetInputConnection(triangleFilter->GetOutputPort());
	//stlWriter->Update();
	stlWriter->Write();
	QMessageBox::information(this, u8"提示", u8"导出完成");
}

void PreDivideWidget::on_CutBtnRestore_clicked()
{

}

void PreDivideWidget::showVolumeDataSlot()
{
	//vtkNew<vtkMultiVolume> multyVolume;
	//QString path = "E:/testdata/test.nii.gz";
	//vtkNew<vtkNIFTIImageReader> reader2;
	//reader2->SetFileName(path.toStdString().c_str());
	//reader2->Update();
	//auto imageData2 = reader2->GetOutput();
	//imageData2->SetOrigin(200,200,200);
	//vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper2 = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	//volumeMapper2->SetInputData(imageData2);
	//vtkSmartPointer<vtkVolumeProperty> volumeProperty2 = vtkSmartPointer<vtkVolumeProperty>::New();
	//volumeProperty2->SetInterpolationTypeToLinear();
	//volumeProperty2->ShadeOn();
	//volumeProperty2->SetAmbient(0.4);
	//volumeProperty2->SetDiffuse(0.6);
	//volumeProperty2->SetSpecular(0.2);
	//vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity2 = vtkSmartPointer<vtkPiecewiseFunction>::New();
	//compositeOpacity2->AddPoint(70, 0.00);
	//compositeOpacity2->AddPoint(90, 0.40);
	//compositeOpacity2->AddPoint(180, 0.60);
	//volumeProperty2->SetScalarOpacity(compositeOpacity2);
	//vtkSmartPointer<vtkColorTransferFunction> color2 = vtkSmartPointer<vtkColorTransferFunction>::New();
	//color2->AddRGBPoint(0.000, 0.00, 0.00, 0.00);
	//color2->AddRGBPoint(64.00, 1.00, 0.52, 0.30);
	//color2->AddRGBPoint(190.0, 1.00, 1.00, 1.00);
	//color2->AddRGBPoint(220.0, 1.00, 1.00, 1.00);
	//volumeProperty2->SetColor(color2);
	//vtkSmartPointer<vtkVolume> volume2 = vtkSmartPointer<vtkVolume>::New();
	//volume2->SetMapper(volumeMapper2);
	//volume2->SetProperty(volumeProperty2);

	QString path = "E:/testdata/aa.nii.gz";
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(path.toStdString().c_str());
	reader->Update();

	image_data = reader->GetOutput();
	//vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputData(image_data);

	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.4);
	volumeProperty->SetDiffuse(0.6);
	volumeProperty->SetSpecular(0.2);

	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity->AddPoint(1, 0.00);
	compositeOpacity->AddPoint(2, 0.40);
	compositeOpacity->AddPoint(3, 0.60);
	compositeOpacity->AddPoint(4, 1);
	volumeProperty->SetScalarOpacity(compositeOpacity);

	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
	color->AddRGBPoint(1.000, 1.00, 1.00, 1.00);
	color->AddRGBPoint(2.00, 1.00, 1.00, 1.00);
	color->AddRGBPoint(3.0, 1.00, 1.00, 1.00);
	color->AddRGBPoint(4.0, 1.00, 1.00, 1.00);
	volumeProperty->SetColor(color);

	volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	m_3DViewWidget.renderWindow()->AddRenderer(m_3DViewWidget.m_renderer);
	iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(m_3DViewWidget.renderWindow());

	//vtkSmartPointer<vtkOutlineFilter> outlineData = vtkSmartPointer<vtkOutlineFilter>::New();//线框
	//outlineData->SetInputConnection(reader->GetOutputPort());
	//vtkSmartPointer<vtkPolyDataMapper> mapOutline = vtkSmartPointer<vtkPolyDataMapper>::New();//该类用于渲染多边形几何数据,将输入的数据转换为几何图元(点/线/多边形)进行渲染
	//mapOutline->SetInputConnection(outlineData->GetOutputPort());
	//vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
	//outline->SetMapper(mapOutline);
	//outline->GetProperty()->SetColor(0, 0, 0);/

	//vtkNew<vtkGPUVolumeRayCastMapper> mapper;
	//mapper->SetUseJittering(0);
	//mapper->SetInputConnection(0, reader->GetOutputPort());
	//mapper->SetInputConnection(1, reader2->GetOutputPort());

	//multyVolume->SetMapper(mapper);
	//multyVolume->SetVolume(volume,0);
	//multyVolume->SetVolume(volume2,1);

	m_3DViewWidget.m_renderer->AddVolume(volume);
	//m_3DViewWidget.m_renderer->AddVolume(multyVolume);
	//m_3DViewWidget.m_renderer->AddActor(outline);
	m_3DViewWidget.m_renderer->ResetCamera();

	m_3dViewStyle = vtkSmartPointer<MyvtkInteractorStyle>::New();
	iren->SetInteractorStyle(m_3dViewStyle);
	m_3DViewWidget.m_renderer->ResetCameraClippingRange();

	m_3DViewWidget.renderWindow()->Render();
	m_WorldPointPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
	iren->SetPicker(m_WorldPointPicker);
	m_3DViewWidget.renderWindow()->GetInteractor()->SetPicker(m_WorldPointPicker);

	iren->Initialize();
	iren->Start();
}

void PreDivideWidget::eraseBtn_clicked()
{
	m_3dViewStyle->interactive_ = false;
	m_3DViewWidget.m_bDrawRandomArea = true;
}

void PreDivideWidget::programClosedSlot()
{
}

void PreDivideWidget::cutDataSlot(QList<QPointF>& l)
{
	//循环设置0，这样慢
	//QPolygonF p(l.toVector());
	//p.append(p[0]);
	//CutingImagedata(image_data, volume, m_3DViewWidget.m_renderer, p, true);


	//用mask
	vtkSmartPointer<vtkCoordinate> coor_transfer =  vtkSmartPointer<vtkCoordinate>::New();
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
	imageStencilToImage->SetOutsideValue(255);
	imageStencilToImage->Update();
	vtkSmartPointer<vtkImageMask> maskFilter =
		vtkSmartPointer<vtkImageMask>::New();
	maskFilter->SetInputData(0, image_data);
	maskFilter->SetInputData(1, imageStencilToImage->GetOutput());
	maskFilter->SetMaskedOutputValue(0, 0, 0);
	maskFilter->Update();
	image_data->DeepCopy(maskFilter->GetOutput());
	volume->Update();

	m_3DViewWidget.m_renderer->RemoveVolume(volume);
	m_3DViewWidget.m_renderer->AddVolume(volume);

	m_3dViewStyle->interactive_ = true;
	m_3DViewWidget.m_bDrawRandomArea = false;
	m_3DViewWidget.renderWindow()->Render();
	m_3DViewWidget.renderWindow()->GetInteractor()->Render();
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
	if (m_bDrawRandomArea && !cutting_points_.isEmpty()) {
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
		if (cutting_points_.length() > 1)
			emit cutDataSignal(cutting_points_);
	}
	QVTKOpenGLNativeWidget::mouseReleaseEvent(event);
}

void QVTKRenderWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (m_bDrawRandomArea && event->buttons() & Qt::LeftButton) {
		cutting_points_.append(event->pos());
		//paintGL();
	}
	QVTKOpenGLNativeWidget::mouseMoveEvent(event);
}

void QVTKRenderWindow::DrawArea(QList<QPointF> &pf, QPainter &painter)
{
	if (pf.length() < 1) {
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
