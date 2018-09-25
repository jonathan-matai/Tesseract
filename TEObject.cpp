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

//Inlcudes
#include "TEObject.hpp"

TEObject::TEObject(TE_OBJECT_DESC & init) : m_ID(init.type), m_subID(init.subType)
{
	m_pos = init.pos;
	m_rotation = init.rotation;
	m_scale = init.scale;
	m_world = XMMatrixIdentity();
}

TEObject::~TEObject()
{
}

void TEObject::render()
{
	
}

void TEObject::tick(double dt)
{
	
}
