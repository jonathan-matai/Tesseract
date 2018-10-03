#pragma once

#include "VGlobal.hpp"
#include "vPoint.hpp"
#include "vVector.hpp"
#include "vPlane.hpp"
#include "vBox.hpp"

class Vampire
{
	public:

		Vampire();
		~Vampire();

		bool vInit();
		bool vExit();

		static float vDotBetweenPlaneAndPoint(vPlane plane, vPoint point);
		static bool vIsPointInBox(vBox hitBox, vPoint toCheck);
		static bool vCheckCollision(vBox box1, vBox box2);

	private:
};
