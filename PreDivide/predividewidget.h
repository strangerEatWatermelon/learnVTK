#ifndef PREDIVIDEWIDGET_H
#define PREDIVIDEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <vector>
#include <map>
#include <set>
#include <QProcess>
#include <QThread>
#include <QPolygonF>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QColorDialog>
#include <QDebug>


#include "QVTKOpenGLNativeWidget.h"
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>  
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>  
#include <vtkImageFlip.h>
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkExtractVOI.h>
#include <vtkMarchingCubes.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkNew.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyDataReader.h>
#include <vtkBoxWidget2.h>
#include <vtkImageProperty.h>
#include <vtkBoxRepresentation.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkNIFTIImageReader.h>
#include <vtkSTLWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOutlineFilter.h>
#include <vtkDICOMImageReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPoints.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkWorldPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkMultiVolume.h>
#include <vtkCoordinate.h>
#include <vtkImageMask.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageLogic.h>
#include <vtkMultiVolume.h>
#include <vtkImageMathematics.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)

#include "itkGDCMSeriesFileNames.h"
#include "itkVTKImageToImageFilter.h"
#include <itkImageToVTKImageFilter.h>
#include <itkImageSeriesReader.h>  
#include <itkImageSeriesWriter.h>  
#include <itkGDCMImageIO.h>
#include <itkExtractImageFilter.h>
#include <itkImage.h>
#include <itkFlipImageFilter.h>
#include <itkNumericSeriesFileNames.h>
#include <itkCropImageFilter.h>
#include <itkNiftiImageIO.h>

using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"rpcrt4.lib")

class MyvtkInteractorStyle;
typedef signed short    PixelType;
const unsigned int      Dimension = 3;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;
typedef itk::Image< PixelType, Dimension >         ImageType;
using ReaderType = itk::ImageSeriesReader< ImageType >;
using ImageIOType = itk::GDCMImageIO;
typedef itk::CropImageFilter <ImageType, ImageType> CropImageFilterType;


namespace Ui {
class PreDivideWidget;
}

struct MyPosData {
	double d[3];
};

enum selWorkType
{
	nothingToDo = 0,
	toCut = 1,
	toDaub = 2
};

class threadWaitAIexit : public QThread
{
	Q_OBJECT;
public:
	threadWaitAIexit();
	~threadWaitAIexit();
	void setHandle(HANDLE h);
signals:
	void programClosed();
protected:
	virtual void run();
private:
	HANDLE m_handle;
};

class listUserData : public QObjectUserData
{
public:
	int btnId;
	int labelId;
};


class QVTKRenderWindow : public QVTKOpenGLNativeWidget
{
	Q_OBJECT
public:
	QVTKRenderWindow()
	{
		m_renderer = vtkSmartPointer<vtkRenderer>::New();
		m_renderer->SetBackground(0, 0, 0);
		//renderWindow()->AddRenderer(m_renderer);
	}
	vtkSmartPointer<vtkRenderer> m_renderer;
	bool m_bDrawRandomArea = false;
#if VTK_MAJOR_VERSION <= 8
	vtkRenderWindow* renderWindow()
	{
		return GetRenderWindow();
	}
#endif
signals:
	void cutDataSignal(QList<QPointF> &l);
protected:
	virtual void paintGL() override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	void DrawArea(QList<QPointF> &pf, QPainter &painter);
private:
	QList<QPointF> cutting_points_;
};


class PreDivideWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreDivideWidget(QWidget *parent = nullptr);
    ~PreDivideWidget();

    QWidget* get2d3dView();
    QWidget* getCtrlView();
	void stopInteractor();
	void CutingImagedata(vtkSmartPointer<vtkImageData> image_data,
		vtkSmartPointer<vtkVolume> volume, vtkSmartPointer<vtkRenderer> renderer,
		const QPolygonF & polygon, bool cutInside);
	bool DeleteDirectory(const QString &path);
signals:
	void showVolumeDataSignal();
