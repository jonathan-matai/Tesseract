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

#include "Logfile.hpp"

//Struktur f�r den einzelnen Listeneintrag
template<typename Type> struct teListEntry
{
	teListEntry<Type>*prevEntry;
	teListEntry<Type>*nextEntry;
	Type data;
};

//Die Klasse aus der die Liste selbst besteht
template<typename Type> class teList
{
private:
	teListEntry<Type>*m_pFirstEntry; //Pointer auf den ersten Eintrag
	teListEntry<Type>*m_pLastEntry; //Pointer auf den letzten Eintrag
	int m_numEntries; //Anzahl der Eintr�ge

public:
	teList();
	~teList();

	teListEntry<Type> * m_addEntry(Type*pData); //F�gt einen Eintrag in die Liste hinzu
	void m_findEntry(Type*pData, teListEntry<Type>**ppOut); //Findet den angegebenen Eintrag und gibt ihn in ppOut zur�ck
	void m_deleteEntry(teListEntry<Type>*pEntry); //L�scht einen Eintrag
	void m_clear(); //L�scht die ganze Liste

	inline Type* m_getEntryData(teListEntry<Type>*pEntry) { return &pEntry->data; }; //Gibt die Daten des Eintrags im gew�nschten Format zur�ck
	inline teListEntry<Type>* m_getFirstEntry() { return m_pFirstEntry; }; //Liefert den ersten Eintrag
	inline teListEntry<Type>* m_getLastEntry() { return m_pLastEntry; }; //Liefert den letzten Eintrag
	inline int m_getNumEntries() { return m_numEntries; }; //Liefert die Anzahl an Eintr�gen
};

extern void teInitMemory(); //Initialisiert den MemoryManager
extern void teExitMemory(); //F�hrt den MemoryManager herunter
extern void* teAlloc(int size); //Alloziert neuen Speicher
extern void* teReAlloc(void * pMem, int newSize); //Realloziert den angegebenen Speicher um die angegebene Gr��e
extern void teFreeMemory(void * pMem, bool criticalFree = false); //L�scht den angegebenen Speicher
extern bool teValidateMemory(void * pMem); //Validiert den Speicher
extern int teGetMemorySize(void * pMem); //Liefert die Gr��e des Speichers
extern void teFreeAll(); //L�scht den gesamten Speicher
