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

#pragma once

#pragma warning(disable:4244)

//Markos & Defines
#define TE_PI (3.141592653589793238462643383279f)
#define TE_PHI (0.25f*TE_PI)
#define TE_THETA (1.5f*TE_PI)
#define TE_DEG_TO_RAD(a){(a)*0.01745329251994329576923690768489f}
#define TE_RAD_TO_DEG(a){(a)*57.295779513082320876798154814092f}

#include <cmath>
#include <chrono>

namespace MathHelper {

	// Konstanten f�r den Zufallsgenerator, sollen gro�e UINT-Werte sein
	const unsigned int rA = 3473028106;
	const unsigned int rB = 1122907613;
	const unsigned int rC = 2000000000;
	// H�lt den letzten Zufallswert/Seed
	extern unsigned int rSeed;

	// Gibt den y-Wert des Punktes I(xI|?) durch lineare Interpolation mit A(xA|yA) und B(xB|yB) zur�ck
	float linearInterpolation(float xA, float yA, float xB, float yB, float xI);

	// Gibt den y-Wert des Punktes I(xI|?) durch kosinale Interpolation mit A(xA|yA) und B(xB|yB) zur�ck
	float cosineInterpolation(float xA, float yA, float xB, float yB, float xI);

	// Erstellt einen zuf�lligen Sartwert in Abh�ngigkeit von der Systemzeit
	void seed();
	// Setzt einen festen Startwert (UINT)
	void seed(const unsigned int);

	// Gibt einen zuf�lligen UINT zur�ck (von 0 bis [MathHelper::rC])
	unsigned int rand();
	// Gibt einen zuf�lligen Float zur�ck (von [start] bis [start]+[range])
	float rand(float start, float range);
}
