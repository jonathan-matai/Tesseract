#include "..\include\TerrainGenerator.hpp"

TerrainGenerator::TerrainGenerator()
{
	LOGFILE->print(TE_CONSTRUCTION, "TerrainGenerator wurde erstellt!");
}

TerrainGenerator::~TerrainGenerator()
{
	LOGFILE->print(TE_CONSTRUCTION, "TerrainGenerator wurde geschlossen!");
}

teResult TerrainGenerator::generate(const GeneratorSettings & settings)
{
	teResult r = true;

	// Memberinitialisierung
	gs = settings;

	// Threads erstellen (THREADS: {-})
	threads.resize(max(gs.layer_count + 2, 7));

	// Wertüberprüfung
	if (gs.layer_count == 0) {
		TRACE(true, "Parameter 'layer' muss > 0 sein!");
		return false;
	}
	if (gs.layer_count > 10)
		TRACE(false, "Mehr als 10 Heighmap-Ebenen können Performance stark beeinträchtigen!");
	if (gs.amplitude_modifier > 1)
		TRACE(false, "Amplitude vergrößert sich; sollte sich verkleinern!");
	if (gs.wavelength_modifier > 1)
		TRACE(false, "Wellenlänge vergrößert sich; sollte kleiner werden!");
	if (gs.n_use_mask > gs.layer_count) {
		TRACE(true, "Parameter 'n_use_mask' muss kleiner oder gleich 'layer_count' sein!");
		return false;
	}
	if (gs.n_fancy_interpolation > gs.layer_count) {
		TRACE(true, "Parameter 'n_fancy_interpolation' muss kleiner oder gleich 'layer_count' sein!");
		return false;
	}

	// Startwert für Mapgenerator festlegen
	if (gs.seed == 0) MathHelper::seed(); else MathHelper::seed(gs.seed);

	LOGTIME(t1t);

	// Speicherreservierung
	heightmap = static_cast<int16_t*>(teAlloc(sizeof(int16_t) * gs.width * gs.height));

	LOGFILE->printf("%i Heightmaps mit Maßen %ix%i werden erstellt.", gs.layer_count, gs.width, gs.height);

	// Heightmaps erstellen (THREADS: {h}//-/-)
	int16_t cur_amplitude = gs.amplitude;
	UINT cur_raster = gs.wavelength;
	int16_t cur_base_height = gs.base_height;

	for (UINT i = 0; i < gs.layer_count; ++i)
	{
		if (cur_raster == 0) {
			TRACE(false, "Heigmapenebenen wurden teils nicht erstellt, da die Wellenlänge 0 ist!");
			gs.layer_count = i;
			continue;
		}

		threads[i] = std::thread([=] { doLayer(i, cur_raster, cur_amplitude, cur_base_height); });

		cur_amplitude = (int16_t)(cur_amplitude * gs.amplitude_modifier);
		cur_base_height = (int16_t)(cur_base_height * gs.amplitude_modifier);
		cur_raster = (UINT)(cur_raster * gs.wavelength_modifier);
	}

	// Speicherreservierung
	temperaturemap = static_cast<uint16_t*>(teAlloc(sizeof(uint16_t) * gs.width * gs.height));
	humiditymap = static_cast<uint16_t*>(teAlloc(sizeof(uint16_t) * gs.width * gs.height));

	// Temperatur und Feuchtigkeitsmap erstellen (THREADS: {h}//p/p)
	threads[max(gs.layer_count, 5)] = std::thread([=] { createParameterMap(temperaturemap); });
	threads[max(gs.layer_count + 1, 6)] = std::thread([=] {createParameterMap(humiditymap); });

	// Speicherreservierung
	gradientmap = static_cast<uint8_t*>(teAlloc(sizeof(uint8_t) * gs.width * gs.height));

	// Auf Heightmap warten, um Steigungsmap erstellen zu können (THREADS: -/-/-/-/-//p/p)
	for (UINT i = 0; i < gs.layer_count; ++i)
		threads[i].join();

	// Heightmap abspeichern (THREADS: °h/-/-/-/-//p/p)
	threads[0] = std::thread([=] {saveHeightmap(&r); });
	
	// Gradientmap erstellen (THREADS: °h/g/g/g/g//p/p)
	threads[1] = std::thread([=] {createGradientMap(1, 0, gs.width / 2, 0, gs.height / 2); });
	threads[2] = std::thread([=] {createGradientMap(2, 0, gs.width / 2, gs.height / 2, gs.height); });
	threads[3] = std::thread([=] {createGradientMap(3, gs.width / 2, gs.width, 0, gs.height / 2); });
	threads[4] = std::thread([=] {createGradientMap(4, gs.width / 2, gs.width, gs.height / 2, gs.height); });
	
	// Auf Temperatur und Feuchtigkeitsmaps warten, um Biommap erstellen zu können (THREADS: °h/g/g/g/g/-/-//)
	threads[max(gs.layer_count, 5)].join();
	threads[max(gs.layer_count + 1, 6)].join();

	// Speicherreservierung
	biomemap = static_cast<uint8_t*>(teAlloc(sizeof(uint8_t) * gs.width * gs.height));

	// Biommap erstellen
	createBiomeMap();

	// Biomemap abspeichern (THREADS: °h/g/g/g/g/°b/-//)
	threads[5] = std::thread([=] {saveBiomemap(&r); });

	// Speicherfreigabe
	RELEASE_PTR(humiditymap);
	RELEASE_PTR(temperaturemap);

	// Speicherreservierung
	weightmap = static_cast<uint8_t*>(teAlloc(sizeof(uint8_t) * gs.width * gs.height));

	// Strukturen erstellen (THREADS: °h/g/g/g/g/°b/s//)
	threads[6] = std::thread([=] {generateStructures(); });

	// Auf Steigungsmap warten, um sie abzuspeichern (THREADS: °h/-/-/-/-/°b/s//)
	for (UINT i = 1; i < 5; ++i)
		threads[i].join();

	// Steigungsmap abspeichern
	saveGradientmap(&r);
	RELEASE_PTR(gradientmap);

	// Auf Strukturmap warten, um diese zu erstellen (THREADS: °h/-/-/-/-/°b/-//)
	threads[6].join();

	// Auf Biommapspeicherung warten, um Speicher freizugeben (THREADS: °h/-/-/-/-/-/-//)
	threads[5].join();
	RELEASE_PTR(biomemap);

	// Auf Threads warten, um Speicher freizugeben (THREADS: {-})
	threads[0].join();
	RELEASE_PTR(heightmap);

	LOGTIME(t2t);
	LOGFILE->printf(TE_SUCCEEDED, "Terrain wurde erstellt. (%i ms)", DELTATIME(t1t, t2t));

	return r;
}

