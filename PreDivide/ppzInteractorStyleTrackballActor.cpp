//ppzInteractorStyleTrackballActor.cpp
#include "ppzInteractorStyleTrackballActor.h"

#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkCallbackCommand.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkProp3D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTransform.h"
vtkStandardNewMacro(InteractorStyleTrackballActor);

//----------------------------------------------------------------------------
InteractorStyleTrackballActor::InteractorStyleTrackballActor()
	: m_isDoTranslation(false)
{
	this->MotionFactor = 10.0;
	this->InteractionProp = nullptr;
	this->InteractionPicker = vtkCellPicker::New();
	this->InteractionPicker->SetTolerance(0.001);

}

//----------------------------------------------------------------------------
InteractorStyleTrackballActor::~InteractorStyleTrackballActor()
{
	this->InteractionPicker->Delete();
}

void InteractorStyleTrackballActor::Rotate(const double x, const double y, const double z)
{

	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor* rwi = this->Interactor;
	if (!rwi)
		return;
	vtkCamera* cam = this->CurrentRenderer->GetActiveCamera();

	// First get the origin of the assembly
	double* obj_center = this->InteractionProp->GetCenter();

	// GetLength gets the length of the diagonal of the bounding box
	double boundRadius = this->InteractionProp->GetLength() * 0.5;

	// Get the view up and view right vectors
	double view_up[3], view_look[3], view_right[3];

	cam->OrthogonalizeViewUp();
	cam->ComputeViewPlaneNormal();
	cam->GetViewUp(view_up);
	vtkMath::Normalize(view_up);
	cam->GetViewPlaneNormal(view_look);
	vtkMath::Cross(view_up, view_look, view_right);
	vtkMath::Normalize(view_right);

	// Get the furtherest point from object position+origin
	double outsidept[3];

	outsidept[0] = obj_center[0] + view_right[0] * boundRadius;
	outsidept[1] = obj_center[1] + view_right[1] * boundRadius;
	outsidept[2] = obj_center[2] + view_right[2] * boundRadius;

	// Convert them to display coord
	double disp_obj_center[3];

	this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
		disp_obj_center);

	this->ComputeWorldToDisplay(outsidept[0], outsidept[1], outsidept[2],
		outsidept);

	double radius = sqrt(vtkMath::Distance2BetweenPoints(disp_obj_center,
		outsidept));
	double nxf = (rwi->GetEventPosition()[0] - disp_obj_center[0]) / radius;

	double nyf = (rwi->GetEventPosition()[1] - disp_obj_center[1]) / radius;

	double oxf = (rwi->GetLastEventPosition()[0] - disp_obj_center[0]) / radius;

	double oyf = (rwi->GetLastEventPosition()[1] - disp_obj_center[1]) / radius;

	/* if (((nxf * nxf + nyf * nyf) <= 1.0) &&
		 ((oxf * oxf + oyf * oyf) <= 1.0))*/
	{


		double scale[3];
		scale[0] = scale[1] = scale[2] = 1.0;

		double** rotate = new double*[3];

		rotate[0] = new double[4];
		rotate[1] = new double[4];
		rotate[2] = new double[4];

		rotate[0][0] = x;
		rotate[0][1] = view_up[0];
		rotate[0][2] = view_up[1];
		rotate[0][3] = view_up[2];


		rotate[1][0] = y;
		rotate[1][1] = view_right[0];
		rotate[1][2] = view_right[1];
		rotate[1][3] = view_right[2];

		rotate[2][0] = z;
		rotate[2][1] = view_look[0];
		rotate[2][2] = view_look[1];
		rotate[2][3] = view_look[2];

		this->Prop3DTransform(this->InteractionProp,
			obj_center,
			3,
			rotate,
			scale);

		delete[] rotate[0];
		delete[] rotate[1];
		delete[] rotate[2];
		delete[] rotate;

		if (this->AutoAdjustCameraClippingRange)
		{
			this->CurrentRenderer->ResetCameraClippingRange();
		}

		rwi->Render();
	}
}


void InteractorStyleTrackballActor::Pan(const double x, const double y, const double z)
{
	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor* rwi = this->Interactor;
	if (!rwi)
		return;

	// Use initial center as the origin from which to pan

	double* obj_center = this->InteractionProp->GetCenter();

	double disp_obj_center[3], new_pick_point[4];
	double old_pick_point[4], motion_vector[3];

	this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
		disp_obj_center);

	this->ComputeDisplayToWorld(
		disp_obj_center[0] + x,
		disp_obj_center[1] + y,
		disp_obj_center[2] + z,
		new_pick_point);

	this->ComputeDisplayToWorld(
		disp_obj_center[0],
		disp_obj_center[1],
		disp_obj_center[2],
		old_pick_point);


	//转为屏幕display物理坐标系的平移
	motion_vector[0] = new_pick_point[0] - old_pick_point[0];
	motion_vector[1] = new_pick_point[1] - old_pick_point[1];
	motion_vector[2] = new_pick_point[2] - old_pick_point[2];

	if (this->InteractionProp->GetUserMatrix() != nullptr)
	{
		vtkTransform* t = vtkTransform::New();
		t->PostMultiply();
		t->SetMatrix(this->InteractionProp->GetUserMatrix());
		t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
		this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
		t->Delete();
	}
	else
	{
		this->InteractionProp->AddPosition(motion_vector[0],
			motion_vector[1],
			motion_vector[2]);
	}

	if (this->AutoAdjustCameraClippingRange)
	{
		this->CurrentRenderer->ResetCameraClippingRange();
	}

	rwi->Render();
}


