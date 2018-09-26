#include "..\include\Player.hpp"

Player::Player(TE_OBJECT_DESC obj)
{
	m_pos = xmTovPoint(obj.pos);
}

Player::~Player()
{

}

void Player::tick(double deltatime)
{

}

void Player::render()
{

}
