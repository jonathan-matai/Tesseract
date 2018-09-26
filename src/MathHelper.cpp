//	Diese Datei ist Teil der Tesseract-Engine
//	Phoenix (c) 2017
//	 ___________________           ____________
//	|_______    ________|         |    ________|
//			|  |				  |   |
//			|  |				  |   |
//			|  |	 _________    |   |_____
//			|  |	/	____  \   |    _____|
//			|  |	|  |____|  |  |   | 
//			|  |	|   _______|  |   |
//			|  |	|  |_______   |   |________ 
//			|__|	\_________/   |____________|
//			

#include "..\include\MathHelper.hpp"

float MathHelper::linearInterpolation(float x_a, float y_a, float x_b, float y_b, float x_i)
{
	return ((x_i - x_a) * (y_b - y_a) / (x_b - x_a)) + y_a;
}

float MathHelper::cosineInterpolation(float x_a, float y_a, float x_b, float y_b, float x_i)
{
	return (-cos(TE_PI * (x_i - x_a) / (x_b - x_a)) / 2 + 0.5) * (y_b - y_a) + y_a;
}

void MathHelper::seed()
{
	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	rSeed = ms.count();
}

void MathHelper::seed(const unsigned int seed)
{
	rSeed = seed;
}

unsigned int MathHelper::rand()
{
	rSeed = (rA * rSeed + rB) % rC;
	return rSeed;
}

float MathHelper::rand(float start, float range)
{
	rSeed = (rA * rSeed + rB) % rC;
	double rSeedD = rSeed; // Nötig, da float = int/int ein Integerwert liefert
	return rSeedD / rC * range + start;
}

unsigned int MathHelper::rSeed = rand();