void TerrainGenerator::doLayer(UINT layer, UINT raster, int16_t amplitude, int16_t base_height)
{
	// Speicherreservierung
	int16_t* layerArr = static_cast<int16_t*>(teAlloc(sizeof(int16_t) * gs.width * gs.height));

	LOGTIME(t1s);
	if (layer < gs.n_use_mask)
		rasterize(layerArr, raster, amplitude, base_height, true);
	else
		rasterize(layerArr, raster, amplitude, base_height);

	if (layer < gs.n_fancy_interpolation) {
		interpolateHorizontal(layerArr, raster, &MathHelper::cosineInterpolation);
		interpolateVertical(layerArr, raster, &MathHelper::cosineInterpolation);
	}
	else {
		interpolateHorizontal(layerArr, raster, &MathHelper::linearInterpolation);
		interpolateVertical(layerArr, raster, &MathHelper::linearInterpolation);
	}

	add(layerArr, heightmap);

	// Speicherfreigabe
	RELEASE_PTR(layerArr);

	LOGTIME(t2s);
	LOGFILE->printf(TE_VALUE, "Heightmapebene %i/%i: a = %i, w = %i (%i ms)",
		layer + 1, gs.layer_count, amplitude, raster, DELTATIME(t1s, t2s));
}

void TerrainGenerator::createParameterMap(uint16_t* arr)
{
	uint16_t* layer = static_cast<uint16_t*>(teAlloc(sizeof(uint16_t) * gs.width * gs.height));

	LOGTIME(t1);

	rasterize(layer, gs.humidity_wavelength, 32000, 0);
	interpolateHorizontal(layer, gs.humidity_wavelength, &MathHelper::cosineInterpolation);
	interpolateVertical(layer, gs.humidity_wavelength, &MathHelper::cosineInterpolation);
	add(layer, arr);

	rasterize(layer, gs.humidity_noise_wavelength, gs.humidity_noise, 0);
	interpolateHorizontal(layer, gs.humidity_noise_wavelength, &MathHelper::linearInterpolation);
	interpolateVertical(layer, gs.humidity_noise_wavelength, &MathHelper::linearInterpolation);
	add(layer, arr);

	RELEASE_PTR(layer);

	LOGTIME(t2);
	LOGFILE->printf(TE_VALUE, "Parametermap: w = %i (%i ms)", gs.humidity_wavelength, DELTATIME(t1, t2));
}

