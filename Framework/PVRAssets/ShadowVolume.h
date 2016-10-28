/*!*********************************************************************************************************************
\file         PVRAssets/ShadowVolume.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains an implementation of Shadow Volume generation.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/Model/Mesh.h"

namespace pvr {

/*!********************************************************************************************************************
\brief    Represents data for handling Shadow volumes of a single Mesh.
**********************************************************************************************************************/
class ShadowVolume
{
public:
	/*!**************************************************************************************************************
	\brief    Enumerates the different options for different kinds of Shadow volumes.
	****************************************************************************************************************/
	enum Flags
	{
		Visible   = 0x01,
		Cap_front = 0x02,
		Cap_back  = 0x04,
		Zfail     = 0x08
	};

public:
	/*!**************************************************************************************************************
	\brief    dtor, releases all resources held by the shadow volume.
	****************************************************************************************************************/
	~ShadowVolume();

	/*!**************************************************************************************************************
	\brief    Initialize a shadow volume from the data of a Mesh.
	\param   mesh  A mesh whose vertex data is used to initialize this ShadowVolume instance. The POSITION semantic
	         must be present in the mesh.
	\description   This method will pre-process the data in the mesh, to calculate all vertices, edges and faces
	         of the mesh as required. In effect it will extract the POSITION semantic data and the face data and
			 use it to create a "light" and cleaned up version of the mesh that will be then used to calculate
			 extruded volumes as required.
	****************************************************************************************************************/
	Result init(const assets::Mesh& mesh);

	/*!**************************************************************************************************************
	\brief    Initialize a shadow volume from raw data.
	\param   data Pointer to the first POSITION attribute of vertex data (so buffer_start + offset)
	\param   numVertices Number of vertices in (data)
	\param   verticesStride Stride between each vertex attribute
	\param   vertexType The DataType of each position coordinate
	\param   faceData Pointer to index data
	\param   numFaces Number of Faces contained in (faceData)
	\param   indexType Type of indexes in faceData (16/32 bit)
	\description   This method will pre-process the data in the mesh, to calculate all vertices, edges and faces
	         of the mesh as required. In effect it will the position data (assumed to be the first in the (data)
			 buffer, so please pre-add the offset if Position data are not the first in the buffer), and the face
			 data and use it to create a "light", cleaned up version of the mesh that will be henceforth be used to
			 calculate extruded shadow volumes as required.
	****************************************************************************************************************/
	Result init(const byte* const data, uint32 numVertices, uint32 verticesStride,
	            types::DataType vertexType, const byte* const faceData, uint32 numFaces,
	            types::IndexType indexType);


	/*!**************************************************************************************************************
	\brief    Allocate memory for a new shadow volume with the specified ID.
	\param   volumeID The ID of the volume. If exists, it will be overwritten.
	****************************************************************************************************************/
	void alllocateShadowVolume(uint32 volumeID);

	/*!**************************************************************************************************************
	\brief    Delete the Shadow Volume with the provided ID.
	****************************************************************************************************************/
	Result releaseVolume(uint32 volumeID);

	/*!**************************************************************************************************************
	\brief    Return the size of each vertex attribute in bytes. Is 2 * numVertices * stride.
	****************************************************************************************************************/
	uint32 getVertexDataSize();

	/*!**************************************************************************************************************
	\brief    Return the stride of each vertex attribute in bytes. Is 4.
	****************************************************************************************************************/
	uint32 getVertexDataStride();

	/*!**************************************************************************************************************
	\brief    Return the offset of the Position vertex attribute in bytes. Is 0.
	****************************************************************************************************************/
	uint32 getVertexDataPositionOffset();

	/*!**************************************************************************************************************
	\brief    Return the offset of each vertex attribute in bytes. Is 3.
	****************************************************************************************************************/
	uint32 getVertexDataExtrudeOffset();

	/*!**************************************************************************************************************
	\brief    Get a pointer to the raw vertex data. Use to bind vertex buffer.
	****************************************************************************************************************/
	byte*  getVertexData();

	/*!**************************************************************************************************************
	\brief    Get the size of the Index data, in bytes.
	****************************************************************************************************************/
	uint32 getIndexDataSize();

