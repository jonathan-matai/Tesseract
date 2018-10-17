//	Diese Datei ist Teil der Tesseract-Engine
//	Phoenix (c) 2017
//	 ___________________           ____________
//	|_______    ________|         |    ________|
//			|  |				  |   |
//			|  |				  |   |
//			|  |	 _________    |   |_____
//			|  |	/	____  \   |    _____|
//			|  |	|  |____|  |  |   | 
//			|  |	|   _______|  |   |
//			|  |	|  |_______   |   |________ 
//			|__|	\_________/   |____________|
//			

//Includes
#include "..\include\TECamera.hpp"

teResult TECamera::teInit()
{
	//Alle Membervars initen
	m_camLookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_camPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_camRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_camUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

	teUpdateView();

	return true;
}

teResult TECamera::teSetLookAtLenght(float length)
{
	//LookAt normalisieren und dann strecken
	XMVECTOR v = XMLoadFloat3(&m_camLookAt);
	XMVector3Normalize(v);

	XMVectorScale(v, length);
	XMStoreFloat3(&m_camLookAt, v);

	return true;
}


void TECamera::teSetLens(float fovY, float aspect, float zn, float zf)
{
	m_fovY = fovY;
	m_aspect = aspect;
	m_zF = zf;
	m_zN = zn;

	m_windowHeightN = 2.0f * m_zN * tanf(0.5f*m_fovY);
	m_windowHeightF = 2.0f * m_zF * tanf(0.5f*m_fovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_zN, m_zF);
	XMStoreFloat4x4(&m_proj, P);
}

float TECamera::teGetFOVX() const
{
	float halfWidth = 0.5f*teGetWindowWidthN();
	return 2.0f*atan(halfWidth / m_zN);
}

void TECamera::teDefCam(const XMFLOAT3 & pos, const XMFLOAT3 & target, const XMFLOAT3 & worldUp)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&worldUp);

	teDefCam(P, T, U);
}

void TECamera::teDefCam(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&m_camPos, pos);
	XMStoreFloat3(&m_camLookAt, L);
	XMStoreFloat3(&m_camRight, R);
	XMStoreFloat3(&m_camUp, U);
}

void TECamera::teStrafe(float d)
{
	//m_camPos += d*m_camRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_camRight);
	XMVECTOR p = XMLoadFloat3(&m_camPos);
	XMStoreFloat3(&m_camPos, XMVectorMultiplyAdd(s, r, p));
}

void TECamera::teWalk(float d)
{
	//m_camPos += d*m_camLookAt
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&m_camLookAt);
	XMVECTOR p = XMLoadFloat3(&m_camPos);
	XMStoreFloat3(&m_camPos, XMVectorMultiplyAdd(s, l, p));
}

void TECamera::tePitch(float angle)
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_camRight), angle);

	XMStoreFloat3(&m_camUp, XMVector3TransformNormal(XMLoadFloat3(&m_camUp), R));
	XMStoreFloat3(&m_camLookAt, XMVector3TransformNormal(XMLoadFloat3(&m_camLookAt), R));
}

void TECamera::teRotateY(float angle)
{
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_camRight, XMVector3TransformNormal(XMLoadFloat3(&m_camRight), R));
	XMStoreFloat3(&m_camUp, XMVector3TransformNormal(XMLoadFloat3(&m_camUp), R));
	XMStoreFloat3(&m_camLookAt, XMVector3TransformNormal(XMLoadFloat3(&m_camLookAt), R));
}

void TECamera::teUpdateView()
{
	XMVECTOR R = XMLoadFloat3(&m_camRight);
	XMVECTOR U = XMLoadFloat3(&m_camUp);
	XMVECTOR L = XMLoadFloat3(&m_camLookAt);
	XMVECTOR P = XMLoadFloat3(&m_camPos);

	//LookAt Vector in UnitLength konvertieren
	L = XMVector3Normalize(L);

	//Neuen Up Vector berechenen und Normalisieren
	U = XMVector3Normalize(XMVector3Cross(L, R));

	//Neuen Rightvector berechnen
	R = XMVector3Cross(U, L);

	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&m_camRight, R);
	XMStoreFloat3(&m_camLookAt, L);
	XMStoreFloat3(&m_camUp, U);

	m_view(0, 0) = m_camRight.x;
	m_view(1, 0) = m_camRight.y;
	m_view(2, 0) = m_camRight.z;
	m_view(3, 0) = x;

	m_view(0, 1) = m_camUp.x;
	m_view(1, 1) = m_camUp.y;
	m_view(2, 1) = m_camUp.z;
	m_view(3, 1) = y;

	m_view(0, 2) = m_camLookAt.x;
	m_view(1, 2) = m_camLookAt.y;
	m_view(2, 2) = m_camLookAt.z;
	m_view(3, 2) = z;

	m_view(0, 3) = 0.0f;
	m_view(1, 3) = 0.0f;
	m_view(2, 3) = 0.0f;
	m_view(3, 3) = 1.0f;
}
