#include "..\include\Vampire.hpp"

Vampire::Vampire()
{
	
}

Vampire::~Vampire()
{
}

bool Vampire::vInit()
{
	return true;
}

bool Vampire::vExit()
{
	return true;
}

float Vampire::vDotBetweenPlaneAndPoint(vPlane plane, vPoint point)
{
	return vVector::vDot(plane.getNormal(), point) - plane.getCheckValue();
}

bool Vampire::vIsPointInBox(vBox hitBox, vPoint toCheck)
{
	//if (!vVector::vValidate(hitBox.getBoxElement(0).getNormal()))
	//{
	//	return false;
	//}

	for (int i = 0; i < hitBox.getUsedPlanes(); ++i)
	{
		if (vDotBetweenPlaneAndPoint(hitBox.getBoxElement(i), toCheck) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Vampire::vCheckCollision(vBox box1, vBox box2)
{
	return false;
}


