#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#define _alloc(type, count) static_cast<type*>(::operator new(sizeof(type) * count))
#define _free(pointer) ::operator delete(pointer)

struct TerrainSettings
{
	uint32_t	chunkSize;			// in Metern, idealerweise immer positive Potenzen von 2 (z.B. 2, 4, 8, 16)
	uint8_t		layers;				// Anzahl der Noiseebenen, je mehr desto feiner die Berge
	float		mod_aplitude;		// Modifizierung der Amplituden zwischen Noiseebenen, je größer, desto felsiger das Terrain:  ] 0.0f; 1.0f ]
	float		mod_wavelength;		// Modifizierung der Wellenlänge, je kleiner, desto feiner die Berge:  ] 0.0f; 1.0f ]
	float		baseAmplitude;		// maximale Amplitude -> Höhen liegen im Intervall  ] -amplitude; +amplitude [
	float		baseWavelength;		// Startwellenlänge, je größer, desto breiter die Berge
	float		textureWidth;		// Breite einer Textur in Metern
};

struct ChunkSettings
{
	int32_t		id_x;				// Chunk ID in x-Richtung
	int32_t		id_z;				// Chunk ID in y-Richtung
	int8_t		tesselation;

	/*
	Zur Tesselation:

	Ein Faktor von 0 erzeugt Dreiecke mit einer Breite von 1m
	Pro Erhöhung des Faktors verviervacht (!!!) sich die Zahl der Dreiecke, also vorsichtig nutzen
	Pro Erniedrigung des Faktors viertelt sich die Zahl der Triangles, der Wert darf nicht -log2(chunkSize) unterschreiten, da pro Chunk minimal 2 Traingles generiert werden müssen!
	*/
};

struct Vertex_3PNU
{
	XMFLOAT3	position;
	XMFLOAT3	normal;
	XMFLOAT2	uv;
};

#include <memory>

template<typename t> bool cropArray2d(t* input, size_t width_input, size_t height_input, t* output, size_t x_start, size_t y_start, size_t x_end, size_t y_end, size_t outSize)
{
	size_t width_output = 1 + x_end - x_start;
	size_t height_output = 1 + y_end - y_start;

	if (width_output * height_output * sizeof(t) != outSize)
		return false;

	for (size_t h = y_start; h <= y_end; ++h)
		memcpy_s(&output[( h - y_start ) * width_output], width_output * sizeof(t), &input[h * width_input + x_start], width_output * sizeof(t));

	return true;
}

/*
Zur Terraingenerierung

const TerrainSettings&	terrainSettings:		Globale Einstellungen zur Gernerierung der Welt
const ChunkSettings&	chunkSettings:			Chunkspezifische Einstellungen
Vertex_3PNT**			vertexBuffer:			Adresse eines Pointer, der den Vertexbuffer hält. Speicher wird automatisch reserviert muss jedoch manuell mit delete[] oder _free() freigegeben werden
size_t*					n_verticies:			Adresse einer Varibale, die die Zahl der Vertizen hält
uint32_t**				indexBuffer:			Adresse eines Pointer, der den Indexbuffer hält. Speicher wird automatisch reserviert muss jedoch manuell mit delete[] oder _free() freigegeben werden
size_t*					n_indicies				Adresse einer Variable, die die Zahl der Indizen hält

*/

void generateChunk(const TerrainSettings& terrainSettings, const ChunkSettings& chunkSettings, Vertex_3PNU** vertexBuffer, size_t* n_verticies, uint32_t** indexBuffer, size_t* n_indicies);
