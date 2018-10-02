#include "..\include\TEMapObject.hpp"

teResult TEMap::teLoadFromTHM(wchar_t * heightMap, wchar_t * biomMap, ID3D11Device * device)
{
	if (heightMap == NULL)
	{
		return false;
	}
	
	std::wstring ws(heightMap);

	if (GetFileAttributes(ws.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		LOGFILE->print(colors::TE_WARNING, "Die angegebene Datei kann nicht gefunden werden.");
		return false;
	}

	//texture loading
	if (!teLoadTextureFromFile(GRAPHICS->teGetDevice(), L"G:/C++/Phoenix/Tesseract/TEEngine/Engine Test Space/res/Data/Textures/map/grass01.dds", &m_pSRVgrass))
	{
		LOGFILE->print(colors::TE_WARNING, "texture (grass) failed to load.");
		return false;
	}

	//AUS DER THM DATEI DIE HEIGHTMAP AUSLESEN
	std::ifstream heightmapFile(ws.c_str(), std::ios::binary);
	if (!heightmapFile.is_open())
	{
		return false;
	}

	m_width = 0; m_height = 0; int version = 0;
	float scalevertical = 0.0f, scalehorizontal = 0.0f;

	heightmapFile.read((char*)&m_width, sizeof(int));
	heightmapFile.read((char*)&m_height, sizeof(int));
	heightmapFile.read((char*)&version, sizeof(int));
	heightmapFile.read((char*)&scalehorizontal, sizeof(float));
	heightmapFile.read((char*)&scalevertical, sizeof(float));

	//float * heightmap = new float[width*height]();
	m_heightmap = (short*)teAlloc(sizeof(short)*m_width*m_height);
	heightmapFile.read((char*)m_heightmap, sizeof(short) * m_width * m_height);

	heightmapFile.close();

	m_scale.x = scalehorizontal;
	m_scale.y = scalevertical;
	m_scale.z = scalehorizontal;

	ws.clear();
	ws = L"";

	ws = biomMap;

	if (GetFileAttributes(ws.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		LOGFILE->error(false, "Die Biommap konnte nicht gefunden werden. Die Map wird ohne Biome geladen.", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}
	else
	{
		//AUS DER TBM DATEI DIE BIOMMAP AUSLESEN
		std::ifstream biommapFile(ws.c_str(), std::ios::binary);
		if (!biommapFile.is_open())
		{
			return false;
		}

		biommapFile.ignore();
		biommapFile.ignore();
		biommapFile.ignore();

		m_biommap = (uint8_t*)teAlloc(sizeof(uint8_t)*m_width*m_height);
		biommapFile.read((char*)m_biommap, sizeof(uint8_t) * m_width * m_height);

		biommapFile.close();
	}

	//Das Chunkoffset festlegen, also wann ein neuer Chunk anfängt
	int xOffset = 0, zOffset = 0;
	int chunksPerLine = sqrt(m_map.numChunks);
	int chunksPerColumn = sqrt(m_map.numChunks);
	UINT currentLine = 0;
	UINT currentColumn = 0;
	UINT indexColumn = 0;

	//Der aktuelle Chunk
	UINT loadedChunks = 0, chunksToLoad = 0;
	UINT currentIndex = 0, currentVertex = 0;
	float texCdu = 1.0f / ((float)m_width);
	float texCdv = 1.0f / ((float)m_height);

	//Die map in chunks unterteilen
	m_map.widthChunk = m_width / chunksPerColumn;
	m_map.heightChunk = m_height / chunksPerLine;
	m_map.verticiesPerChunk = m_map.widthChunk * m_map.heightChunk;
	m_map.cellsPerChunk = (m_map.widthChunk - 1)*(m_map.heightChunk - 1);
	int neededIndicesPerChunk = ((m_map.widthChunk * m_map.heightChunk) - m_map.widthChunk - 1);
	m_map.indicesPerChunk = neededIndicesPerChunk * 6;

	//Die Chunks initialisieren
	m_map.chunks = (teChunk*)teAlloc(sizeof(teChunk) * m_map.numChunks);

	//Die Chunks laden
	for (int chunkIterator = 0; chunkIterator < m_map.numChunks; ++chunkIterator)
	{
		m_map.chunks[chunkIterator].verticiesInChunk = (Vertex*)teAlloc(sizeof(Vertex) * m_map.verticiesPerChunk);
		m_map.chunks[chunkIterator].indiciesInChunk = (UINT*)teAlloc(sizeof(UINT) * m_map.indicesPerChunk);

		m_map.chunks[chunkIterator].isActive = true;
		m_map.chunks[chunkIterator].toRender = false;

		if (currentColumn == (chunksPerColumn))
		{
			currentLine++;

			currentColumn = 0;

			xOffset = 0;

			zOffset = (currentLine * m_map.heightChunk);
		}

		for (int x = xOffset; x < (xOffset + m_map.widthChunk); ++x)
		{
			for (int z = zOffset; z < (zOffset + m_map.heightChunk); ++z)
			{
				XMFLOAT3 currentNormal;
				XMFLOAT3 A, B, C;
				XMVECTOR ab, ac;
				XMFLOAT2 currentLoc(x, z);
				XMFLOAT2 texcoord(0.0f, 0.0f);
				XMFLOAT2 currentChunkCorners[4];
				currentChunkCorners[0] = XMFLOAT2(currentColumn * m_map.widthChunk, currentLine * m_map.heightChunk);
				currentChunkCorners[1] = XMFLOAT2((currentColumn + 1) * (m_map.widthChunk - 1), currentLine * m_map.heightChunk);
				currentChunkCorners[2] = XMFLOAT2(currentColumn * m_map.widthChunk, (currentLine + 1) * (m_map.heightChunk - 1));
				currentChunkCorners[3] = XMFLOAT2((currentColumn + 1) * (m_map.widthChunk - 1), (currentLine + 1) * (m_map.heightChunk - 1));

				A = XMFLOAT3((float)(x), (float)m_scale.y * m_heightmap[x * m_height + z], (float)(z));

				if (xmAreEqual(currentLoc, currentChunkCorners[0]))
				{
					B = XMFLOAT3((float)(x), (float)m_scale.y * m_heightmap[x * m_height + z + 1], ((float)(z) ) + 1.0f);
					C = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1) * m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&currentNormal, XMVector3Normalize(XMVector3Cross(ab, ac)));
				}
				else if (xmAreEqual(currentLoc, currentChunkCorners[1]))
				{
					B = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1) * m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x) * m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&currentNormal, XMVector3Normalize(XMVector3Cross(ab, ac)));
				}
				else if (xmAreEqual(currentLoc, currentChunkCorners[2]))
				{
					B = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1) * m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z - 1], ((float)(z)) - 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&currentNormal, XMVector3Normalize(XMVector3Cross(ab, ac)));
				}
				else if(xmAreEqual(currentLoc, currentChunkCorners[3]))
				{
					B = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x) * m_height + z - 1], ((float)(z)) - 1.0f);
					C = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z - 1], ((float)(z)) - 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&currentNormal, XMVector3Normalize(XMVector3Cross(ab, ac)));
				}
				else if (A.x == xOffset)
				{
					B = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1)* m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z - 1], ((float)(z)) - 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					XMFLOAT3 plane1, plane2;

					DirectX::XMStoreFloat3(&plane1, XMVector3Normalize(XMVector3Cross(ab, ac)));

					C = B;
					B = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane2, XMVector3Normalize(XMVector3Cross(ab, ac)));

					DirectX::XMStoreFloat3(&currentNormal, averageVector(DirectX::XMLoadFloat3(&plane1), DirectX::XMLoadFloat3(&plane2)));
				}
				else if (A.x == (xOffset + m_map.widthChunk - 1))
				{
					B = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z - 1], ((float)(z)) - 1.0f);
					C = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z], ((float)(z)));

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					XMFLOAT3 plane1, plane2;

					DirectX::XMStoreFloat3(&plane1, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = C;
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane2, XMVector3Normalize(XMVector3Cross(ab, ac)));

					DirectX::XMStoreFloat3(&currentNormal, averageVector(DirectX::XMLoadFloat3(&plane1), DirectX::XMLoadFloat3(&plane2)));
				}
				else if (A.z == zOffset)
				{
					B = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z + 1], ((float)(z)) + 1.0f);
					C = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					XMFLOAT3 plane1, plane2;

					DirectX::XMStoreFloat3(&plane1, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane2, XMVector3Normalize(XMVector3Cross(ab, ac)));

					DirectX::XMStoreFloat3(&currentNormal, averageVector(DirectX::XMLoadFloat3(&plane1), DirectX::XMLoadFloat3(&plane2)));
				}
				else if (A.z == (zOffset + m_map.heightChunk - 1))
				{
					B = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z - 1], ((float)(z)) - 1.0f);
					C = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z], ((float)(z)));

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					XMFLOAT3 plane1, plane2;

					DirectX::XMStoreFloat3(&plane1, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1)* m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z - 1], ((float)(z)) - 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane2, XMVector3Normalize(XMVector3Cross(ab, ac)));

					DirectX::XMStoreFloat3(&currentNormal, averageVector(DirectX::XMLoadFloat3(&plane1), DirectX::XMLoadFloat3(&plane2)));
				}
				else
				{
					B = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z - 1], ((float)(z)) - 1.0f);
					C = XMFLOAT3(((float)(x)) - 1.0f, (float)m_scale.y * m_heightmap[(x - 1)* m_height + z], ((float)(z)));

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					XMFLOAT3 plane1, plane2, plane3, plane4;

					DirectX::XMStoreFloat3(&plane1, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = C;
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane2, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = C;
					C = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1)* m_height + z + 1], ((float)(z)) + 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane3, XMVector3Normalize(XMVector3Cross(ab, ac)));

					B = XMFLOAT3(((float)(x)) + 1.0f, (float)m_scale.y * m_heightmap[(x + 1)* m_height + z], ((float)(z)));
					C = XMFLOAT3(((float)(x)), (float)m_scale.y * m_heightmap[(x)* m_height + z - 1], ((float)(z)) - 1.0f);

					ab = vectorBetweenPoints(A, B);
					ac = vectorBetweenPoints(A, C);

					DirectX::XMStoreFloat3(&plane4, XMVector3Normalize(XMVector3Cross(ab, ac)));

					DirectX::XMStoreFloat3(&currentNormal, averageVector(DirectX::XMLoadFloat3(&plane1), DirectX::XMLoadFloat3(&plane2), DirectX::XMLoadFloat3(&plane3), DirectX::XMLoadFloat3(&plane4)));
				}

				texcoord.x = (z * texCdu);
				texcoord.y = (x * texCdv);

				m_map.chunks[chunkIterator].verticiesInChunk[currentVertex] = { XMFLOAT3(
																				(float)(x),
																				(float)m_scale.y * m_heightmap[x * m_height + z],
																				(float)(z)),
																				currentNormal,
																				texcoord};
				currentVertex++;
			}
		}

		currentVertex = 0;
		currentColumn++;
		xOffset = (currentColumn * m_map.widthChunk);

		for (int i = 0; i <= neededIndicesPerChunk; ++i)
		{
			if (i != ((m_map.heightChunk - 1) + (indexColumn * m_map.heightChunk)))
			{
				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 0] = (UINT)i;
				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 1] = (UINT)i + 1;
				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 2] = (UINT)i + m_map.widthChunk + 1;

				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 3] = (UINT)i;
				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 4] = (UINT)i + m_map.widthChunk + 1;
				m_map.chunks[chunkIterator].indiciesInChunk[currentIndex + 5] = (UINT)i + m_map.widthChunk;

				currentIndex += 6;
			}
			else
			{
				indexColumn++;
			}
		}

		currentIndex = 0;
		indexColumn = 0;

		if (!teCreateBuffer(device, chunkIterator))
		{
			return false;
		}

		loadedChunks++;
	}

	float chunkPercentage = (loadedChunks / m_map.numChunks) * 100;
	LOGFILE->printf(colors::TE_INFO, "%d chunks of 100 loaded. ( %f %%)", loadedChunks, chunkPercentage);

	//if (!teLoadTextureFromFile(GRAPHICS->teGetDevice(), L"G:/C++/Phoenix/Tesseract/TEEngine/Engine Test Space/res/Data/Textures/map/rock01.dds", &m_pSRVrock))
	//{
	//	LOGFILE->print(colors::TE_WARNING, "texture (rock) failed to load.");
	//	return false;
	//}
	//
	//if (!teLoadTextureFromFile(GRAPHICS->teGetDevice(), L"G:/C++/Phoenix/Tesseract/TEEngine/Engine Test Space/res/Data/Textures/map/sand01.dds", &m_pSRVsand))
	//{
	//	LOGFILE->print(colors::TE_WARNING, "texture (sand) failed to load.");
	//	return false;
	//}
	//
	//if (!teLoadTextureFromFile(GRAPHICS->teGetDevice(), L"G:/C++/Phoenix/Tesseract/TEEngine/Engine Test Space/res/Data/Textures/map/notset.dds", &m_pSRVnotset))
	//{
	//	LOGFILE->print(colors::TE_WARNING, "texture (notset) failed to load.");
	//	return false;
	//}

	return true;
}

