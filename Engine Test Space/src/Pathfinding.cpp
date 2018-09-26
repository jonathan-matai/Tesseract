#include "Pathfinding.hpp"

void Pathfinding::Algorithm::findPath(std::vector<XMFLOAT2>* path, XMFLOAT2 start, XMFLOAT2 end)
{
	XMINT2 endNode = { (int)(end.x * NPM), (int)(end.y * NPM) };
	XMINT2 startNode = { (int)(start.x * NPM), (int)(start.y * NPM) };
	openNodes.push_back({ startNode });

	while (openNodes.size() > 0) {
		LIN parentOld = getSmallestElement(openNodes);
		closedNodes.push_back(*parentOld); // Der Liste "Geschlossen" hinzufügen
		openNodes.erase(parentOld); // Aus der Liste "Offen" entfernen
		VIN parent = std::prev(closedNodes.end()); // Adresse updaten
		
		// Endnode gefunden?
		if (equals((*parent).p, endNode)) {
			finish(path);
			return;
		}

		// Alle Nachbarn loopen
		Node child;
		LIN m_child;
		for(child.p.x = (*parent).p.x - 1; child.p.x < (*parent).p.x + 2; ++child.p.x)
			for (child.p.y = (*parent).p.y - 1; child.p.y < (*parent).p.y + 2; ++child.p.y) {

				// Ist Nachbar die eigene Node?
				if (equals((*parent).p, child.p))
					continue;

				// Liegt Nachbar außerhalb der Map?
				if (child.p.x < 0 || child.p.x >= width || child.p.y < 0 || child.p.y >= height)
					continue;

				// Ist Nachbar nicht begehbar?
				if (!isAcessable(child.p))
					continue;

				// Wurde Nachbar bereits geschlossen?
				if (isInVector(closedNodes, child.p))
					continue;

				int newDist = (*parent).startDistance + getDistance((*parent).p, child.p);

				if (!getListMember(m_child, openNodes, child.p)) { // Wenn Nachbar nicht geöffnet ist, öffnen
					child.startDistance = newDist;
					child.endDistance = getDistance(child.p, endNode);
					child.parent = parent - closedNodes.begin();
					openNodes.push_back(child);
				}
				else if (newDist < (*m_child).startDistance) { // Wenn Nachbar geöffnet ist prüfen, ob neue Distanz kürzer ist, wenn ja, Node updaten
					(*m_child).startDistance = newDist;
					(*m_child).endDistance = getDistance(child.p, endNode);
					(*m_child).parent = parent - closedNodes.begin();
				}
			}
	}
}

void Pathfinding::Algorithm::finish(std::vector<XMFLOAT2>* path)
{
	path->clear();

	Node* parent = &(*std::prev(closedNodes.end()));
	while (!equals(parent->p, { 0,0 })) {
		path->push_back({ (float)parent->p.x / NPM, (float)parent->p.y / NPM });
		parent = &closedNodes[parent->parent];
	}
}

bool Pathfinding::Algorithm::isAcessable(XMINT2 p)
{
	return true;
}

bool Pathfinding::Algorithm::isInList(std::list<Node>& list, XMINT2 p)
{
	for (LIN k = list.begin(); k != list.end(); ++k)
		if (equals((*k).p, p))
			return true;
	
	return false;
}

bool Pathfinding::Algorithm::isInVector(std::vector<Node>& list, XMINT2 p)
{
	for (VIN k = list.begin(); k != list.end(); ++k)
		if (equals((*k).p, p))
			return true;

	return false;
}

bool Pathfinding::Algorithm::equals(XMINT2 a, XMINT2 b)
{
	return (a.x == b.x && a.y == b.y) ? true : false;
}

UINT Pathfinding::Algorithm::getDistance(XMINT2 a, XMINT2 b)
{
	int deltaX = abs(a.x - b.x);
	int deltaY = abs(a.y - b.y);

	return min(deltaX, deltaY) * 14 + abs(deltaX - deltaY) * 10;
}

bool Pathfinding::Algorithm::getListMember(LIN& element, std::list<Node>& list, XMINT2 p)
{
	for (LIN k = list.begin(); k != list.end(); ++k)
		if (equals((*k).p, p)) {
			element = k;
			return true;
		}
	return false;
}

Pathfinding::LIN Pathfinding::Algorithm::getSmallestElement(std::list<Node>& list)
{
	LIN buffer = list.begin();

	for (LIN i = list.begin(); i != list.end(); ++i) {
		if ((*i).startDistance + (*i).endDistance <= (*buffer).startDistance + (*buffer).endDistance &&
			(*i).endDistance < (*buffer).endDistance)
			buffer = i;
	}

	return buffer;
}
