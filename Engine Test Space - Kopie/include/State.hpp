#pragma once

typedef bool TEResult;

// Abstrakte Klasse
class State
{
public:
	State() {};
	virtual ~State() {};

	virtual TEResult init() { return false; };
	virtual void tick(double deltatime) {};
	virtual void render() {};
};