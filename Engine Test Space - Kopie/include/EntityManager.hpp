#pragma once

#include <Windows.h>
#include <list>
#include <fstream>
#include <iterator>

#include "Logfile.hpp"
#include "Creature.hpp"
#include "TEMapObject.hpp"
#include "Player.hpp"

typedef bool TEResult;

class EntityManager
{
public:
	EntityManager(const char* path);
	~EntityManager();

	void tick(double deltatime);
	void render();

	TEResult load();
	TEResult save();

	void remove(UINT index);
	void add(TEObject * entity);

private:
	size_t getSize(UINT id);

	std::list<TEObject*> objects;

	struct Settings {
		const char* path;

		UINT oneiro_cap;
		UINT hostile_cap;
		UINT neutral_cap;
		UINT passive_cap;
	};
	Settings settings;

};