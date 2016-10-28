/*!*********************************************************************************************************************
\file         PVRAssets\ShadowVolume.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definitions for methods of the ShadowVolume class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/ShadowVolume.h"
#include "PVRAssets/Helper.h"

#include "PVRCore/Log.h"
using std::pair;
using std::map;
// TODO: Test the hyper cube bounding code as it is untested

const static unsigned short c_linesHyperCube[64] =
{
	// Cube0
	0, 1,  2, 3,  0, 2,  1, 3,
	4, 5,  6, 7,  4, 6,  5, 7,
	0, 4,  1, 5,  2, 6,  3, 7,
	// Cube1
	8, 9,  10, 11,  8, 10,  9, 11,
	12, 13,  14, 15,  12, 14,  13, 15,
	8, 12,  9, 13,  10, 14,  11, 15,
	// Hyper cube jn
	0, 8,  1, 9,  2, 10,  3, 11,
	4, 12,  5, 13,  6, 14,  7, 15

};

const static glm::vec3 c_rect0(-1, -1, 1), c_rect1(-1,  1, 1), c_rect2(1, -1, 1), c_rect3(1,  1, 1);
namespace pvr {
using namespace types;
ShadowVolume::~ShadowVolume()
{

	std::map<uint32, ShadowVolumeData>::iterator walk = m_shadowVolumes.begin();
	for (; walk != m_shadowVolumes.end(); ++walk)
	{
		const ShadowVolumeData& volume = walk->second;
		delete [] volume.indexData;
	}
	delete [] m_shadowMesh.vertices;
	delete [] m_shadowMesh.edges;
	delete [] m_shadowMesh.triangles;
	delete [] m_shadowMesh.vertexData;
}

uint32 ShadowVolume::findOrCreateVertex(const glm::vec3& vertex, bool& existed)
{
	// First check whether we already have a vertex here
	for (uint32 i = 0; i < m_shadowMesh.numVertices; ++i)
	{
		if (m_shadowMesh.vertices[i].x == vertex.x && m_shadowMesh.vertices[i].y == vertex.y && m_shadowMesh.vertices[i].z == vertex.z)
		{
			// Don't do anything more if the vertex already exists
			existed = true;
			return i;
		}
	}

	if (m_shadowMesh.numVertices == 0)
	{
		m_shadowMesh.minimum = m_shadowMesh.maximum = vertex;
	}
	else
	{
		if (vertex.x < m_shadowMesh.minimum.x)
		{ m_shadowMesh.minimum.x = vertex.x; }

		if (vertex.y < m_shadowMesh.minimum.y)
		{ m_shadowMesh.minimum.y = vertex.y; }

		if (vertex.z < m_shadowMesh.minimum.z)
		{ m_shadowMesh.minimum.z = vertex.z; }

		if (vertex.x > m_shadowMesh.maximum.x)
		{ m_shadowMesh.maximum.x = vertex.x; }

		if (vertex.y > m_shadowMesh.maximum.y)
		{ m_shadowMesh.maximum.y = vertex.y; }

		if (vertex.z > m_shadowMesh.maximum.z)
		{ m_shadowMesh.maximum.z = vertex.z; }
	}

	// Add the vertex
	memcpy(&m_shadowMesh.vertices[m_shadowMesh.numVertices], &vertex, sizeof(vertex));
	existed = false;
	return m_shadowMesh.numVertices++;
}

uint32 ShadowVolume::findOrCreateEdge(const glm::vec3& v0, const glm::vec3& v1, bool& existed)
{
	uint32 vertexIndices[2];
	bool alreadyExisted[2];
	vertexIndices[0] = findOrCreateVertex(v0, alreadyExisted[0]);
	vertexIndices[1] = findOrCreateVertex(v1, alreadyExisted[1]);

	if (alreadyExisted[0] && alreadyExisted[1])
	{
		// Check whether we already have an edge here
		for (uint32 i = 0; i < m_shadowMesh.numEdges; ++i)
		{
			if ((m_shadowMesh.edges[i].vertexIndices[0] == vertexIndices[0] && m_shadowMesh.edges[i].vertexIndices[1] == vertexIndices[1]) ||
			    (m_shadowMesh.edges[i].vertexIndices[0] == vertexIndices[1] && m_shadowMesh.edges[i].vertexIndices[1] == vertexIndices[0]))
			{
				// Don't do anything more if the edge already exists
				existed = true;
				return i;
			}
		}
	}

	// Add the edge
	m_shadowMesh.edges[m_shadowMesh.numEdges].vertexIndices[0] = vertexIndices[0];
	m_shadowMesh.edges[m_shadowMesh.numEdges].vertexIndices[1] = vertexIndices[1];
	m_shadowMesh.edges[m_shadowMesh.numEdges].visibilityFlags = 0;
	existed = false;
	return m_shadowMesh.numEdges++;
}

void ShadowVolume::findOrCreateTriangle(const glm::vec3& v0, const glm::vec3& v1,
                                        const glm::vec3& v2)
{
	ShadowVolumeEdge* edge0, *edge1, *edge2;
	uint32 edgeIndex0, edgeIndex1, edgeIndex2;
	bool alreadyExisted[3];

	edgeIndex0 = findOrCreateEdge(v0, v1, alreadyExisted[0]);
	edgeIndex1 = findOrCreateEdge(v1, v2, alreadyExisted[1]);
	edgeIndex2 = findOrCreateEdge(v2, v0, alreadyExisted[2]);

	if (edgeIndex0 == edgeIndex1 || edgeIndex1 == edgeIndex2 || edgeIndex2 == edgeIndex0)
	{
		// Degenerate triangle
		return;
	}

	// First check whether we already have a triangle here
	if (alreadyExisted[0] && alreadyExisted[1] && alreadyExisted[2])
	{
		for (uint32 i = 0; i < m_shadowMesh.numTriangles; ++i)
		{
			if ((m_shadowMesh.triangles[i].edgeIndices[0] == edgeIndex0 || m_shadowMesh.triangles[i].edgeIndices[0] == edgeIndex1
			     || m_shadowMesh.triangles[i].edgeIndices[0] == edgeIndex2) &&
			    (m_shadowMesh.triangles[i].edgeIndices[1] == edgeIndex0 || m_shadowMesh.triangles[i].edgeIndices[1] == edgeIndex1
			     || m_shadowMesh.triangles[i].edgeIndices[1] == edgeIndex2) &&
			    (m_shadowMesh.triangles[i].edgeIndices[2] == edgeIndex0 || m_shadowMesh.triangles[i].edgeIndices[2] == edgeIndex1
			     || m_shadowMesh.triangles[i].edgeIndices[2] == edgeIndex2))
			{
				// Don't do anything more if the triangle already exists
				return;
			}
		}
	}

	// Add the triangle then
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].edgeIndices[0] = edgeIndex0;
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].edgeIndices[1] = edgeIndex1;
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].edgeIndices[2] = edgeIndex2;

	// Store the triangle indices; these are indices into the shadow mesh, not the source model indices
	edge0 = &m_shadowMesh.edges[edgeIndex0];
	edge1 = &m_shadowMesh.edges[edgeIndex1];
	edge2 = &m_shadowMesh.edges[edgeIndex2];

	if (edge0->vertexIndices[0] == edge1->vertexIndices[0] || edge0->vertexIndices[0] == edge1->vertexIndices[1])
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[0] = edge0->vertexIndices[1]; }
	else
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[0] = edge0->vertexIndices[0]; }

	if (edge1->vertexIndices[0] == edge2->vertexIndices[0] || edge1->vertexIndices[0] == edge2->vertexIndices[1])
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[1] = edge1->vertexIndices[1]; }
	else
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[1] = edge1->vertexIndices[0]; }

	if (edge2->vertexIndices[0] == edge0->vertexIndices[0] || edge2->vertexIndices[0] == edge0->vertexIndices[1])
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[2] = edge2->vertexIndices[1]; }
	else
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].vertexIndices[2] = edge2->vertexIndices[0]; }

	// Calculate the triangle normal
	glm::vec3 n0(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
	glm::vec3 n1(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

	m_shadowMesh.triangles[m_shadowMesh.numTriangles].normal.x = n0.y * n1.z - n0.z * n1.y;
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].normal.y = n0.z * n1.x - n0.x * n1.z;
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].normal.z = n0.x * n1.y - n0.y * n1.x;

	// Check which edges have the correct winding order for this triangle
	m_shadowMesh.triangles[m_shadowMesh.numTriangles].winding = 0;

	if (memcmp(&m_shadowMesh.vertices[edge0->vertexIndices[0]], &v0, sizeof(v0)) == 0)
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].winding |= 0x01; }

	if (memcmp(&m_shadowMesh.vertices[edge1->vertexIndices[0]], &v1, sizeof(v1)) == 0)
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].winding |= 0x02; }

	if (memcmp(&m_shadowMesh.vertices[edge2->vertexIndices[0]], &v2, sizeof(v2)) == 0)
	{ m_shadowMesh.triangles[m_shadowMesh.numTriangles].winding |= 0x04; }

	++m_shadowMesh.numTriangles;
	return;
}

Result ShadowVolume::init(const assets::Mesh& mesh)
{
	const assets::Mesh::VertexAttributeData* positions = mesh.getVertexAttributeByName("POSITION");

	if (positions == NULL)
	{ return Result::NoData; }

	uint32 posIdx = positions->getDataIndex();
	if (posIdx) { return Result::NoData; }

	const assets::Mesh::FaceData& faceData = mesh.getFaces();

	return init(static_cast<const byte*>(mesh.getData(posIdx)), mesh.getNumVertices(), mesh.getStride(posIdx),
	            positions->getVertexLayout().dataType, faceData.getData(), mesh.getNumFaces(), faceData.getDataType());
}

Result ShadowVolume::init(const byte* const data, uint32 numVertices,
                          uint32 verticesStride, DataType vertexType, const byte* const faceData,
                          uint32 numFaces, IndexType indexType)
{
	delete [] m_shadowMesh.vertices;
	m_shadowMesh.numVertices = 0;

	delete [] m_shadowMesh.edges;
	m_shadowMesh.numEdges = 0;

	delete [] m_shadowMesh.triangles;
	m_shadowMesh.numTriangles = 0;

	m_shadowMesh.vertices = new glm::vec3[numVertices];

	if (faceData)
	{
		m_shadowMesh.edges = new ShadowVolumeEdge[3 * numFaces];
		m_shadowMesh.triangles = new ShadowVolumeTriangle[3 * numFaces];

		uint32 indexStride = indexTypeSizeInBytes(indexType);

		byte* facePtr = (byte*) faceData;

		for (uint32 i = 0; i < numFaces; ++i)
		{
			uint32 indices[3];
			VertexIndexRead(facePtr, indexType, &indices[0]);
			facePtr += indexStride;
			VertexIndexRead(facePtr, indexType, &indices[1]);
			facePtr += indexStride;
			VertexIndexRead(facePtr, indexType, &indices[2]);
			facePtr += indexStride;

			glm::vec3 vertex0, vertex1, vertex2;
			VertexRead(data + (verticesStride * indices[0]), vertexType, 3, &vertex0.x);
			VertexRead(data + (verticesStride * indices[1]), vertexType, 3, &vertex1.x);
			VertexRead(data + (verticesStride * indices[2]), vertexType, 3, &vertex2.x);

			findOrCreateTriangle(vertex0, vertex1, vertex2);
		}
	}
	else     // Non-index
	{
		m_shadowMesh.edges = new ShadowVolumeEdge[numVertices / 3];
		m_shadowMesh.triangles = new ShadowVolumeTriangle[numVertices / 3];

		for (uint32 i = 0; i < numVertices; i += 3)
		{
			glm::vec3 vertex0, vertex1, vertex2;
			VertexRead(data + (verticesStride * (i + 0)), vertexType, 3, &vertex0.x);
			VertexRead(data + (verticesStride * (i + 1)), vertexType, 3, &vertex1.x);
			VertexRead(data + (verticesStride * (i + 2)), vertexType, 3, &vertex2.x);

			findOrCreateTriangle(vertex0, vertex1, vertex2);
		}
	}

#ifdef DEBUG
	// Check the data is valid
	for (uint32 edge = 0; edge < m_shadowMesh.numEdges; ++edge)
	{
		uint32 count = 0;
		for (uint32 triangle = 0; triangle < m_shadowMesh.numTriangles; ++triangle)
		{
			if (m_shadowMesh.triangles[triangle].edgeIndices[0] == edge)
			{ ++count; }

			if (m_shadowMesh.triangles[triangle].edgeIndices[1] == edge)
			{ ++count; }

			if (m_shadowMesh.triangles[triangle].edgeIndices[2] == edge)
			{ ++count; }
		}

		/*
			Every edge should be referenced exactly twice.
			If they aren't then the mesh isn't closed which will cause problems when rendering the shadows.
		*/
		assertion(count == 2);
	}

