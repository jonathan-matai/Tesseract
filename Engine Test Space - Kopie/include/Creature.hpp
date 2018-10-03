#pragma once

#include "TEObject.hpp"
#include "GameGlobal.hpp"

typedef enum CreatureState {
	ST_ATTACK,	// Objekt attackiert ein Objekt
	ST_FLEE,	// Objekt flieht vor anderem Objekt
	ST_IDLE,	// Objekt ist im Idle
	ST_MOVE		// Objekt bewegt sich zu zufällig gewählter Position
} CreatureState;

class Creature : public TEObject
{
public:
	Creature(TE_OBJECT_DESC & init);
	~Creature();

	void render();
	void tick(double dt);

	// Getter und Setter
	XMFLOAT3 getVelocity() { return m_velocity; }
	float getHealth() { return m_health; }
	float getStamina() { return m_stamina; }

	void hurt(float amt) { m_health -= amt; }

private:
	// Geschwindigkeit
	XMFLOAT3 m_velocity = { 0, 0, 0 };
	// Gesundheit
	float m_health = 100;
	// Ausdauer
	float m_stamina = 100;
	// Sprungkraft des Objekts, O.0f für fliegende Objekte
	float m_gravity_multiplier = 1.0f;
	// Prüft, ob Objekt den Boden berührt (notwendig für Gravitation)
	bool m_is_on_ground = false;
	// Status des Objekts
	CreatureState state = ST_IDLE;
	// Angriff-/Fluchtobjekt
	Creature* target = NULL;

	float m_max_health = 100;
	float m_max_speed = 2;
	float m_max_stamina = 100;

	//Creature data
	teData m_data;

private:
	void track();
	void loadMesh(const wchar_t * meshFile);
};

