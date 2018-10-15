#pragma once

#include "State.hpp"
#include "EntityManager.hpp"
#include "TerrainGenerator.hpp"

class GameState : public State
{
	public:
		GameState();
		~GameState();
	
		TEResult init(const char * mapPath);
		void tick(double deltatime);
		void render();
	
		EntityManager * getEntityManager() { return entityManager; }
	
	private:
		EntityManager * entityManager;
		TerrainGenerator * terrainGenerator;
		DirectionalLight sun[3];
};