void TerrainGenerator::createBiomeMap()
{
	LOGTIME(t1);

	for (UINT i = 0; i < gs.height * gs.width; ++i) {
		if (heightmap[i] < -0.1*INT16_MAX) // Ocean Level
		{
			biomemap[i] = OCEAN;
		}
		else if (heightmap[i] < 0 * INT16_MAX) // Beach Level
		{
			if (temperaturemap[i] < 0.15*INT16_MAX)
				biomemap[i] = SNOWBEACH;
			else if (temperaturemap[i] < 0.6*INT16_MAX)
				biomemap[i] = STONEBEACH;
			else
				biomemap[i] = SANDBEACH;
		}
		else if (heightmap[i] < 0.3*INT16_MAX) // Plains Level
		{
			if (temperaturemap[i] < 0.15*INT16_MAX)
				biomemap[i] = SNOWPLAINS;
			else if (temperaturemap[i] < 0.2*INT16_MAX) {
				if (humiditymap[i] < 0.3*INT16_MAX)
					biomemap[i] = SWAMP;
				else if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = PLAINS;
				else
					biomemap[i] = TUNDRA;
			}
			else if (temperaturemap[i] < 0.4*INT16_MAX) {
				if (humiditymap[i] < 0.25*INT16_MAX)
					biomemap[i] = SWAMP;
				else
					biomemap[i] = PLAINS;
			}
			else if (temperaturemap[i] < 0.6*INT16_MAX) {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = FOREST;
				else
					biomemap[i] = SAVANNA;
			}
			else {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = JUNGLE;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = SAVANNA;
				else
					biomemap[i] = DESERT;
			}

		}
		else if (heightmap[i] < 0.4*INT16_MAX) // Forest Level
		{
			if (temperaturemap[i] < 0.2*INT16_MAX) {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = SNOWTAIGAEDGE;
				if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = TAIGAEDGE;
				else
					biomemap[i] = PINEFOREST;
			}
			else if (temperaturemap[i] < 0.4*INT16_MAX) {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = PINEFOREST;
				if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = FOREST;
				else
					biomemap[i] = PLAINS;
			}
			else if (temperaturemap[i] < 0.6*INT16_MAX) {
				if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = FOREST;
				else
					biomemap[i] = PLAINS;
			}
			else {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = JUNGLE;
				else
					biomemap[i] = SAVANNA;
			}
		}
		else if (heightmap[i] < 0.45*INT16_MAX) // Taiga Level
		{
			if (temperaturemap[i] < 0.2*INT16_MAX) {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = SNOWTAIGA;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = SNOWTAIGAEDGE;
				else
					biomemap[i] = SNOWPLAINS;
			}
			else if (temperaturemap[i] < 0.4*INT16_MAX) {
				if (humiditymap[i] < 0.25*INT16_MAX)
					biomemap[i] = SNOWTAIGA;
				else if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = TAIGA;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = TAIGAEDGE;
				else
					biomemap[i] = PLAINS;
			}
			else if (temperaturemap[i] < 0.6*INT16_MAX) {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = TAIGA;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = TAIGAEDGE;
				else
					biomemap[i] = PLAINS;
			}
			else if (temperaturemap[i] < 0.8*INT16_MAX) {
				if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = PINEFOREST;
				else
					biomemap[i] = PLAINS;
			}
			else {
				if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = FOREST;
				else
					biomemap[i] = PLAINS;
			}
		}
		else if (heightmap[i] < 0.6*INT16_MAX) // Tundra Level
		{
			if (temperaturemap[i] < 0.4*INT16_MAX) {
				if (humiditymap[i] < 0.25*INT16_MAX)
					biomemap[i] = SNOWTAIGAEDGE;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = SNOWPLAINS;
				else
					biomemap[i] = TUNDRA;
			}
			else if (temperaturemap[i] < 0.6*INT16_MAX) {
				if (humiditymap[i] < 0.25*INT16_MAX)
					biomemap[i] = TAIGAEDGE;
				else if (humiditymap[i] < 0.75*INT16_MAX)
					biomemap[i] = TUNDRA;
				else
					biomemap[i] = PLAINS;
			}
			else {
				if (humiditymap[i] < 0.5*INT16_MAX)
					biomemap[i] = TAIGAEDGE;
				else
					biomemap[i] = PLAINS;
			}
		}
		else if (heightmap[i] < 0.7*INT16_MAX) // Alpine Level
		{
			if (temperaturemap[i] < 0.4*INT16_MAX)
				biomemap[i] = SNOWMOUNTAINS;
			else
				biomemap[i] = MOUNTAINS;
		}
		else // Snow Level
		{
			biomemap[i] = SNOWMOUNTAINS;
		}
	}
	LOGTIME(t2);
	LOGFILE->printf("Biommap wurde erstellt. (%i ms)", DELTATIME(t1, t2));
}