#endif

	// Create the real mesh
	{
		glm::vec3* tmp = new glm::vec3[m_shadowMesh.numVertices];

		memcpy(tmp, m_shadowMesh.vertices, m_shadowMesh.numVertices * sizeof(*m_shadowMesh.vertices));
		delete [] m_shadowMesh.vertices;
		m_shadowMesh.vertices = tmp;
	}

	{
		ShadowVolumeEdge* tmp = new ShadowVolumeEdge[m_shadowMesh.numEdges];

		memcpy(tmp, m_shadowMesh.edges, m_shadowMesh.numEdges * sizeof(*m_shadowMesh.edges));
		delete [] m_shadowMesh.edges;
		m_shadowMesh.edges = tmp;
	}

	{
		ShadowVolumeTriangle* tmp = new ShadowVolumeTriangle[m_shadowMesh.numTriangles];

		memcpy(tmp, m_shadowMesh.triangles, m_shadowMesh.numTriangles * sizeof(*m_shadowMesh.triangles));
		delete [] m_shadowMesh.triangles;
		m_shadowMesh.triangles = tmp;
	}

	m_shadowMesh.needs32BitIndices = (m_shadowMesh.numTriangles * 2 * 3) > 65535;

	initializeVertexData();
	return Result::Success;
}

void ShadowVolume::initializeVertexData(byte** externalBuffer)
{
	byte* tmp;

	if (externalBuffer && *externalBuffer != NULL)
	{
		tmp = *externalBuffer;
	}
	else
	{
		delete [] m_shadowMesh.vertexData;
		m_shadowMesh.vertexData = new byte[getVertexDataSize()];

		tmp = m_shadowMesh.vertexData;
	}

	struct LocalVertex
	{
		float32 x, y, z;
		uint32 extrude;
	};

	uint32 stride = getVertexDataStride();

	// Fill the vertex buffer with two subtly different copies of the vertices
	for (uint32 i = 0; i < m_shadowMesh.numVertices; ++i)
	{
		LocalVertex& vertex = *reinterpret_cast<LocalVertex*>(&tmp[i * stride]);
		LocalVertex& mirrorMirrorVertex = *reinterpret_cast<LocalVertex*>(&tmp[(i + m_shadowMesh.numVertices) * stride]);

		vertex.x = mirrorMirrorVertex.x = m_shadowMesh.vertices[i].x;
		vertex.y = mirrorMirrorVertex.y = m_shadowMesh.vertices[i].y;
		vertex.z = mirrorMirrorVertex.z = m_shadowMesh.vertices[i].z;

		vertex.extrude = 0;
		mirrorMirrorVertex.extrude = 0x04030201; // The order is wzyx
	}

}

