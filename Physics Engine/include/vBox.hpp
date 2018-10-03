#pragma once

#include "vPlane.hpp"

#define RIGHT_BORDER (0)
#define LEFT_BORDER (1)
#define UPPER_BORDER (2)
#define LOWER_BORDER (3)
#define FAR_BORDER (4)
#define NEAR_BORDER (5)

//class to define a hitbox e.g.
//the plane array should be in the following order:
//1st element right border
//2nd element left border
//3rd element upper
//4th lower
//5th far
//6th near
//for the initialization process better use the defined makros for making sure the elements are in the right order and
//to access the array after initialization, please do the same
class vBox
{
	private:

		//vPlane rightPlane, leftPlane, upperPlane, lowerPlane, farPlane, nearPlane ==== ORDER OF THE PLANESARRAY
		std::vector<vPlane> m_box;
		std::vector<vPlane>::iterator m_boxIterator;
		int usedPlanes;
		vPoint center;
		float cx, cy, cz; //values indicating the distance from center to positive x,y,z plane

	public:

		vBox(); //creating a box with standard parameters
		vBox(pvPlane newbox); //creating a box from an array of planes
		vBox(vPoint * newbox); //creating a box from an array of points
		vBox(vVector * newbox); //creating a box from an array of vectors

		vPlane getBoxElement(int planeID); //returns the plane element at the given position
		int getUsedPlanes() { return usedPlanes; } //returns the number of planes the box consits of

		void defineBox(); //defining the box with standard parameters
		void defineBox(pvPlane newbox, int planeElements = 6); //defining the box from a new array of planes
		void defineBox(vPoint * newbox, int pointElements = 18); //defining the box from a new array of points
		void defineBox(vVector * newbox); //defining the box from a new array of vectors

		void recalculateBox(pvPlane newbox); //just create a box with completely new planes, but keep the instance of your box
		void recalculateBox(vPlane newbox, int planeID); //recreate the box with a change in just one plane
		void recalculateBox(vPoint * newplane, int planeID); //recreate the box with a change in just one plane, but describe the plane with points
		void recalculateBox(vVector newbox, int planeID); //recreate the box with a change in just one plane, but describe the plane with vectors

		void changeBoxDimension(pvPlane newbox, int dimension); //just add one plane and make the box bigger
		void changeBoxDimension(int planeToRemove); //take one plane away; make the box smaller

		int optimizeBox();

		~vBox();
};