#pragma once

#include "vVector.hpp"

typedef class vPlane
{
	private:

		vVector n;
		float d;
		vPoint center, geometricalCenter;

	public:

		vPlane();
		vPlane(vPoint a, vPoint b, vPoint c);
		vPlane(vPoint * abc, int stride = 0);
		vPlane(vVector v1, vVector v2);

		vVector getNormal() { return n; }
		float getCheckValue() { return d; }

		void recalculatePlane(vPoint * abc);
		void recalculatePlane(vPoint a, vPoint b, vPoint c);
		void recalculatePlaneA(vPoint a);
		void recalculatePlaneB(vPoint b);
		void recalculatePlaneC(vPoint c);

		vPoint getCenter() { return center; };
		vPoint getGeometricalCenter() { return geometricalCenter; };

}vPlane, *pvPlane, **lpvPlane;
