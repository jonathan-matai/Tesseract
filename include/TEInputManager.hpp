#pragma once

#include "TEGlobal.hpp"

enum teKeyMap
{
	W = 0x57,
	S = 0x53,
	A = 0x41,
	D = 0x44,
	Q = 0x51,
	E = 0x45,
	TAB = 0x09,
	SHIFT = 0x10
};

class TEInputManager
{
	public:

		TEInputManager();
		~TEInputManager();

		static void tick(float deltatime);
};