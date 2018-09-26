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

/*
Template zur Vererbung in anderen Klassen, um die Funktion eines Singleton-Patterns zu erhalten

*/

template <class t>
class Singleton
{
public:
	// Virtueller Destruktor
	virtual ~Singleton() {}

	// Gibt die Instanz zur�ck und erstellt gegebenenfalls eine
	inline static t * get() {
		if (!instance)
			instance = new t;
		return instance;
	}

	// L�scht die Instanz und setzt den Zeiger zur�ck
	static void del() {
		if (instance) {
			delete instance;
			instance = nullptr;
		}

	}
protected:
	// H�lt die Instanz
	static t * instance;
};

template <class t>
t* Singleton<t>::instance = 0;
