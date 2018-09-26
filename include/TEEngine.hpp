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

/*============================BENUTZUNG=======================
	
	Eine TE_INIT-Struktur ausfüllen

	Die Engine wird durch den Aufruf von teInit() initialisiert
	Sämtliche Subsystems werden hochgefahren

	Wenn ein Object hinzugefügt werden soll, die entsprechende
	Description ausfüllen und als Parameter der teAddObject()
	übergeben

	Um die Engine zu "starten" teDoMessageLoop aufrufen
	und die Pointer auf Render() und Move() übergeben

	Die Engine fährt mit teExit() herunter


	Zugriff auf die Engine möglich durch ENGINE

	Beispiel:

	ENGINE->teInit(init);
	ENGINE->teAddObject(object);
	ENGINE->teExit();

=============================================================*/

//Includes
#include "TEGlobal.hpp"
#include "Language.hpp"
//#include "TEAudio.hpp" //OVER UNKNOWN TIME OUT OF ORDER DUE TO SOME HEADER-CHANGES; NOT IN BUILD ANYMORE
#include "Timer.hpp"
#include "TEObject.hpp"
#include "TEGraphics.hpp"

//Defines
#define ENGINE teEngine::get()

class teEngine : public Singleton<teEngine>
{
	public:

		//==========Steuerfunktionen der Engine===============

		//Initfunktion, in der alle Subsystems initialisiert werden; danach ist die Engine lauffähig
		teResult teInit(TE_INIT & init, WNDPROC wndProc);

		//Aufrufen zum Herunterfahren und sicheren Beenden der Engine
		teResult teExit();

		//Betritt die MessageLoop
		teResult teDoMessageLoop(void (*Update)(float), void (*Render)());

		//Getter der Subsystems

		TEGraphics * teGetGraphics() { return m_teGraphics; }
		QUICK_CAM teGetQuickCam() { return m_quickCam; }

	private:

		//Funktionen

		//Berechnet und zeigt die FPS und Frametime an
		void teCalculateFPS();

		//========Subsystems===================

		//Graphicsengine
		TEGraphics * m_teGraphics;

		//Timer
		Timer * m_teTimer;

		//========Members======================

		//Mapvars
		std::string thmname = "thmap.thm";
		std::string tbmname = "tbmap.tbm";

		//Window
		HWND m_hWnd;
		std::wstring m_windowTitle;

		//QUICK_CAM
		QUICK_CAM m_quickCam;
};