teResult TEMap::teCreateBuffer(ID3D11Device * device, int chunk)
{
	//Den Buffer des Chunks erstellen
	m_map.chunks[chunk].vertexData.pSysMem = &m_map.chunks[chunk].verticiesInChunk[0];
	m_map.chunks[chunk].indexData.pSysMem = &m_map.chunks[chunk].indiciesInChunk[0];

	m_map.chunks[chunk].vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_map.chunks[chunk].vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_map.verticiesPerChunk;
	m_map.chunks[chunk].vertexBufferDesc.CPUAccessFlags = 0;
	m_map.chunks[chunk].vertexBufferDesc.MiscFlags = 0;
	m_map.chunks[chunk].vertexBufferDesc.StructureByteStride = 0;
	m_map.chunks[chunk].vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	m_map.chunks[chunk].indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_map.chunks[chunk].indexBufferDesc.ByteWidth = sizeof(UINT) * m_map.indicesPerChunk;
	m_map.chunks[chunk].indexBufferDesc.CPUAccessFlags = 0;
	m_map.chunks[chunk].indexBufferDesc.MiscFlags = 0;
	m_map.chunks[chunk].indexBufferDesc.StructureByteStride = 0;
	m_map.chunks[chunk].indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (m_map.chunks[chunk].vertexData.pSysMem == NULL || m_map.chunks[chunk].indexData.pSysMem == NULL)
	{
		LOGFILE->printf(colors::TE_ERROR, "Die Bufferdaten von Chunk %d sind leer.", chunk);
		TRACE(true, "Es können keine Buffer erstellt werden.");
	}

	device->CreateBuffer(&m_map.chunks[chunk].vertexBufferDesc, &m_map.chunks[chunk].vertexData, &m_map.chunks[chunk].vertexBuffer);
	device->CreateBuffer(&m_map.chunks[chunk].indexBufferDesc, &m_map.chunks[chunk].indexData, &m_map.chunks[chunk].indexBuffer);

	if (m_map.chunks[chunk].vertexBuffer == NULL || m_map.chunks[chunk].indexBuffer == NULL)
	{
		return false;
	}

	m_map.chunks[chunk].vertexData.pSysMem = NULL;
	m_map.chunks[chunk].indexData.pSysMem = NULL;

	teFreeMemory((void*)m_map.chunks[chunk].verticiesInChunk);
	teFreeMemory((void*)m_map.chunks[chunk].indiciesInChunk);

	return true;
}

