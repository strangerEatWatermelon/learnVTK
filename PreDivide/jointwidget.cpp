#include "jointwidget.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QTextStream>

std::function<void()> PointPickerInteractorStyle::m_func = nullptr;
jointWidget::jointWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.m_3dView->layout()->addWidget(&m_3DViewWidget);
	connect(ui.m_selSTL1, SIGNAL(pressed()), this, SLOT(readSTLAndShowActor()));
	connect(ui.m_selSTL2, SIGNAL(pressed()), this, SLOT(readSTLAndShowActor()));
	connect(ui.m_selSTL3, SIGNAL(pressed()), this, SLOT(readSTLAndShowActor()));
	connect(ui.m_selSTL4, SIGNAL(pressed()), this, SLOT(readSTLAndShowActor()));
	connect(ui.m_selSTL5, SIGNAL(pressed()), this, SLOT(readSTLAndShowActor()));
	connect(ui.m_color1, SIGNAL(pressed()), this, SLOT(changeActorColor()));
	connect(ui.m_color2, SIGNAL(pressed()), this, SLOT(changeActorColor()));
	connect(ui.m_color3, SIGNAL(pressed()), this, SLOT(changeActorColor()));
	connect(ui.m_color4, SIGNAL(pressed()), this, SLOT(changeActorColor()));
	connect(ui.m_color5, SIGNAL(pressed()), this, SLOT(changeActorColor()));
	connect(ui.m_showActor1, SIGNAL(pressed()), this, SLOT(ShowHideActor()));
	connect(ui.m_showActor2, SIGNAL(pressed()), this, SLOT(ShowHideActor()));
	connect(ui.m_showActor3, SIGNAL(pressed()), this, SLOT(ShowHideActor()));
	connect(ui.m_showActor4, SIGNAL(pressed()), this, SLOT(ShowHideActor()));
	connect(ui.m_showActor5, SIGNAL(pressed()), this, SLOT(ShowHideActor()));
	connect(ui.m_readPos, SIGNAL(pressed()), this, SLOT(readPostion()));
	connect(ui.m_sphere1, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_sphere2, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_sphere3, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_sphere4, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_sphere5, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_sphere6, SIGNAL(pressed()), this, SLOT(selectSphere()));
	connect(ui.m_saveCameraPos, SIGNAL(pressed()), this, SLOT(SaveCameraPosition()));
	connect(ui.m_recoverCameraPos, SIGNAL(pressed()), this, SLOT(RecoverCameraPosition()));
	connect(ui.m_cut, SIGNAL(pressed()), this, SLOT(cutWithPlane()));
	connect(ui.m_isTranslation, SIGNAL(pressed()), this, SLOT(ctrlTranslatin()));
	connect(ui.m_up, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_down, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_left, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_right, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_front, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_back, SIGNAL(pressed()), this, SLOT(moveActor()));
	connect(ui.m_showPlane1, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_showPlane2, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_deletePlane1, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_deletePlane2, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_rotatePlane2_L, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_rotatePlane2_R, SIGNAL(pressed()), this, SLOT(processPlaneCommand()));
	connect(ui.m_move3rdPointLeft, SIGNAL(pressed()), this, SLOT(movePoint3()));
	connect(ui.m_move3rdPointRight, SIGNAL(pressed()), this, SLOT(movePoint3()));
	connect(ui.m_showCuttedActor1, SIGNAL(pressed()), this, SLOT(processCuttedActorCommand()));
	connect(ui.m_showCuttedActor2, SIGNAL(pressed()), this, SLOT(processCuttedActorCommand()));
}

jointWidget::~jointWidget()
{}

void jointWidget::readSTLAndShowActor()
{
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	QString filePath = QFileDialog::getOpenFileName(this, u8"请选择STL文件路径",QString(),"STL (*.stl);;*.*");
	if (filePath.isEmpty())
		return;
	vtkNew<vtkSTLReader> reader;
	reader->SetFileName(filePath.toStdString().c_str());
	reader->Update();
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());
	init3DWindow();
	if (btn == ui.m_selSTL1)
	{
		if (!m_atcor1)
			m_atcor1 = vtkSmartPointer<vtkActor>::New();
		m_polydata1 = vtkSmartPointer<vtkPolyData>::New();
		m_polydata1 = reader->GetOutput();
		m_atcor1->SetMapper(mapper);
		m_atcor1->GetProperty()->SetOpacity(0.3);//透明度
		if (m_atcor1color.isValid())
			m_atcor1->GetProperty()->SetColor((double)m_atcor1color.red() / 255.0,
			(double)m_atcor1color.green() / 255.0, (double)m_atcor1color.blue() / 255.0);
		m_3DViewWidget.m_renderer->AddActor(m_atcor1);
	}
	else if(btn == ui.m_selSTL2)
	{
		if (!m_atcor2)
			m_atcor2 = vtkSmartPointer<vtkActor>::New();
		m_polydata2 = vtkSmartPointer<vtkPolyData>::New();
		m_polydata2 = reader->GetOutput();
		m_atcor2->SetMapper(mapper);
		m_atcor2->GetProperty()->SetOpacity(0.3);//透明度
		if (m_atcor2color.isValid())
			m_atcor2->GetProperty()->SetColor((double)m_atcor2color.red() / 255.0,
			(double)m_atcor2color.green() / 255.0, (double)m_atcor2color.blue() / 255.0);
		m_3DViewWidget.m_renderer->AddActor(m_atcor2);
	}
	else if(btn == ui.m_selSTL3)
	{
		if (!m_atcor3)
			m_atcor3 = vtkSmartPointer<vtkActor>::New();
		m_polydata3 = vtkSmartPointer<vtkPolyData>::New();
		m_polydata3 = reader->GetOutput();
		m_atcor3->SetMapper(mapper);
		m_atcor3->GetProperty()->SetOpacity(0.3);//透明度
		if (m_atcor3color.isValid())
			m_atcor3->GetProperty()->SetColor((double)m_atcor3color.red() / 255.0,
			(double)m_atcor3color.green() / 255.0, (double)m_atcor3color.blue() / 255.0);
		m_3DViewWidget.m_renderer->AddActor(m_atcor3);
	}
	else if(btn == ui.m_selSTL4)
	{
		if (!m_atcor4)
			m_atcor4 = vtkSmartPointer<vtkActor>::New();
		m_polydata4 = vtkSmartPointer<vtkPolyData>::New();
		m_polydata4 = reader->GetOutput();
		m_atcor4->SetMapper(mapper);
		m_atcor4->GetProperty()->SetOpacity(0.3);//透明度
		if (m_atcor4color.isValid())
			m_atcor4->GetProperty()->SetColor((double)m_atcor4color.red() / 255.0,
			(double)m_atcor4color.green() / 255.0, (double)m_atcor4color.blue() / 255.0);
		m_3DViewWidget.m_renderer->AddActor(m_atcor4);
	}
	else if(btn == ui.m_selSTL5)
	{
		if (!m_atcor5)
			m_atcor5 = vtkSmartPointer<vtkActor>::New();
		m_polydata5 = vtkSmartPointer<vtkPolyData>::New();
		m_polydata5 = reader->GetOutput();
		m_atcor5->SetMapper(mapper);
		m_atcor5->GetProperty()->SetOpacity(0.3);//透明度
		if (m_atcor5color.isValid())
			m_atcor5->GetProperty()->SetColor((double)m_atcor5color.red() / 255.0,
			(double)m_atcor5color.green() / 255.0, (double)m_atcor5color.blue() / 255.0);
		m_3DViewWidget.m_renderer->AddActor(m_atcor5);
	}
	useMoveActorStyle();
	m_3DViewWidget.renderWindow()->Render();
	//计算画面的面积
	vtkSmartPointer<vtkAppendPolyData> appendFilter =
		vtkSmartPointer<vtkAppendPolyData>::New();
	if (m_atcor1)
	{
		appendFilter->AddInputData(m_polydata1);
	}
	if (m_atcor2)
	{
		appendFilter->AddInputData(m_polydata2);
	}
	if (m_atcor3)
	{
		appendFilter->AddInputData(m_polydata3);
	}
	if (m_atcor4)
	{
		appendFilter->AddInputData(m_polydata4);
	}
	if (m_atcor5)
	{
		appendFilter->AddInputData(m_polydata5);
	}
	appendFilter->Update();
	vtkNew<vtkTriangleFilter> triF;
	triF->SetInputData(appendFilter->GetOutput());
	triF->Update();
	vtkNew<vtkMassProperties> polygonProperties;
	polygonProperties->SetInputData(triF->GetOutput());
	polygonProperties->Update();
	m_planeArea = polygonProperties->GetSurfaceArea();
}