void ShadowVolume::alllocateShadowVolume(uint32 volumeID)
{
	ShadowVolumeData volume;
	volume.indexData = new byte[getIndexDataSize()];
	m_shadowVolumes.insert(pair<uint32, ShadowVolumeData>(volumeID, volume));
}

Result ShadowVolume::releaseVolume(uint32 volumeID)
{
	std::map<uint32, ShadowVolumeData>::iterator found = m_shadowVolumes.find(volumeID);
	assertion(found != m_shadowVolumes.end());

	if (found == m_shadowVolumes.end())
	{
		return Result::OutOfBounds;
	}
	{
		const ShadowVolumeData& volume = found->second;
		delete [] volume.indexData;
	}
	m_shadowVolumes.erase(found);
	return Result::Success;
}

byte* ShadowVolume::getVertexData()
{
	return m_shadowMesh.vertexData;
}

uint32 ShadowVolume::getVertexDataPositionOffset()
{
	return 0;
}

uint32 ShadowVolume::getVertexDataExtrudeOffset()
{
	return sizeof(float32) * 3;
}

uint32 ShadowVolume::getVertexDataSize()
{
	return m_shadowMesh.numVertices * 2 * getVertexDataStride();
}

uint32 ShadowVolume::getVertexDataStride()
{
	return 3 * sizeof(float32) + sizeof(uint32);
}

