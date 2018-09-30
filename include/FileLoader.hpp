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
#include "DDSTextureLoader.hpp"

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

	static DirectX::XMFLOAT2 teBiomToTextureCoord(uint8_t biom)
	{
		switch (biom)
		{
			case 0:
			{
				return DirectX::XMFLOAT2(0.5f, 0.0f);
			}
			case 5:
			{
				return DirectX::XMFLOAT2(0.0f, 0.0f);
			}
			case 17:
			{
				return DirectX::XMFLOAT2(0.0f, 0.5f);
			}
			default:
			{
				return DirectX::XMFLOAT2(0.5f, 0.5f);
			}
		}
	}

	static bool teLoadTextureFromFile(ID3D11Device * device, wchar_t * filename, ID3D11ShaderResourceView**srv)
	{
		if (FAILED(CreateDDSTextureFromFile(device, filename, srv)))
		{
			return false;
		}

		return true;
	}
}

using namespace FileLoader;