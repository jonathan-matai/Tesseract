#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#define _alloc(type, count) static_cast<type*>(::operator new(sizeof(type) * count))
#define _free(pointer) ::operator delete(pointer)

struct Vertex_3PNU
{
	XMFLOAT3	position;
	XMFLOAT3	normal;
	XMFLOAT2	uv;
};

struct TerrainSettings
{
	uint32_t	chunkSize;			// in Metern, idealerweise immer positive Potenzen von 2 (z.B. 2, 4, 8, 16)
	uint8_t		layers;				// Anzahl der Noiseebenen, je mehr desto feiner die Berge
	float		mod_aplitude;		// Modifizierung der Amplituden zwischen Noiseebenen, je gr��er, desto felsiger das Terrain:  ] 0.0f; 1.0f ]
	float		mod_wavelength;		// Modifizierung der Wellenl�nge, je kleiner, desto feiner die Berge:  ] 0.0f; 1.0f ]
	float		baseAmplitude;		// maximale Amplitude -> H�hen liegen im Intervall  ] -amplitude; +amplitude [
	float		baseWavelength;		// Startwellenl�nge, je gr��er, desto breiter die Berge
	float		textureWidth;		// Breite einer Textur in Metern
	uint64_t	seed;				// Seed f�r die Terraingenerierung
};

struct ChunkSettings
{
	int32_t		id_x;				// Chunk ID in x-Richtung
	int32_t		id_z;				// Chunk ID in y-Richtung
	int8_t		tesselation;

	/*
	Zur Tesselation:

	Ein Faktor von 0 erzeugt Dreiecke mit einer Breite von 1m
	Pro Erh�hung des Faktors verviervacht (!!!) sich die Zahl der Dreiecke, also vorsichtig nutzen
	Pro Erniedrigung des Faktors viertelt sich die Zahl der Triangles, der Wert darf nicht -log2(chunkSize) unterschreiten, da pro Chunk minimal 2 Traingles generiert werden m�ssen!
	*/
};

struct ChunkGenerationInfo
{
	TerrainSettings*	terrainSettings;	// Globale Einstellungen zur Gernerierung der Welt
	ChunkSettings*		chunkSettings;		// Chunkspezifische Einstellungen
	Vertex_3PNU**		vertexBuffer;		// Adresse eines Pointer, der den Vertexbuffer h�lt. Speicher wird automatisch reserviert muss jedoch manuell mit delete[] oder _free() freigegeben werden
	size_t*				n_verticies;		// Adresse einer Varibale, die die Zahl der Vertizen h�lt
	uint32_t**			indexBuffer;		// Adresse eines Pointer, der den Indexbuffer h�lt. Speicher wird automatisch reserviert muss jedoch manuell mit delete[] oder _free() freigegeben werden
	size_t*				n_indicies;			// Adresse einer Variable, die die Zahl der Indizen h�lt
};


// Generate 1 chunk on the main thread
void generateChunk(const ChunkGenerationInfo& info);

// Generate multiple chunks using multithreading
void generateChunks(ChunkGenerationInfo* p_infos, size_t n_infos);