bool ShadowVolume::isVertexDataInternal()
{
	return m_shadowMesh.vertexData != NULL;
}

bool ShadowVolume::isIndexDataInternal(uint32 volumeID)
{

	ShadowVolumeMapType::iterator found = m_shadowVolumes.begin();
	std::advance(found, volumeID);
	return (found != m_shadowVolumes.end() && found->second.indexData != NULL);
}

uint32 ShadowVolume::getIndexDataSize()
{
	return m_shadowMesh.numTriangles * 2 * 3 * getIndexDataStride();
}

uint32 ShadowVolume::getIndexDataStride()
{
	return m_shadowMesh.needs32BitIndices ? sizeof(uint32) : sizeof(uint16);
}

uint32 ShadowVolume::getIndexCount(uint32 volumeID)
{

	ShadowVolumeMapType::iterator found = m_shadowVolumes.find(volumeID);
	assertion(found != m_shadowVolumes.end());

	if (found == m_shadowVolumes.end())
	{
		return 0;
	}

	return found->second.indexCount;
}

byte* ShadowVolume::getIndices(uint32 volumeID)
{
	ShadowVolumeMapType::iterator found = m_shadowVolumes.find(volumeID);
	assertion(found != m_shadowVolumes.end());

	if (found == m_shadowVolumes.end())
	{
		return 0;
	}

	return found->second.indexData;
}

Result ShadowVolume::projectSilhouette(uint32 volumeID, uint32 flags, const glm::vec3& lightModel, bool isPointLight,
                                       byte** externalIndexBuffer)
{
	if (m_shadowMesh.needs32BitIndices)
	{
		return project<uint32>(volumeID, flags, lightModel, isPointLight, reinterpret_cast<uint32**>(externalIndexBuffer));
	}
	else
	{
		return project<uint16>(volumeID, flags, lightModel, isPointLight, reinterpret_cast<uint16**>(externalIndexBuffer));
	}
}

