#pragma once

//Includes
#include "GameGlobal.hpp"
#include "Terrain.hpp"

typedef struct teChunk
{
	ChunkSettings * m_chunkSettings;
	Vertex * vertecies;
	UINT * indecies;
	ID3D11Buffer * m_pVertexBuffer, *m_pIndexBuffer;
	int n_vertecies, n_indecies;
	bool m_isGenerated;
	bool m_toGenerate;
	bool m_toRender;
	bool m_changed;
}teChunk;

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

		//Erstellt den Vertex - und Indexbuffer
		teResult teCreateBuffer(ID3D11Device * device);

		//Updated den Vertex und Indexbuffer, sodass immer nur die Vertizen im Buffer sind, die tatsächlich gezeichnet werden
		teResult teUpdateChunks(XMFLOAT3 pos);

		//Variablen

		//Breite und Höhe der Map
		int m_width, m_height;

		//render system
		int m_chunksToRender;
		TerrainSettings m_terrainSettings;
		ChunkGenerationInfo m_genInfo;
		std::list<teChunk> m_chunks;
		std::list<teChunk>::iterator m_chunkIter;
		int m_chunkGenRange;
		int m_lowestTesselation, m_heighestTesselation;
		Vertex ** m_tempVertecies;
		UINT ** m_tempIndecies;

		LightMaterial m_material;
		ID3D11ShaderResourceView * m_pSRVgrass, * m_pSRVrock, * m_pSRVsand, * m_pSRVnotset;
		XMMATRIX m_texTransform;
		D3D11_SUBRESOURCE_DATA m_vertexData, m_indexData; 
		D3D11_BUFFER_DESC m_vertexBufferDesc, m_indexBufferDesc;
};