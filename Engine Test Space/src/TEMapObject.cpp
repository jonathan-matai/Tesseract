#include "..\include\TEMapObject.hpp"

teResult TEMap::teCreateBuffer(ID3D11Device * device)
{
	//alle chunks durchlaufen und buffer von denen erstellen, die sich in reichweite befinden
	for (m_chunkIter = m_chunks.begin(); m_chunkIter != m_chunks.end(); ++m_chunkIter)
	{
		if ((*m_chunkIter).m_isGenerated && (*m_chunkIter).m_changed)
		{
			//wenn der chunk generiert wurde, buffer von ihm ertellen
			m_vertexData.pSysMem = &(*m_chunkIter).vertecies[0];
			m_indexData.pSysMem = &(*m_chunkIter).indecies[0];

			m_vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			m_vertexBufferDesc.ByteWidth = sizeof(Vertex) * (*m_chunkIter).n_vertecies;
			m_vertexBufferDesc.CPUAccessFlags = 0;
			m_vertexBufferDesc.MiscFlags = 0;
			m_vertexBufferDesc.StructureByteStride = 0;
			m_vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

			m_indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			m_indexBufferDesc.ByteWidth = sizeof(UINT) * (*m_chunkIter).n_indecies;
			m_indexBufferDesc.CPUAccessFlags = 0;
			m_indexBufferDesc.MiscFlags = 0;
			m_indexBufferDesc.StructureByteStride = 0;
			m_indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

			if (FAILED(device->CreateBuffer(&m_vertexBufferDesc, &m_vertexData, &(*m_chunkIter).m_pVertexBuffer)))
			{
				LOGFILE->print(colors::TE_WARNING, "vertex buffer creation failed.");
				return false;
			}
			if(FAILED(device->CreateBuffer(&m_indexBufferDesc, &m_indexData, &(*m_chunkIter).m_pIndexBuffer)))
			{
				LOGFILE->print(colors::TE_WARNING, "index buffer creation failed.");
				return false;
			}
		}
	}

	m_vertexData.pSysMem = NULL;
	m_indexData.pSysMem = NULL;

	return true;
}

//WARNING: ONLY WORKS WITH SQUARED MAPS!!
teResult TEMap::teUpdateChunks(XMFLOAT3 pos)
{
	//allgemeine chunkinformationen festlegen
	m_genInfo.terrainSettings = &m_terrainSettings;

	//position in chunkID umwandeln
	int dx = pos.x / m_terrainSettings.chunkSize;
	int dz = pos.z / m_terrainSettings.chunkSize;
	int expectedTess = 0;

	int posToChunkX = 0, posToChunkZ = 0;

	if (!m_chunks.begin()->m_isGenerated && m_chunks.begin()->m_changed)
	{
		m_genInfo.chunkSettings = m_chunks.begin()->m_chunkSettings;
		m_genInfo.vertexBuffer = &m_chunks.begin()->vertecies;
		m_genInfo.indexBuffer = &m_chunks.begin()->indecies;
		m_genInfo.n_verticies = reinterpret_cast<size_t*>(&m_chunks.begin()->n_vertecies);
		m_genInfo.n_indicies = reinterpret_cast<size_t*>(&m_chunks.begin()->n_indecies);

		generateChunk(m_genInfo);

		m_chunks.begin()->m_isGenerated = true;
		m_chunks.begin()->m_toRender = true;
		m_chunks.begin()->m_changed = true;
		m_chunks.begin()->vertecies = m_genInfo.vertexBuffer[0];
		m_chunks.begin()->indecies = m_genInfo.indexBuffer[0];
	}

	if (!teCreateBuffer(GRAPHICS->teGetDevice()))
	{
		return false;
	}

	m_chunks.begin()->m_changed = false;

	return true;
}

TEMap::TEMap(TE_OBJECT_DESC & init) : TEObject(init)
{
	//Members setzen
	m_pos = init.pos;
	m_scale = init.scale;
	m_world = XMMatrixIdentity();

	//Mapgeneration system initialisieren
	m_terrainSettings.chunkSize = 256;
	m_terrainSettings.layers = 5;
	m_terrainSettings.baseAmplitude = 400.0f;
	m_terrainSettings.baseWavelength = 500.0f;
	m_terrainSettings.mod_aplitude = 0.5f;
	m_terrainSettings.mod_wavelength = 0.5f;
	m_terrainSettings.textureWidth = 8.0f;
	MathHelper::seed();
	m_terrainSettings.seed = MathHelper::rSeed;

	m_chunkGenRange = 5;
	m_lowestTesselation = -log2f(m_terrainSettings.chunkSize);
	m_heighestTesselation = 3;

	int x = 0, z = 0;
	//chunk liste mit ids versehen (10 x 10) chunk map
	for (int i = 0; i < 100; ++i)
	{
		//eine zeile voll
		if ((i % 9) == 0 && x != 0)
		{
			ChunkSettings * chunksett = new ChunkSettings();
			chunksett->id_x = x;
			chunksett->id_z = z;
			chunksett->tesselation = 0;

			teChunk chunk;
			chunk.m_chunkSettings = chunksett;
			chunk.m_isGenerated = false;
			chunk.m_toGenerate = false;
			chunk.m_toRender = false;
			chunk.m_changed = true;
			chunk.n_indecies = 0;
			chunk.n_vertecies = 0;

			m_chunks.push_back(chunk);
			z++;
			x = 0;
		}
		else
		{
			ChunkSettings * chunksett = new ChunkSettings();
			chunksett->id_x = x;
			chunksett->id_z = z;
			chunksett->tesselation = 0;

			teChunk chunk;
			chunk.m_chunkSettings = chunksett;
			chunk.m_isGenerated = false;
			chunk.m_toGenerate = false;
			chunk.m_toRender = false;
			chunk.m_changed = true;
			chunk.n_indecies = 0;
			chunk.n_vertecies = 0;

			m_chunks.push_back(chunk);

			x++;
		}
	}

	m_material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f);
	m_material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	if (!teLoadTextureFromFile(GRAPHICS->teGetDevice(), L"res//Data//Textures//map//grass01.dds", &m_pSRVgrass))
	{
		LOGFILE->print(colors::TE_WARNING, "texture (grass) failed to load.");
	}

	LOGFILE->print(colors::TE_CONSTRUCTION, "Die Map wurde erfolgreich erstellt.");
}