private slots:
    void tableBtnClicked();
    void on_PreCut_clicked();
    void on_OpenFile_clicked();
    void on_preBtnCut_clicked();
    void on_preBtnSelect_clicked();
    void on_CutBtnExport_clicked();
    void on_CutBtnRestore_clicked();
	void showVolumeDataSlot();
	void eraseBtn_clicked();
	void programClosedSlot();
	void cutDataSlot(QList<QPointF> &l);
	void SlotMenuClicked(QAction* act);
	void tableContexMenuRequested(const QPoint &pos);
	void on_DuabBtn_clicked();
	void on_CheckLabelCountBtn_clicked();
private:
	void saveRestoreData();
	void rebuildList();
	void addEmptyVolume(int idNumber);
	void addOneRowToMaskList(int idNumber);
	bool isLoadDicom = false;
	bool isShowNii = false;
	threadWaitAIexit waitThread;
    Ui::PreDivideWidget *ui;
    map<int, vector<QPushButton*>> m_bt;
	QVTKRenderWindow m_3DViewWidget;
	QVTKRenderWindow m_2DViewWidget[3];
	QProcess myProcess;
	QMenu* m_pContextMenu;
	QAction* m_pActionDel;
	QAction* m_pActionMerge;
	QMenu* m_pSaveMenu;
	QAction* m_pActionSaveAsLeft;
	QAction* m_pActionSaveAsRight;
	QMenu* m_pAddMenu;
	QAction* m_pActionAddLabel1n;
	QAction* m_pActionAddLabel2n;
	QAction* m_pActionAddLabel3n;
	vtkSmartPointer<vtkRenderWindowInteractor> iren;
	vtkSmartPointer<MyvtkInteractorStyle> m_3dViewStyle;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor1;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor2;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3;
	vtkSmartPointer<vtkInteractorStyleImage> imagestyle1;
	vtkSmartPointer<vtkInteractorStyleImage> imagestyle2;
	vtkSmartPointer<vtkInteractorStyleImage> imagestyle3;
	vtkSmartPointer<vtkImageActor> imgActor1;
	vtkSmartPointer<vtkImageActor> imgActor2;
	vtkSmartPointer<vtkImageActor> imgActor3;
	int extent[6];
	int dimensions[3];
	double spacing[3];
	double origin[3];
	vtkSmartPointer<vtkImageReslice> reslice1;
	vtkSmartPointer<vtkImageReslice> reslice2;
	vtkSmartPointer<vtkImageReslice> reslice3;
	vtkSmartPointer<vtkBoxWidget2> boxWidget1;
	vtkSmartPointer<vtkBoxWidget2> boxWidget2;
	vtkSmartPointer<vtkBoxWidget2> boxWidget3;

	vtkSmartPointer<vtkImageData> dicom_data;
	vtkSmartPointer<vtkWorldPointPicker> m_WorldPointPicker;
	vtkSmartPointer<vtkImageData> image_data;
	vtkSmartPointer<vtkImageData> empty_data;
	map<int, vtkSmartPointer<vtkImageData>> imagedata;
	map<int, vtkSmartPointer<vtkImageData>> m_restoreData;
	map<int, vtkSmartPointer<vtkVolume>> volume;
	vtkSmartPointer<vtkMultiVolume> volumeMulti;
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper;
	map<int,vtkSmartPointer<vtkVolumeProperty>> volumeProperty;
	map<int, vtkSmartPointer<vtkPiecewiseFunction>> compositeOpacity;
	map<int, vtkSmartPointer<vtkColorTransferFunction>> color;
	map<int, unsigned char*> m_pDataPointer;
	map<int, bool> m_labelVisible;
	map<int, QColor> m_labelColor;
	vector<int> m_removedRow;
	vector<int> m_removedComboList;
	int m_curOutValue = 100;
	QList<vtkSmartPointer<vtkImageStencilToImage>> m_undoData;
	set<int> m_MaxDataDimension;
	int m_iMaxDataDimension = 0;
	selWorkType m_curSelWorkType;
	double m_direction[3];
	ReaderType::Pointer reader = ReaderType::New();
	CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();
};



class vtkImageInteractionCallback : public vtkCommand
{
public:
	static vtkImageInteractionCallback *New()
	{
		return new vtkImageInteractionCallback;
	}

	vtkImageInteractionCallback()
	{
		this->Slicing = 0;
	}

	void SetImageReslice(vtkSmartPointer<vtkImageReslice> reslice)
	{
		this->ImageReslice = reslice;
	}