void jointWidget::changeActorColor()
{
	QColor color = QColorDialog::getColor(Qt::white, this);
	if (!color.isValid())
		return;
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	QString style = "background-color:rgb("
		+ QString::number(color.red()) + ", "
		+ QString::number(color.green()) + ", "
		+ QString::number(color.blue()) + ")";
	if (btn == ui.m_color1)
	{
		ui.m_selSTL1->setStyleSheet(style);
		if (m_atcor1)
			m_atcor1->GetProperty()->SetColor((double)color.red()/255.0,
			(double)color.green() / 255.0, (double)color.blue() / 255.0);
		m_atcor1color = color;
	}
	else if (btn == ui.m_color2)
	{
		ui.m_selSTL2->setStyleSheet(style);
		if (m_atcor2)
			m_atcor2->GetProperty()->SetColor((double)color.red() / 255.0,
			(double)color.green() / 255.0, (double)color.blue() / 255.0);
		m_atcor2color = color;
	}
	else if (btn == ui.m_color3)
	{
		ui.m_selSTL3->setStyleSheet(style);
		if (m_atcor3)
			m_atcor3->GetProperty()->SetColor((double)color.red() / 255.0,
			(double)color.green() / 255.0, (double)color.blue() / 255.0);
		m_atcor3color = color;
	}
	else if (btn == ui.m_color4)
	{
		ui.m_selSTL4->setStyleSheet(style);
		if (m_atcor4)
			m_atcor4->GetProperty()->SetColor((double)color.red() / 255.0,
			(double)color.green() / 255.0, (double)color.blue() / 255.0);
		m_atcor4color = color;
	}
	else if (btn == ui.m_color5)
	{
		ui.m_selSTL5->setStyleSheet(style);
		if (m_atcor5)
			m_atcor5->GetProperty()->SetColor((double)color.red() / 255.0,
			(double)color.green() / 255.0, (double)color.blue() / 255.0);
		m_atcor5color = color;
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::ShowHideActor()
{
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	bool bShow = true;
	if (btn->text() == u8"隐藏")
	{
		bShow = false;
		btn->setText(u8"显示");
	}
	else
	{
		btn->setText(u8"隐藏");
	}
	if (btn == ui.m_showActor1)
	{
		if (m_atcor1)
		{
			m_atcor1->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showActor2)
	{
		if (m_atcor2)
		{
			m_atcor2->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showActor3)
	{
		if (m_atcor3)
		{
			m_atcor3->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showActor4)
	{
		if (m_atcor4)
		{
			m_atcor4->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showActor5)
	{
		if (m_atcor5)
		{
			m_atcor5->SetVisibility(bShow);
		}
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::readPostion()
{
	QString filePath = QFileDialog::getExistingDirectory(this, u8"请选择文件路径");
	if (filePath.isEmpty())
		return;
	QDir dir(filePath);
	bool findTxt = false;
	int actorCount = 0;
	foreach(QFileInfo mfi, dir.entryInfoList())
	{
		if (mfi.isFile())
		{
			if (mfi.suffix().toLower() == "txt")
			{
				if (!findTxt)
				{
					QFile file(mfi.filePath());
					if (!file.open(QFile::ReadOnly))
						return;
					QTextStream stream(&file);
					int lineNum = 0;
					while (!stream.atEnd())
					{
						QString line = stream.readLine().trimmed();
						QStringList strs = line.split(",", QString::SkipEmptyParts);
						if (strs.size() > 2)
						{
							m_posData[lineNum][0] = strs.at(0).toDouble();
							m_posData[lineNum][1] = strs.at(1).toDouble();
							m_posData[lineNum][2] = strs.at(2).toDouble();
						}
						else
						{
							strs = line.split(" ", QString::SkipEmptyParts);
							if (strs.size() > 2)
							{
								m_posData[lineNum][0] = strs.at(0).toDouble();
								m_posData[lineNum][1] = strs.at(1).toDouble();
								m_posData[lineNum][2] = strs.at(2).toDouble();
							}
						}
						++lineNum;
						if (lineNum > 6)
							break;
					}
					file.close();
					if (!sphereSource1)
					{
						sphereSource1 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource2 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource3 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource4 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource5 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource6 = vtkSmartPointer<vtkSphereSource>::New();
						sphereSource7 = vtkSmartPointer<vtkSphereSource>::New();
					}
					init3DWindow();
					sphereSource1->SetCenter(m_posData[0]);//设置球的中心
					sphereSource1->SetRadius(5);//设置球的半径
					sphereSource1->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper1->SetInputConnection(sphereSource1->GetOutputPort());
					m_sphereActor1 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor1->SetMapper(mapper1);
					m_sphereActor1->GetProperty()->SetColor(68.0 / 255.0, 114.0 / 255.0, 194.0 / 255.0);

					sphereSource2->SetCenter(m_posData[1]);//设置球的中心
					sphereSource2->SetRadius(5);//设置球的半径
					sphereSource2->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper2->SetInputConnection(sphereSource2->GetOutputPort());
					m_sphereActor2 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor2->SetMapper(mapper2);
					m_sphereActor2->GetProperty()->SetColor(237.0 / 255.0, 125.0 / 255.0, 49.0 / 255.0);

					sphereSource3->SetCenter(m_posData[2]);//设置球的中心
					sphereSource3->SetRadius(5);//设置球的半径
					sphereSource3->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper3 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper3->SetInputConnection(sphereSource3->GetOutputPort());
					m_sphereActor3 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor3->SetMapper(mapper3);
					m_sphereActor3->GetProperty()->SetColor(255.0 / 255.0, 192.0 / 255.0, 0);

					sphereSource4->SetCenter(m_posData[3]);//设置球的中心
					sphereSource4->SetRadius(5);//设置球的半径
					sphereSource4->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper4 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper4->SetInputConnection(sphereSource4->GetOutputPort());
					m_sphereActor4 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor4->SetMapper(mapper4);
					m_sphereActor4->GetProperty()->SetColor(112.0 / 255.0, 173.0 / 255.0, 71.0 / 255.0);

					sphereSource5->SetCenter(m_posData[4]);//设置球的中心
					sphereSource5->SetRadius(5);//设置球的半径
					sphereSource5->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper5 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper5->SetInputConnection(sphereSource5->GetOutputPort());
					m_sphereActor5 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor5->SetMapper(mapper5);
					m_sphereActor5->GetProperty()->SetColor(91.0 / 255.0, 155.0 / 255.0, 213.0 / 255.0);

					sphereSource6->SetCenter(m_posData[5]);//设置球的中心
					sphereSource6->SetRadius(5);//设置球的半径
					sphereSource6->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper6 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper6->SetInputConnection(sphereSource6->GetOutputPort());
					m_sphereActor6 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor6->SetMapper(mapper6);
					m_sphereActor6->GetProperty()->SetColor(112.0 / 255.0, 48.0 / 255.0, 160.0 / 255.0);

					m_posData[6][0] = m_posData[3][0] * 0.5 + m_posData[4][0] * 0.5;
					m_posData[6][1] = m_posData[3][1] * 0.5 + m_posData[4][1] * 0.5;
					m_posData[6][2] = m_posData[3][2] * 0.5 + m_posData[4][2] * 0.5;
					sphereSource7->SetCenter(m_posData[6]);//设置球的中心
					sphereSource7->SetRadius(5);//设置球的半径
					sphereSource7->SetThetaResolution(30);//设置球表面精度，值越大球的光滑程度越高
					vtkSmartPointer<vtkPolyDataMapper> mapper7 = vtkSmartPointer<vtkPolyDataMapper>::New();//定义局部PolydataMapper对象
					mapper7->SetInputConnection(sphereSource7->GetOutputPort());
					m_sphereActor7 = vtkSmartPointer<vtkActor>::New();//定义局部actor对象
					m_sphereActor7->SetMapper(mapper7);
					m_sphereActor7->GetProperty()->SetColor(1, 0, 0);

					m_lineSource = vtkSmartPointer<vtkLineSource>::New();
					m_lineSource->SetPoint1(m_posData[0]);
					m_lineSource->SetPoint2(m_posData[1]);
					m_lineSource->Update();
					vtkSmartPointer<vtkDataSetMapper> linemapper =
						vtkSmartPointer<vtkDataSetMapper>::New();
					linemapper->SetInputConnection(m_lineSource->GetOutputPort());
					m_lineActor = vtkSmartPointer<vtkActor>::New();
					m_lineActor->SetMapper(linemapper);
					m_lineActor->GetProperty()->SetColor(1, 0, 0);
					m_lineActor->GetProperty()->SetLineWidth(3);

					m_planeSource = vtkSmartPointer<vtkPlaneSource>::New();
					m_planeSource2 = vtkSmartPointer<vtkPlaneSource>::New();
					m_planeSource->SetCenter(m_posData[6]);
					calculatePlaneNormal();
					m_planeSource->SetNormal(m_planeNormal);
					m_planeSource->Update();
					vtkSmartPointer<vtkPolyDataMapper> mapperPlane = vtkSmartPointer<vtkPolyDataMapper>::New();
					mapperPlane->SetInputData(m_planeSource->GetOutput());
					m_planeActor = vtkSmartPointer<vtkActor>::New();
					m_planeActor->SetMapper(mapperPlane);

					m_lineSource2 = vtkSmartPointer<vtkLineSource>::New();
					m_lineSource2->SetPoint1(m_posData[2]);
					m_lineSource2->SetPoint2(m_posData[5]);
					m_lineSource2->Update();
					vtkSmartPointer<vtkDataSetMapper> linemapper2 =
						vtkSmartPointer<vtkDataSetMapper>::New();
					linemapper2->SetInputConnection(m_lineSource2->GetOutputPort());
					m_lineActor2 = vtkSmartPointer<vtkActor>::New();
					m_lineActor2->SetMapper(linemapper2);
					m_lineActor2->GetProperty()->SetColor(1, 0, 0);
					m_lineActor2->GetProperty()->SetLineWidth(3);

					m_planeSource2->SetCenter(m_posData[2]);
					m_planeSource2->SetNormal(m_plane2Normal);
					m_planeSource2->Update();
					m_mapperPlane2 = vtkSmartPointer<vtkPolyDataMapper>::New();
					m_mapperPlane2->SetInputData(m_planeSource2->GetOutput());
					m_polydataPlane2 = vtkSmartPointer<vtkPolyData>::New();
					m_polydataPlane2 = m_planeSource2->GetOutput();
					m_planeActor2 = vtkSmartPointer<vtkActor>::New();
					m_planeActor2->SetMapper(m_mapperPlane2);

					m_lineSource3 = vtkSmartPointer<vtkLineSource>::New();
					m_lineSource3->SetPoint1(m_posData[2]);
					m_lineSource3->SetPoint2(m_verticalLinePoint2);
					m_lineSource3->Update();
					vtkSmartPointer<vtkDataSetMapper> linemapper3 =
						vtkSmartPointer<vtkDataSetMapper>::New();
					linemapper3->SetInputConnection(m_lineSource3->GetOutputPort());
					m_lineActor3 = vtkSmartPointer<vtkActor>::New();
					m_lineActor3->SetMapper(linemapper3);
					m_lineActor3->GetProperty()->SetColor(1, 0, 0);
					m_lineActor3->GetProperty()->SetLineWidth(3);

					m_3DViewWidget.m_renderer->AddActor(m_sphereActor1);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor2);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor3);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor4);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor5);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor6);
					m_3DViewWidget.m_renderer->AddActor(m_sphereActor7);
					m_3DViewWidget.m_renderer->AddActor(m_lineActor);
					m_3DViewWidget.m_renderer->AddActor(m_planeActor);
					m_3DViewWidget.m_renderer->AddActor(m_lineActor2);
					m_3DViewWidget.m_renderer->AddActor(m_planeActor2);
					m_3DViewWidget.m_renderer->AddActor(m_lineActor3);

				}
				findTxt = true;
			}
			else if (mfi.suffix().toLower() == "stl")
			{
				vtkNew<vtkSTLReader> reader;
				reader->SetFileName(mfi.filePath().toStdString().c_str());
				reader->Update();
				vtkSmartPointer<vtkPolyDataMapper> mapper =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper->SetInputConnection(reader->GetOutputPort());
				init3DWindow();
				if (0 == actorCount)
				{
					if (!m_atcor1)
						m_atcor1 = vtkSmartPointer<vtkActor>::New();
					m_polydata1 = vtkSmartPointer<vtkPolyData>::New();
					m_polydata1 = reader->GetOutput();
					m_atcor1->SetMapper(mapper);
					m_atcor1->GetProperty()->SetOpacity(0.3);//透明度
					if (m_atcor1color.isValid())
						m_atcor1->GetProperty()->SetColor((double)m_atcor1color.red() / 255.0,
						(double)m_atcor1color.green() / 255.0, (double)m_atcor1color.blue() / 255.0);
					m_3DViewWidget.m_renderer->AddActor(m_atcor1);
				}
				else if (1 == actorCount)
				{
					if (!m_atcor2)
						m_atcor2 = vtkSmartPointer<vtkActor>::New();
					m_polydata2 = vtkSmartPointer<vtkPolyData>::New();
					m_polydata2 = reader->GetOutput();
					m_atcor2->SetMapper(mapper);
					m_atcor2->GetProperty()->SetOpacity(0.3);//透明度
					if (m_atcor2color.isValid())
						m_atcor2->GetProperty()->SetColor((double)m_atcor2color.red() / 255.0,
						(double)m_atcor2color.green() / 255.0, (double)m_atcor2color.blue() / 255.0);
					m_3DViewWidget.m_renderer->AddActor(m_atcor2);
				}
				else if (2 == actorCount)
				{
					if (!m_atcor3)
						m_atcor3 = vtkSmartPointer<vtkActor>::New();
					m_polydata3 = vtkSmartPointer<vtkPolyData>::New();
					m_polydata3 = reader->GetOutput();
					m_atcor3->SetMapper(mapper);
					m_atcor3->GetProperty()->SetOpacity(0.3);//透明度
					if (m_atcor3color.isValid())
						m_atcor3->GetProperty()->SetColor((double)m_atcor3color.red() / 255.0,
						(double)m_atcor3color.green() / 255.0, (double)m_atcor3color.blue() / 255.0);
					m_3DViewWidget.m_renderer->AddActor(m_atcor3);
				}
				else if (3 == actorCount)
				{
					if (!m_atcor4)
						m_atcor4 = vtkSmartPointer<vtkActor>::New();
					m_polydata4 = vtkSmartPointer<vtkPolyData>::New();
					m_polydata4 = reader->GetOutput();
					m_atcor4->SetMapper(mapper);
					m_atcor4->GetProperty()->SetOpacity(0.3);//透明度
					if (m_atcor4color.isValid())
						m_atcor4->GetProperty()->SetColor((double)m_atcor4color.red() / 255.0,
						(double)m_atcor4color.green() / 255.0, (double)m_atcor4color.blue() / 255.0);
					m_3DViewWidget.m_renderer->AddActor(m_atcor4);
				}
				else if (4 == actorCount)
				{
					if (!m_atcor5)
						m_atcor5 = vtkSmartPointer<vtkActor>::New();
					m_polydata5 = vtkSmartPointer<vtkPolyData>::New();
					m_polydata5 = reader->GetOutput();
					m_atcor5->SetMapper(mapper);
					m_atcor5->GetProperty()->SetOpacity(0.3);//透明度
					if (m_atcor5color.isValid())
						m_atcor5->GetProperty()->SetColor((double)m_atcor5color.red() / 255.0,
						(double)m_atcor5color.green() / 255.0, (double)m_atcor5color.blue() / 255.0);
					m_3DViewWidget.m_renderer->AddActor(m_atcor5);
				}

				++actorCount;
			}
		}
		if (actorCount > 4 && findTxt)
			break;
	}
	useMoveActorStyle();
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::selectSphere()
{
	QToolButton *btn = dynamic_cast<QToolButton*>(this->sender());
	init3DWindow();
	if (!m_PickStyle)
	{
		m_PickStyle = vtkSmartPointer<PointPickerInteractorStyle>::New();
		m_PickStyle->RenderWidget = &m_3DViewWidget;
		m_PickStyle->m_func = std::bind(&jointWidget::redrawLineAndPlane, this);
	}
	if (btn == ui.m_sphere1)
	{
		m_PickStyle->sphereSource = sphereSource1;
		if(curSelPoint == 1)
			curSelPoint = -1;
		else
			curSelPoint = 1;
		ui.m_sphere2->setChecked(false);
		ui.m_sphere3->setChecked(false);
		ui.m_sphere4->setChecked(false);
		ui.m_sphere5->setChecked(false);
		ui.m_sphere6->setChecked(false);
	}
	else if (btn == ui.m_sphere2)
	{
		m_PickStyle->sphereSource = sphereSource2;
		if (curSelPoint == 2)
			curSelPoint = -1;
		else
			curSelPoint = 2;
		ui.m_sphere1->setChecked(false);
		ui.m_sphere3->setChecked(false);
		ui.m_sphere4->setChecked(false);
		ui.m_sphere5->setChecked(false);
		ui.m_sphere6->setChecked(false);
	}
	else if (btn == ui.m_sphere3)
	{
		m_PickStyle->sphereSource = sphereSource3;
		if (curSelPoint == 3)
			curSelPoint = -1;
		else
			curSelPoint = 3;
		ui.m_sphere2->setChecked(false);
		ui.m_sphere1->setChecked(false);
		ui.m_sphere4->setChecked(false);
		ui.m_sphere5->setChecked(false);
		ui.m_sphere6->setChecked(false);
	}
	else if (btn == ui.m_sphere4)
	{
		m_PickStyle->sphereSource = sphereSource4;
		if (curSelPoint == 4)
			curSelPoint = -1;
		else
			curSelPoint = 4;
		ui.m_sphere2->setChecked(false);
		ui.m_sphere3->setChecked(false);
		ui.m_sphere1->setChecked(false);
		ui.m_sphere5->setChecked(false);
		ui.m_sphere6->setChecked(false);
	}
	else if (btn == ui.m_sphere5)
	{
		m_PickStyle->sphereSource = sphereSource5;
		if (curSelPoint == 5)
			curSelPoint = -1;
		else
			curSelPoint = 5;
		ui.m_sphere2->setChecked(false);
		ui.m_sphere3->setChecked(false);
		ui.m_sphere4->setChecked(false);
		ui.m_sphere1->setChecked(false);
		ui.m_sphere6->setChecked(false);
	}
	else if (btn == ui.m_sphere6)
	{
		m_PickStyle->sphereSource = sphereSource6;
		if (curSelPoint == 6)
			curSelPoint = -1;
		else
			curSelPoint = 6;
		ui.m_sphere2->setChecked(false);
		ui.m_sphere3->setChecked(false);
		ui.m_sphere4->setChecked(false);
		ui.m_sphere5->setChecked(false);
		ui.m_sphere1->setChecked(false);
	}
	if (curSelPoint > 0)
	{
		vtkSmartPointer<vtkPropPicker> pointPicker =
			vtkSmartPointer<vtkPropPicker>::New();
		m_3DViewWidget.renderWindow()->GetInteractor()->SetPicker(pointPicker);
		m_PickStyle->enable = true;
		m_3DViewWidget.renderWindow()->GetInteractor()->SetInteractorStyle(m_PickStyle);
	}
	else
	{
		m_3DViewWidget.renderWindow()->GetInteractor()->SetPicker(nullptr);
		m_PickStyle->enable = false;
		useMoveActorStyle();
	}
}

void jointWidget::SaveCameraPosition()
{
	init3DWindow();
	auto c = m_3DViewWidget.m_renderer->GetActiveCamera();
	if (!m_savedCamera)
		m_savedCamera = vtkSmartPointer<vtkCamera>::New();
	m_savedCamera->DeepCopy(c);
}

void jointWidget::RecoverCameraPosition()
{
	init3DWindow();
	if (m_savedCamera)
	{
		m_3DViewWidget.m_renderer->SetActiveCamera(m_savedCamera);
		m_3DViewWidget.renderWindow()->GetInteractor()->Render();
		m_3DViewWidget.renderWindow()->Render();
	}
}

void jointWidget::cutWithPlane()
{
	cutAllActor(false);
}

void jointWidget::ctrlTranslatin()
{
	m_isDoTranslation = !m_isDoTranslation;
	if (m_isDoTranslation)
	{
		ui.m_isTranslation->setText(u8"平移");
	}
	else
	{
		ui.m_isTranslation->setText(u8"旋转");
	}
}

void jointWidget::moveActor()
{
	if (!m_moveActorStyle)
		return;
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	if (btn == ui.m_up)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(0, 1, 0);
		else
			m_moveActorStyle->Rotate(0, -3, 0);
	}
	else if(btn == ui.m_down)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(0, -1, 0);
		else
			m_moveActorStyle->Rotate(0, 3, 0);
	}
	else if(btn == ui.m_left)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(-1, 0, 0);
		else
			m_moveActorStyle->Rotate(-3, 0, 0);
	}
	else if(btn == ui.m_right)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(1, 0, 0);
		else
			m_moveActorStyle->Rotate(3, 0, 0);
	}
	else if(btn == ui.m_front)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(0, 0, 0.01);
		else
			m_moveActorStyle->Rotate(0, 0, 3);
	}
	else if(btn == ui.m_back)
	{
		if (m_isDoTranslation)
			m_moveActorStyle->Pan(0, 0, -0.01);
		else
			m_moveActorStyle->Rotate(0, 0, -3);
	}
}

