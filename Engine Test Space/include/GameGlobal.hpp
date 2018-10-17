#pragma once

//Includes
#include <../../Physics Engine/include/Vampire.hpp>
#include <../../include/TEEngine.hpp>

//Defines
#define GRAPHICS (ENGINE->teGetGraphics())

//Usings
using namespace DirectX;

static vPoint xmTovPoint(XMVECTOR pos)
{
	vPoint p;

	p.setPosition(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);

	return p;
}

static vPoint xmTovPoint(XMFLOAT3 pos)
{
	vPoint p;

	p.setPosition(pos.x, pos.y, pos.z);

	return p;
}

static XMFLOAT3 vPointToXM(vPoint pos)
{
	XMFLOAT3 p;

	p.x = pos.getX();
	p.y = pos.getY();
	p.z = pos.getZ();

	return p;
}

static XMFLOAT3 xmvectorToXmfloat3(XMVECTOR v)
{
	return XMFLOAT3(v.m128_f32[0], v.m128_f32[1], v.m128_f32[2]);
}

static XMVECTOR rotateXM(FXMVECTOR toRotate, FXMVECTOR axis1, float angle1, FXMVECTOR axis2 = { 0 }, float angle2 = 0)
{
	XMMATRIX rot1 = XMMatrixRotationAxis(axis1, angle1);

	if(angle2 == 0)
	{
		return XMVector3Transform(toRotate, rot1);
	}

	XMMATRIX rot2 = XMMatrixRotationAxis(axis2, angle2);

	XMMATRIX rotres = XMMatrixMultiply(rot1, rot2);

	return XMVector3Transform(toRotate, rotres);
}

static bool xmAreEqual(XMFLOAT2 v1, XMFLOAT2 v2)
{
	if (v1.x == v2.x)
	{
		if (v1.y == v2.y)
		{
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

//get vector from param 1 to param 2
static XMVECTOR vectorBetweenPoints(XMFLOAT3 p1, XMFLOAT3 p2)
{
	XMVECTOR v;

	v.m128_f32[0] = p2.x - p1.x;
	v.m128_f32[1] = p2.y - p1.y;
	v.m128_f32[2] = p2.z - p1.z;

	return v;
}

static XMVECTOR nullVector()
{
	return XMVECTOR({ 0.0f, 0.0f, 0.0f, 0.0f });
}

static XMVECTOR averageVector(XMVECTOR v1, XMVECTOR v2, XMVECTOR v3 = { 0.0f, 0.0f, 0.0f, 0.0f }, XMVECTOR v4 = { 0.0f, 0.0f, 0.0f, 0.0f })
{
	int usedParams;
	if (XMVector3Equal(v3, nullVector()))
	{
		usedParams = 2;
	}
	else if (XMVector3Equal(v4, nullVector()))
	{
		usedParams = 3;
	}
	else
		usedParams = 4;

	float x, y, z;

	x = v1.m128_f32[0] + v2.m128_f32[0] + v3.m128_f32[0] + v4.m128_f32[0];
	x /= usedParams;

	y = v1.m128_f32[1] + v2.m128_f32[1] + v3.m128_f32[1] + v4.m128_f32[1];
	y /= usedParams;

	z = v1.m128_f32[2] + v2.m128_f32[2] + v3.m128_f32[2] + v4.m128_f32[2];
	z /= usedParams;

	return XMVECTOR({ x, y, z, 0.0f });
}

static vBox * calculateViewFrustum()
{
	vBox * frustum = (vBox*)teAlloc(sizeof(vBox));

	vPlane * box = (vPlane*)teAlloc(sizeof(vPlane) * 5);

	vPoint a, b, c;

	QUICK_CAM quickcam = ENGINE->teGetQuickCam();

	//Punkte für rechte Clippingebene festlegen
	a = xmTovPoint(quickcam.pos);

	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[0].recalculatePlane(a, b, c);

	//Punkte für linke Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[1].recalculatePlane(a, b, c);

	//Punkte für obere Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[2].recalculatePlane(a, b, c);

	//Punkte für untere Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[3].recalculatePlane(a, b, c);

	//Punkte für ferne Clippingebene festlegen
	a = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	a += xmTovPoint(quickcam.pos);

	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += xmTovPoint(quickcam.pos);

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += xmTovPoint(quickcam.pos);

	box[4].recalculatePlane(a, b, c);

	frustum->defineBox(box, 5);

	teFreeMemory((void*)box);

	return frustum;
}