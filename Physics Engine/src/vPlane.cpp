#include "..\include\vPlane.hpp"

vPlane::vPlane() :n(0.0f, 1.0f, 0.0f), d(0.0f)
{
	center = { 0.0f, 0.0f, 0.0f };
	geometricalCenter = { 0.0f, 0.0f, 0.0f };
}

vPlane::vPlane(vPoint a, vPoint b, vPoint c)
{
	vVector * ab = new vVector(a, b);
	vVector * ac = new vVector(a, c);

	this->n = vVector::normalize(vVector::vCross(ab, ac));
	this->d = vVector::vDot(this->n, a);

	center = { (ac->getX() - ab->getX()), (ac->getY() - ab->getY()), (ac->getZ() - ab->getZ())};
	geometricalCenter = {((a.getX()+b.getX()+c.getX()) / 3), ((a.getY()+b.getY()+c.getY())/3), ((a.getZ()+b.getZ()+c.getZ())/3)};
}

vPlane::vPlane(vPoint * abc, int stride)
{
	vVector * ab = new vVector(abc[0 + stride], abc[1 + stride]);
	vVector * ac = new vVector(abc[0 + stride], abc[2 + stride]);

	this->n = vVector::normalize(vVector::vCross(ab, ac));
	this->d = vVector::vDot(this->n, abc[0]);
}

vPlane::vPlane(vVector v1, vVector v2)
{
	this->n = vVector::normalize(vVector::vCross(v1, v2));
	this->d = vVector::vDot(this->n, v1);
}

void vPlane::recalculatePlane(vPoint * abc)
{
	vVector * ab = new vVector(abc[0], abc[1]);
	vVector * ac = new vVector(abc[0], abc[2]);

	this->n = vVector::normalize(vVector::vCross(ab, ac));
	this->d = vVector::vDot(this->n, abc[0]);
}

void vPlane::recalculatePlane(vPoint a, vPoint b, vPoint c)
{
	vVector * ab = new vVector(a, b);
	vVector * ac = new vVector(a, c);

	this->n = vVector::normalize(vVector::vCross(ab, ac));
	this->d = vVector::vDot(this->n, a);
}

void vPlane::recalculatePlaneA(vPoint a)
{

}

void vPlane::recalculatePlaneB(vPoint b)
{

}

void vPlane::recalculatePlaneC(vPoint c)
{

}
