#include "Creature.hpp"

Creature::Creature(TE_OBJECT_DESC & init) : TEObject(init)
{
	loadMesh(init.file1);
}

Creature::~Creature()
{
}

void Creature::track()
{
}

void Creature::loadMesh(const wchar_t * meshFile)
{
	//Loading Mesh from file in m_data
}

void Creature::render()
{
	LOGFILE->print("Creature render called!");
}

void Creature::tick(double dt)
{
	// Prüfen, ob Kreatur nach unten beschleunigt werden muss
	if (!m_is_on_ground)
		m_velocity.y -= GRAVITY * m_gravity_multiplier;
	else
		m_velocity.y = 0;

	LOGFILE->print("Creature tick called!");
}