void InteractorStyleTrackballActor::OnChar()
{
	return;
	if (m_isDoTranslation)//平移
	{
		switch (this->Interactor->GetKeyCode())
		{
		case 'w'://上
			Pan(0, 1, 0);
			break;

		case 'a'://左
			Pan(-1, 0, 0);
			break;

		case 's'://下
			Pan(0, -1, 0);
			break;

		case 'd'://右
			Pan(1, 0, 0);
			break;

		case 'q'://前
			Pan(0, 0, 0.01);
			break;

		case 'e'://后
			Pan(0, 0, -0.01);
			break;
		}
	}

	else//旋转
	{
		switch (this->Interactor->GetKeyCode())
		{
		case 'w'://上
			Rotate(0, -3, 0);
			break;

		case 'a'://左
			Rotate(-3, 0, 0);
			break;

		case 's'://下
			Rotate(0, 3, 0);
			break;

		case 'd'://右
			Rotate(3, 0, 0);
			break;

		case 'q'://前
			Rotate(0, 0, 3);
			break;

		case 'e'://后
			Rotate(0, 0, -3);
			break;
		}
	}

}

void InteractorStyleTrackballActor::OnKeyPress()
{
	return;
	auto keyChar = this->Interactor->GetKeySym();
	const char* Control_L = "Control_L";
	auto isEqual = strcmp(keyChar, Control_L);
	if (isEqual == 0)
	{
		m_isDoTranslation = !m_isDoTranslation;
		if (m_isDoTranslation)
		{
			std::cout << "translation" << std::endl;
		}
		else
		{
			std::cout << "rotation" << std::endl;
		}
	}
}

//----------------------------------------------------------------------------
void InteractorStyleTrackballActor::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}

void InteractorStyleTrackballActor::OnLeftButtonDown()
{
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	this->FindPickedActor(x, y);

	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

//----------------------------------------------------------------------------
void InteractorStyleTrackballActor::FindPickedActor(int x, int y)
{
	this->InteractionPicker->Pick(x, y, 0.0, this->CurrentRenderer);
	vtkProp* prop = this->InteractionPicker->GetViewProp();
	if (prop != nullptr)
	{
		this->InteractionProp = vtkProp3D::SafeDownCast(prop);
	}
	else
	{
		this->InteractionProp = nullptr;
	}
}

//----------------------------------------------------------------------------
void InteractorStyleTrackballActor::Prop3DTransform(vtkProp3D* prop3D,
	double* boxCenter,
	int numRotation,
	double** rotate,
	double* scale)
{
	vtkMatrix4x4* oldMatrix = vtkMatrix4x4::New();
	prop3D->GetMatrix(oldMatrix);

	double orig[3];
	prop3D->GetOrigin(orig);

	vtkTransform* newTransform = vtkTransform::New();
	newTransform->PostMultiply();
	if (prop3D->GetUserMatrix() != nullptr)
	{
		newTransform->SetMatrix(prop3D->GetUserMatrix());
	}
	else
	{
		newTransform->SetMatrix(oldMatrix);
	}

	newTransform->Translate(-(boxCenter[0]), -(boxCenter[1]), -(boxCenter[2]));

	for (int i = 0; i < numRotation; i++)
	{
		newTransform->RotateWXYZ(rotate[i][0], rotate[i][1],
			rotate[i][2], rotate[i][3]);
	}

	if ((scale[0] * scale[1] * scale[2]) != 0.0)
	{
		newTransform->Scale(scale[0], scale[1], scale[2]);
	}

	newTransform->Translate(boxCenter[0], boxCenter[1], boxCenter[2]);

	// now try to get the composite of translate, rotate, and scale
	newTransform->Translate(-(orig[0]), -(orig[1]), -(orig[2]));
	newTransform->PreMultiply();
	newTransform->Translate(orig[0], orig[1], orig[2]);

	if (prop3D->GetUserMatrix() != nullptr)
	{
		newTransform->GetMatrix(prop3D->GetUserMatrix());
	}
	else
	{
		prop3D->SetPosition(newTransform->GetPosition());
		prop3D->SetScale(newTransform->GetScale());
		prop3D->SetOrientation(newTransform->GetOrientation());
	}
	oldMatrix->Delete();
	newTransform->Delete();
}

