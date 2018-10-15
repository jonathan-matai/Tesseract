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

//Includes
#include "TEGlobal.hpp"
#include "TECamera.hpp"

//Enum für die Rendermodes
typedef enum TERenderMode{TE_RENDER_SOLID = 0, TE_RENDER_WIREFRAME = 1}TERenderMode;

//Die Graphicsengine
class TEGraphics
{
	public:

		//Zum Initen der Graphicsengine aufrufen
		teResult teInit(wchar_t * iniFile, HWND hWnd, wchar_t * shaderFile);

		//Zum Herunterfahren der Graphicsengine aufrufen
		teResult teExit();

		//Setzt den Rendermode (TERenderMode)
		void teSetRenderMode(short rendermode);

		//Kompiliert die Shader neu; setzt also eine neue Shaderdatei ein
		void teSetShader(char * techniquename);

		//Präsentiert den Backbuffer
		void tePresent();

		//Startet die Szene; bereitet alles für das Rendern vor
		teResult teStartScene();

		//Zeichnet den String auf den Bildschirm
		teResult teDrawString(char * message, int x, int y);

		//Getter
		ID3D11Device * teGetDevice() { return m_pDevice; };
		ID3D11DeviceContext * teGetDeviceContext() { return m_pImmidiateContext; };
		ID3DX11EffectTechnique * teGetEffectTechnique() { return m_pFXTechnique; };
		TECamera * teGetCamera() { return m_pCamera; };
		

		//Setter
		//void teSetWVPMatrix(XMMATRIX objectWorldMatrix) { XMMATRIX wvpm = objectWorldMatrix * m_pCamera->teGetViewMatrix()*m_pCamera->teGetProjectionMatrix(); m_pFXWVPM->SetMatrix(reinterpret_cast<float*>(&wvpm)); };
		void teSetObjectRenderStates(XMMATRIX objectWorldMatrix, LightMaterial objectMat, XMMATRIX textransform);
		void teSetObjectTexture(ID3D11ShaderResourceView * tex);
		void teSetGeneralRenderStates(DirectionalLight * sun, UINT numLightSources = 1);
		void teSyncCameras(XMFLOAT3 pos) { m_pCamera->teSetCamPos(pos); };

	private:

		//Memberfunktionen

		teResult teLoadGraphicsIni(); //Lädt die Informationen aus der Ini-Datei
		void teCreateGraphicsIni(); //Erstellt die Informationen der Graphicsengine und speichert sie in der Ini-Datei
		void teGetMaxSample(); //Findet die maximal möglichen Multisamplesettings heraus und weist sie den Variablen zu
		teResult teCreateShader(); //Erstellt die FXEffectklasse
		teResult teCreateInputLayout(); //Erstellt und bindet das InputLayout der Vertizen und verknüpft dieses mit dem Shader

		//Membervariablen
		wchar_t * m_iniFile, * m_iniSection = L"TEGraphics", * m_shaderFile;
		char * m_techniqueName;
		UINT m_width, m_height, m_bufferCount, m_num, m_denom, m_sampleQuality, m_sampleCount;
		bool m_enableMultisampling, m_fullscreen;
		float m_aspectRatio, m_farRenderDist;
		short m_usedRenderMode;

		//Kamera
		TECamera * m_pCamera;

		//DX11-spezifische Variablen
		ID3D11Device * m_pDevice;
		ID3D11DeviceContext * m_pImmidiateContext;
		IDXGIFactory * m_pFactory;
		IDXGISwapChain * m_pSwapChain;
		ID3D11Texture2D * m_pBackBuffer, * m_pDepthStencilBuffer;
		ID3D11RenderTargetView * m_pRenderTargetView;
		ID3D11DepthStencilView * m_pDepthStencilView;
		ID3D11RasterizerState * m_pRasterizerStateSolid, * m_pRasterizerStateWireframe;
		ID3DX11Effect * m_pFX;
		ID3DX11EffectMatrixVariable * m_pFXWVPM;
		ID3DX11EffectMatrixVariable * m_pFXInvTranspose;
		ID3DX11EffectMatrixVariable * m_pFXTexTransform;
		ID3DX11EffectMatrixVariable * m_pFXWorld;
		ID3DX11EffectVariable * m_pFXmat;
		ID3DX11EffectVariable * m_pFXdirLight;
		ID3DX11EffectVariable * m_pFXfogStart, *m_pFXfogRange;
		ID3DX11EffectTechnique * m_pFXTechnique;
		ID3DX11EffectVectorVariable * m_pFXcameraPos;
		ID3DX11EffectVectorVariable * m_pFXfogColor;
		ID3D11InputLayout * m_pInputLayout;
		ID3D11ShaderResourceView * m_pFontResourceView;
		ID3DX11EffectShaderResourceVariable * m_pSRVariable;
		ID3D11BlendState * m_pBlendState;

		D3D_FEATURE_LEVEL m_featureLevels[3] = { D3D_FEATURE_LEVEL_11_0,
											    D3D_FEATURE_LEVEL_10_1,
											    D3D_FEATURE_LEVEL_10_0 };
		D3D_FEATURE_LEVEL m_featureLevel;
		D3D11_VIEWPORT m_viewPort;
		DXGI_FORMAT m_usedFormat;
		DXGI_SWAP_CHAIN_DESC m_swapDesc;
		D3D11_TEXTURE2D_DESC m_depthDesc;
		D3D11_RASTERIZER_DESC m_rasterizerDesc;
};