void jointWidget::processPlaneCommand()
{
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	if (btn == ui.m_deletePlane1)
	{
		if (m_planeActor)
		{
			m_3DViewWidget.m_renderer->RemoveActor(m_planeActor);
		}
	}
	else if (btn == ui.m_deletePlane2)
	{
		if (m_planeActor2)
		{
			m_3DViewWidget.m_renderer->RemoveActor(m_planeActor2);
		}
	}
	else if (btn == ui.m_showPlane1)
	{
		bool bShow = true;
		if (btn->text() == u8"隐藏")
		{
			bShow = false;
			btn->setText(u8"显示");
		}
		else
		{
			btn->setText(u8"隐藏");
		}
		if (m_planeActor)
		{
			m_planeActor->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showPlane2)
	{
		bool bShow = true;
		if (btn->text() == u8"隐藏")
		{
			bShow = false;
			btn->setText(u8"显示");
		}
		else
		{
			btn->setText(u8"隐藏");
		}
		if (m_planeActor2)
		{
			m_planeActor2->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_rotatePlane2_L)
	{
		if (m_planeActor2)
		{
			++m_rotateTimes;
			tranFormPlane2();
		}
	}
	else if (btn == ui.m_rotatePlane2_R)
	{
		if (m_planeActor2)
		{
			--m_rotateTimes;
			tranFormPlane2();
		}
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::movePoint3()
{
	if (!sphereSource3)
		return;
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	if (btn == ui.m_move3rdPointLeft)
	{
		double picked[3];
		sphereSource3->GetCenter(picked);
		picked[0] -= m_Point3MoveLen * m_line2Normal[0];
		picked[1] -= m_Point3MoveLen * m_line2Normal[1];
		picked[2] -= m_Point3MoveLen * m_line2Normal[2];
		sphereSource3->SetCenter(picked);
		curSelPoint = 3;
		redrawLineAndPlane();
		--m_MoveTimes;
		tranFormPlane2();
	}
	else if (btn == ui.m_move3rdPointRight)
	{
		double picked[3];
		sphereSource3->GetCenter(picked);
		picked[0] += m_Point3MoveLen * m_line2Normal[0];
		picked[1] += m_Point3MoveLen * m_line2Normal[1];
		picked[2] += m_Point3MoveLen * m_line2Normal[2];
		sphereSource3->SetCenter(picked);
		curSelPoint = 3;
		redrawLineAndPlane();
		++m_MoveTimes;
		tranFormPlane2();
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::processCuttedActorCommand()
{
	QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
	if (btn == ui.m_showCuttedActor1)
	{
		if (m_cuttedActor1)
		{
			bool bShow = true;
			if (btn->text() == u8"隐藏")
			{
				bShow = false;
				btn->setText(u8"显示");
			}
			else
			{
				btn->setText(u8"隐藏");
			}
			m_cuttedActor1->SetVisibility(bShow);
		}
	}
	else if (btn == ui.m_showCuttedActor2)
	{
		if (m_cuttedActor2)
		{
			bool bShow = true;
			if (btn->text() == u8"隐藏")
			{
				bShow = false;
				btn->setText(u8"显示");
			}
			else
			{
				btn->setText(u8"隐藏");
			}
			m_cuttedActor2->SetVisibility(bShow);
		}
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::redrawLineAndPlane()
{
	if (curSelPoint == 1 || curSelPoint == 2)
	{
		memcpy(m_posData[0], sphereSource1->GetCenter(), sizeof(double) * 3);
		memcpy(m_posData[1], sphereSource2->GetCenter(), sizeof(double) * 3);
		m_lineSource->SetPoint1(m_posData[0]);
		m_lineSource->SetPoint2(m_posData[1]);
		m_lineSource->Update();
	}
	else if (curSelPoint >= 3 && curSelPoint <= 6)
	{
		memcpy(m_posData[2], sphereSource3->GetCenter(), sizeof(double) * 3);
		memcpy(m_posData[3], sphereSource4->GetCenter(), sizeof(double) * 3);
		memcpy(m_posData[4], sphereSource5->GetCenter(), sizeof(double) * 3);
		memcpy(m_posData[5], sphereSource6->GetCenter(), sizeof(double) * 3);
		m_posData[6][0] = m_posData[3][0] * 0.5 + m_posData[4][0] * 0.5;
		m_posData[6][1] = m_posData[3][1] * 0.5 + m_posData[4][1] * 0.5;
		m_posData[6][2] = m_posData[3][2] * 0.5 + m_posData[4][2] * 0.5;
		sphereSource7->SetCenter(m_posData[6]);
		m_planeSource->SetCenter(m_posData[6]);
		calculatePlaneNormal();
		m_planeSource->SetNormal(m_planeNormal);
		m_planeSource->Update();
		m_lineSource3->SetPoint1(m_posData[2]);
		m_lineSource3->SetPoint2(m_verticalLinePoint2);
		m_lineSource3->Update();
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::cutAllActor(bool bCombined)
{
	if (!m_planeActor2)
		return;
	//以面2为准切割,算面2的法向量
	vtkSmartPointer<vtkPolyDataNormals> pdNormals =
		vtkSmartPointer<vtkPolyDataNormals>::New();
	pdNormals->SetInputData(m_planeActor2->GetMapper()->GetInput());
	pdNormals->ComputeCellNormalsOn();
	pdNormals->Update();
	auto ptNormals = pdNormals->GetOutput()->GetPointData()->GetNormals();

	vtkSmartPointer<vtkPlane> plane1 = vtkSmartPointer<vtkPlane>::New();
	plane1->SetOrigin(m_posData[2]);
	if (ptNormals->GetNumberOfTuples() > 0)
	{
		double value[3];
		ptNormals->GetTuple(0, value);
		plane1->SetNormal(value);
	}
	else
		plane1->SetNormal(m_planeSource2->GetNormal());
	//截面2是取反向
	vtkSmartPointer<vtkPlane> plane2 = vtkSmartPointer<vtkPlane>::New();
	plane2->SetOrigin(m_posData[2]);
	if (ptNormals->GetNumberOfTuples() > 0)
	{
		double value[3];
		ptNormals->GetTuple(0, value);
		plane2->SetNormal(-value[0], -value[1], -value[2]);
	}
	else
		plane2->SetNormal(-m_planeSource2->GetNormal()[0],
			-m_planeSource2->GetNormal()[1],-m_planeSource2->GetNormal()[2]);
	if (bCombined)
	{
		vtkSmartPointer<vtkAppendPolyData> appendFilter =
			vtkSmartPointer<vtkAppendPolyData>::New();
		if (m_atcor1)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor1->GetOrientation()[1]);
			pTransform->RotateX(m_atcor1->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor1->GetOrientation()[2]);
			pTransform->Scale(m_atcor1->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor1->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata1);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			appendFilter->AddInputData(pTransformPolyDataFilterSTL->GetOutput());
			//appendFilter->AddInputData(m_polydata1);
		}
		if (m_atcor2)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor2->GetOrientation()[1]);
			pTransform->RotateX(m_atcor2->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor2->GetOrientation()[2]);
			pTransform->Scale(m_atcor2->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor2->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata2);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			appendFilter->AddInputData(pTransformPolyDataFilterSTL->GetOutput());
			//appendFilter->AddInputData(m_polydata2);
		}
		if (m_atcor3)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor3->GetOrientation()[1]);
			pTransform->RotateX(m_atcor3->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor3->GetOrientation()[2]);
			pTransform->Scale(m_atcor3->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor3->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata3);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			appendFilter->AddInputData(pTransformPolyDataFilterSTL->GetOutput());
			//appendFilter->AddInputData(m_polydata3);
		}
		if (m_atcor4)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor4->GetOrientation()[1]);
			pTransform->RotateX(m_atcor4->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor4->GetOrientation()[2]);
			pTransform->Scale(m_atcor4->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor4->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata4);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			appendFilter->AddInputData(pTransformPolyDataFilterSTL->GetOutput());
			//appendFilter->AddInputData(m_polydata4);
		}
		if (m_atcor5)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor5->GetOrientation()[1]);
			pTransform->RotateX(m_atcor5->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor5->GetOrientation()[2]);
			pTransform->Scale(m_atcor5->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor5->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata5);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			appendFilter->AddInputData(pTransformPolyDataFilterSTL->GetOutput());
			//appendFilter->AddInputData(m_polydata5);
		}
		appendFilter->Update();
		// Remove any duplicate points.
		vtkNew<vtkCleanPolyData> cleanFilter;
		cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
		cleanFilter->Update();
		vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
		clipper->SetInputData(cleanFilter->GetOutput());
		clipper->SetClipFunction(plane1);
		clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
		clipper->Update();
		vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
		clipper2->SetInputData(cleanFilter->GetOutput());
		clipper2->SetClipFunction(plane2);
		clipper2->GenerateClippedOutputOn();
		clipper2->Update();
		if (!m_cuttedActor1)
		{
			m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
			m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
		}
		vtkSmartPointer<vtkPolyDataMapper> mapper1 =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		vtkSmartPointer<vtkPolyDataMapper> mapper2 =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper1->SetInputData(clipper->GetOutput());
		mapper2->SetInputData(clipper2->GetOutput());
		m_cuttedActor1->SetMapper(mapper1);
		m_cuttedActor2->SetMapper(mapper2);
		m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
		m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
		if (m_atcor1)
			m_atcor1->SetVisibility(false);
		if (m_atcor2)
			m_atcor2->SetVisibility(false);
		if (m_atcor3)
			m_atcor3->SetVisibility(false);
		if (m_atcor4)
			m_atcor4->SetVisibility(false);
		if (m_atcor5)
			m_atcor5->SetVisibility(false);
		vtkNew<vtkSTLWriter> stlWriter;
		QString filePath = QFileDialog::getSaveFileName(this, u8"请选择STL文件路径", QString(), "STL (*.stl)");
		if (filePath.isEmpty())
			return;
		stlWriter->SetFileName(filePath.toStdString().c_str());
		stlWriter->SetInputData(clipper->GetOutput());
		stlWriter->SetFileTypeToBinary();
		stlWriter->Update();
	}
	else
	{
		//QString filePath = QFileDialog::getExistingDirectory(this, u8"请选择保存切割后STL文件路径");
		//if (filePath.isEmpty())
		//	return;
		if (m_atcor1)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor1->GetOrientation()[1]);
			pTransform->RotateX(m_atcor1->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor1->GetOrientation()[2]);
			pTransform->Scale(m_atcor1->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor1->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata1);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
			clipper->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper->SetClipFunction(plane1);
			clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
			clipper->Update();
			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper2->SetClipFunction(plane2);
			clipper2->GenerateClippedOutputOn();
			clipper2->Update();
			double bounds[6];
			m_atcor1->GetBounds(bounds);
			bool inside = m_posData[2][0] >= bounds[0] && m_posData[2][0] <= bounds[1]
				&& m_posData[2][1] >= bounds[2] && m_posData[2][1] <= bounds[3]
				&& m_posData[2][2] >= bounds[4] && m_posData[2][2] <= bounds[5];
			if (inside)
			{
				if (!m_cuttedActor1)
				{
					m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
					m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
				}
				vtkSmartPointer<vtkPolyDataMapper> mapper1 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				vtkSmartPointer<vtkPolyDataMapper> mapper2 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper1->SetInputData(clipper->GetOutput());
				mapper2->SetInputData(clipper2->GetOutput());
				m_cuttedActor1->SetMapper(mapper1);
				m_cuttedActor2->SetMapper(mapper2);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
				m_atcor1->SetVisibility(false);
			}
			//vtkNew<vtkSTLWriter> stlWriter;
			//QString s(filePath);
			//s += "/actor1.stl";
			//stlWriter->SetFileName(s.toStdString().c_str());
			//stlWriter->SetInputData(clipper->GetOutput());
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->Update();
		}
		if (m_atcor2)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor2->GetOrientation()[1]);
			pTransform->RotateX(m_atcor2->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor2->GetOrientation()[2]);
			pTransform->Scale(m_atcor2->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor2->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata2);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
			clipper->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper->SetClipFunction(plane1);
			clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
			clipper->Update();
			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper2->SetClipFunction(plane2);
			clipper2->GenerateClippedOutputOn();
			clipper2->Update();
			double bounds[6];
			m_atcor2->GetBounds(bounds);
			bool inside = m_posData[2][0] >= bounds[0] && m_posData[2][0] <= bounds[1]
				&& m_posData[2][1] >= bounds[2] && m_posData[2][1] <= bounds[3]
				&& m_posData[2][2] >= bounds[4] && m_posData[2][2] <= bounds[5];
			if (inside)
			{
				if (!m_cuttedActor1)
				{
					m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
					m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
				}
				vtkSmartPointer<vtkPolyDataMapper> mapper1 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				vtkSmartPointer<vtkPolyDataMapper> mapper2 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper1->SetInputData(clipper->GetOutput());
				mapper2->SetInputData(clipper2->GetOutput());
				m_cuttedActor1->SetMapper(mapper1);
				m_cuttedActor2->SetMapper(mapper2);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
				m_atcor2->SetVisibility(false);
			}
			//vtkNew<vtkSTLWriter> stlWriter;
			//QString s(filePath);
			//s += "/actor2.stl";
			//stlWriter->SetFileName(s.toStdString().c_str());
			//stlWriter->SetInputData(clipper->GetOutput());
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->Update();
		}
		if (m_atcor3)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor3->GetOrientation()[1]);
			pTransform->RotateX(m_atcor3->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor3->GetOrientation()[2]);
			pTransform->Scale(m_atcor3->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor3->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata3);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
			clipper->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper->SetClipFunction(plane1);
			clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
			clipper->Update();
			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper2->SetClipFunction(plane2);
			clipper2->GenerateClippedOutputOn();
			clipper2->Update();
			double bounds[6];
			m_atcor3->GetBounds(bounds);
			bool inside = m_posData[2][0] >= bounds[0] && m_posData[2][0] <= bounds[1]
				&& m_posData[2][1] >= bounds[2] && m_posData[2][1] <= bounds[3]
				&& m_posData[2][2] >= bounds[4] && m_posData[2][2] <= bounds[5];
			if (inside)
			{
				if (!m_cuttedActor1)
				{
					m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
					m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
				}
				vtkSmartPointer<vtkPolyDataMapper> mapper1 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				vtkSmartPointer<vtkPolyDataMapper> mapper2 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper1->SetInputData(clipper->GetOutput());
				mapper2->SetInputData(clipper2->GetOutput());
				m_cuttedActor1->SetMapper(mapper1);
				m_cuttedActor2->SetMapper(mapper2);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
				m_atcor3->SetVisibility(false);
			}
			//vtkNew<vtkSTLWriter> stlWriter;
			//QString s(filePath);
			//s += "/actor3.stl";
			//stlWriter->SetFileName(s.toStdString().c_str());
			//stlWriter->SetInputData(clipper->GetOutput());
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->Update();
		}
		if (m_atcor4)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor4->GetOrientation()[1]);
			pTransform->RotateX(m_atcor4->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor4->GetOrientation()[2]);
			pTransform->Scale(m_atcor4->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor4->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata4);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
			clipper->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper->SetClipFunction(plane1);
			clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
			clipper->Update();
			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper2->SetClipFunction(plane2);
			clipper2->GenerateClippedOutputOn();
			clipper2->Update();
			double bounds[6];
			m_atcor4->GetBounds(bounds);
			bool inside = m_posData[2][0] >= bounds[0] && m_posData[2][0] <= bounds[1]
				&& m_posData[2][1] >= bounds[2] && m_posData[2][1] <= bounds[3]
				&& m_posData[2][2] >= bounds[4] && m_posData[2][2] <= bounds[5];
			if (inside)
			{
				if (!m_cuttedActor1)
				{
					m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
					m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
				}
				vtkSmartPointer<vtkPolyDataMapper> mapper1 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				vtkSmartPointer<vtkPolyDataMapper> mapper2 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper1->SetInputData(clipper->GetOutput());
				mapper2->SetInputData(clipper2->GetOutput());
				m_cuttedActor1->SetMapper(mapper1);
				m_cuttedActor2->SetMapper(mapper2);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
				m_atcor4->SetVisibility(false);
			}
			//vtkNew<vtkSTLWriter> stlWriter;
			//QString s(filePath);
			//s += "/actor4.stl";
			//stlWriter->SetFileName(s.toStdString().c_str());
			//stlWriter->SetInputData(clipper->GetOutput());
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->Update();
		}
		if (m_atcor5)
		{
			vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
			pTransform->RotateY(m_atcor5->GetOrientation()[1]);
			pTransform->RotateX(m_atcor5->GetOrientation()[0]);
			pTransform->RotateZ(m_atcor5->GetOrientation()[2]);
			pTransform->Scale(m_atcor5->GetScale());
			pTransform->Update();
			pTransform->SetMatrix(m_atcor5->GetMatrix());
			pTransform->Update();
			vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			pTransformPolyDataFilterSTL->SetInputData(m_polydata5);
			pTransformPolyDataFilterSTL->SetTransform(pTransform);
			pTransformPolyDataFilterSTL->Update();
			vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
			clipper->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper->SetClipFunction(plane1);
			clipper->GenerateClippedOutputOn();//如果没有这一行，那么切割完后将不会产生polydata数据
			clipper->Update();
			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
			clipper2->SetClipFunction(plane2);
			clipper2->GenerateClippedOutputOn();
			clipper2->Update();
			double bounds[6];
			m_atcor5->GetBounds(bounds);
			bool inside = m_posData[2][0] >= bounds[0] && m_posData[2][0] <= bounds[1]
				&& m_posData[2][1] >= bounds[2] && m_posData[2][1] <= bounds[3]
				&& m_posData[2][2] >= bounds[4] && m_posData[2][2] <= bounds[5];
			if (inside)
			{
				if (!m_cuttedActor1)
				{
					m_cuttedActor1 = vtkSmartPointer<vtkActor>::New();
					m_cuttedActor2 = vtkSmartPointer<vtkActor>::New();
				}
				vtkSmartPointer<vtkPolyDataMapper> mapper1 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				vtkSmartPointer<vtkPolyDataMapper> mapper2 =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper1->SetInputData(clipper->GetOutput());
				mapper2->SetInputData(clipper2->GetOutput());
				m_cuttedActor1->SetMapper(mapper1);
				m_cuttedActor2->SetMapper(mapper2);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor1);
				m_3DViewWidget.m_renderer->AddActor(m_cuttedActor2);
				m_atcor5->SetVisibility(false);
			}
			//vtkNew<vtkSTLWriter> stlWriter;
			//QString s(filePath);
			//s += "/actor5.stl";
			//stlWriter->SetFileName(s.toStdString().c_str());
			//stlWriter->SetInputData(clipper->GetOutput());
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->Update();
		}
	}
	m_3DViewWidget.renderWindow()->Render();
}

