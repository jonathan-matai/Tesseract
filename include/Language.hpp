#pragma once

#include <map>
#include <fstream>
#include <string>
#include "Singleton.hpp"
#include "Logfile.hpp"

#define LANG Language::get()

/*
BENUTZUNG DER LANGUAGE-KLASSE:
LANG->setLanguage("Beispiel.lng");
LANG->getT("Beispiel");
(Falls Eintrag nicht vorhanden ist gibt Funktion --- zurück)

AUFBAU EINER .lng DATEI
index1=name1
index2=name2
...
(Zeilenumbruch ist Pflicht!)

*/

class Language : public Singleton<Language>
{
public:
	Language();
	~Language();
	void setLanguage(const char * path);
	std::string getT(std::string index);
private:
	std::map<std::string, std::string> lang;
	std::map<std::string, std::string>::iterator itr;
};