//WARNING: ONLY WORKS WITH SQUARED MAPS!!
teResult TEMap::teUpdateChunks(QUICK_CAM & cam, vBox frustum)
{
	//Position prüfen und die benötigten Winkel für die Offsetbox festlegen

	//Position des Spielers
	XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, cam.pos);

	//Variablen zur Unterteilung der Map
	//Das Chunkoffset festlegen, also wann ein neuer Chunk anfängt
	int chunksPerColumn = sqrt(m_map.numChunks);
	int currentLine = 0;
	int currentColumn = 0;

	//Alle Chunks durchgehen
	for (int chunkIterator = 0; chunkIterator < m_map.numChunks; ++chunkIterator)
	{
		//Aktuelle Zeile abfragen und setzen (Prüfen ob die aktuelle Zeile fertig ist)
		if (currentColumn == chunksPerColumn)
		{
			//Wenn ja, dann eine Zeile nach oben
			currentLine++;

			//Und die aktuelle Spalte wieder auf 0 setzen
			currentColumn = 0;
		}

		//Die Chunkecken als Positionsangaben
		vPoint posChunkCorner00, posChunkCorner10, posChunkCorner01, posChunkCorner11;

		//Jede Ecke eines Chunks
		posChunkCorner00 = { (float)(currentColumn*m_map.widthChunk), (float)(m_scale.y * m_heightmap[(currentColumn*m_map.widthChunk)*m_height + (currentLine*m_map.heightChunk)]), (float)(currentLine*m_map.heightChunk) };
		posChunkCorner10 = { (float)((currentColumn + 1)*(m_map.widthChunk - 1)), (float)(m_scale.y * m_heightmap[(currentColumn + 1*m_map.widthChunk - 1)*m_height + (currentLine*m_map.heightChunk)]), (float)(currentLine*m_map.heightChunk) };
		posChunkCorner01 = { (float)(currentColumn*m_map.widthChunk), (float)(m_scale.y * m_heightmap[(currentColumn*m_map.widthChunk)*m_height + (currentLine + 1*m_map.heightChunk - 1)]), (float)((currentLine + 1)*(m_map.heightChunk - 1)) };
		posChunkCorner11 = { (float)((currentColumn + 1)*(m_map.widthChunk - 1)), (float)(m_scale.y * m_heightmap[(currentColumn + 1*m_map.widthChunk - 1)*m_height + (currentLine + 1*m_map.heightChunk - 1)]), (float)((currentLine + 1)*(m_map.heightChunk - 1)) };

		if (Vampire::vIsPointInBox(frustum, posChunkCorner00) || Vampire::vIsPointInBox(frustum, posChunkCorner10) || 
			Vampire::vIsPointInBox(frustum, posChunkCorner01) || Vampire::vIsPointInBox(frustum, posChunkCorner11))
		{
			m_map.chunks[chunkIterator].toRender = true;
			m_map.chunksToRender++;
		}
		else
		{
			m_map.chunks[chunkIterator].toRender = false;
		}

		//Die aktuelle Spalte für den nächsten Chunk erhöhen
		currentColumn++;
	}

	if (m_map.chunksToRender == NULL)
	{
		LOGFILE->print(colors::TE_WARNING, "Es werden keine Chunks gerendert.");
	}
	else if (m_map.chunksToRender == m_map.numChunks)
	{
		LOGFILE->print(colors::TE_WARNING, "Es werden alle Chunks gerendert.");
	}

	m_map.chunksToRender = 0;

	return true;
}