void jointWidget::init3DWindow()
{
	if (!init3dView)
	{
		init3dView = true;
		m_3DViewWidget.renderWindow()->AddRenderer(m_3DViewWidget.m_renderer);
	}
}

void jointWidget::calculatePlaneNormal()
{
	//点3m_posData[2]，点6m_posData[5]，点7m_posData[6]
	m_planeNormal[0] = (m_posData[5][1] - m_posData[2][1])*(m_posData[6][2] - m_posData[2][2]) - (m_posData[5][2] - m_posData[2][2])*(m_posData[6][1] - m_posData[2][1]);
	m_planeNormal[1] = (m_posData[5][2] - m_posData[2][2])*(m_posData[6][0] - m_posData[2][0]) - (m_posData[5][0] - m_posData[2][0])*(m_posData[6][2] - m_posData[2][2]);
	m_planeNormal[2] = (m_posData[5][0] - m_posData[2][0])*(m_posData[6][1] - m_posData[2][1]) - (m_posData[5][1] - m_posData[2][1])*(m_posData[6][0] - m_posData[2][0]);
	if (m_planeNormal[0] == m_planeNormal[1] &&
		m_planeNormal[0] == m_planeNormal[2] &&
		m_planeNormal[0] == 0.0)
	{
		m_planeNormal[0] = 1;
		m_planeNormal[1] = 1;
		m_planeNormal[2] = 1;
	}
	double len = 0.5*sqrt(m_planeArea);
	m_planeSource->SetOrigin(m_posData[6][0]-len, m_posData[6][1]-len, m_posData[6][2]);
	m_planeSource->SetPoint1(m_posData[6][0]-len, m_posData[6][1]+len, m_posData[6][2]);
	m_planeSource->SetPoint2(m_posData[6][0]+len, m_posData[6][1]-len, m_posData[6][2]);
	m_plane2Normal[0] = m_posData[5][0] - m_posData[2][0];
	m_plane2Normal[1] = m_posData[5][1] - m_posData[2][1];
	m_plane2Normal[2] = m_posData[5][2] - m_posData[2][2];
	double len2 = 200;//平面2先写死
	m_planeSource2->SetOrigin(m_posData[2][0] - len, m_posData[2][1] - len, m_posData[2][2]);
	m_planeSource2->SetPoint1(m_posData[2][0] - len, m_posData[2][1] + len, m_posData[2][2]);
	m_planeSource2->SetPoint2(m_posData[2][0] + len, m_posData[2][1] - len, m_posData[2][2]);
	//line 2
	m_line2Normal[0] = m_plane2Normal[0];
	m_line2Normal[1] = m_plane2Normal[1];
	m_line2Normal[2] = m_plane2Normal[2];
	vtkMath::Normalize(m_line2Normal);
	/*
	平面1：a1x+b1y+c1z+d1=0；平面2：a2x+b2y+c2z+d2=0
	平面法向量;n1=(a1,b1,c1),n2=(a2,b2,c2)
	交线的方向向量n=n1×n2=(b1c2-c1b2,c1a2-a1c2,a1b2-b1a2)
	设直线上任意一点为（x,y,z）
	令x=0,得：
	b1y+c1z+d1=0，b2y+c2z+d2=0,
	即
	y=-(c1z+d1)/b1=-(c2z+d2)/b2
	解得：
	z=-(d1/b1-d2/b2)/(c1/b1-c2/b2)
	y=-c1/b1z-d1/b1
	*/
	//a×b=（Y1Z2-Y2Z1,Z1X2-Z2X1,X1Y2-X2Y1）
	//m_intersectingLineNormal[0] = m_planeNormal[1] * m_plane2Normal[2] - m_plane2Normal[1] * m_planeNormal[2];
	//m_intersectingLineNormal[1] = m_planeNormal[2] * m_plane2Normal[0] - m_plane2Normal[2] * m_planeNormal[0];
	//m_intersectingLineNormal[2] = m_planeNormal[0] * m_plane2Normal[1] - m_plane2Normal[0] * m_planeNormal[1];
	////ax+by+cz+d=0,
	//double d1 = -m_planeNormal[0] * m_posData[6][0] - m_planeNormal[1] * m_posData[6][1] - m_planeNormal[2] * m_posData[6][2];
	//double d2 = -m_plane2Normal[0] * m_posData[2][0] - m_plane2Normal[1] * m_posData[2][1] - m_plane2Normal[2] * m_posData[2][2];
	//m_intersectingLinePoint[0][0] = 0;
	//m_intersectingLinePoint[0][2] = -(d1 / m_planeNormal[1] - d2 / m_plane2Normal[1])
	//	/ (m_planeNormal[2] / m_planeNormal[1] - m_plane2Normal[2] / m_plane2Normal[1]);
	//m_intersectingLinePoint[0][1] = (-m_planeNormal[2] / m_planeNormal[1])*m_intersectingLinePoint[0][2]
	//	- d1 / m_planeNormal[1];
	//double normalintersectingLine[3]{ m_intersectingLineNormal[0],m_intersectingLineNormal[1],
	//m_intersectingLineNormal[2] };
	//vtkMath::Normalize(normalintersectingLine);
	//m_intersectingLinePoint[1][0] = m_intersectingLinePoint[0][0] + 5000* normalintersectingLine[0];
	//m_intersectingLinePoint[1][1] = m_intersectingLinePoint[0][1] + 5000* normalintersectingLine[1];
	//m_intersectingLinePoint[1][2] = m_intersectingLinePoint[0][2] + 5000* normalintersectingLine[2];
	//计算点3移动多少倍归一化的向量
	//0.5mm｜AB｜=(x1-x2)^2+(y1-y2)^2+(z1-z2)^2的开方
	//0.25 = n^2*m_line2Normal[0]^2 + n^2*m_line2Normal[1]^2 + n^2*m_line2Normal[2]^2
	m_Point3MoveLen = sqrt(0.25 / (m_line2Normal[0] * m_line2Normal[0] +
		m_line2Normal[1] * m_line2Normal[1] + m_line2Normal[2] * m_line2Normal[2]));

	memcpy(m_verticalLineNormal, m_planeNormal, sizeof(double) * 3);
	vtkMath::Normalize(m_verticalLineNormal);
	m_verticalLinePoint2[0] = m_posData[2][0] + m_verticalLineNormal[0] * 300;
	m_verticalLinePoint2[1] = m_posData[2][1] + m_verticalLineNormal[1] * 300;
	m_verticalLinePoint2[2] = m_posData[2][2] + m_verticalLineNormal[2] * 300;
}

