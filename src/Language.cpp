#include "Language.hpp"

Language::Language(){}

Language::~Language(){}

void Language::setLanguage(const char * path)
{
	// Map leeren
	lang.clear();

	// Datei laden
	std::ifstream input(path);
	if (!input.is_open()) {
		LOGFILE->printf(0, "LANGUAGE: Unable to open language file %s", path);
		return;
	}

	// Temporäre Variablen
	std::string index, content;

	// Datei auslesen
	while (std::getline(input, index, '=') && std::getline(input, content, '\n'))
		lang[index] = content;

	LOGFILE->printf("LANGUAGE: Set language to %s!", path);
	input.close();
}

std::string Language::getT(std::string index)
{
	itr = lang.find(index);
	return(itr == lang.end() ? "---" : itr->second);
}