template<typename INDEXTYPE>
Result ShadowVolume::project(uint32 volumeID, uint32 flags, const glm::vec3& lightModel, bool isPointLight,
                             INDEXTYPE** externalIndexBuffer)
{
	ShadowVolumeMapType::iterator found = m_shadowVolumes.find(volumeID);
	assertion(found != m_shadowVolumes.end());

	if (found != m_shadowVolumes.end())
	{
		return Result::OutOfBounds;
	}

	ShadowVolumeData& volume = found->second;
	INDEXTYPE* indices = externalIndexBuffer ? *externalIndexBuffer : reinterpret_cast<INDEXTYPE*>(volume.indexData);

	if (indices == NULL)
	{ return Result::NoData; }

	float32 f;
	volume.indexCount = 0;

	// Run through triangles, testing which face the From point
	for (uint32 i = 0; i < m_shadowMesh.numTriangles; ++i)
	{
		ShadowVolumeEdge* edge0, *edge1, *edge2;
		edge0 = &m_shadowMesh.edges[m_shadowMesh.triangles[i].edgeIndices[0]];
		edge1 = &m_shadowMesh.edges[m_shadowMesh.triangles[i].edgeIndices[1]];
		edge2 = &m_shadowMesh.edges[m_shadowMesh.triangles[i].edgeIndices[2]];

		if (isPointLight)
		{
			glm::vec3 v;
			v.x = m_shadowMesh.vertices[edge0->vertexIndices[0]].x - lightModel.x;
			v.y = m_shadowMesh.vertices[edge0->vertexIndices[0]].y - lightModel.y;
			v.z = m_shadowMesh.vertices[edge0->vertexIndices[0]].z - lightModel.z;

			// Dot product
			f = m_shadowMesh.triangles[i].normal.x * v.x + m_shadowMesh.triangles[i].normal.y * v.y + m_shadowMesh.triangles[i].normal.z *
			    v.z;
		}
		else
		{
			f = m_shadowMesh.triangles[i].normal.x * lightModel.x + m_shadowMesh.triangles[i].normal.y * lightModel.y +
			    m_shadowMesh.triangles[i].normal.z * lightModel.z;
		}

		if (f >= 0)
		{
			// Triangle is in the light
			edge0->visibilityFlags |= 0x01;
			edge1->visibilityFlags |= 0x01;
			edge2->visibilityFlags |= 0x01;

			if (flags & Cap_front)
			{
				// Add the triangle to the volume, un-extruded.
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[0]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[1]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[2]);
			}
		}
		else
		{
			// Triangle is in shade; set Bit3 if the winding order needs reversing
			edge0->visibilityFlags |= 0x02 | (m_shadowMesh.triangles[i].winding & 0x01) << 2;
			edge1->visibilityFlags |= 0x02 | (m_shadowMesh.triangles[i].winding & 0x02) << 1;
			edge2->visibilityFlags |= 0x02 | (m_shadowMesh.triangles[i].winding & 0x04);

			if (flags & Cap_back)
			{
				// Add the triangle to the volume, extruded.
				// numVertices is used as an offset so that the new index refers to the
				// corresponding position in the second array of vertices (which are extruded)
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[0] + m_shadowMesh.numVertices);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[1] + m_shadowMesh.numVertices);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.triangles[i].vertexIndices[2] + m_shadowMesh.numVertices);
			}
		}
	}

#ifdef DEBUG // Sanity checks
	assertion(volume.indexCount * sizeof(INDEXTYPE) <= getIndexDataSize()); // Have we accessed memory we shouldn't have?

	for (uint32 i = 0; i < volume.indexCount; ++i)
	{
		assertion(indices[i] < m_shadowMesh.numVertices * 2);
	}
#endif

	// Run through edges, testing which are silhouette edges
	for (uint32 i = 0; i < m_shadowMesh.numEdges; ++i)
	{
		if ((m_shadowMesh.edges[i].visibilityFlags & 0x03) == 0x03)
		{
			/*
				Silhouette edge found!
				The edge is both visible and hidden, so it is along the silhouette of the model (See header notes for more info)
			*/
			if (m_shadowMesh.edges[i].visibilityFlags & 0x04)
			{
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0] + m_shadowMesh.numVertices);

				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0] + m_shadowMesh.numVertices);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1] + m_shadowMesh.numVertices);
			}
			else
			{
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1] + m_shadowMesh.numVertices);

				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[1] + m_shadowMesh.numVertices);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0]);
				indices[volume.indexCount++] = static_cast<INDEXTYPE>(m_shadowMesh.edges[i].vertexIndices[0] + m_shadowMesh.numVertices);
			}
		}

		// Zero for next render
		m_shadowMesh.edges[i].visibilityFlags = 0;
	}