void TerrainGenerator::createGradientMap(UINT sector, UINT xStart, UINT xEnd, UINT yStart, UINT yEnd)
{
	LOGTIME(t1);
	for (UINT i = xStart; i < xEnd; ++i)
		for (UINT j = yStart; j < yEnd; ++j) {
			uint8_t gradient = 0;
			for (UINT k = i - 1; k < i + 2; ++k)
				for (UINT l = j - 1; l < j + 2; ++l) {
					if (k == i && l == j) // Ist Nachbarfeld das eigene Feld? -> weiter
						continue;
					if (k < 0 || l < 0 || k >= gs.width || l >= gs.height) // Ist Nachbarfeld außerhalb der Map? -> weiter
						continue;
					uint8_t newGradient = static_cast<uint8_t>(abs(heightmap[gs.width * j + i] - heightmap[gs.width * l + k]) / 256);
					if (newGradient > gradient)
						gradient = newGradient;
				}
			gradientmap[gs.width * j + i] = gradient;
		}
	LOGTIME(t2);
	LOGFILE->printf(TE_VALUE, "Sektor %i der Steigungsmap erstellt! (%i ms)", sector, DELTATIME(t1, t2));
}

void TerrainGenerator::generateStructures()
{
	LOGTIME(t1);
	double dither;
	TE_OBJECT_DESC initDesc;

	for (UINT i = 0; i < gs.width; ++i)
		for (UINT j = 0; j < gs.height; ++j) {

			dither = MathHelper::rand(0, 1);

			initDesc.pos = { i*gs.scale_horizontal, heightmap[j*gs.width + i] * gs.scale_vertical, j*gs.scale_horizontal };
			initDesc.scale = { 1.0f, 1.0f, 1.0f };
			initDesc.rotation = { 0.0f, 0.0f, 0.0f };
			initDesc.file1 = NULL;
			initDesc.file2 = NULL;
			initDesc.type = TE_OBJECT_TYPE_OBJECT;

			switch (biomemap[j*gs.width + i]) {
			case OCEAN:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = CORAL;

					gs.entityManager->add(new TEObject(initDesc));
				}	
				continue;
			case SNOWBEACH:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = ICEFLOE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case STONEBEACH:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = ROCK;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case SANDBEACH:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = PALMTREE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case SWAMP:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					if (MathHelper::rand(0, 1) < 0.3)
					{
						initDesc.subType = WILLOW;

						gs.entityManager->add(new TEObject(initDesc));
					}
					else
					{
						initDesc.subType = SAPLING;

						gs.entityManager->add(new TEObject(initDesc));
					}
				}
				continue;
			case PLAINS:
			case SNOWPLAINS:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = BUSH;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case DESERT:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = DEADBUSH;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case FOREST:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = OAK;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case JUNGLE:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = JUNGLETREE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case SAVANNA:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = ACACIA;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case PINEFOREST:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = PINE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case TAIGA:
			case SNOWTAIGA:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = SPRUCE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case TAIGAEDGE:
			case SNOWTAIGAEDGE:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = SPRUCE;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case TUNDRA:
				if (dither > 0.99 && spaceAvailable(i, j, 1, 4, 2))
				{
					initDesc.subType = BUSH;

					gs.entityManager->add(new TEObject(initDesc));
				}
				continue;
			case MOUNTAINS:
			case SNOWMOUNTAINS:
				continue;
			default:
				continue;
			}
		}

	LOGTIME(t2);
	LOGFILE->printf("Strukturen wurden platziert. (%i ms)", DELTATIME(t1, t2));
}