TEMap::~TEMap()
{
	_free(m_genInfo.vertexBuffer);
	_free(m_genInfo.indexBuffer);
	m_chunks.clear();
}

void TEMap::render()
{
	UINT stride = sizeof(Vertex), offset = 0;

	m_texTransform = XMMatrixScaling(8.0f, 8.0f, 0.0f);

	GRAPHICS->teSetObjectRenderStates(m_world, m_material, m_texTransform);
	GRAPHICS->teSetObjectTexture(m_pSRVgrass);

	//Alle Chunks durchgehen; die zu Rendernden rendern
	for (m_chunkIter = m_chunks.begin(); m_chunkIter != m_chunks.end(); ++m_chunkIter)
	{
		if ((*m_chunkIter).m_toRender)
		{
			GRAPHICS->teGetDeviceContext()->IASetVertexBuffers(0, 1, &(*m_chunkIter).m_pVertexBuffer, &stride, &offset);
			GRAPHICS->teGetDeviceContext()->IASetIndexBuffer((*m_chunkIter).m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			GRAPHICS->teGetEffectTechnique()->GetPassByName("lightPass2Light")->Apply(0, GRAPHICS->teGetDeviceContext());
			GRAPHICS->teGetDeviceContext()->DrawIndexed((*m_chunkIter).n_indecies, 0, 0);
		}
	}
}

void TEMap::tick(double dt)
{
	m_chunksToRender = 0;

	//view frustum festlegen, um zu bestimmen, welche chunks gerendert werden können
	vBox * frustum = calculateViewFrustum();

	if (!teUpdateChunks(xmvectorToXmfloat3(ENGINE->teGetQuickCam().pos)))
	{
		return;
	}

	//wenn die chunks erfolgreich geupdated wurden, bestimmen der zu rendernden chunks
	for (m_chunkIter = m_chunks.begin(); m_chunkIter != m_chunks.end(); ++m_chunkIter)
	{
		if ((*m_chunkIter).m_isGenerated)
		{
			//chunkecken festlegen
			/*  ___________________pos11
				|pos01			  |
				|				  |
				|				  |
				|                 |
				|				  |
				|				  |
				|pos00____________|pos10
			*/
			XMFLOAT3 pos00, pos10, pos01, pos11;

			pos00 = XMFLOAT3((*m_chunkIter).m_chunkSettings->id_x * m_terrainSettings.chunkSize, 
							 (*m_chunkIter).vertecies->pos.y, 
							 (*m_chunkIter).m_chunkSettings->id_z * m_terrainSettings.chunkSize);

			pos10 = XMFLOAT3(((*m_chunkIter).m_chunkSettings->id_x + 1) * m_terrainSettings.chunkSize,
							 (*m_chunkIter).vertecies->pos.y,
							 (*m_chunkIter).m_chunkSettings->id_z * m_terrainSettings.chunkSize);

			pos01 = XMFLOAT3((*m_chunkIter).m_chunkSettings->id_x * m_terrainSettings.chunkSize,
							 (*m_chunkIter).vertecies->pos.y,
							 ((*m_chunkIter).m_chunkSettings->id_z + 1) * m_terrainSettings.chunkSize);

			pos11 = XMFLOAT3(((*m_chunkIter).m_chunkSettings->id_x + 1) * m_terrainSettings.chunkSize,
							 (*m_chunkIter).vertecies->pos.y,
							 ((*m_chunkIter).m_chunkSettings->id_z + 1) * m_terrainSettings.chunkSize);

			//wenn ein eckpunkt des chunks sichtbar ist, den chunk zum rendern freigeben
			if (Vampire::vIsPointInBox(frustum[0], xmTovPoint(pos00)) || Vampire::vIsPointInBox(frustum[0], xmTovPoint(pos10)) ||
				Vampire::vIsPointInBox(frustum[0], xmTovPoint(pos01)) || Vampire::vIsPointInBox(frustum[0], xmTovPoint(pos11)))
			{
				(*m_chunkIter).m_toRender = true;
				m_chunksToRender++;
			}
			else
			{
				(*m_chunkIter).m_toRender = false;
			}
		}
	}

	m_chunks.begin()->m_toRender = true;
}