#ifdef DEBUG // Sanity checks
	assertion(volume.indexCount * sizeof(INDEXTYPE) <= getIndexDataSize()); // Have we accessed memory we shouldn't have?

	for (uint32 i = 0; i < volume.indexCount; ++i)
	{
		assertion(indices[i] < m_shadowMesh.numVertices * 2);
	}
#endif

	return Result::Success;
}

static inline void transformPoint(const glm::mat4x4& projection, float32 bx, float32 by, float32 bz, float lightProjZ,
                                  glm::vec4& out, uint32& clipZCount, uint32& clipFlagsA)
{
	out.x = (projection[0][0] * bx) + (projection[1][0] * by) + (projection[2][0] * bz) + projection[3][0];
	out.y = (projection[0][1] * bx) + (projection[1][1] * by) + (projection[2][1] * bz) + projection[3][1];
	out.z = (projection[0][2] * bx) + (projection[1][2] * by) + (projection[2][2] * bz) + projection[3][2];
	out.w = (projection[0][3] * bx) + (projection[1][3] * by) + (projection[2][3] * bz) + projection[3][3];

	if (out.z <= 0)
	{ ++clipZCount; }

	if (out.z <= lightProjZ)
	{ ++clipFlagsA; }
}

static inline void extrudeAndTransformPoint(const glm::mat4x4& projection, float32 bx, float32 by, float32 bz,
    const glm::vec3& lightModel, bool isPointLight, float extrudeLength, glm::vec4& out)
{
	// Extrude ...
	if (isPointLight)
	{
		out.x = bx + extrudeLength * (bx - lightModel.x);
		out.y = by + extrudeLength * (by - lightModel.y);
		out.z = bz + extrudeLength * (bz - lightModel.z);
	}
	else
	{
		out.x = bx + extrudeLength * lightModel.x;
		out.y = by + extrudeLength * lightModel.y;
		out.z = bz + extrudeLength * lightModel.z;
	}

	// ... and transform
	out.x = (projection[0][0] * out.x) + (projection[1][0] * out.y) + (projection[2][0] * out.z) + projection[3][0];
	out.y = (projection[0][1] * out.x) + (projection[1][1] * out.y) + (projection[2][1] * out.z) + projection[3][1];
	out.z = (projection[0][2] * out.x) + (projection[1][2] * out.y) + (projection[2][2] * out.z) + projection[3][2];
	out.w = (projection[0][3] * out.x) + (projection[1][3] * out.y) + (projection[2][3] * out.z) + projection[3][3];
}

static inline bool isBoundingHyperCubeVisible(const glm::vec4(&boundingHyperCube)[16], float cameraZProj)
{
	uint32 clipFlagsA(0), clipFlagsB(0);
	const glm::vec4* extrudedVertices(&boundingHyperCube[8]);

	for (uint32 i = 0; i < 8; ++i)
	{
		// Far
		if (extrudedVertices[i].x < extrudedVertices[i].w)
		{ clipFlagsA |= 1 << 0; }

		if (extrudedVertices[i].x > -extrudedVertices[i].w)
		{ clipFlagsA |= 1 << 1; }

		if (extrudedVertices[i].y < extrudedVertices[i].w)
		{ clipFlagsA |= 1 << 2; }

		if (extrudedVertices[i].y > -extrudedVertices[i].w)
		{ clipFlagsA |= 1 << 3; }

		if (extrudedVertices[i].z > 0)
		{ clipFlagsA |= 1 << 4; }

		// Near
		if (boundingHyperCube[i].x < boundingHyperCube[i].w)
		{ clipFlagsA |= 1 << 0; }

		if (boundingHyperCube[i].x > -boundingHyperCube[i].w)
		{ clipFlagsA |= 1 << 1; }

		if (boundingHyperCube[i].y < boundingHyperCube[i].w)
		{ clipFlagsA |= 1 << 2; }

		if (boundingHyperCube[i].y > -boundingHyperCube[i].w)
		{ clipFlagsA |= 1 << 3; }

		if (boundingHyperCube[i].z > 0)
		{ clipFlagsA |= 1 << 4; }
	}

	// Volume is hidden if all the vertices are over a screen edge
	if ((clipFlagsA | clipFlagsB) != 0x1F)
	{
		return false;
	}

	/*
		Well, according to the simple bounding box check, it might be
		visible. Let's now test the view frustum against the bounding
		hyper cube. (Basically the reverse of the previous test!)

		This catches those cases where a diagonal hyper cube passes near a
		screen edge.
	*/

	// Subtract the camera position from the vertices. I.e. move the camera to 0,0,0
	glm::vec3 shifted[16];

	for (uint32 i = 0; i < 16; ++i)
	{
		shifted[i].x = boundingHyperCube[i].x;
		shifted[i].y = boundingHyperCube[i].y;
		shifted[i].z = boundingHyperCube[i].z - cameraZProj;
	}

	unsigned short w0, w1;
	uint32 clipFlags;
	glm::vec3 v;

	for (uint32 i = 0; i < 12; ++i)
	{
		w0 = c_linesHyperCube[2 * i + 0];
		w1 = c_linesHyperCube[2 * i + 1];

		v = glm::cross(shifted[w0], shifted[w1]);

		clipFlags = 0;

		if (glm::dot(c_rect0, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect1, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect2, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect3, v) < 0)
		{ ++clipFlags; }

		// clipFlags will be 0 or 4 if the screen edges are on the outside of this bounding-box-silhouette-edge.
		if (clipFlags % 4)
		{ continue; }

		for (uint32 j = 0; j < 8; ++j)
		{
			if ((j != w0) & (j != w1) && (glm::dot(shifted[j], v) > 0))
			{ ++clipFlags; }
		}

		// clipFlags will be 0 or 12 if this is a silhouette edge of the bounding box
		if (clipFlags % 12)
		{ continue; }

		return false;
	}

	return true;
}

