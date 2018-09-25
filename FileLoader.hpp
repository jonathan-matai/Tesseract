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

#include <Windows.h>
#include <string>
#include <sstream>
#include "TEMLoader.hpp"

namespace FileLoader
{
	//Lädt einen String aus der angegebenen Sektion der Inidatei
	static std::string teLoadIniString(wchar_t * section, wchar_t* key, wchar_t*file)
	{
		wchar_t out[265];
		DWORD buffer = ARRAYSIZE(out);
		GetPrivateProfileString(section, key, L"NOT FOUND", out, buffer, file);

		if (out != L"NOT FOUND")
		{
			std::wstring wos(out);

			std::string os(wos.begin(), wos.end());

			return os;
		}

		return "NOT FOUND";
	}

	//Lädt einen Bool aus der angegebenen Sektion der Inidatei
	static bool teLoadIniBool(wchar_t*section, wchar_t*key, wchar_t*file)
	{
		bool b = false;
		std::istringstream(teLoadIniString(section, key, file)) >> std::boolalpha >> b;
		return b;
	}

	//Lädt einen Integer aus der angegebenen Sektion der Inidatei
	static int teLoadIniInt(wchar_t * section, wchar_t* key, wchar_t*file)
	{
		return std::atoi(teLoadIniString(section, key, file).c_str());
	}

	//Lädt einen Float aus der angegebenen Sektion der Inidatei
	static float teLoadIniFloat(wchar_t * section, wchar_t* key, wchar_t*file)
	{
		return (float)std::atof(teLoadIniString(section, key, file).c_str());
	}

	//Lädt einen Double aus der angegebenen Sektion der Inidatei
	static double teLoadIniDouble(wchar_t * section, wchar_t* key, wchar_t*file)
	{
		return std::atof(teLoadIniString(section, key, file).c_str());
	}

	//Prüft ob die Datei existiert
	static bool teFileExists(wchar_t*file)
	{
		return ((GetFileAttributes(file) == INVALID_FILE_ATTRIBUTES) ? false : true);
	}

	//Wandelt ein als uint angegebenes Biom zu einer Farbe
	static XMFLOAT4 teBiomToColor(uint8_t biom)
	{
		switch (biom)
		{
			case 0:
			{
				return XMFLOAT4(0.0f, 0.81f, 0.82f, 1.0f);
			}
			case 1:
			{
				return XMFLOAT4(0.863f, 0.863f, 0.863f, 1.0f);
			}
			case 2:
			{
				return XMFLOAT4(0.545f, 0.545f, 0.48f, 1.0f);
			}
			case 3:
			{
				return XMFLOAT4(0.96f, 0.64f, 0.376f, 1.0f);
			}
			case 4:
			{
				return XMFLOAT4(1.0f, 0.98f, 0.98f, 1.0f);
			}
			case 5:
			{
				return XMFLOAT4(0.196f, 0.98f, 0.196f, 1.0f);
			}
			case 6:
			{
				return XMFLOAT4(0.545f, 0.396f, 0.031f, 1.0f);
			}
			case 7:
			{
				return XMFLOAT4(1.0f, 0.843f, 0.0f, 1.0f);
			}
			case 8:
			{
				return XMFLOAT4(0.1333f, 0.545f, 0.1333f, 1.0f);
			}
			case 9:
			{
				return XMFLOAT4(0.0f, 0.545f, 0.0f, 1.0f);
			}
			case 10:
			{
				return XMFLOAT4(0.9333f, 0.9f, 0.522f, 1.0f);
			}
			case 11:
			{
				return XMFLOAT4(0.0f, 1.0f, 0.498f, 1.0f);
			}
			case 12:
			case 13:
			{
				return XMFLOAT4(0.486f, 0.803f, 0.486f, 1.0f);
			}
			case 14:
			case 15:
			{
				return XMFLOAT4(1.0f, 0.98f, 0.98f, 1.0f);
			}
			case 16:
			{
				return XMFLOAT4(0.803f, 0.2f, 0.2f, 1.0f);
			}
			case 17:
			{
				return XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			}
			case 18:
			{
				return XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			default:
			{
				return XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			}
		}
	}
}

using namespace FileLoader;