TEMap::TEMap(TE_OBJECT_DESC & init) : TEObject(init)
{
	//Members setzen
	m_pos = init.pos;
	m_scale = init.scale;
	m_map.numChunks = 100;
	m_world = XMMatrixIdentity();

	//Prüfen, ob die angegebenen MapObjekt-Dateien existieren
	if (!teFileExists(init.file1) || !teFileExists(init.file2))
	{
		LOGFILE->print(colors::TE_ERROR, "Die Mapdateien können nicht gefunden werden.");
		return;
	}

	//Die Daten aus der angegebenen TesseractHightMap-Datei laden
	if (!teLoadFromTHM(init.file1, init.file2, ENGINE->teGetGraphics()->teGetDevice()))
	{
		LOGFILE->print(colors::TE_WARNING, "Die Erstellung des Objekts ist fehlgeschlagen. Das Objekt konnte nicht aus der Datei gelesen werden.");
		return;
	}

	m_material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f);
	m_material.specular = XMFLOAT4(0.0f, 0.23f, 0.77f, 16.0f);

	LOGFILE->print(colors::TE_CONSTRUCTION, "Die Map wurde erfolgreich erstellt.");
}

TEMap::~TEMap()
{
	for (int chunkIterator = 0; chunkIterator < m_map.numChunks; ++chunkIterator)
	{
		m_map.chunks[chunkIterator].vertexBuffer->Release();
		m_map.chunks[chunkIterator].indexBuffer->Release();

		m_map.chunks[chunkIterator].vertexBuffer = NULL;
		m_map.chunks[chunkIterator].indexBuffer = NULL;
	}

	teFreeMemory((void*)m_map.chunks);
}