static inline bool isFrontClipInVolume(const glm::vec4(&boundingHyperCube)[16])
{
	uint32 clipFlags(0);
	float32 scale, x, y, w;

	/*
		OK. The hyper-bounding-box is in the view frustum.

		Now decide if we can use Z-pass instead of Z-fail.

		TODO: if we calculate the convex hull of the front-clip intersection
		points, we can use the connecting lines to do a more accurate on-
		screen check (currently it just uses the bounding box of the
		intersection points.)
	*/

	for (uint32 i = 0; i < 32; ++i)
	{
		const glm::vec4& v0 = boundingHyperCube[c_linesHyperCube[2 * i + 0]];
		const glm::vec4& v1 = boundingHyperCube[c_linesHyperCube[2 * i + 1]];

		// If both coordinates are negative, or both coordinates are positive, it doesn't cross the Z=0 plane
		if (v0.z * v1.z > 0)
		{ continue; }

		// TODO: if fScale > 0.5f, do the lerp in the other direction; this is
		// because we want fScale to be close to 0, not 1, to retain accuracy.
		scale = (0 - v0.z) / (v1.z - v0.z);

		x = scale * v1.x + (1.0f - scale) * v0.x;
		y = scale * v1.y + (1.0f - scale) * v0.y;
		w = scale * v1.w + (1.0f - scale) * v0.w;

		if (x > -w)
		{ clipFlags |= 1 << 0; }

		if (x < w)
		{ clipFlags |= 1 << 1; }

		if (y > -w)
		{ clipFlags |= 1 << 2; }

		if (y < w)
		{ clipFlags |= 1 << 3; }
	}

	if (clipFlags == 0x0F)
	{ return true; }

	return false;
}

static inline bool isBoundingBoxVisible(const glm::vec4* const boundingHyperCube, float32 cameraZProj)
{
	glm::vec3 v, shifted[16];
	uint32 clipFlags(0);
	unsigned short		w0, w1;

	for (uint32 i = 0; i < 8; ++i)
	{
		if (boundingHyperCube[i].x <  boundingHyperCube[i].w)
		{ clipFlags |= 1 << 0; }

		if (boundingHyperCube[i].x > -boundingHyperCube[i].w)
		{ clipFlags |= 1 << 1; }

		if (boundingHyperCube[i].y <  boundingHyperCube[i].w)
		{ clipFlags |= 1 << 2; }

		if (boundingHyperCube[i].y > -boundingHyperCube[i].w)
		{ clipFlags |= 1 << 3; }

		if (boundingHyperCube[i].z > 0)
		{ clipFlags |= 1 << 4; }
	}

	// Volume is hidden if all the vertices are over a screen edge
	if (clipFlags != 0x1F)
	{ return false; }

	/*
		Well, according to the simple bounding box check, it might be
		visible. Let's now test the view frustum against the bounding
		cube. (Basically the reverse of the previous test!)

		This catches those cases where a diagonal cube passes near a
		screen edge.
	*/

	// Subtract the camera position from the vertices. I.e. move the camera to 0,0,0
	for (uint32 i = 0; i < 8; ++i)
	{
		shifted[i].x = boundingHyperCube[i].x;
		shifted[i].y = boundingHyperCube[i].y;
		shifted[i].z = boundingHyperCube[i].z - cameraZProj;
	}

	for (uint32 i = 0; i < 12; ++i)
	{
		w0 = c_linesHyperCube[2 * i + 0];
		w1 = c_linesHyperCube[2 * i + 1];

		v = glm::cross(shifted[w0], shifted[w1]);
		clipFlags = 0;

		if (glm::dot(c_rect0, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect1, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect2, v) < 0)
		{ ++clipFlags; }

		if (glm::dot(c_rect3, v) < 0)
		{ ++clipFlags; }

		// clipFlags will be 0 or 4 if the screen edges are on the outside of this bounding-box-silhouette-edge.
		if (clipFlags % 4)
		{ continue; }

		for (uint32 j = 0; j < 8; ++j)
		{
			if ((j != w0) & (j != w1) && (glm::dot(shifted[j], v) > 0))
			{ ++clipFlags; }
		}

		// clipFlags will be 0 or 12 if this is a silhouette edge of the bounding box
		if (clipFlags % 12)
		{ continue; }

		return false;
	}

	return true;
}

