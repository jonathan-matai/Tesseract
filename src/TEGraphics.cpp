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
#include "..\include\TEGraphics.hpp"

teResult TEGraphics::teInit(wchar_t * iniFile, HWND hWnd, wchar_t * shaderFile)
{
	//Die Membervariablen initen
	m_iniFile = iniFile;
	m_usedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_shaderFile = shaderFile;
	m_techniqueName = "LightTech";

	//Deviceflags setzen
	UINT deviceFlags = NULL;
#if defined(TE_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//Das Device und den Devicekontext erstellen
	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, m_featureLevels, 3, D3D11_SDK_VERSION, &m_pDevice, &m_featureLevel, &m_pImmidiateContext)))
	{
		LOGFILE->error(true, "DirectX 11 konnte nicht initialisiert werden.\nDas Device wurde nicht erstellt.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}
	if (m_featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		LOGFILE->error(true, "DirectX ist nicht auf dem neusten Stand.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	//DirectX 11 Factory erstellen um den Swapchain erstellen zu können
	IDXGIDevice * dev = 0;
	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dev);

	IDXGIAdapter * ad = 0;
	dev->GetParent(__uuidof(IDXGIAdapter), (void**)&ad);

	ad->GetParent(__uuidof(IDXGIFactory), (void**)&m_pFactory);

	ad->Release();
	dev->Release();

	//Prüfen, ob die Ini schon existiert; wenn ja dann Daten laden, wenn nein dann Inidatei erstellen
	teFileExists(m_iniFile) ? teLoadGraphicsIni() : teCreateGraphicsIni();

	//Die SwapChainDesc ausfüllen
	m_swapDesc.Windowed = m_fullscreen ? false : true;
	m_swapDesc.OutputWindow = hWnd;
	m_swapDesc.BufferCount = m_bufferCount;
	m_swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_swapDesc.BufferDesc.Width = m_width;
	m_swapDesc.BufferDesc.Height = m_height;
	m_swapDesc.BufferDesc.RefreshRate.Denominator = m_denom;
	m_swapDesc.BufferDesc.RefreshRate.Numerator = m_num;
	m_swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapDesc.Flags = 0;
	m_swapDesc.SampleDesc.Count = m_sampleCount;
	m_swapDesc.SampleDesc.Quality = m_sampleQuality;
	m_swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//SwapChain erstellen
	if (FAILED(m_pFactory->CreateSwapChain(m_pDevice, &m_swapDesc, &m_pSwapChain)))
	{
		LOGFILE->error(true, "Swapchain konnte nicht erstellt werden.\nInitialisierung der Graphicsengine ist fehlgeschlagen.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	//RenderTargetView erstellen
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pBackBuffer));
	if (m_pBackBuffer == nullptr)
	{
		LOGFILE->error(true, "Grafikengine konnte nicht erstellt werden. m_pBackBuffer war nullptr.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	m_pDevice->CreateRenderTargetView(m_pBackBuffer, 0, &m_pRenderTargetView);
	m_pBackBuffer->Release();

	//DepthStencilDesc ausfüllen
	m_depthDesc.Width = m_width;
	m_depthDesc.Height = m_height;
	m_depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_depthDesc.ArraySize = 1;
	m_depthDesc.CPUAccessFlags = 0;
	m_depthDesc.MipLevels = 1;
	m_depthDesc.MiscFlags = 0;
	m_depthDesc.SampleDesc.Count = m_sampleCount;
	m_depthDesc.SampleDesc.Quality = m_sampleQuality;
	m_depthDesc.Usage = D3D11_USAGE_DEFAULT;
	m_depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	//Die Textur(also DepthStencilBuffer) und den DepthStencilView erstellen
	m_pDevice->CreateTexture2D(&m_depthDesc, 0, &m_pDepthStencilBuffer);
	if (m_pDepthStencilBuffer == nullptr)
	{
		LOGFILE->error(true, "Grafikengine konnte nicht erstellt werden. m_pDepthStencilBuffer war nullptr.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView);

	//Die Rendertargets setzen
	m_pImmidiateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Die ViewportDesc wurde schon in LoadFromIni ausgefüllt
	//Hier wird nur der ViewPort gesetzt
	m_pImmidiateContext->RSSetViewports(1, &m_viewPort);

	//Seitenverhältnis des Bildschirms berechnen
	m_aspectRatio = (float)m_width / (float)m_height;

	//Den RasterrizerState erstellen und setzen
	m_rasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	m_rasterizerDesc.FrontCounterClockwise = false;
	m_rasterizerDesc.DepthBias = 0;
	m_rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_rasterizerDesc.DepthBiasClamp = 0.0f;
	m_rasterizerDesc.DepthClipEnable = true;
	m_rasterizerDesc.ScissorEnable = false;
	m_rasterizerDesc.MultisampleEnable = false;
	m_rasterizerDesc.AntialiasedLineEnable = false;

	m_pDevice->CreateRasterizerState(&m_rasterizerDesc, &m_pRasterizerStateSolid);

	m_pImmidiateContext->RSSetState(m_pRasterizerStateSolid);

	m_usedRenderMode = 0;

	//Andere Rasterizermodes erstellen
	m_rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	m_pDevice->CreateRasterizerState(&m_rasterizerDesc, &m_pRasterizerStateWireframe);

	//Kamera initialisieren
	m_farRenderDist = 2000.0f;
	m_pCamera = new TECamera();
	m_pCamera->teInit();
	m_pCamera->teSetLens(TE_DEG_TO_RAD(45.0f), m_aspectRatio, 1.0f, m_farRenderDist);

	//Shader erstellen
	if (!teCreateShader())
	{
		return false;
	}

	//InputLayout erstellen
	if (!teCreateInputLayout())
	{
		return false;
	}

	//D3D11_SHADER_RESOURCE_VIEW_DESC rvd;

	//m_pDevice->CreateShaderResourceView(NULL, NULL, &m_pFontResourceView);

	//BlendState erstellen
	D3D11_BLEND_DESC blenddesc;
	blenddesc.AlphaToCoverageEnable = false;
	blenddesc.IndependentBlendEnable = false;
	blenddesc.RenderTarget[0].BlendEnable = true;
	blenddesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blenddesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	blenddesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	blenddesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_COLOR;
	blenddesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;


	m_pDevice->CreateBlendState(&blenddesc, &m_pBlendState);

	DirectX::ScratchImage image;

	//m_pImmidiateContext->OMSetBlendState(m_pBlendState, DirectX::LoadFromDDSFile(L"", NULL, NULL, NULL));

	LOGFILE->print(colors::TE_SUCCEEDED, "DirectX 11 wurde erfolgreich initialisiert.");

	return true;
}

teResult TEGraphics::teExit()
{
	TE_SAFE_RELEASE(m_pFX);
	TE_SAFE_RELEASE(m_pRasterizerStateWireframe);
	TE_SAFE_RELEASE(m_pRasterizerStateSolid);
	TE_SAFE_RELEASE(m_pDepthStencilView);
	TE_SAFE_RELEASE(m_pRenderTargetView);
	TE_SAFE_RELEASE(m_pFactory);
	TE_SAFE_RELEASE(m_pSwapChain);
	TE_SAFE_RELEASE(m_pImmidiateContext);
	TE_SAFE_RELEASE(m_pDevice);

	return true;
}

void TEGraphics::teSetObjectRenderStates(XMMATRIX objectWorldMatrix, LightMaterial objectMat, XMMATRIX textransform)
{
	XMMATRIX wvpm = objectWorldMatrix * m_pCamera->teGetViewMatrix()*m_pCamera->teGetProjectionMatrix();
	m_pFXWVPM->SetMatrix(reinterpret_cast<float*>(&wvpm));
	m_pFXInvTranspose->SetMatrix(reinterpret_cast<float*>(&inverseTranspose(objectWorldMatrix)));
	m_pFXWorld->SetMatrix(reinterpret_cast<float*>(&objectWorldMatrix));
	m_pFXmat->SetRawValue(&objectMat, 0, sizeof(LightMaterial));
	m_pFXTexTransform->SetMatrix(reinterpret_cast<float*>(&textransform));
}

void TEGraphics::teSetObjectTexture(ID3D11ShaderResourceView * tex)
{
	m_pSRVariable->SetResource(tex);
}

void TEGraphics::teSetGeneralRenderStates(DirectionalLight sun)
{
	m_pFXdirLight->SetRawValue(&sun, 0, sizeof(DirectionalLight));
	m_pFXcameraPos->SetRawValue(&m_pCamera->m_camPos, 0, sizeof(XMFLOAT3));
}

teResult TEGraphics::teLoadGraphicsIni()
{
	//Alle Werte aus der Inidatei laden
	m_width = teLoadIniInt(m_iniSection, L"iWidth", m_iniFile);
	m_height = teLoadIniInt(m_iniSection, L"iHeight", m_iniFile);
	m_bufferCount = teLoadIniInt(m_iniSection, L"iBufferCount", m_iniFile);
	m_denom = teLoadIniInt(m_iniSection, L"iDenom", m_iniFile);
	m_num = teLoadIniInt(m_iniSection, L"iNum", m_iniFile);
	m_fullscreen = teLoadIniBool(m_iniSection, L"bWindowed", m_iniFile); m_fullscreen = m_fullscreen ? false : true;
	m_sampleCount = teLoadIniInt(m_iniSection, L"iSampleCount", m_iniFile);
	m_sampleQuality = teLoadIniInt(m_iniSection, L"iSampleQuality", m_iniFile);
	m_enableMultisampling = teLoadIniBool(m_iniSection, L"bEnableMultiSampling", m_iniFile);

	m_viewPort.Width = teLoadIniFloat(m_iniSection, L"fViewPortW", m_iniFile);
	m_viewPort.Height = teLoadIniFloat(m_iniSection, L"fViewPortH", m_iniFile);
	m_viewPort.MaxDepth = teLoadIniFloat(m_iniSection, L"fViewPortF", m_iniFile);
	m_viewPort.MinDepth = teLoadIniFloat(m_iniSection, L"fViewPortN", m_iniFile);
	m_viewPort.TopLeftX = teLoadIniFloat(m_iniSection, L"fViewPortX", m_iniFile);
	m_viewPort.TopLeftY = teLoadIniFloat(m_iniSection, L"fViewPortY", m_iniFile);

	if (m_width == 0 || m_height == 0)
	{
		LOGFILE->error(true, "Daten konnten nicht aus der Ini-Datei gelesen werden.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	LOGFILE->print(colors::TE_INFO, "Alle Werte aus Inidatei eingelesen.");

	return true;
}

void TEGraphics::teCreateGraphicsIni()
{
	//Lokale Variablen
	UINT i = 0, numModes = 0;
	IDXGIAdapter * pAdapter;
	std::vector<IDXGIAdapter*> pAdapterList;
	std::vector<IDXGIAdapter*>::iterator aiter;
	IDXGIOutput * pOutput;
	std::vector<IDXGIOutput*> pOutputList;
	std::vector<IDXGIOutput*>::iterator oiter;
	std::vector<DXGI_MODE_DESC*> pModeDescList;
	std::vector<DXGI_MODE_DESC*>::iterator mditer;
	UINT width = 0, height = 0, denom = 1, num = 0;

	//Alle Adapter (Grafikkarten) durchlaufen
	while (m_pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		pAdapterList.push_back(pAdapter);
		++i;
	}

	LOGFILE->printf(colors::TE_INFO, "Insgesamt erkannte Grafikkarten: %d", pAdapterList.size());

	//Alle Modes durchlaufen
	for (aiter = pAdapterList.begin(); aiter != pAdapterList.end(); aiter++)
	{
		i = 0;
		while ((*aiter)->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			pOutputList.push_back(pOutput);
			++i;
		}
	}

	LOGFILE->printf(colors::TE_INFO, "Insgesamt erkannte Videomodi: %d", pOutputList.size());

	//Für alle Modes eine Description füllen
	for (oiter = pOutputList.begin(); oiter != pOutputList.end(); oiter++)
	{
		(*oiter)->GetDisplayModeList(m_usedFormat, NULL, &numModes, NULL);

		DXGI_MODE_DESC * pModeDesc = new DXGI_MODE_DESC[numModes];

		(*oiter)->GetDisplayModeList(m_usedFormat, NULL, &numModes, pModeDesc);

		for (UINT k = 0; k < numModes; k++)
		{
			pModeDescList.push_back(&pModeDesc[k]);
		}
	}

	UINT min = 0, max = (UINT)pModeDescList.size();
	int counter = -1;

	//Alle Modes durchlaufen und den mit höchster Auflösung aktiv setzen
	//X Res ermitteln
	for (mditer = pModeDescList.begin(); mditer != pModeDescList.end(); ++mditer)
	{
		counter++;
		if ((*mditer)->Width > width && (*mditer)->Width <= MAX_XRES)
		{
			min = (UINT)counter;
			width = (*mditer)->Width;
		}
		else if ((*mditer)->Width < width || (*mditer)->Width > MAX_XRES)
		{
			max = (UINT)counter - 1;
		}
	}
	//Y Res ermitteln
	counter = min;
	for (counter; counter <= max; counter++)
	{
		if (pModeDescList.at(counter)->Height > height)
		{
			min = (UINT)counter;
			height = pModeDescList.at(counter)->Height;
		}
		else if (pModeDescList.at(counter)->Height < height)
		{
			max = (UINT)counter - 1;
			break;
		}
	}
	//Maximale FPS Rate ermitteln
	for (counter = min; counter <= max; counter++)
	{
		if ((pModeDescList.at(counter)->RefreshRate.Numerator / pModeDescList.at(counter)->RefreshRate.Denominator) >(num / denom))
		{
			num = pModeDescList.at(counter)->RefreshRate.Numerator;
			denom = pModeDescList.at(counter)->RefreshRate.Denominator;
		}
	}

	LOGFILE->printf(colors::TE_INFO, "Videomodus festgelegt mit %d x %d Auflösung und %f maximalen FPS.", width, height, (num / denom));

	//Alles in die Ini schreiben
	std::wostringstream ss;

	ss << width;

	WritePrivateProfileString(m_iniSection, L"iWidth", ss.str().c_str(), m_iniFile);
	ss << L".f";
	WritePrivateProfileString(m_iniSection, L"fViewPortW", ss.str().c_str(), m_iniFile);

	ss.str(L"");
	ss.clear();
	ss << height;
	WritePrivateProfileString(m_iniSection, L"iHeight", ss.str().c_str(), m_iniFile);
	ss << L".f";
	WritePrivateProfileString(m_iniSection, L"fViewPortH", ss.str().c_str(), m_iniFile);
	teGetMaxSample();
	ss.str(L"");
	ss.clear();
	ss << m_sampleCount;
	WritePrivateProfileString(m_iniSection, L"iSampleCount", ss.str().c_str(), m_iniFile);
	ss.str(L"");
	ss.clear();
	ss << m_sampleQuality;
	WritePrivateProfileString(m_iniSection, L"iSampleQuality", ss.str().c_str(), m_iniFile);
	m_fullscreen ? WritePrivateProfileString(m_iniSection, L"bWindowed", L"false", m_iniFile) : WritePrivateProfileString(m_iniSection, L"bWindowed", L"true", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"bEnableMultiSampling", L"true", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"iBufferCount", L"1", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"fViewPortF", L"1.0f", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"fViewPortN", L"0.0f", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"fViewPortX", L"0.0f", m_iniFile);
	WritePrivateProfileString(m_iniSection, L"fViewPortY", L"0.0f", m_iniFile);
	ss.str(L"");
	ss.clear();
	ss << denom;
	WritePrivateProfileString(m_iniSection, L"iDenom", ss.str().c_str(), m_iniFile);
	ss.str(L"");
	ss.clear();
	ss << num;
	WritePrivateProfileString(m_iniSection, L"iNum", ss.str().c_str(), m_iniFile);

	LOGFILE->print(colors::TE_INFO, "Alle Werte wurden in die Inidatei geschrieben.");

	teLoadGraphicsIni();
}

void TEGraphics::teGetMaxSample()
{
	//Maximale Samplequalität und Anzahl ermitteln
	m_sampleCount = MAX_MULTI_SAMPLE;

	//Alle Samplecounts startend bei 4 durchlaufen und Unterstützung checken
	for (m_sampleCount; m_sampleCount > 1; m_sampleCount -= m_sampleCount / 2)
	{
		m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_sampleCount, &m_sampleQuality);
		//Wenn Unterstützung gefunden wurde for abbrechen und Multisampling count und quality setzen
		if (m_sampleQuality != 0)
		{
			std::stringstream ms;
			ms << "Maximale Multisampling-Unterstützung wurde festgelegt auf " << m_sampleCount << "mit Qualität " << m_sampleQuality;
			LOGFILE->print(colors::TE_VALUE, ms.str().c_str());
			break;
		}
	}
	//Wenn keine gefunden wurde nachdem alle Counts durchlaufen wurden, Multisampling auf 1 setzen
	if (m_sampleQuality == 0)
	{
		LOGFILE->print(colors::TE_WARNING, "Warnung: Es konnte keine Multisampling-Unterstützung festgestellt werden");
		m_sampleCount = 1;
		m_sampleQuality = 0;
		return;
	}
	m_sampleQuality--;
}

void TEGraphics::teSetRenderMode(short rendermode)
{
	if (rendermode != m_usedRenderMode)
	{
		switch (rendermode)
		{
			case 0:
			{
				m_pImmidiateContext->RSSetState(m_pRasterizerStateSolid);
				teSetShader("LightTech");
				m_usedRenderMode = rendermode;
				return;
			}
			case 1:
			{
				m_pImmidiateContext->RSSetState(m_pRasterizerStateWireframe);
				teSetShader("LightTechWire");
				m_usedRenderMode = rendermode;
				return;
			}
			default:
			{
				m_pImmidiateContext->RSSetState(m_pRasterizerStateSolid);
				teSetShader("LightTech");
				m_usedRenderMode = 0;
			}
		}
	}
}

teResult TEGraphics::teCreateShader()
{
	//Shaderflags festlegen
	UINT shaderFlags = NULL;
#if defined(_DEBUG) || defined(DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob * errorMsg = NULL;

	//Effekt kompilieren und Effect erstellen
	HRESULT hr = D3DX11CompileEffectFromFile(m_shaderFile, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderFlags, NULL, m_pDevice, &m_pFX, &errorMsg);

	if (FAILED(hr))
	{
		LOGFILE->printf(colors::TE_WARNING, "Die Shaderdatei wurde nicht kompiliert: %s", (char*)errorMsg->GetBufferPointer());
		return false;
	}

	//Die Technique und die WVPMatrix bekommen
	m_pFXTechnique = m_pFX->GetTechniqueByName(m_techniqueName);

	m_pFXWVPM = m_pFX->GetVariableByName("gWorldViewProjM")->AsMatrix();

	m_pFXcameraPos = m_pFX->GetVariableByName("gEyePosW")->AsVector();

	m_pFXdirLight = m_pFX->GetVariableByName("gDirLight");

	m_pFXInvTranspose = m_pFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();

	m_pFXmat = m_pFX->GetVariableByName("gMaterial");

	m_pFXWorld = m_pFX->GetVariableByName("gWorld")->AsMatrix();

	m_pSRVariable = m_pFX->GetVariableByName("gDiffuseMap")->AsShaderResource();

	m_pFXTexTransform = m_pFX->GetVariableByName("gTexTransform")->AsMatrix();

	return true;
}

void TEGraphics::teSetShader(char * techniquename)
{
	m_pFXTechnique = m_pFX->GetTechniqueByName(techniquename);
	m_techniqueName = techniquename;
}

void TEGraphics::tePresent()
{
	m_pSwapChain->Present(0, 0);
}

teResult TEGraphics::teStartScene()
{
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_pImmidiateContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pImmidiateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pImmidiateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

teResult TEGraphics::teDrawString(char * message, int x, int y)
{


	return teResult();
}

teResult TEGraphics::teCreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
											  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
											  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(XMFLOAT3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0} };

	D3DX11_PASS_DESC passDesc;
	m_pFXTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	if (FAILED(m_pDevice->CreateInputLayout(vertexDesc, (sizeof(vertexDesc) / sizeof(*vertexDesc)), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout)))
	{
		LOGFILE->print(colors::TE_WARNING, "Das Inputlayout wurde nicht erstellt.");
		return false;
	}

	m_pImmidiateContext->IASetInputLayout(m_pInputLayout);

	return true;
}
