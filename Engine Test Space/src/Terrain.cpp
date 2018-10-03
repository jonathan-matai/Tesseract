#include "..\include\Terrain.hpp"

#include <memory> // memset(), memcpy()

uint32_t _i = 0;

uint32_t hash(uint32_t i, uint32_t j, uint64_t seed)
{
	uint64_t x = (i + ((uint64_t) j << 32)) ^ seed;
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
	x = x ^ (x >> 31);
	return x & 0xffffffffu;
}

float cubicInterpolation(XMFLOAT4 p, float x)
{
	return p.y + 0.5f * x * (p.z - p.x + x * (2.0f * p.x - 5.0f * p.y + 4.0f * p.z - p.w + x * (3.0f * (p.y - p.z) + p.w - p.x)));
}

XMVECTOR cubicInterpolation(XMMATRIX p, float x)
{
	return p.r[1] + 0.5f * x * (p.r[2] - p.r[0] + x * (2.0 * p.r[0] - 5.0 * p.r[1] + 4.0 * p.r[2] - p.r[3] + x * (3.0 * (p.r[1] - p.r[2]) + p.r[3] - p.r[0])));
}

float bicubicInterpolation(XMMATRIX p, float x, float y)
{
	XMFLOAT4 v;
	XMStoreFloat4(&v, cubicInterpolation(p, y));
	return cubicInterpolation(v, x);
}

XMMATRIX hash4x4(float x_base, float y_base, float triangle_width, float amplitude, uint64_t seed)
{
	++_i;
	return XMMATRIX((int) hash((uint32_t) (int) ( 1024 * (x_base -		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base -		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base						)), (uint32_t) (int) ( 1024 * ( y_base -		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base +		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base -		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base + 2 *	triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base -		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base -		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base							)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base						)), (uint32_t) (int) ( 1024 * ( y_base							)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base +		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base							)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base + 2 *	triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base							)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base -		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base +		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base						)), (uint32_t) (int) ( 1024 * ( y_base +		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base +		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base +		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base + 2 *	triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base +		triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base -		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base + 2 *	triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base						)), (uint32_t) (int) ( 1024 * ( y_base + 2 *	triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base +		triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base + 2 *	triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude,
					(int) hash((uint32_t) (int) ( 1024 * (x_base + 2 *	triangle_width	)), (uint32_t) (int) ( 1024 * ( y_base + 2 *	triangle_width	)), seed) / (float) 0x7FFFFFFF * amplitude);
}



