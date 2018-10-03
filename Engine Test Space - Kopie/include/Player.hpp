#pragma once

#include "GameGlobal.hpp"

class Player
{
	private:

		float m_health, m_stamina, m_attackdmg, m_defensvalue;
		vPoint m_pos;
		

		TECamera * camera;

	public:

		Player(TE_OBJECT_DESC obj);
		~Player();

		void tick(double deltatime);
		void render();

		vPoint getPlayersPos() { return m_pos; };
		TECamera * getCamera() { return camera; };
};
