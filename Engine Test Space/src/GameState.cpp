#include "..\include\GameState.hpp"

GameState::GameState()
{
	entityManager = new EntityManager("entities.ool");
	

	ZeroMemory(&sun, sizeof(sun));
	sun[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	sun[0].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 2.0f);
	sun[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	sun[0].direction = XMFLOAT3(0.0f, -0.4f, 1.0f);
	
	//sun[1].ambient = XMFLOAT4(0.84f, 0.45f, 0.07f, 1.0f);
	//sun[1].diffuse = XMFLOAT4(0.84f, 0.45f, 0.07f, 10.0f);
	//sun[1].specular = XMFLOAT4(0.84f, 0.45f, 0.07f, 1.0f);
	//sun[1].direction = XMFLOAT3(0.3f, -0.4f, 1.0f);

	sun[1].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	sun[1].diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.3f);
	sun[1].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	sun[1].direction = XMFLOAT3(0.5f, -0.4f, 1.0f);
	
	sun[2].ambient = sun[2].diffuse = sun[2].specular = XMFLOAT4();
	sun[2].direction = XMFLOAT3();
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
		terrainGenerator = new TerrainGenerator();

		delete wcmapPath;

		GeneratorSettings gs = {
			8000, // Breite
			8000, // Höhe
			5, // Layers
			30000, // Amplitude
			0.3f, // Mod_Amplitude
			-10000, // Basishöhe
			800, // Wellenlänge
			0.3f, // Mod_Wellenlänge
			4, // Mask_Offset
			1, // Fancy_Interpolation
			2, // Maske
			3000, // Wellenlänge Feuchtigkeit/Temperatur
			500, // Noise Feuchtigkeit/Temperatur
			40, // Wellenlänge Noise Feuchtigkeit/Temperatur
			0.25f, // Maßstab horizontal
			0.01f, // Maßstab vertikal
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
	GRAPHICS->teSetGeneralRenderStates(sun, 2);
	entityManager->render();
	GRAPHICS->tePresent();
}
