
//ppzInteractorStyleTrackballActor.h

#ifndef PPZ_INTERACTORSTYLETRACKBALLACTOR_H
#define PPZ_INTERACTORSTYLETRACKBALLACTOR_H

#include <vtkInteractionStyleModule.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkSmartPointer.h>

#include <vtkAutoInit.h>

class vtkCellPicker;

class InteractorStyleTrackballActor : public vtkInteractorStyleTrackballCamera
{
public:
	static InteractorStyleTrackballActor* New();
	vtkTypeMacro(InteractorStyleTrackballActor, vtkInteractorStyleTrackballCamera);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnLeftButtonDown() override;

	void Rotate(const double x, const double y, const double z);//Ðý×ª ÑØÆÁÄ»

	void Pan(const double x, const double y, const double z);//Æ½ÒÆ ÑØÆÁÄ»

	void OnChar() override;
	void OnKeyPress() override;


protected:
	InteractorStyleTrackballActor();
	~InteractorStyleTrackballActor() override;

	void FindPickedActor(int x, int y);

	void Prop3DTransform(vtkProp3D* prop3D,
		double* boxCenter,
		int NumRotation,
		double** rotate,
		double* scale);

	double MotionFactor;

	vtkProp3D* InteractionProp;
	vtkCellPicker* InteractionPicker;

private:
	InteractorStyleTrackballActor(const InteractorStyleTrackballActor&) = delete;
	void operator=(const InteractorStyleTrackballActor&) = delete;

	bool m_isDoTranslation;

};

#endif