void TEMap::render()
{
	UINT stride = sizeof(Vertex), offset = 0;

	m_texTransform = XMMatrixScaling(800.0f, 800.0f, 0.0f);

	GRAPHICS->teSetObjectRenderStates(m_world, m_material, m_texTransform);
	GRAPHICS->teSetObjectTexture(m_pSRVgrass);

	//Alle Chunks durchgehen; die zu Rendernden rendern
	for (int chunkIterator = 0; chunkIterator < m_map.numChunks; ++chunkIterator)
	{
		if (m_map.chunks[chunkIterator].toRender == true)
		{
			GRAPHICS->teGetDeviceContext()->IASetVertexBuffers(0, 1, &m_map.chunks[chunkIterator].vertexBuffer, &stride, &offset);
			GRAPHICS->teGetDeviceContext()->IASetIndexBuffer(m_map.chunks[chunkIterator].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			D3DX11_TECHNIQUE_DESC techDesc;
			GRAPHICS->teGetEffectTechnique()->GetDesc(&techDesc);
			for (UINT p = 0; p < techDesc.Passes; ++p)
			{
				GRAPHICS->teGetEffectTechnique()->GetPassByIndex(p)->Apply(0, GRAPHICS->teGetDeviceContext());
				GRAPHICS->teGetDeviceContext()->DrawIndexed(m_map.indicesPerChunk, 0, 0);
			}
		}
	}
}

void TEMap::tick(double dt)
{
	vBox * frustum = (vBox*)teAlloc(sizeof(vBox));

	vPlane * box = (vPlane*)teAlloc(sizeof(vPlane) * 5);

	vPoint a, b, c;

	QUICK_CAM quickcam = ENGINE->teGetQuickCam();

	//Punkte für rechte Clippingebene festlegen
	a = xmTovPoint(quickcam.pos);

	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[0].recalculatePlane(a, b, c);

	//Punkte für linke Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[1].recalculatePlane(a, b, c);

	//Punkte für obere Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[2].recalculatePlane(a, b, c);

	//Punkte für untere Clippingebene festlegen
	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += a;

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += a;

	box[3].recalculatePlane(a, b, c);

	//Punkte für ferne Clippingebene festlegen
	a = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	a += xmTovPoint(quickcam.pos);

	b = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, -quickcam.fovx / 2) * quickcam.farplane);
	b += xmTovPoint(quickcam.pos);

	c = xmTovPoint(rotateXM(quickcam.lookAt, quickcam.right, -quickcam.fovy / 2, quickcam.up, quickcam.fovx / 2) * quickcam.farplane);
	c += xmTovPoint(quickcam.pos);

	box[4].recalculatePlane(a, b, c);

	frustum->defineBox(box, 5);

	teFreeMemory((void*)box);

	if (!teUpdateChunks(quickcam, frustum[0]))
	{
		return;
	}
}
