#include <Windows.h>
#include <iostream>

#include "../../include/Logfile.hpp"
#include "..\include\GameState.hpp"
#include "../../include/MemoryManager.hpp"
#include "../../include/Language.hpp"
#include "..\include\GameGlobal.hpp"

void update(float deltatime);
void render();
State* curState = nullptr;
Player * player = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Einstiegspunkt des Spiels
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pcLine, int showPcLine)
{
	TE_INIT init;

	init.hInstance = hInstance;
	init.iniFile = L"res/Data/TEConfig.ini";
	init.shaderFiles = L"res/Data/Shader/DefaultEffects.fx";
	init.windowName = L"Oneiro Alpha";

	ENGINE->teInit(init, WndProc);

	// State erstellen und setzen
	curState = new GameState();
	static_cast<GameState*>(curState)->init("res/Data/Meshes/Map/");

	TE_OBJECT_DESC playerdesc;
	ZeroMemory(&playerdesc, sizeof(TE_OBJECT_DESC));
	playerdesc.pos = XMFLOAT3(2000.0f, 100.0f, 2000.0f);
	playerdesc.type = TE_OBJECT_TYPE_PLAYER;

	player = new Player(playerdesc);

	//GRAPHICS->teGetCamera()->teSetCamPos(vPointToXM(player->getPlayersPos()));

	TE_OBJECT_DESC objectdesc;

	objectdesc.file1 = L"res/Data/Meshes/Map/thmap.thm";
	objectdesc.file2 = L"res/Data/Meshes/Map/tbmap.tbm";
	objectdesc.pos = { 0.0f, 0.0f, 0.0f };
	objectdesc.scale = { 1.0f, 1.0f, 1.0f };
	objectdesc.type = TE_OBJECT_TYPE_MAP;

	static_cast<GameState*>(curState)->getEntityManager()->add(new TEMap(objectdesc));

	//set mouse in screen middle
	SetCursorPos(960, 540);

	ENGINE->teDoMessageLoop(update, render);

	delete curState;

	return 0;
}

void update(float deltatime)
{
	// Tick- und Rendermethoden des aktuellen States
	if (GetAsyncKeyState('W') & 0x8000)
	{
		GRAPHICS->teGetCamera()->teWalk(100.0f * deltatime);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		GRAPHICS->teGetCamera()->teWalk(-100.0f * deltatime);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		GRAPHICS->teGetCamera()->teStrafe(-100.0f * deltatime);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		GRAPHICS->teGetCamera()->teStrafe(100.0f * deltatime);
	}

	float dx = 0.0f, dy = 0.0f;
	POINT mp;

	//Get the current mouse position on screen
	GetCursorPos(&mp);

	//Get the pixels mouse moved
	dx = mp.x - (1920 / 2);
	dy = mp.y - (1080 / 2);

	//change in radians to provide fitting format for camera functions
	dx = TE_DEG_TO_RAD(dx);
	dy = TE_DEG_TO_RAD(dy);

	//clip the value, so not every minimal movement is visible
	GRAPHICS->teGetCamera()->teRotateY(dx * 0.15f);
	GRAPHICS->teGetCamera()->tePitch(dy * 0.15f);

	SetCursorPos(960, 540);

	player->tick(deltatime);
	GRAPHICS->teGetCamera()->teUpdateView();
	curState->tick(deltatime);
}

void render()
{
	curState->render();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_QUIT:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			DestroyWindow(hWnd);
			return 0;
		}break;
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case 0x09:
				{
					GRAPHICS->teSetRenderMode(0);
				}break;
				case 0x10:
				{
					GRAPHICS->teSetRenderMode(1);
				}break;
				case VK_ESCAPE:
				{
					PostQuitMessage(0);
					DestroyWindow(hWnd);
					return (int)(wParam);
				}
				
			}
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
