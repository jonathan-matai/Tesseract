#pragma once

#include <Windows.h>
#include <cmath>
#include <list>
#include <vector>
#include <DirectXMath.h>
#include "../../include/Logfile.hpp"

#define NPM 4

using namespace DirectX;

namespace Pathfinding {

	typedef struct Node {
		XMINT2 p;
		int startDistance = 0;
		int endDistance = 0;
		UINT parent = 0;
	} Node;

	typedef std::list<Node>::iterator LIN;
	typedef std::vector<Node>::iterator VIN;

	const int width = 8000;
	const int height = 8000;

	class Algorithm {

	public:
		void findPath(std::vector<XMFLOAT2>* path, XMFLOAT2 a, XMFLOAT2 b);

	private:
		std::list<Node> openNodes;
		std::vector<Node> closedNodes;

		// Gibt den Pfad rückwärts (!) zurück
		void finish(std::vector<XMFLOAT2>* path);

		// Gibt Zurück, ob Punkt p betretbar ist
		bool isAcessable(XMINT2 p);
		// Prüft, ob Punkt identisch ist
		bool equals(XMINT2 a, XMINT2 b);
		// Gibt Schätzung für die Distanz (Luftlinie) zrück
		UINT getDistance(XMINT2 a, XMINT2 b);
		// Gibt zurück, ob eine Node mit abgegebenen Koordinaten in der angegebenen Liste existiert
		bool isInList(std::list<Node>& list, XMINT2 p);
		bool isInVector(std::vector<Node>& list, XMINT2 p);
		// wie isInList(), gibt zusätzlich Iterator auf das Element zurück
		bool getListMember(LIN& elemtent, std::list<Node>& list, XMINT2 p);
		LIN getSmallestElement(std::list<Node>& list);
	};
}