template <class c>
void TerrainGenerator::rasterize(c* arr, UINT raster, int16_t amplitude, int16_t base_height)
{
	for (UINT i = 0; i < gs.width; i += raster)
		for (UINT j = 0; j < gs.height; j += raster)
			arr[gs.width * j + i] = (c)(MathHelper::rand(0, amplitude) + base_height);
}

void TerrainGenerator::rasterize(int16_t* arr, UINT raster, int16_t amplitude, int16_t base_height, bool doMask)
{
	for (UINT i = 0; i < gs.width; i += raster)
		for (UINT j = 0; j < gs.height; j += raster)
			arr[gs.width * j + i] = mask((int16_t)MathHelper::rand(0, amplitude), i, j) + base_height;
}

int16_t TerrainGenerator::mask(int16_t y, int x, int z)
{
	return (int16_t)(y * min(1.0f, max(0.0f, -POW2(4.0f * (x - (int)gs.width / 2) / (int)gs.width) - POW2(4.0f * (z - (int)gs.height / 2) / (int)gs.height) + gs.mask_offset)));
}

template <class c>
void TerrainGenerator::interpolateHorizontal(c* arr, UINT raster,
	float(*intAlgorithm)(float, float, float, float, float))
{
	// Aufteilen in Vertizen, die zwischen den selben bereits definierten Vertizen liegen
	int remainder;
	UINT units = gs.width / raster;
	// Berechnung der Restmenge, die nicht zwischen zwei Vertizen liegt
	switch (int modulo = gs.width % raster) {
	case 0: remainder = raster - 1; units--; break;
	case 1: remainder = 0; break;
	default: remainder = modulo - 1; break;
	}

	for (UINT j = 0; j < gs.height; j += raster) {
		//Interpolation zwei Vertizen
		for (UINT k = 0; k < units; ++k) {
			for (UINT i = 1; i < raster; ++i) {
				arr[gs.width * j + (k*raster + i)] = (c)intAlgorithm(
					(float)(k * raster),
					(float)arr[gs.width * j + k*raster],
					(float)((k + 1) * raster),
					(float)arr[gs.width * j + (k + 1)*raster],
					(float)(k * raster + i));
			}
		}
		// Interpolation Vertiz - Ende
		int k = gs.width - remainder;
		for (UINT i = k; i < gs.width; ++i) {
			arr[gs.width * j + i] = (c)intAlgorithm(
				(float)k - 1,
				(float)arr[gs.width * j + (k - 1)],
				(float)gs.width,
				(float)arr[gs.width * j],
				(float)i);
		}
	}
}

template <class c>
void TerrainGenerator::interpolateVertical(c* arr, UINT raster,
	float(*intAlgorithm)(float, float, float, float, float))
{
	int remainder;
	UINT units = gs.height / raster;

	switch (int modulo = gs.height % raster) {
	case 0: remainder = raster - 1; units--; break;
	case 1: remainder = 0; break;
	default: remainder = modulo - 1; break;
	}

	for (UINT i = 0; i < gs.width; ++i) {

		for (UINT k = 0; k < units; ++k) {
			for (UINT j = 1; j < raster; ++j) {
				arr[gs.width * (j + k * raster) + i] = (c)intAlgorithm(
					(float)(k * raster),
					(float)arr[gs.width * (k * raster) + i],
					(float)((k + 1) * raster),
					(float)arr[gs.width * (k + 1) * raster + i],
					(float)(k * raster + j));
			}
		}

		int k = gs.height - remainder;
		for (UINT j = k; j < gs.height; ++j) {
			arr[gs.width * j + i] = (c)intAlgorithm(
				(float)k - 1,
				(float)arr[gs.width * (k - 1) + i],
				(float)gs.height,
				(float)arr[i],
				(float)j);
		}
	}
}

