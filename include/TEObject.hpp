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
#include "G:\C++\Phoenix\Tesseract\TEEngine\include\TEGlobal.hpp"

using namespace DirectX;

//Typ des Objekts
typedef enum TE_OBJECT_TYPE {TE_OBJECT_TYPE_PLAYER, 
							 TE_OBJECT_TYPE_NPC, 
							 TE_OBJECT_TYPE_LIGHT, 
							 TE_OBJECT_TYPE_SOUND, 
							 TE_OBJECT_TYPE_PARTICLE, 
							 TE_OBJECT_TYPE_MAP, 
							 TE_OBJECT_TYPE_OBJECT} TE_OBJECT_TYPE;
						    
//Initialisierungsstruktur für das Objekt
typedef struct TE_OBJECT_DESC
{
	XMFLOAT3 pos; //Position des neu zu erzeugnenden Objekts
	XMFLOAT3 scale; //Skalierung des neu zu erzeugenden Objekts
	XMFLOAT3 rotation;//Rotation des Objekts als Normalisierter Vektor
	wchar_t * file1, *file2; //Die Datei, in der die Informationen zu dem Objekt liegen (Je nachdem ist es eine .thm-Datei oder .tem-Datei usw.) === file2 ist optional (Beim Map Objekt ist file1 .thm; file2 .tbm)
	TE_OBJECT_TYPE type;
	UINT subType; //Der Typ des Objekts
}TE_OBJECT_DESC;

//Die Datenstruktur, die alle gerenderten Objekte gemeinsam haben
typedef struct teData
{
	Vertex * vertices; //Die Vertizen des Objekts
	UINT * indices; //Die Indizen des Objekts
	UINT numVertices, numInidices; //Anzahl an Vertizen bzw. Indizen des Objekts
}teData;

class TEObject
{
	public:

		//Virtuelle Erzeugungsfunktion des Objekts
		TEObject(TE_OBJECT_DESC & init);
		virtual ~TEObject();

		//Virtuelle Renderfunktion des Objekts
		virtual void render();

		//Virtuelle Updatefunktion des Objekts
		virtual void tick(double dt);

		//Getter & Setter
		XMFLOAT3 getPostion() { return m_pos; }
		XMFLOAT3 getRotation() { return m_rotation; }
		XMFLOAT3 getDimension() { return m_scale; }
		UINT getID() { return m_ID; }
		UINT getSubID() { return m_subID; }
		XMMATRIX teGetWorldMatrix() { return m_world; }

		//Position des Objekts
		XMFLOAT3 m_pos;

		//Skalierung des Objekts
		XMFLOAT3 m_scale;

		//Rotation des Objekts
		XMFLOAT3 m_rotation;

		//ID des Objekts (ObjectType)
		const UINT m_ID, m_subID;

		//Die Worldmatrix des Objekts
		XMMATRIX m_world;
};