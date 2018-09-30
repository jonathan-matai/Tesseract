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

//Includes
#include "..\include\TEEngine.hpp"

//==========Funktionsdeklarationen============

//Aufgaben: Fenster erstellen und alle Subsystems initen
teResult teEngine::teInit(TE_INIT & init, WNDPROC wndProc)
{
	m_windowTitle = init.windowName;

	//Zufallsseed erstellen
	MathHelper::seed();

	//Fenster erstellen
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;

	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 5;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = init.windowName;
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Fensterklasse konnte nicht initialisiert werden!", L"Fehler", TE_OK_ERROR);
	}

	m_hWnd = CreateWindowEx(NULL, init.windowName, init.windowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 1, 1, 1920, 1080, NULL, NULL, init.hInstance, NULL);
	if (m_hWnd == NULL)
	{
		MessageBox(NULL, L"Fenster konnte nicht erstellt werden!", L"Fehler", TE_OK_ERROR);
	}
	//Fenstererstellung komplett

	//Subsystems in der richtigen Reihenfolge nacheinander erstellne
	//Zuerst MemoryManager und Logfile
	LOGFILE->createLogfile(TE_VERSION);
	teInitMemory();

	LOGFILE->print(colors::TE_SUCCEEDED, "MemoryManager und Logfile wurden erfolgreich initialisiert.");

	//Graphicsengine initialisieren
	m_teGraphics = new TEGraphics();
	if (!m_teGraphics->teInit(init.iniFile, m_hWnd, init.shaderFiles))
	{
		LOGFILE->error(true, "Graphicsengine konnte nicht initialisiert werden!", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
	{
		return false;
	}
		// error
#else
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
		// error
#endif

	m_quickCam.pos = m_teGraphics->teGetCamera()->teGetCamPosXM();
	m_quickCam.lookAt = m_teGraphics->teGetCamera()->teGetLookAtXM();
	m_quickCam.right = m_teGraphics->teGetCamera()->teGetRightXM();
	m_quickCam.up = m_teGraphics->teGetCamera()->teGetUpXM();
	m_quickCam.fovx = m_teGraphics->teGetCamera()->teGetFOVX();
	m_quickCam.fovy = m_teGraphics->teGetCamera()->teGetFOVY();
	m_quickCam.farplane = m_teGraphics->teGetCamera()->teGetFarZ();
	m_quickCam.nearplane = m_teGraphics->teGetCamera()->teGetNearZ();

	LOGFILE->print(colors::TE_SUCCEEDED, "Graphicsengine wurde erfolgreich initialisiert.");

	//Timer initialisieren
	m_teTimer = new Timer();
	m_teTimer->Reset();

	LOGFILE->print(colors::TE_SUCCEEDED, "Tesseract-Engine wurde erfolgreich initialisiert.\nAlle Subsystems wurden hochgefahren und sind betriebsbereit.");

	return true;
}

teResult teEngine::teExit()
{
	m_teGraphics->teExit();
	teExitMemory();

	LOGFILE->print(colors::TE_SUCCEEDED, "Engine erfolgreich heruntergefahren.");

	LOGFILE->del();

	return true;
}

teResult teEngine::teDoMessageLoop(void(*Update)(float), void(*Render)())
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_quickCam.pos = m_teGraphics->teGetCamera()->teGetCamPosXM();
			m_quickCam.lookAt = m_teGraphics->teGetCamera()->teGetLookAtXM();
			m_quickCam.right = m_teGraphics->teGetCamera()->teGetRightXM();
			m_quickCam.up = m_teGraphics->teGetCamera()->teGetUpXM();
			m_quickCam.fovx = m_teGraphics->teGetCamera()->teGetFOVX();
			m_quickCam.fovy = m_teGraphics->teGetCamera()->teGetFOVY();
			m_quickCam.farplane = m_teGraphics->teGetCamera()->teGetFarZ();
			m_quickCam.nearplane = m_teGraphics->teGetCamera()->teGetNearZ();
			m_teTimer->Tick();
			Update(m_teTimer->DeltaTime());
			Render();
			teCalculateFPS();
		}
	}

	return true;
}

void teEngine::teCalculateFPS()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_teTimer->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;

		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_windowTitle << L"   " << L"FPS: " << fps << L" / " << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
