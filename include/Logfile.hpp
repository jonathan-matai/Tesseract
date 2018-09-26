#pragma once

#include <stdio.h>
#include <time.h>
#include <string>
#include "Singleton.hpp"
#include <Windows.h>
#include <chrono>


// Makro zum einfachen Aufruf, z.B. LOGFILE->print("");
#define LOGFILE Logfile::get()

// Makros zur Zeitmessung
#define LOGTIME(var) std::chrono::high_resolution_clock::time_point (var) = std::chrono::high_resolution_clock::now()
#define DELTATIME(start, end) std::chrono::duration_cast<std::chrono::milliseconds>((end) - (start)).count()

enum colors {
	TE_CONSTRUCTION,	// TÜRKIS
	TE_IMPORTANT,		// ORANGE
	TE_VALUE,			// GRAU
	TE_SUCCEEDED,		// GRÜN
	TE_INFO,			// BLAU
	TE_ERROR,			// ROT
	TE_WARNING			// GELB
};

/*
Benutzung der Logfile-Klasse:
LOGFILE->createLogfile();
{Befehle} z.B. LOGFILE->print(0, "Fehler!");
LOGFILE->del();
*/
class Logfile : public Singleton<Logfile>
{
public:
	Logfile();
	// Beendet das Logfile
	~Logfile();

	// Erstellt neues Logfile
	void createLogfile();
	void createLogfile(const char * version);
	// Schreibt Text mit Timestamp in das Logfile
	void print(const char * text);
	// Schreibt farbigen Text mit Timestamp in das Logfile
	void print(int color, const char * text);
	// Schreibt formatierten Text im printf-Syntax in das Logfile
	void printf(const char * text, ...);
	// Schreibt farbigen, formatierten Text in das Logfile
	void printf(int color, const char * text, ...);
	// Gibt einen Fehler aus und beendet das Programm
	void error(bool critical, const char* msg, const char* file, const char* function, int line);

private:
	static std::string colors[7];
	// Gibt aktuelle Zeit im strftime-Syntax zurueck; muss evtl. mit c_str() zu char* gecastet werden
	std::string getTime(char * format);
	// Haelt die Instanz der Logfile
	FILE * file;
};

#if defined (_DEBUG) || (DEBUG)
#define TRACE(bIsCritical, msg) LOGFILE->error((bool)bIsCritical, (const char*)msg, (const char*)__FILE__, (const char*)__FUNCTION__, (int)__LINE__)
#else
#define TRACE(bIsCritical, msg) LOGFILE->error((bool)bIsCritical, (const char*)msg, (const char*)__FILE__, "-", (int)__LINE__)
#endif