void jointWidget::useMoveActorStyle()
{
	if (!m_moveActorStyle)
	{
		m_moveActorStyle = vtkSmartPointer<InteractorStyleTrackballActor>::New();
	}
	m_3DViewWidget.renderWindow()->GetInteractor()->SetInteractorStyle(m_moveActorStyle);
}

void jointWidget::tranFormPlane2()
{
	double angel = (m_rotateTimes * 3) % 360;
	vtkSmartPointer<vtkTransform> pTransform = vtkSmartPointer<vtkTransform>::New();
	pTransform->Translate(m_posData[2]);
	pTransform->RotateWXYZ(angel, m_planeNormal);
	pTransform->Translate(-m_posData[2][0], -m_posData[2][1], -m_posData[2][2]);
	pTransform->Translate(m_MoveTimes*m_Point3MoveLen * m_line2Normal[0],
		m_MoveTimes*m_Point3MoveLen * m_line2Normal[1],
		m_MoveTimes*m_Point3MoveLen * m_line2Normal[2]);
	pTransform->Update();
	vtkSmartPointer<vtkTransformPolyDataFilter> pTransformPolyDataFilterSTL = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	pTransformPolyDataFilterSTL->SetInputData(m_polydataPlane2);
	pTransformPolyDataFilterSTL->SetTransform(pTransform);
	pTransformPolyDataFilterSTL->Update();
	m_mapperPlane2->SetInputData(pTransformPolyDataFilterSTL->GetOutput());
}
