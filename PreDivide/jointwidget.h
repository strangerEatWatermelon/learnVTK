#pragma once

#include <QWidget>
#include "ui_jointwidget.h"
#include "predividewidget.h"
#include "ppzInteractorStyleTrackballActor.h"

#include "vtkSTLReader.h"
#include "vtkSTLWriter.h"
#include "vtkSphereSource.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkPointPicker.h"
#include "vtkPropPicker.h"
#include "vtkRendererCollection.h"
#include "vtkCamera.h"
#include "vtkLineSource.h"
#include "vtkDataSetMapper.h"
#include "vtkPlaneSource.h"
#include "vtkPlane.h"
#include "vtkClipPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkCleanPolyData.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkMatrix4x4.h"
#include "vtkTriangleFilter.h"
#include "vtkMassProperties.h"
#include "vtkPolyDataNormals.h"
#include "vtkPointData.h"
#include "vtkSelectEnclosedPoints.h"


class PointPickerInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static PointPickerInteractorStyle* New()
	{
		return new PointPickerInteractorStyle;
	}
	vtkSmartPointer<vtkSphereSource> sphereSource = NULL;
	QVTKRenderWindow * RenderWidget = nullptr;
	bool enable = false;
	static std::function<void()> m_func;
	vtkTypeMacro(PointPickerInteractorStyle, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDoubleClick()
	{
		if (enable && sphereSource)
		{
			this->Interactor->GetPicker()->Pick(
				this->Interactor->GetEventPosition()[0],
				this->Interactor->GetEventPosition()[1], 0,  // always zero.
				this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()
			);
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);
			sphereSource->SetCenter(picked);
			if (m_func)
				m_func();
		}
		//vtkInteractorStyleTrackballCamera::OnLeftButtonDoubleClick();
	}
};


class jointWidget : public QWidget
{
	Q_OBJECT

public:
	jointWidget(QWidget *parent = nullptr);
	~jointWidget();
public slots:
	void readSTLAndShowActor();
	void changeActorColor();
	void ShowHideActor();
	void readPostion();
	void selectSphere();
	void SaveCameraPosition();
	void RecoverCameraPosition();
	void cutWithPlane();
	void ctrlTranslatin();
	void moveActor();
	void processPlaneCommand();
	void movePoint3();
	void processCuttedActorCommand();
public:
	void redrawLineAndPlane();
	void cutAllActor(bool bCombined);
private:
	void init3DWindow();
	void calculatePlaneNormal();
	void useMoveActorStyle();
	void tranFormPlane2();
private:
	Ui::jointWidgetClass ui;
	QVTKRenderWindow m_3DViewWidget;
	bool m_isDoTranslation = false;
	bool init3dView = false;
	vtkSmartPointer<vtkActor> m_atcor1;
	vtkSmartPointer<vtkActor> m_atcor2;
	vtkSmartPointer<vtkActor> m_atcor3;
	vtkSmartPointer<vtkActor> m_atcor4;
	vtkSmartPointer<vtkActor> m_atcor5;
	vtkSmartPointer<vtkPolyData> m_polydata1;
	vtkSmartPointer<vtkPolyData> m_polydata2;
	vtkSmartPointer<vtkPolyData> m_polydata3;
	vtkSmartPointer<vtkPolyData> m_polydata4;
	vtkSmartPointer<vtkPolyData> m_polydata5;
	QColor m_atcor1color = QColor::Invalid;
	QColor m_atcor2color = QColor::Invalid;
	QColor m_atcor3color = QColor::Invalid;
	QColor m_atcor4color = QColor::Invalid;
	QColor m_atcor5color = QColor::Invalid;
	double m_posData[7][3]{};
	vtkSmartPointer<vtkCamera> m_savedCamera;
	int curSelPoint = -1;
	vtkSmartPointer<vtkSphereSource> sphereSource1;
	vtkSmartPointer<vtkSphereSource> sphereSource2;
	vtkSmartPointer<vtkSphereSource> sphereSource3;
	vtkSmartPointer<vtkSphereSource> sphereSource4;
	vtkSmartPointer<vtkSphereSource> sphereSource5;
	vtkSmartPointer<vtkSphereSource> sphereSource6;
	vtkSmartPointer<vtkSphereSource> sphereSource7;
	vtkSmartPointer<vtkActor> m_sphereActor1;
	vtkSmartPointer<vtkActor> m_sphereActor2;
	vtkSmartPointer<vtkActor> m_sphereActor3;
	vtkSmartPointer<vtkActor> m_sphereActor4;
	vtkSmartPointer<vtkActor> m_sphereActor5;
	vtkSmartPointer<vtkActor> m_sphereActor6;
	vtkSmartPointer<vtkActor> m_sphereActor7;
	vtkSmartPointer<vtkLineSource> m_lineSource;
	vtkSmartPointer<vtkActor> m_lineActor;
	vtkSmartPointer<vtkPlaneSource> m_planeSource;
	double m_planeNormal[3]{};
	double m_planeArea = 200*200;
	vtkSmartPointer<vtkActor> m_planeActor;
	vtkSmartPointer<PointPickerInteractorStyle> m_PickStyle;
	vtkSmartPointer<InteractorStyleTrackballActor> m_moveActorStyle;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
	vtkSmartPointer<vtkLineSource> m_lineSource2;
	vtkSmartPointer<vtkActor> m_lineActor2;
	vtkSmartPointer<vtkPlaneSource> m_planeSource2;
	vtkSmartPointer<vtkActor> m_planeActor2;
	vtkSmartPointer<vtkPolyData> m_polydataPlane2;
	vtkSmartPointer<vtkPolyDataMapper> m_mapperPlane2;
	double m_plane2Normal[3]{};
	double m_line2Normal[3]{};
	double m_verticalLineNormal[3]{};
	double m_verticalLinePoint2[3]{};
	int m_rotateTimes = 0;
	int m_MoveTimes = 0;
	double m_Point3MoveLen = 20;
	vtkSmartPointer<vtkLineSource> m_lineSource3;
	vtkSmartPointer<vtkActor> m_lineActor3;
	vtkSmartPointer<vtkActor> m_cuttedActor1;
	vtkSmartPointer<vtkActor> m_cuttedActor2;
};
