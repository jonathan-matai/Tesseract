#pragma once

#include "EntityManager.hpp"
#include "../../include/Logfile.hpp" // windows.h
#include "../../include/MathHelper.hpp"
#include "../../include/MemoryManager.hpp"
#include <fstream>
#include <thread>
#include <vector>

#define RELEASE_PTR(x){teFreeMemory(static_cast<void*>(x)); x = NULL;}
#define POW2(x) ((x)*(x))

#define THREADCOUNT (7) // n der zu erstellenden Threads (\Hauptthread)

#define THM_VER (3)
#define TBM_VER (3)
#define TSL_VER (0)
#define TGM_VER (1)

typedef struct
{
	UINT width; // Breite der Map (8000)
	UINT height; // Höhe der Map (8000)
	UINT layer_count; // Anzahl der zu generierenden Heightmap-Lagen (5)
	int16_t amplitude; // Amplitude, die für die erste Lage verwendet (30000)
	float amplitude_modifier; // Wert um die die Amplitude pro Lage verkleinert wird (0.3f)
	int16_t base_height; // Minimum der ersten Lage (-10000)
	UINT wavelength; // Wellenlänge, die in der ersten Lage verwendet wird (800)
	float wavelength_modifier; // Wert um die die Wellenlänge pro Lage verkleinert wird (0.3f)
	float mask_offset; // y-Verschiebung der Maskenkurve (kleiner -> kleinere Insel) (4.0f)
	UINT n_fancy_interpolation; // Anzahl der Lagen, für die kosinale Interpolation verwendet wird (1)
	UINT n_use_mask; // Anzahl der Lagen, die mit der Maske unterlegt werden (2)
	UINT humidity_wavelength; // Wellenlänge, die zur Generierung der Feutchtigkeitsmap verwendet wird (2000)
	int16_t humidity_noise; // Noise-Level für Feuchtigkeitsmap (kleiner -> saubere Biomübergänge) (500)
	UINT humidity_noise_wavelength; // kleiner -> glattere Biomübergänge (40)
	float scale_horizontal; // Meter von Index i zu Index i+1 (0.25f)
	float scale_vertical; // Meter zwischen y und y+1 (0.01f)
	double min_dither; // Ditheringwert, das ein Objetk mindestens haben muss (0 - 1)
	std::string path; // Speicherverzeichnis (z.B. C:\\Maps\\), auch relativer Pfad möglich
	EntityManager* entityManager; // EntityManager der zur Erstellung der Strukturen verwendet wird
	UINT seed = 0; // Startwert für den Mapgenerator
} GeneratorSettings;

typedef bool teResult;

enum Biomes
{
	OCEAN,
	SNOWBEACH, STONEBEACH, SANDBEACH,
	SNOWPLAINS, PLAINS, SWAMP, DESERT,
	FOREST, JUNGLE, SAVANNA, PINEFOREST,
	TAIGA, TAIGAEDGE, SNOWTAIGA, SNOWTAIGAEDGE,
	TUNDRA, MOUNTAINS, SNOWMOUNTAINS
};

enum Flora
{
	CORAL,
	ICEFLOE, PALMTREE, ROCK,
	BUSH, SAPLING, DEADBUSH, BERRYBUSH,
	SPRUCE, FIR, PINE, MAPLE, OAK, BIRCH, BEECH, JUNGLETREE, ACACIA, WILLOW
};

class TerrainGenerator
{
public:
	TerrainGenerator();
	~TerrainGenerator();

	// Generiert Map
	teResult generate(const GeneratorSettings& settings);

private:
	// Erstellt Heightmap-Ebene
	void doLayer(UINT layer, UINT raster, int16_t amplitude, int16_t base_height);
	// Erstellt die Feutchtigkeits-/Temperaturmap
	void createParameterMap(uint16_t* arr);
	// Erstellt die Biommap
	void createBiomeMap();
	// Erstellt die Steigungsmap (nötig für Pathfinding
	void createGradientMap(UINT sector, UINT xStart, UINT xEnd, UINT yStart, UINT yEnd);
	// Generiert Strukturen
	void generateStructures();
	// Erstellt ein Raster aus Zufallswerten
	template <class c> void rasterize(c* arr, UINT raster, int16_t amplitude, int16_t base_height);
	void rasterize(int16_t* arr, UINT raster, int16_t amplitude, int16_t base_height, bool doMask);
	// Legt eine runde Maske auf die Map
	int16_t mask(int16_t y, int x, int z);
	// Interpoliert die Werte zwischen den Punkten (horizontal)
	template <class c> void interpolateHorizontal(c* arr, UINT raster, float (*intAlgorithm)(float, float, float, float, float));
	// Interpoliert die Werte zwischen den Punkten und interpolierten Werten (vertikal)
	template <class c> void interpolateVertical(c* arr, UINT raster, float(*intAlgorithm)(float, float, float, float, float));
	// Fügt Ebenen zusammen
	template <class c1, class c2> void add(c1* in, c2* out);
	// Überprüft, ob Platz für Struktur vorhanden ist
	bool spaceAvailable(UINT x, UINT y, UINT rBlocked, UINT rClaimed, UINT rHindered);
	// Speichert Heightmap
	void saveHeightmap(const teResult* result);
	// Speichert Biommap
	void saveBiomemap(const teResult* result);
	// Speichert Steigungsmap
	void saveGradientmap(const teResult* result);
	
	// Höhenmatrix
	int16_t* heightmap;
	// Feuchtigkeitsmatrix
	uint16_t* humiditymap;
	// Temperaturmatrix
	uint16_t* temperaturemap;
	// Biommatrix
	uint8_t* biomemap;
	// Gibt an, wie begehbar das Terrain ist
	uint8_t* weightmap;
	// Gibt die Steigung des Terrains an
	uint8_t* gradientmap;

	// Member
	std::vector<std::thread> threads;
	GeneratorSettings gs;
	UINT createdStructures = 0;
};