	void SetImageMapToColors(vtkSmartPointer<vtkImageMapToColors> mapToColors)
	{
		this->mapToColors = mapToColors;
	}

	vtkSmartPointer<vtkImageReslice> GetImageReslice()
	{
		return this->ImageReslice;
	}

	void SetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor)
	{
		this->Interactor = interactor;
	}

	vtkSmartPointer<vtkRenderWindowInteractor> GetInteractor()
	{
		return this->Interactor;
	}

	virtual void Execute(vtkObject *, unsigned long event, void *)
	{
		vtkSmartPointer<vtkRenderWindowInteractor> interactor = this->GetInteractor();
		mapToColors->Update();
		if (event == vtkCommand::MouseWheelForwardEvent)
		{
			vtkSmartPointer<vtkImageReslice> reslice = this->ImageReslice;
			reslice->Update();
			double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
			vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
			// move the center point that we are slicing through
			double point[4];
			double center[4];
			point[0] = 0.0;
			point[1] = 0.0;
			point[2] = sliceSpacing;
			point[3] = 1.0;
			matrix->MultiplyPoint(point, center);
			matrix->SetElement(0, 3, center[0]);
			matrix->SetElement(1, 3, center[1]);
			matrix->SetElement(2, 3, center[2]);
			interactor->Render();
		}
		else if (event == vtkCommand::MouseWheelBackwardEvent)
		{
			vtkSmartPointer<vtkImageReslice> reslice = this->ImageReslice;
			reslice->Update();
			double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
			vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
			// move the center point that we are slicing through
			double point[4];
			double center[4];
			point[0] = 0.0;
			point[1] = 0.0;
			point[2] = -sliceSpacing;
			point[3] = 1.0;
			matrix->MultiplyPoint(point, center);
			matrix->SetElement(0, 3, center[0]);
			matrix->SetElement(1, 3, center[1]);
			matrix->SetElement(2, 3, center[2]);
			interactor->Render();
		}
	}

private:
	int Slicing;
	vtkSmartPointer<vtkImageReslice> ImageReslice;
	vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
	vtkSmartPointer<vtkImageMapToColors> mapToColors;
};


class vtkBoxCallback : public vtkCommand {
public:
	static vtkBoxCallback* New() {
		return new vtkBoxCallback;
	}

	vtkSmartPointer<vtkBoxWidget2> box1;
	vtkSmartPointer<vtkBoxWidget2> box2;
	vtkSmartPointer<vtkBoxWidget2> box3;
	double m_direction[3];

