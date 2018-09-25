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
#include <vector>
#include <fstream>
#include <DirectXMath.h>

using std::vector;
using namespace DirectX;

struct corner {
	XMFLOAT3 vertex;
	XMFLOAT2 texpoint;
	XMFLOAT3 normal;
};

struct Material {
	char name[64];
	float ns;
	XMFLOAT3 ka;
	XMFLOAT3 kd;
	XMFLOAT3 ks;
	XMFLOAT3 ke;
	float ni;
	float d;
	unsigned int illum;
	char map_ka[64];
	char map_kd[64];
};

struct Object {
	char name[64];
	std::vector<corner> vertexList;
	std::vector<unsigned int> indexList;
	std::vector<Material> materialList;
	std::vector<DirectX::XMUINT2> useMaterial;
};

static bool teLoadTEM(char* path, vector<Object>* objects)
{
	Object* o = NULL;

	std::ifstream istr(path, std::ios::binary);
	if (!istr.is_open())
		return false;

	uint8_t type;
	unsigned int size;
	while (true) {
		type = 5;
		istr.read((char*)&type, sizeof(uint8_t));
		switch (type) {
		case 0:
			if (o)
				objects->push_back(*o);
			o = new Object();
			istr.read((char*)&size, sizeof(int));
			istr.read(o->name, size);
			break;
		case 1:
			istr.read((char*)&size, sizeof(int));
			for (unsigned int i = 0; i < size; ++i) {
				corner c = {};
				istr.read((char*)&c, sizeof(corner));
				o->vertexList.push_back(c);
			}
			break;
		case 2:
			istr.read((char*)&size, sizeof(int));
			for (unsigned int i = 0; i < size; ++i) {
				unsigned int u;
				istr.read((char*)&u, sizeof(int));
				o->indexList.push_back(u);
			}
			break;
		case 3:
			istr.read((char*)&size, sizeof(int));
			for (unsigned int i = 0; i < size; ++i) {
				Material m;
				istr.read((char*)&m, sizeof(Material));
				o->materialList.push_back(m);
			}
			break;
		case 4:
			istr.read((char*)&size, sizeof(int));
			for (unsigned int i = 0; i < size; ++i) {
				DirectX::XMUINT2 x;
				istr.read((char*)&x, sizeof(DirectX::XMUINT2));
				o->useMaterial.push_back(x);
			}
			break;
		default:
			objects->push_back(*o);
			return true;
		}
	}

}

//Using
//vector<Object> objs;
//
//if (!load(argv[1], &objs))
//return -1;
//
//for (int i = 0; i < objs.size(); ++i) {
//	printf("Object %i/%i: %s: %i verticies, %i indicies, %i material, %i material uses\n",
//		i + 1,
//		(unsigned int)objs.size(),
//		objs[i].name,
//		(unsigned int)objs[i].vertexList.size(),
//		(unsigned int)objs[i].indexList.size(),
//		(unsigned int)objs[i].materialList.size(),
//		(unsigned int)objs[i].useMaterial.size());
//}
//
//printf("Done!\n");