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
#pragma warning(disable:4005 4838 4477 4018 4267)

//Alle Includes, die in der Engine global verteilt werden sollen
//DX 11
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <DXGI.h>
#include <XAudio2.h>
#include <XAudio2fx.h>
#include <X3DAudio.h>
#include <DirectXMath.h>
#include <dsound.h>
#include <d3dx11effect.h>
#include <d3dxGlobal.h>
//Windows-spezifische
#include <Windows.h>
#include <vector>
#include <list>
#include <fstream>
#include <Psapi.h>
#include <string>
#include <sstream>
#include <stdlib.h>
//Engine-spezifische
#include "Logfile.hpp"
#include "FileLoader.hpp"
#include "MemoryManager.hpp"
#include "MathHelper.hpp"

//Namespace
using namespace DirectX;

//Rückgabetyp der Engine (false = Fehler; true = Alles OK)
typedef bool teResult;

// Utility-Funktionen
#define TE_MIN(a, b){(a) < (b) ? (a) : (b)}
#define TE_MAX(a, b){(a) > (b) ? (a) : (b)}
#define TE_SAFE_RELEASE(x){if((x)){(x)->Release();(x)=NULL;}}
#define TE_SAFE_DELETE_ARRAY(x){if((x)){delete[](x);(x)=NULL;}}
#define TE_SAFE_MEMFREE(x){teFreeMemory(x); (x) = NULL;}

//Andere Defines
#define TE_OK_ERROR (MB_OK | MB_ICONERROR)
#define TE_VERSION ("0.0.26a")
#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

//DX-Debugmode
#if defined(DEBUG) || defined(_DEBUG)
#define TE_DEBUG
#endif

// Casting-Methode mit Runtime-Fehlerabfrage und -ausgabe
template <class c, class d> c * te_cast(d* in) {
	if (c* out = dynamic_cast<c*>(in))
		return value;
	else {
		char msg[128];
		sprintf_s(msg, "Cannot convert %s to %s!", typeid(d).name(), typeid(c).name());
		TRACE(true, msg);
	}
	return NULL;
}

//char* in wchar_t* =========VORSICHT: Nach Aufruf muss die Vaiable wc wieder gelöscht werden: delete[] wc;=================
static const wchar_t * teCharToWchar(const char * c)
{
	//size_t cSize = strlen(c) + 1;
	//wchar_t* wc = new wchar_t[cSize];
	////mbstowcs(wc, c, cSize);
	//
	//size_t size = NULL;
	//
	//mbstowcs_s(&size, wc, size, c, cSize - 1);
	//
	//return wc;

	wchar_t * wc = new wchar_t();

	mbtowc(&wc[0], c, sizeof(c));

	return wc;
}

static XMVECTOR setLength(XMVECTOR toScale, float length) { return length * toScale; }

static XMMATRIX inverseTranspose(CXMMATRIX matrix)
{
	XMMATRIX A = matrix;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

// Display-Einstellungen
#define MAX_XRES (1980)
#define MAX_MULTI_SAMPLE (4)

//Die Parameterstruktur, die ausgefüllt werden muss, um TE zu initialisieren
typedef struct TE_INIT
{
	wchar_t * windowName; //Der Name, der in der Fenstermenuleiste angezeigt werden soll
	HINSTANCE hInstance; //Die Instance, die als Parameter in der WinMain Fkt. übergeben wird
	wchar_t * shaderFiles; //Die Shaderdatei, die die Effekte und Shader enthält
	wchar_t * iniFile; //Die Inidatei mit allen Infromationen zur Engineinitialisierung
}TE_INIT;

//Globale und in der ganzen Engine gültige Vertexstruktur, die sowohl für die Objekte als auch in der Graphicsengine für das InputLayout verwendet wird
typedef struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texcoord;
}Vertex;

//globale materialstruktur, zur lichtsimulation
typedef struct LightMaterial
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 reflect;
}LightMaterial;

typedef struct DirectionalLight
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT3 direction;
	float pad;
}DirectionalLight;

typedef struct PointLight
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT3 position;
	float range;

	XMFLOAT3 att;
}PointLight;

typedef struct SpotLight
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT3 position;
	float range;
	XMFLOAT3 direction;
	float spot;
	XMFLOAT3 att;
	float pad;
}SpotLight;

//Struktur, die die wichtigsten Informationen über die Kamera enthält
typedef struct QUICK_CAM
{
	XMVECTOR pos; //Position der Kamera
	XMVECTOR lookAt; //Wohin die Kamera guckt
	XMVECTOR right; //Vektor der die x Achse der Camera definiert
	XMVECTOR up; //Vektor der die y Achse der Camera definiert
	float fovx; //Den Winkel, den die Kamera in x-Richtung sieht
	float fovy; //Den Winkel, den die Kamera in y-Richtung sieht
	float farplane; //Die entfernte Clipebene
	float nearplane; //Die nahe Clipebene
}QUICK_CAM;

//========================================================
//========================================================
//=============GLOBALS====================================
