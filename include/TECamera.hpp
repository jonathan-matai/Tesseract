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

#pragma once

//Includes
#include "TEGlobal.hpp"

class TECamera
{
	friend class TEGraphics;

	public:

		//Initialisiert die Kamera
		teResult teInit();

		//Setzt die Länge des LookAt-Vektors der Kamera, was einen gewissen Zoom (abhängig von der Länge) zur Folge hat
		teResult teSetLookAtLenght(float length);

		//Kamera spezifische Funktionen
		XMVECTOR teGetCamPosXM()const { return XMLoadFloat3(&m_camPos); };
		XMFLOAT3 teGetCamPos()const { return m_camPos; };
		void teSetCamPos(float x, float y, float z) { m_camPos = XMFLOAT3(x, y, z); };
		void teSetCamPos(const XMFLOAT3 &v) { m_camPos = v; };

		XMVECTOR teGetRightXM()const { return XMLoadFloat3(&m_camRight); };
		XMFLOAT3 teGetRight()const { return m_camRight; };
		XMVECTOR teGetUpXM()const { return XMLoadFloat3(&m_camUp); };
		XMFLOAT3 teGetUp()const { return m_camUp; };
		XMVECTOR teGetLookAtXM()const { return XMLoadFloat3(&m_camLookAt); };
		XMFLOAT3 teGetLookAt()const { return m_camLookAt; };

		XMMATRIX teGetViewMatrix()const { return XMMATRIX(XMLoadFloat4x4(&m_view)); };
		XMMATRIX teGetProjectionMatrix()const { return XMMATRIX(XMLoadFloat4x4(&m_proj)); };

		float teGetNearZ()const { return m_zN; };
		float teGetFarZ()const { return m_zF; };
		float teGetAspect()const { return m_aspect; };
		float teGetFOVX()const;
		float teGetFOVY()const { return m_fovY; };

		float teGetWindowWidthN()const { return m_aspect * m_windowHeightN; };
		float teGetWindowHeightN()const { return m_windowHeightN; };
		float teGetWindowWidthF()const { return m_aspect * m_windowHeightF; };
		float teGetWindowHeightF()const { return m_windowHeightF; };

		//Die Rendersceneabmessungen festlegen
		void teSetLens(float fovY, float aspect, float zn, float zf);

		//Die Kamera definieren
		void teDefCam(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
		void teDefCam(const XMFLOAT3 &pos, const XMFLOAT3 &target, const XMFLOAT3 &worldUp);

		//Strafen/Gehen(Strecke d)
		void teStrafe(float d);
		void teWalk(float d);

		//Die Kamera rotieren
		void tePitch(float angle);
		void teRotateY(float angle);

		//Die ViewMatrix rebuilden, nachdem Kamerapos und Ausrichtung verändert wurde jedes Frame aufrufen
		void teUpdateView();

	private:

		//Kameravariablen
		XMFLOAT4X4 m_view, m_proj;

		XMFLOAT3 m_camPos;
		XMFLOAT3 m_camRight;
		XMFLOAT3 m_camUp;
		XMFLOAT3 m_camLookAt;

		float m_zN, m_zF, m_aspect, m_fovY, m_windowHeightN, m_windowHeightF;
};
