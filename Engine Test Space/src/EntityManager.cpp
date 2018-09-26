#include "EntityManager.hpp"

EntityManager::EntityManager(const char* path)
{
	settings.path = path;
	LOGFILE->print(TE_CONSTRUCTION, "EntityManager wurde erstellt!");
}

EntityManager::~EntityManager()
{
	save();

	for (auto i = objects.begin(); i != objects.end(); ++i)
		if(*i) delete *i;
	objects.clear();
	
	LOGFILE->print(TE_CONSTRUCTION, "EntityManager wurde geschlossen!");
}

void EntityManager::tick(double deltatime)
{
	for (auto i = objects.begin(); i != objects.end(); ++i)
		(**i).tick(deltatime);
}

void EntityManager::render()
{
	for (auto i = objects.begin(); i != objects.end(); ++i)
		(**i).render();
}

TEResult EntityManager::load()
{
	std::ifstream input(settings.path);

	if (!input.good() || !input.is_open()) {
		input.close();
		LOGFILE->printf(TE_WARNING, "ObjectList %s existiert nicht und kann nicht geöffnet werden!", settings.path);
		return false;
	}

	UINT n_entities;
	input.read((char*)&n_entities, sizeof(UINT));

	for (UINT i = 0; i < n_entities; ++i) {

		size_t object_size;
		input.read((char*)&object_size, sizeof(size_t));

		void* obj = malloc(object_size);
		input.read((char*)obj, object_size);
		add(new TEObject(*static_cast<TEObject*>(obj)));
		delete obj;
	}

	input.close();
	return true;
}

TEResult EntityManager::save()
{
	std::ofstream output(settings.path);

	if (!output.is_open()) {
		char msg[128];
		sprintf_s(msg, "Auf %s konnte nicht zugegriffen werden!", settings.path);
		TRACE(true, msg);
		return false;
	}

	UINT size = (UINT) objects.size();
	output.write((const char*)&size, sizeof(UINT));

	std::list<TEObject*>::iterator i;
	for (i = objects.begin(); i != objects.end(); ++i) {
		size_t object_size = getSize((**i).getID());
		output.write((const char*) &object_size, sizeof(size_t));
		output.write((const char*)*i, object_size);
	}

	LOGFILE->printf(TE_SUCCEEDED, "%i Strukturen wurden in %s wurde gespeichert!",objects.size() ,settings.path);
	output.close();
	return true;
}

void EntityManager::remove(UINT index)
{
	std::list<TEObject*>::iterator i = objects.begin();
	std::advance(i, index);
	objects.erase(i);
}

void EntityManager::add(TEObject * entity)
{
	objects.push_back(entity);
}


size_t EntityManager::getSize(UINT id)
{
	switch (id) {
	case TE_OBJECT_TYPE_OBJECT: return sizeof(TEObject);
	case TE_OBJECT_TYPE_NPC: return sizeof(Creature);
	case TE_OBJECT_TYPE_MAP: return sizeof(TEMap);
	default: TRACE(true, "Ungültiges Objekt!"); return NULL;
	}
}