template <class c1, class c2>
void TerrainGenerator::add(c1* in, c2* out)
{
	for (UINT i = 0; i < gs.width * gs.height; ++i)
		out[i] += (c2)in[i];
}

bool TerrainGenerator::spaceAvailable(UINT x, UINT y, UINT rBlocked, UINT rClaimed, UINT rHindered)
{
	// Platz überprüfen
	for (UINT i = max(0, x - rClaimed); i < min(x + rClaimed, gs.width); ++i)
		for (UINT j = max(0, y - rClaimed); j < min(y + rClaimed, gs.height); ++j)
			if (weightmap[j * gs.width + i] > 0)
				return false;
	// Erschwertes Gehen festlegen
	for (UINT i = max(0, x - rHindered); i < min(x + rHindered, gs.width); ++i)
		for (UINT j = max(0, y - rHindered); j < min(y + rHindered, gs.height); ++j)
			weightmap[j * gs.width + i] += 4;
	// Kein Gehen festlegen
	for (UINT i = max(0, x - rHindered); i < min(x + rHindered, gs.width); ++i)
		for (UINT j = max(0, y - rHindered); j < min(y + rHindered, gs.height); ++j)
			weightmap[j * gs.width + i] = 128;

	return true;
}

void TerrainGenerator::saveHeightmap(const teResult* result)
{
	std::string path = gs.path + "heightmap.thm";
	std::ofstream output(path, std::ios::binary);

	if (!output.is_open()) {
		char msg[128];
		sprintf_s(msg, "Auf %s konnte nicht zugegriffen werden!", path.c_str());
		TRACE(true, msg);
		result = false;
		return;
	}

	UINT thm_ver = THM_VER;

	output.write((char*)&gs.width, sizeof(UINT));
	output.write((char*)&gs.height, sizeof(UINT));
	output.write((char*)&thm_ver, sizeof(UINT));
	output.write((char*)&gs.scale_horizontal, sizeof(float));
	output.write((char*)&gs.scale_vertical, sizeof(float));
	output.write((char*)heightmap, sizeof(short) * gs.width * gs.height);

	output.close();
	LOGFILE->printf("%s wurde gespeichert!", path.c_str());
}

void TerrainGenerator::saveBiomemap(const teResult* result)
{
	std::string path = gs.path + "biomemap.tbm";
	std::ofstream output(path, std::ios::binary);

	if (!output.is_open()) {
		char msg[128];
		sprintf_s(msg, "Auf %s konnte nicht zugegriffen werden!", path.c_str());
		TRACE(true, msg);
		result = false;
		return;
	}

	UINT tbm_ver = TBM_VER;

	output.write((char*)&gs.width, sizeof(UINT));
	output.write((char*)&gs.height, sizeof(UINT));
	output.write((char*)&tbm_ver, sizeof(UINT));
	output.write((char*)biomemap, sizeof(uint8_t) * gs.width * gs.height);

	output.close();
	LOGFILE->printf("%s wurde gespeichert!", path.c_str());
}

void TerrainGenerator::saveGradientmap(const teResult* result)
{
	std::string path = gs.path + "gradientmap.tgm";
	std::ofstream output(path, std::ios::binary);

	if (!output.is_open()) {
		char msg[128];
		sprintf_s(msg, "Auf %s konnte nicht zugegriffen werden!", path.c_str());
		TRACE(true, msg);
		result = false;
		return;
	}

	UINT tgm_ver = TGM_VER;

	output.write((char*)&gs.width, sizeof(UINT));
	output.write((char*)&gs.height, sizeof(UINT));
	output.write((char*)&tgm_ver, sizeof(UINT));
	output.write((char*)gradientmap, sizeof(uint8_t) * gs.width * gs.height);

	output.close();
	LOGFILE->printf("%s wurde gespeichert!", path.c_str());
}