	/*!**************************************************************************************************************
	\brief    Get the stride of the Index data, in bytes. Is sizeof(IndexType).
	****************************************************************************************************************/
	uint32 getIndexDataStride();

	/*!**************************************************************************************************************
	\brief    Get the number of indexes of the specified shadow volume.
	\param[in] volumeID shadow volume id
	****************************************************************************************************************/
	uint32 getIndexCount(uint32 volumeID);

	/*!**************************************************************************************************************
	\brief    Get the indexes of the specified shadow volume.
	\param[in] volumeID shadow volume id
	****************************************************************************************************************/
	byte*  getIndices(uint32 volumeID);

	/*!**************************************************************************************************************
	\brief    Query if this shadow volume is using internal vertex data.
	\return Return true if is using internal vertex data
	****************************************************************************************************************/
	bool isVertexDataInternal();

	/*!**************************************************************************************************************
	\brief    Query if this shadow volume is using internal index data.
	\return Return true if is using internal index data
	****************************************************************************************************************/
	bool isIndexDataInternal(uint32 volumeID);

	/*!**************************************************************************************************************
	\brief    Query if this shadow volume is visible.
	****************************************************************************************************************/
	uint32 isVisible(const glm::mat4x4 projection, const glm::vec3& lightModel, bool isPointLight, float cameraZProj,
	                 float extrudeLength);

	/*!**************************************************************************************************************
	\brief Find the silhouette of the shadow volume for the specified light and prepare it for projection.
	\param volumeID The Shadow Volume to prepare. Must have had alllocateShadowVolume called on it
	\param flags   The properties of the shadow volume to generate (caps, technique)
	\param lightModel The Model-space light. Either point-light(or spot) or directional light supported
	\param isPointLight Pass true for point (or spot) light, false for directional
	\param externalIndexBuffer An external buffer that contains custom, user provided index data.
	****************************************************************************************************************/
	Result projectSilhouette(uint32 volumeID, uint32 flags, const glm::vec3& lightModel, bool isPointLight,
	                         byte** externalIndexBuffer = NULL);

private:
	void initializeVertexData(byte** externalBuffer = NULL);

	struct ShadowVolumeEdge
	{
		uint32 vertexIndices[2];
		uint32 visibilityFlags;
	};

	struct ShadowVolumeTriangle
	{
		uint32 vertexIndices[3];
		uint32 edgeIndices[3];
		glm::vec3 normal;
		int32 winding;
	};

	//preprocessed data needed to create volumes out of a mesh
	struct ShadowMesh
	{
		glm::vec3* vertices;
		ShadowVolumeEdge* edges;
		ShadowVolumeTriangle* triangles;
		glm::vec3 minimum;
		glm::vec3 maximum;
		uint32 numVertices;
		uint32 numEdges;
		uint32 numTriangles;

		byte* vertexData;
		bool needs32BitIndices;

		ShadowMesh() :
			vertices(NULL),
			edges(NULL),
			triangles(NULL),
			numVertices(0),
			numEdges(0),
			numTriangles(0),
			vertexData(NULL),
			needs32BitIndices(false)
		{
		}
	};

	//A silhouette?
	struct ShadowVolumeData
	{
		byte* indexData;
		uint32 indexCount; // If the index count is greater than 0 and indexData is NULL then the data is handled externally

		ShadowVolumeData() : indexData(NULL), indexCount(0)
		{
		}
		~ShadowVolumeData()
		{
			delete indexData;
		}
	};

	uint32 findOrCreateVertex(const glm::vec3& vertex, bool& existed);
	uint32 findOrCreateEdge(const glm::vec3& v0, const glm::vec3& v1, bool& existed);
	void findOrCreateTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

	//Extrude
	template<typename INDEXTYPE>
	Result project(uint32 volumeID, uint32 flags, const glm::vec3& lightModel, bool isPointLight,
	               INDEXTYPE** externalIndexBuffer);


	typedef std::map<uint32, ShadowVolumeData> ShadowVolumeMapType;
	ShadowMesh m_shadowMesh;
	std::map<uint32, ShadowVolumeData> m_shadowVolumes;
};
}