void generateChunk(const TerrainSettings& terrainSettings, const ChunkSettings& chunkSettings, Vertex_3PNU** vertexBuffer, size_t* n_verticies, uint32_t** indexBuffer, size_t* n_indicies)
{
	// Breite der Dreiecke und Zahl der Vertexreihen bestimmen

	float triangleWidth = powf(2.0f, (float) -chunkSettings.tesselation);
	uint32_t n_rows = (uint32_t) ( ( terrainSettings.chunkSize / triangleWidth + 2 /* Überlappung für Normalen */ )  ) + 1; /* n_Vertizenreihen = n_Dreieckreihen + 1 */

	// Speicher reservieren und alle Bits auf 0 setzen

	Vertex_3PNU* t_vertexBuffer = _alloc(Vertex_3PNU, n_rows * n_rows );
	memset(t_vertexBuffer, 0x00, sizeof(Vertex_3PNU) * n_rows * n_rows);

	float cur_amplitude = terrainSettings.baseAmplitude / 2;
	float cur_wavelength = terrainSettings.baseWavelength;

	// Durch alle Noiselayers loopen
	for (uint8_t i = 0; i < terrainSettings.layers; ++i)
	{
		// Position der Interpolationsankerpunkte
		XMFLOAT2 hashLocation = { floorf(0.0f / cur_wavelength) * cur_wavelength, floorf(0.0f / cur_wavelength) * cur_wavelength };

		// Werte der Ankerpunkte
		XMMATRIX hashValues = hash4x4(hashLocation.x, hashLocation.y, cur_wavelength, cur_amplitude, 0x0123456789abcdefu);

		// in z-Richtung iterieren
		for (size_t j = 0; j < n_rows; ++j)
		{
			// in x-Richtung iterieren
			for (size_t k = 0; k < n_rows; ++k)
			{
				// Vertexposition bestimmen
				XMFLOAT2 vertexLocation = {	(float) terrainSettings.chunkSize * chunkSettings.id_x /* Kleinere Seite */+ ( (float)k - 1 ) /* Überlappung für Normalen */ * triangleWidth,
											(float) terrainSettings.chunkSize * chunkSettings.id_z /* Kleinere Seite */+ ( (float)j - 1 ) /* Überlappung für Normalen */ * triangleWidth };

				// Position in Vertexbuffer schrieben
				t_vertexBuffer[j * n_rows + k].position.x = vertexLocation.x;
				t_vertexBuffer[j * n_rows + k].position.z = vertexLocation.y;

				// Interpolationsankerpunkte bestimmen
				XMFLOAT2 hashLocNew = { floorf(vertexLocation.x / cur_wavelength) * cur_wavelength,
										floorf(vertexLocation.y / cur_wavelength) * cur_wavelength };

				// Wenn sich die Position der Ankerpunkte verändert hat, neu hashen
				if (!XMVector2Equal(XMLoadFloat2(&hashLocNew), XMLoadFloat2(&hashLocation)))
				{
					hashLocation = hashLocNew;
					hashValues = hash4x4(hashLocation.x, hashLocation.y, cur_wavelength, cur_amplitude, 0x0123456789abcdefu);
				}

				// y-Koordinate bestimmen
				t_vertexBuffer[j * n_rows + k].position.y += 
					bicubicInterpolation(hashValues, fmodf(vertexLocation.x, cur_wavelength) / cur_wavelength, fmodf(vertexLocation.y, cur_wavelength) / cur_wavelength);

				// UV-Koordinate bestimmen
				XMStoreFloat2(&t_vertexBuffer[j * n_rows + k].uv, XMLoadFloat2(&vertexLocation) / terrainSettings.textureWidth);
			}
		}

		// Wellenlänge und Amplitude für nächste Ebene modifizieren

		cur_amplitude *= terrainSettings.mod_aplitude;
		cur_wavelength *= terrainSettings.mod_wavelength;
	}

	// Normalen berechnen

	for (size_t i = 0; i < n_rows - 1; ++i)
	{
		for (size_t j = 0; j < n_rows - 1; ++j)
		{
			// Pointer zu den umliegenden 4 Vertizen
			Vertex_3PNU* pv0 = &t_vertexBuffer[ j		* n_rows + i	 ];
			Vertex_3PNU* pv1 = &t_vertexBuffer[ j		* n_rows + i + 1 ];
			Vertex_3PNU* pv2 = &t_vertexBuffer[ (j + 1)	* n_rows + i + 1 ];
			Vertex_3PNU* pv3 = &t_vertexBuffer[ (j + 1)	* n_rows + i	 ];

			// Normalen der Triangles berechnen
			XMVECTOR normaltr = XMVector3Cross(XMLoadFloat3(&pv1->position) - XMLoadFloat3(&pv0->position), XMLoadFloat3(&pv2->position) - XMLoadFloat3(&pv0->position));
			XMVECTOR normalbl = XMVector3Cross(XMLoadFloat3(&pv1->position) - XMLoadFloat3(&pv3->position), XMLoadFloat3(&pv2->position) - XMLoadFloat3(&pv3->position));

			// Normalen auf die umgebenden 4 Vertizen addieren
			XMStoreFloat3(&pv0->normal, XMLoadFloat3(&pv0->normal) + normaltr + normalbl);
			XMStoreFloat3(&pv1->normal, XMLoadFloat3(&pv1->normal) + normaltr);
			XMStoreFloat3(&pv2->normal, XMLoadFloat3(&pv2->normal) + normalbl + normaltr);
			XMStoreFloat3(&pv3->normal, XMLoadFloat3(&pv3->normal) + normalbl);
		}
	}

	// Normalen normalisieren

	for (size_t i = 0; i < n_rows * n_rows; ++i)
		XMStoreFloat3(&t_vertexBuffer[i].normal, XMVector3Normalize(XMLoadFloat3(&t_vertexBuffer[i].normal)));
		
	*vertexBuffer = _alloc(Vertex_3PNU, ( n_rows - 2 ) * ( n_rows - 2 ));

	if (!cropArray2d(t_vertexBuffer, n_rows, n_rows, *vertexBuffer, 1, 1, n_rows - 2, n_rows - 2, sizeof(Vertex_3PNU) * ( n_rows - 2 ) * ( n_rows - 2 )))
		throw std::exception("Output buffer doesn't have the same size!");

	_free(t_vertexBuffer);
	
	*n_verticies = ( n_rows - 2 ) * ( n_rows - 2 ); /// Square Later
}
