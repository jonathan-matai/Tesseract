#pragma once

//Includes
#include "GameGlobal.hpp"

typedef struct teTerrainVertex
{
	Vertex vertex;
	uint8_t biom;
}teTerrainVertex;

typedef struct teChunk
{
	Vertex *verticiesInChunk;//Die einzelnen Vertizen
	UINT *indiciesInChunk;//Die einzelnen Indizen
	bool isActive;//Gibt an ob der Chunk aktiv ist
	bool toRender;//Gibt an, ob der Chunk gerendert wird
	D3D11_SUBRESOURCE_DATA vertexData, indexData;//Die Daten der Buffer im DX11 Format
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;//Beschreibung der Buffer
	ID3D11Buffer *vertexBuffer, *indexBuffer;//Die eigentlichen Buffer
}teChunk;

typedef struct teTerrain
{
	int numChunks;//Anzahl an Chunks
	int widthChunk, heightChunk;//Breite und Höhe der einzelnen Chunks in Vertizen
	int verticiesPerChunk, cellsPerChunk, indicesPerChunk;//Vertizen und Zellen pro Chunk
	teChunk * chunks;//Alle Chunks
	int chunksToRender;//Wie viele Chunks gerendert werden sollen
}teTerrain;

//Der Objekttyp, der die Map darstellt (erbt von TEObject)
class TEMap : public TEObject
{
	public:

		TEMap(TE_OBJECT_DESC & init);
		~TEMap();

		//Rendert die Map
		void render();

		//Updated die Map
		void tick(double dt);

	private:

		//Lädt die .thm-Datei (Der file Parameter gibt nur den Pfad an; d.h. der Dateiname mit Endung wird automatisch angehängt)
		//Aufruf also: teLoadFromTHM("Data//Meshes//Map");
		//Die Dateien sollten thmap.thm und tbmap.tbm heißen
		teResult teLoadFromTHM(wchar_t * heightMap, wchar_t * biomMap, ID3D11Device * device);

		//Erstellt den Vertex - und Indexbuffer
		teResult teCreateBuffer(ID3D11Device * device, int chunk);

		//Updated den Vertex und Indexbuffer, sodass immer nur die Vertizen im Buffer sind, die tatsächlich gezeichnet werden
		teResult teUpdateChunks(QUICK_CAM & cam, vBox frustum);

		//Variablen

		//Breite und Höhe der Map
		int m_width, m_height;

		//Eindimensionales Array wobei jeder Wert ein y-Wert ist, der dem x-ten und dem z + x * height-ten Platz zugeordnet wird
		short * m_heightmap;

		//Eindimensionales Array, das die zum Eintrag in der Heightmap gehörende Biominformation speichert
		uint8_t * m_biommap;

		//Eigentliche Map; hier sind die Daten aus Höhenmap und Biommap zusammengefasst und in Chunks eingeteilt verfügbar
		teTerrain m_map;

		LightMaterial m_material;
		ID3D11ShaderResourceView * m_pSRVgrass, * m_pSRVrock, * m_pSRVsand, * m_pSRVnotset;
		XMMATRIX m_texTransform;
};