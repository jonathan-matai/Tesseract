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

#include "..\include\MemoryManager.hpp"

struct teMemoryEntry
{
	void* pMemory;
	void* pUserMemory;
	int size;
	char aCheck[10]; //Checkcode = "Tesseract"
};

teList<teMemoryEntry> * g_pMemoryList = NULL;
int g_byteCounter = 0;

bool GetMemEntry(void* pMem, teListEntry<teMemoryEntry>** ppOut)
{
	if (pMem == NULL)
		return false;

	if (!teValidateMemory(pMem))
		return false;

	pMem = (BYTE*)(pMem)-sizeof(teListEntry<teMemoryEntry>*);

	*ppOut = *((teListEntry<teMemoryEntry>**)(pMem));

	return true;
}

bool AddMemoryToList(void* pMem, int size)
{
	teMemoryEntry newEntry;
	teListEntry<teMemoryEntry>* pNewMemoryListEntry;

	if (pMem == NULL)
		return false;
	if (size < 0) return false;

	newEntry.pMemory = pMem;
	newEntry.pUserMemory = (BYTE*)(pMem)+sizeof(teListEntry<teMemoryEntry>*);
	newEntry.size = size;
	strcpy_s(newEntry.aCheck, "Tesseract");

	pNewMemoryListEntry = g_pMemoryList->m_addEntry(&newEntry);
	if (!pNewMemoryListEntry)
		return false;

	*((teListEntry<teMemoryEntry>**)(pMem)) = pNewMemoryListEntry;

	return true;
}

void teInitMemory()
{
	g_pMemoryList = new teList<teMemoryEntry>;
	if (!g_pMemoryList)
		return;

	LOGFILE->print(4, "MemoryManager wurde initialisiert");

	return;
}

void teExitMemory()
{
	teFreeAll();

	delete g_pMemoryList;
	g_pMemoryList = NULL;

	if (g_byteCounter > 0)
		LOGFILE->print(1, "Nicht alle Speicherbereiche freigegeben");

	LOGFILE->print(3, "Alle Speicherbereiche freigegeben");

	return;
}

void* teAlloc(int size)
{
	void* pMemory;

	if (size < 0)
		return NULL;

	pMemory = ::operator new(size + sizeof(teListEntry<teMemoryEntry>*));
	if (!pMemory) return NULL;

	g_byteCounter += size + sizeof(teListEntry<teMemoryEntry>*);
	ZeroMemory(pMemory, size + sizeof(teListEntry<teMemoryEntry>*));

	if (!AddMemoryToList(pMemory, size))
		return NULL;

	return (BYTE*)(pMemory) + sizeof(teListEntry<teMemoryEntry>*);
}

void * teReAlloc(void * pMemory, int newSize)
{
	int size;
	void * pNewMemory;

	size = teGetMemorySize(pMemory);
	if (size == -1)
		return teAlloc(abs(newSize));

	if (newSize < 0)
		newSize = size - newSize;

	pNewMemory = teAlloc(newSize);
	if (!pNewMemory)
		return NULL;

	size = size < newSize ? size : newSize;

	memcpy(pNewMemory, pMemory, size);
	teFreeMemory(pMemory);
	pMemory = NULL;

	return pNewMemory;
}

void teFreeMemory(void * pMem, bool criticalFree)
{
	if(criticalFree)
		LOGFILE->printf(2, "Es werden %d Bytes Speicher freigegeben", teGetMemorySize(pMem));

	teListEntry<teMemoryEntry>* pMemoryListEntry;
	teMemoryEntry Entry;

	if (!pMem)
		return;

	if (!GetMemEntry(pMem, &pMemoryListEntry))
		return;

	Entry = pMemoryListEntry->data;
	g_byteCounter -= Entry.size + sizeof(teListEntry<teMemoryEntry>*);
	::operator delete(Entry.pMemory);
	g_pMemoryList->m_deleteEntry(pMemoryListEntry);

	return;
}

bool teValidateMemory(void * pMem)
{
	teListEntry<teMemoryEntry>* pEntry;

	if (!pMem)
		return false;

	pEntry = *((teListEntry<teMemoryEntry>**)((BYTE*)(pMem)-sizeof(teListEntry<teMemoryEntry>*)));
	if (!pEntry->data.pMemory || !pEntry->data.pUserMemory)
		return false;

	if (pEntry->data.size < 0)
		return false;
	if (strncmp(pEntry->data.aCheck, "Tesseract", 10))
		return false;

	return true;
}

int teGetMemorySize(void * pMem)
{
	teListEntry<teMemoryEntry>* pMemListEntry;

	if (!pMem)
		return -1;

	if (!GetMemEntry(pMem, &pMemListEntry))
		return -1;

	return pMemListEntry->data.size;
}

void teFreeAll()
{
	teListEntry<teMemoryEntry>* pEntry;

	pEntry = g_pMemoryList->m_getFirstEntry();

	while (pEntry)
	{
		teFreeMemory(pEntry->data.pUserMemory);

		pEntry = g_pMemoryList->m_getFirstEntry();
	}

	return;
}

template <typename Type> teList<Type>::teList()
{
	ZeroMemory(this, sizeof(teList<Type>));
}

template <typename Type> teList<Type>::~teList()
{
	m_clear();
}

template <typename Type> teListEntry<Type>* teList<Type>::m_addEntry(Type* pData)
{
	teListEntry<Type>* pNewEntry;

	if (!pData)
		return NULL;

	pNewEntry = new teListEntry<Type>;
	if (!pNewEntry)
		return NULL;

	pNewEntry->prevEntry = m_pLastEntry;
	if (m_pLastEntry)
		m_pLastEntry->nextEntry = pNewEntry;
	pNewEntry->nextEntry = NULL;
	m_pLastEntry = pNewEntry;

	if (!m_pFirstEntry)
		m_pFirstEntry = pNewEntry;

	memcpy(&pNewEntry->data, pData, sizeof(Type));

	m_numEntries++;

	return pNewEntry;
}

template <typename Type> void teList<Type>::m_findEntry(Type*pData, teListEntry<Type>**ppOut)
{
	teListEntry<Type>* pCurrentEntry;

	if (!pData)
		return;

	pCurrentEntry = m_pFirstEntry;

	while (pCurrentEntry)
	{
		if (!memcmp(&pCurrentEntry->data, pData, sizeof(Type)))
		{
			if (ppOut)
				*ppOut = pCurrentEntry;
			return TE_OK;
		}

		pCurrentEntry = pCurrentEntry->nextEntry;
	}

	return;
}

template <typename Type> void teList<Type>::m_deleteEntry(teListEntry<Type>*pEntry)
{
	if (pEntry == NULL)
		return;

	if (pEntry == m_pFirstEntry && pEntry == m_pLastEntry)
	{
		m_pFirstEntry = m_pLastEntry = NULL;
	}
	else if (pEntry == m_pFirstEntry)
	{
		m_pFirstEntry = pEntry->nextEntry;
		m_pFirstEntry->prevEntry = NULL;
	}
	else if (pEntry == m_pLastEntry)
	{
		m_pLastEntry = pEntry->prevEntry;
		m_pLastEntry->nextEntry = NULL;
	}
	else
	{
		pEntry->prevEntry->nextEntry = pEntry->nextEntry;
		pEntry->nextEntry->prevEntry = pEntry->prevEntry;
	}

	delete pEntry;
	pEntry = NULL;

	m_numEntries--;

	return;
}

template <typename Type> void teList<Type>::m_clear()
{
	while (m_pFirstEntry)
		m_deleteEntry(m_pFirstEntry);

	return;
}