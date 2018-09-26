#include "..\include\GameState.hpp"

GameState::GameState()
{
	entityManager = new EntityManager("entities.ool");
	terrainGenerator = new TerrainGenerator;

	sun.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	sun.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sun.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	sun.direction = XMFLOAT3(0.0f, -0.2f, 1.0f);
}

GameState::~GameState()
{
	delete terrainGenerator;
	delete entityManager;
}

TEResult GameState::init(const char * mapPath)
{
	size_t size = strlen(mapPath) + 1;
	wchar_t * wcmapPath = new wchar_t[size];

	size_t outSize;
	mbstowcs_s(&outSize, wcmapPath, size, mapPath, size - 1);

	if (!FileLoader::teFileExists(wcmapPath))
	{
		delete wcmapPath;

		GeneratorSettings gs = {
			8000, // Breite
			8000, // H�he
			5, // Layers
			30000, // Amplitude
			0.3f, // Mod_Amplitude
			-10000, // Basish�he
			800, // Wellenl�nge
			0.3f, // Mod_Wellenl�nge
			4, // Mask_Offset
			1, // Fancy_Interpolation
			2, // Maske
			3000, // Wellenl�nge Feuchtigkeit/Temperatur
			500, // Noise Feuchtigkeit/Temperatur
			40, // Wellenl�nge Noise Feuchtigkeit/Temperatur
			0.25f, // Ma�stab horizontal
			0.01f, // Ma�stab vertikal
			0.95, // Dither
			mapPath,
			entityManager
		};

		terrainGenerator->generate(gs);
	}

	return true;
}

void GameState::tick(double deltatime)
{
	entityManager->tick(deltatime);
}

void GameState::render()
{
	GRAPHICS->teStartScene();
	GRAPHICS->teSetGeneralRenderStates(sun);
	entityManager->render();
	GRAPHICS->tePresent();
}