	virtual void Execute(vtkObject* caller, unsigned long l, void* p) {
		vtkSmartPointer<vtkBoxWidget2> boxWidget = dynamic_cast<vtkBoxWidget2*>(caller);
		if(boxWidget == box1)
		{
			//横断面Y方向移动，矢状面X跟随移动。横断面X方向移动，冠状面X跟随移动
			double bArray[6], b2[6], b3[6];
			auto b = dynamic_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation())->GetBounds();
			memcpy(bArray, b, sizeof(double) * 6);
			memcpy(b2, dynamic_cast<vtkBoxRepresentation*>(box2->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			memcpy(b3, dynamic_cast<vtkBoxRepresentation*>(box3->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			//b2[0] = bArray[2];
			//b2[1] = bArray[3];
			b2[0] = -bArray[3];
			b2[1] = -bArray[2];
			b3[0] = bArray[0];
			b3[1] = bArray[1];
			dynamic_cast<vtkBoxRepresentation*>(box2->GetRepresentation())->PlaceWidget(b2);
			box2->GetInteractor()->Render();
			dynamic_cast<vtkBoxRepresentation*>(box3->GetRepresentation())->PlaceWidget(b3);
			box3->GetInteractor()->Render();
		}
		else if (boxWidget == box2)
		{
			//矢状面Y方向移动，冠状面Y跟随移动。矢状面X方向移动，横断面Y跟随移动
			double bArray[6], b1[6], b3[6];
			auto b = dynamic_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation())->GetBounds();
			memcpy(bArray, b, sizeof(double) * 6);
			memcpy(b1, dynamic_cast<vtkBoxRepresentation*>(box1->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			memcpy(b3, dynamic_cast<vtkBoxRepresentation*>(box3->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			//b1[2] = bArray[0];
			//b1[3] = bArray[1];
			b1[2] = -bArray[1];
			b1[3] = -bArray[0];
			b3[2] = bArray[2];
			b3[3] = bArray[3];
			dynamic_cast<vtkBoxRepresentation*>(box3->GetRepresentation())->PlaceWidget(b3);
			box3->GetInteractor()->Render();
			dynamic_cast<vtkBoxRepresentation*>(box1->GetRepresentation())->PlaceWidget(b1);
			box1->GetInteractor()->Render();
		}
		else if (boxWidget == box3)
		{
			//冠状面Y方向移动，矢状面Y跟随移动。冠状面X方向移动，横断面X跟随移动
			double bArray[6],b1[6],b2[6];
			auto b = dynamic_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation())->GetBounds();
			memcpy(bArray, b, sizeof(double) * 6);
			memcpy(b1, dynamic_cast<vtkBoxRepresentation*>(box1->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			memcpy(b2, dynamic_cast<vtkBoxRepresentation*>(box2->GetRepresentation())->GetBounds(), sizeof(double) * 6);
			b2[2] = bArray[2];
			b2[3] = bArray[3];
			b1[0] = bArray[0];
			b1[1] = bArray[1];
			dynamic_cast<vtkBoxRepresentation*>(box2->GetRepresentation())->PlaceWidget(b2);
			box2->GetInteractor()->Render();
			dynamic_cast<vtkBoxRepresentation*>(box1->GetRepresentation())->PlaceWidget(b1);
			box1->GetInteractor()->Render();
		}
	}
	vtkBoxCallback() {}
};

class drawCurveCallback : public vtkCommand
{
public:
	static drawCurveCallback* New() {
		return new drawCurveCallback;
	}
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkParametricSpline> spline =
		vtkSmartPointer<vtkParametricSpline>::New();
	vtkSmartPointer<vtkParametricFunctionSource> functionSource =
		vtkSmartPointer<vtkParametricFunctionSource>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	virtual void Execute(vtkObject *, unsigned long event, void *)
	{
		if (event == vtkCommand::MouseMoveEvent)
		{
			if (addPoint)
			{
				AddNextPointToPoints();
			}
		}
		else if (event == vtkCommand::LeftButtonReleaseEvent)
		{
			addPoint = false;
			AddNextPointToPoints();
			spline->SetPoints(points);
			functionSource->SetParametricFunction(spline);
			functionSource->Update();
			mapper->SetInputConnection(functionSource->GetOutputPort());
			actor->SetMapper(mapper);
			actor->GetProperty()->SetLineWidth(3.0);
			actor->GetProperty()->SetColor(1.0,0,0);
			render->AddActor(actor);
			Interactor->Render();
			render->Render();
		}
		else if (event == vtkCommand::LeftButtonPressEvent)
		{
			addPoint = true;
			points = vtkSmartPointer<vtkPoints>::New();
			AddNextPointToPoints();
			render->RemoveActor(actor);
		}
	}
	void SetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> i)
	{
		this->Interactor = i;
	}
	void SetRenderer(vtkSmartPointer<vtkRenderer> r)
	{
		this->render = r;
	}
private:
	bool addPoint = false;
	vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
	vtkSmartPointer<vtkRenderer> render;
	void AddNextPointToPoints()
	{
		this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);
		points->InsertNextPoint(picked);
	}
};

class MyvtkInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static MyvtkInteractorStyle *New()
	{
		return new MyvtkInteractorStyle;
	}
	bool interactive_ = true;
	void OnMouseMove()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnMouseMove();
		}
	}

	void OnLeftButtonDown()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
		}
	}

	void OnLeftButtonUp()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
		}
	}

	void OnMiddleButtonDown()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
		}
	}

	void OnMiddleButtonUp()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
		}
	}

	void OnRightButtonDown()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnRightButtonDown();
		}
	}

	void OnRightButtonUp()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnRightButtonUp();
		}
	}

	void OnMouseWheelForward()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnMouseWheelForward();
		}
	}

	void OnMouseWheelBackward()
	{
		if (interactive_) {
			vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();
		}
	}

};

#endif // PREDIVIDEWIDGET_H