uint32 ShadowVolume::isVisible(const glm::mat4x4 projection, const glm::vec3& lightModel, bool isPointLight, float cameraZProj,
                               float extrudeLength)
{
	glm::vec4 boundingHyperCubeT[16];
	uint32 result(0), clipZCount(0), clipFlagsA(0);

	// Get the light z coordinate in projection space
	float lightProjZ = projection[0][2] * lightModel.x + projection[1][2] * lightModel.y + projection[2][2] * lightModel.z +
	                   projection[3][2];

	// Transform the eight bounding box points into projection space
	// Transform the 8 points
	transformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.minimum.y, m_shadowMesh.minimum.z, lightProjZ,
	               boundingHyperCubeT[0], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.minimum.y, m_shadowMesh.maximum.z, lightProjZ,
	               boundingHyperCubeT[1], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.maximum.y, m_shadowMesh.minimum.z, lightProjZ,
	               boundingHyperCubeT[2], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.maximum.y, m_shadowMesh.maximum.z, lightProjZ,
	               boundingHyperCubeT[3], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.minimum.y, m_shadowMesh.minimum.z, lightProjZ,
	               boundingHyperCubeT[4], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.minimum.y, m_shadowMesh.maximum.z, lightProjZ,
	               boundingHyperCubeT[5], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.maximum.y, m_shadowMesh.minimum.z, lightProjZ,
	               boundingHyperCubeT[6], clipZCount, clipFlagsA);
	transformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.maximum.y, m_shadowMesh.maximum.z, lightProjZ,
	               boundingHyperCubeT[7], clipZCount, clipFlagsA);

	if (clipZCount == 8 && clipFlagsA == 8)
	{
		// We're hidden
		return 0;
	}

	// Extrude the bounding box and transform into projection space
	extrudeAndTransformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.minimum.y, m_shadowMesh.minimum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[8]);
	extrudeAndTransformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.minimum.y, m_shadowMesh.maximum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[9]);
	extrudeAndTransformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.maximum.y, m_shadowMesh.minimum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[10]);
	extrudeAndTransformPoint(projection, m_shadowMesh.minimum.x, m_shadowMesh.maximum.y, m_shadowMesh.maximum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[11]);
	extrudeAndTransformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.minimum.y, m_shadowMesh.minimum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[12]);
	extrudeAndTransformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.minimum.y, m_shadowMesh.maximum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[13]);
	extrudeAndTransformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.maximum.y, m_shadowMesh.minimum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[14]);
	extrudeAndTransformPoint(projection, m_shadowMesh.maximum.x, m_shadowMesh.maximum.y, m_shadowMesh.maximum.z, lightModel,
	                         isPointLight, extrudeLength, boundingHyperCubeT[15]);

	// Check whether any part of the hyper bounding box is visible
	if (!isBoundingHyperCubeVisible(boundingHyperCubeT, cameraZProj))
	{
		// Not visible
		return 0;
	}

	// It's visible, so return the appropriate visibility flags
	result = Visible;

	if (clipZCount == 8)
	{
		if (isFrontClipInVolume(boundingHyperCubeT))
		{
			result |= Zfail;

			if (isBoundingBoxVisible(&boundingHyperCubeT[0], cameraZProj))
			{
				result |= Cap_back;
			}
		}
	}
	else
	{
		if (!(clipZCount | clipFlagsA))
		{
			// 3

			// Nothing to do
		}
		else if (isFrontClipInVolume(boundingHyperCubeT))
		{
			// 5
			result |= Zfail;

			if (isBoundingBoxVisible(&boundingHyperCubeT[0], cameraZProj))
			{
				result |= Cap_front;
			}

			if (isBoundingBoxVisible(&boundingHyperCubeT[8], cameraZProj))
			{
				result |= Cap_back;
			}
		}
	}

	return result;
}
}
//!\endcond
