/*!*********************************************************************************************************************
\file         PVRAssets/Model/Mesh.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Represent a Mesh, usually an object (collection of primitives) that use the same transformation (but can
			be skinned) and material.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {
/*!*********************************************************************************************************************
\brief Mesh class. Represent a Mesh, a collection of primitives (usually, but not necessarily, triangles) together with
		their per-vertex information. A mesh's is a grouping where all vertices/primitives will have the same basic
		transformation (but can then be skinned) and material applied.
***********************************************************************************************************************/
class Mesh
{
public:
	const FreeValue* getMeshSemantic(const StringHash& semantic) const
	{
		auto it = m_data.semantics.find(semantic);
		if (it == m_data.semantics.end())
		{
			return NULL;
		}
		return &it->second;
	}
	const RefCountedResource<void>& getUserDataPtr() const
	{
		return this->m_data.userDataPtr;
	}
	RefCountedResource<void> getUserDataPtr()
	{
		return this->m_data.userDataPtr;
	}
	void setUserDataPtr(const RefCountedResource<void>& ptr)
	{
		m_data.userDataPtr = ptr;
	}
	/*!*********************************************************************************************************************
	\brief	Definition of a single VertexAttribute.
	***********************************************************************************************************************/
	class VertexAttributeData // Needs a better name
	{
	private:
		StringHash m_semantic;  // Semantic for this element
		VertexAttributeLayout m_layout;
		uint16 m_dataIndex;  // Getters, setters and constructors
	public:
		/*!*********************************************************************************************************************
		\brief	Constructor.
		***********************************************************************************************************************/
		VertexAttributeData() : m_layout(types::DataType::None, (uint8)0, (uint16)0), m_dataIndex((uint16) - 1) { }

		/*!*********************************************************************************************************************
		\brief	Constructor.
		\param	semantic The semantic that this Vertex Attribute represents.
		\param	type The type of the data of this attribute.
		\param	n The number of values of DataType that form each attribute
		\param	offset The offset from the begining of its Buffer that this attribute begins
		\param	dataIndex The index of this attribute
		***********************************************************************************************************************/
		VertexAttributeData(const StringHash& semantic, types::DataType type, uint8 n, uint16 offset, uint16 dataIndex)
			: m_semantic(semantic), m_layout(type, n, offset), m_dataIndex(dataIndex) { }

		/*!*********************************************************************************************************************
		\brief	Get the semantic of this attribute.
		***********************************************************************************************************************/
		const StringHash& getSemantic() const  { return m_semantic; }

		/*!*********************************************************************************************************************
		\brief	Get the offset of this attribute.
		***********************************************************************************************************************/
		uint32 getOffset() const { return m_layout.offset; }

		/*!*********************************************************************************************************************
		\brief	Get the layout of this attribute.
		***********************************************************************************************************************/
		const VertexAttributeLayout& getVertexLayout() const { return m_layout; }


		/*!*********************************************************************************************************************
		\brief	Get number of values per vertex.
		\return	uint32
		***********************************************************************************************************************/
		uint32 getN() const { return m_layout.width; }

		/*!*********************************************************************************************************************
		\brief	Get the index of this vertex attribute.
		***********************************************************************************************************************/
		int32	getDataIndex() const { return m_dataIndex; }

		/*!*********************************************************************************************************************
		\brief	Set the Semantic Name of this vertex attribute.
		***********************************************************************************************************************/
		void setSemantic(const StringHash& semantic) 	{ m_semantic = semantic; }

		/*!*********************************************************************************************************************
		\brief	Set the DataType of this vertex attribute.
		***********************************************************************************************************************/
		void setDataType(types::DataType type);

		/*!*********************************************************************************************************************
		\brief	Set the Offset of this vertex attribute.
		***********************************************************************************************************************/
		void setOffset(uint32 offset);

		/*!*********************************************************************************************************************
		\brief	Set the number of values of each entry of this vertex attribute.
		***********************************************************************************************************************/
		void setN(uint8 n);

		/*!*********************************************************************************************************************
		\brief	Set the Index of this vertex attribute.
		***********************************************************************************************************************/
		void setDataIndex(uint16 dataIndex);

		bool operator==(const VertexAttributeData& rhs) { return m_semantic == rhs.m_semantic; }
		bool operator<(const VertexAttributeData& rhs) { return m_semantic < rhs.m_semantic; }
	};

	/*!*********************************************************************************************************************
	\brief	The FaceData class contains the information of the Indexes that defines the Faces of a Mesh.
	***********************************************************************************************************************/
	class FaceData
	{
	protected:
		types::IndexType m_indexType;
		UCharBuffer m_data;

	public:
		FaceData();

		/*!*********************************************************************************************************************
		\brief	Get the data type of the face data (16-bit or 32 bit integer).
		***********************************************************************************************************************/
		types::IndexType getDataType() const { return m_indexType; }

		/*!*********************************************************************************************************************
		\brief	Get a pointer to the actual face data.
		***********************************************************************************************************************/
		const byte* getData() const  { return m_data.data(); }

		/*!*********************************************************************************************************************
		\brief	Get total size of the face data.
		***********************************************************************************************************************/
		uint32 getDataSize() const { return (uint32)m_data.size(); }

		/*!*********************************************************************************************************************
		\brief	Get the size of this face data type in Bits.
		***********************************************************************************************************************/
		uint32 getDataTypeSize() const { return m_indexType == types::IndexType::IndexType16Bit ? 16 : 32; }

		/*!*********************************************************************************************************************
		\brief	Set the data type of this instance.
		***********************************************************************************************************************/
		void setData(const byte* data, uint32 size, const types::IndexType indexType = types::IndexType::IndexType16Bit);
	};

	/*!*********************************************************************************************************************
	\brief This class is used to break meshes into different batches in order to avoid overflowing the number of uniforms that
	       would otherwise be required to load all the bones into.
	***********************************************************************************************************************/
	struct BoneBatches
	{
		//BATCH STRIDE
		uint32  boneBatchStride;			//!< Is the number of bones per batch.
		std::vector<uint32> batches;		//!< Space for batchBoneMax bone indices, per batch
		std::vector<uint32> boneCounts;		//!< Actual number of bone indices per batch
		std::vector<uint32> offsets;		//!< Offset in triangle array per batch

		/*!*********************************************************************************************************************
		\brief	Get number of bone indices of the batches.
		***********************************************************************************************************************/
		uint16 getCount() const { return static_cast<uint16>(boneCounts.size()); }

		/*!*********************************************************************************************************************
		\brief	Default Constructor.
		***********************************************************************************************************************/
		BoneBatches() : boneBatchStride(0) { }
	};

	/*!*********************************************************************************************************************
	\brief Contains mesh information.
	***********************************************************************************************************************/
	struct MeshInfo
	{
		uint32 numVertices; //!< Number of vertices in this mesh
		uint32 numFaces;    //!< Number of faces in this mesh

		std::vector<uint32> stripLengths; //!< If triangle strips exist, the length of each. Otherwise empty.

		uint32 numPatchSubdivisions;      //!< Number of Patch subdivisions
		uint32 numPatches;                //!< Number of Patches
		uint32 numControlPointsPerPatch;  //!< Number of Control points per patch

		float32 units;                    //!< Scaling of the units
		types::PrimitiveTopology primitiveType; //!< Type of primitive in this Mesh
		bool isIndexed;                   //!< Contains indexes (as opposed to being a flat list of vertices)
		bool isSkinned;                   //!< Contains indexes (as opposed to being a flat list of vertices)

		MeshInfo() : numVertices(0), numFaces(0), numPatchSubdivisions(0), numPatches(0), numControlPointsPerPatch(0), units(1.0f),
			primitiveType(types::PrimitiveTopology::TriangleList), isIndexed(true), isSkinned(0) { }
	};

	//This container should always be kept sorted so that binary search can be done.
	typedef IndexedArray<VertexAttributeData, StringHash> VertexAttributeContainer;

	/*!************************************************************************************************************
	\brief Raw internal structure of the Mesh.
	***************************************************************************************************************/
	struct InternalData
	{
		ContiguousMap<StringHash, FreeValue> semantics;
		VertexAttributeContainer vertexAttributes;            //!<Contains information on the vertices, such as semantic names, strides etc.
		std::vector<StridedBuffer> vertexAttributeDataBlocks; //!<Contains the actual raw data (as in, the bytes of information), plus
		uint32 boneCount;          //!< Faces information

		FaceData faces;            //!< Faces information
		MeshInfo primitiveData;    //!< Primitive data information
		BoneBatches boneBatches;   //!< Faces information
		glm::mat4x4 unpackMatrix;  //!< This matrix is used to move from an int16 representation to a float
		RefCountedResource<void> userDataPtr; //!< This is a pointer that is in complete control of the user, used for per-mesh data.
	};

private:
	InternalData m_data;
	class PredicateVertAttribMinOffset
	{
	public:
		PredicateVertAttribMinOffset& operator=(const PredicateVertAttribMinOffset&);
		const VertexAttributeContainer& container;
		PredicateVertAttribMinOffset(const VertexAttributeContainer& container) : container(container) {}
		bool operator()(uint16 lhs, uint16 rhs)
		{ return container[lhs].getOffset() < container[rhs].getOffset(); }
	};


public:

	/*!*********************************************************************************************************************
	\brief	Set the stride of a Data block.
	\param index		The ordinal of the data block (as it was defined by the addData call). If no block exists, it will be created
							along with all the ones before it, as blocks are always assumed to be continuous
	\param stride	The stride that the block (index) will be set to.
	***********************************************************************************************************************/
	void setStride(uint32 index, uint32 stride);  // a size of 0 is supported


	/*!*********************************************************************************************************************
	\brief				Implicitly append a block of vertex data to the mesh and (optionally) populate it with data.
	\param  data  		A pointer to data that will be copied to the new block. If \p data is NULL, the block remains uninitialized.
	\param  size  		The ordinal of the data block. If no block exists, it will be created along with all the ones before it, as
	\param  stride 		The stride that the block index will be set to.
	\return 			The index of the block that was just created.
	\description		With this call, a new data block will be appended to the end of the mesh, and will be populated with (size)
						bytes of data copied from the (data) pointer. (stride) will be saved as metadata with the data of the block
						and will be queriable with the (getStride) call with the same index as the data.
						\return 			The index of the block that was just created.
	***********************************************************************************************************************/
	int32 addData(const byte* const data, uint32 size, uint32 stride);  // a size of 0 is supported

	/*!*********************************************************************************************************************
	\brief				Add a block of vertex data to the mesh at the specified index and (optionally) populate it with data.
	\param  data  		A pointer to data that will be copied to the new block. If \p data is NULL, the block remains uninitialized.
	\param  size  		The ordinal of the data block. If no block exists, it will be created along with all the ones before it, as
	\param  stride 		The stride that the block index will be set to.
	\param  index 		The index where this block will be created on.
	\return 			The index of the block that was just created.
	\description		With this call, a new data block will be added to the specified index of the mesh, and will be populated with
	                    (size) bytes of data copied from the (data) pointer. (stride) will be saved as metadata with the data of the
						block and will be queriable with the (getStride) call with the same index as the data.
	***********************************************************************************************************************/
	int32 addData(const byte* const data, uint32 size, uint32 stride, uint32 index);  // a size of 0 is supported

	/*!*********************************************************************************************************************
	\brief			Delete a block of data.
	\param  		index The index of the block to delete
	***********************************************************************************************************************/
	void removeData(uint32 index); // Will update Vertex Attributes so they don't point at this data

	/*!*********************************************************************************************************************
	\brief			Remove all data blocks.
	***********************************************************************************************************************/
	void clearAllData()
	{
		m_data.vertexAttributeDataBlocks.clear();
	}

	/*!*********************************************************************************************************************
	\brief			Get a pointer to the data of a specified Data block.  Read only overload.
	\return			A const pointer to the specified data block.
	***********************************************************************************************************************/
	const void* getData(uint32 index) const
	{
		return static_cast<const void*>(m_data.vertexAttributeDataBlocks[index].data());
	}

	/*!*********************************************************************************************************************
	\brief			Get a pointer to the data of a specified Data block.  Read/write overload.
	\return			A pointer to the specified data block.
	***********************************************************************************************************************/
	byte* getData(uint32 index)
	{
		return (index >= m_data.vertexAttributeDataBlocks.size()) ? NULL : m_data.vertexAttributeDataBlocks[index].data();
	}
	/*!*********************************************************************************************************************
	\brief			Get the size of the specified Data block.
	\return			The size in bytes of the specified Data block.
	***********************************************************************************************************************/
	size_t getDataSize(uint32 index) const
	{
		return m_data.vertexAttributeDataBlocks[index].size();
	}
	/*!*********************************************************************************************************************
	\brief	Get distance in bytes from vertex in an array to the next.
	\return	The distance in bytes from one array entry to the next.
	***********************************************************************************************************************/
	uint32 getStride(uint32 index) const  { return m_data.vertexAttributeDataBlocks[index].stride; }

	/*!*********************************************************************************************************************
	\brief	Add face information to the mesh.
	\param data A pointer to the face data
	\param size The size, in bytes, of the face data
	\param indexType The actual datatype contained in (data). (16 or 32 bit)
	***********************************************************************************************************************/
	void addFaces(const byte* data, uint32 size, const types::IndexType indexType);

	/*!*********************************************************************************************************************
	\brief	Add a vertex attribute to the mesh.
	\param element The vertex attribute to add
	\param forceReplace If set to true, the element will be replaced if it already exists. Otherwise, the insertion will fail.
	\return The index where the element was added (or where the already existing item was)
	***********************************************************************************************************************/
	int32 addVertexAttribute(const VertexAttributeData& element, bool forceReplace = false);

	/*!*********************************************************************************************************************
	\brief	Add a vertex attribute to the mesh.
	\param semanticName The semantic that the vertex attribute to add represents
	\param type The DataType of the Vertex Attribute
	\param width The number of (type) values per Vertex Attribute
	\param offset The Offset of this Vertex Attribute from the start of its DataBlock
	\param dataIndex The DataBlock this Vertex Attribute belongs to
	\param forceReplace force replace the attribute
	\return The index where the element was added (or where the already existing item was)
	***********************************************************************************************************************/
	int32 addVertexAttribute(const StringHash& semanticName, const types::DataType& type, uint32 width,
	                         uint32 offset, uint32 dataIndex, bool forceReplace = false);

	/*!*********************************************************************************************************************
	\brief	Remove a vertex attribute to the mesh.
	\param semanticName The semantic that the vertex attribute to remove has
	***********************************************************************************************************************/
	void removeVertexAttribute(const StringHash& semanticName);

	/*!*********************************************************************************************************************
	\brief	Remove all vertex attribute to the mesh.
	***********************************************************************************************************************/
	void removeAllVertexAttributes(void);

	/*!*********************************************************************************************************************
	\brief	Get the number of vertices that comprise this mesh.
	***********************************************************************************************************************/
	uint32 getNumVertices() const
	{
		return m_data.primitiveData.numVertices;
	}

	/*!*********************************************************************************************************************
	\brief	Get the number of faces that comprise this mesh.
	***********************************************************************************************************************/
	uint32 getNumFaces() const
	{
		return m_data.primitiveData.numFaces;
	}

	/*!*********************************************************************************************************************
	\brief	Get the number of faces that comprise the designated bonebatch.
	***********************************************************************************************************************/
	uint32 getNumFaces(uint32 boneBatch) const;

	/*!*********************************************************************************************************************
	\brief	Get the number of indexes that comprise this mesh. Takes TriangleStrips into consideration.
	***********************************************************************************************************************/
	uint32 getNumIndices() const
	{
		return (uint32)(m_data.primitiveData.stripLengths.size() ?
		                m_data.primitiveData.numFaces + (m_data.primitiveData.stripLengths.size() * 2) :
		                m_data.primitiveData.numFaces * 3);
	}

	/*!*********************************************************************************************************************
	\brief	Get the number of different vertex attributes that this mesh has.
	***********************************************************************************************************************/
	uint32 getNumElements() const
	{
		return (uint32)m_data.vertexAttributes.size();
	}

	/*!*********************************************************************************************************************
	\brief	Get the number of vertex data blocks that this mesh has.
	***********************************************************************************************************************/
	uint32 getNumDataElements() const
	{
		return (uint32)m_data.vertexAttributeDataBlocks.size();
	}

	/*!*********************************************************************************************************************
	\brief	Get the number of BoneBatches the bones of this mesh are organised into.
	***********************************************************************************************************************/
	uint32 getNumBoneBatches() const
	{
		return m_data.primitiveData.isIndexed ? (m_data.boneBatches.getCount() ? m_data.boneBatches.getCount() : 1) : 0;
	}

	/*!*********************************************************************************************************************
	\brief	Get the offset in the Faces data that the specified batch begins at.
	\param batch The index of a BoneBatch
	\return The offset, in bytes, in the Faces data that the specified batch begins at.
	***********************************************************************************************************************/
	uint32 getBatchFaceOffset(uint32 batch) const
	{
		return batch < m_data.boneBatches.getCount() ? m_data.boneBatches.offsets[batch] : 0;
	}

	/*!*********************************************************************************************************************
	\brief	Get how many bones the specified bone batch has.
	\param batch The index of a BoneBatch
	\return The number of bones in the batch with index (batch)
	***********************************************************************************************************************/
	uint32 getBatchBoneCount(uint32 batch) const
	{
		return m_data.boneBatches.boneCounts[batch];
	}

	/*!*********************************************************************************************************************
	\brief	Get the global index of a bone from its batch and index in the batch.
	\param batch The index of a BoneBatch
	\param bone The index in the of a bone in the batch
	\return The index of the bone
	***********************************************************************************************************************/
	uint32 getBatchBone(uint32 batch, uint32 bone) const
	{
		return m_data.boneBatches.batches[batch * m_data.boneBatches.boneBatchStride + bone];
	}

	/*!*********************************************************************************************************************
	\brief	Get the primitive topology that the data in this Mesh represent.
	\return The primitive topology that the data in this Mesh represent (Triangles, TriangleStrips, TriangleFans, Patch etc.)
	***********************************************************************************************************************/
	types::PrimitiveTopology getPrimitiveType() const
	{
		return m_data.primitiveData.primitiveType;
	}

	/*!*********************************************************************************************************************
	\brief	Set the primitive topology that the data in this Mesh represent.
	\param type The primitive topology that the data in this Mesh will represent (Triangles, TriangleStrips, TriangleFans, Patch etc.)
	***********************************************************************************************************************/
	void setPrimitiveType(const types::PrimitiveTopology& type)
	{
		m_data.primitiveData.primitiveType = type;
	}

	/*!*********************************************************************************************************************
	\brief	Get information on this Mesh.
	\return A Mesh::MeshInfo object containing information on this Mesh
	***********************************************************************************************************************/
	const MeshInfo& getMeshInfo() const
	{
		return m_data.primitiveData;
	}

	/*!*********************************************************************************************************************
	\brief	Get the Unpack Matrix of this Mesh. The unpack matrix is used for some exotic types of vertex position compression.
	***********************************************************************************************************************/
	const glm::mat4x4& getUnpackMatrix() const
	{
		return m_data.unpackMatrix;
	}
	/*!*********************************************************************************************************************
	\brief	Set the Unpack Matrix of this Mesh. The unpack matrix is used for some exotic types of vertex position compression.
	***********************************************************************************************************************/
	void setUnpackMatrix(const glm::mat4x4& unpackMatrix)
	{
		m_data.unpackMatrix = unpackMatrix;
	}

	/*!*********************************************************************************************************************
	\brief Get all DataBlocks of this Mesh.
	\return The datablocks, as an std::vector of StridedBuffers that additionally have a stride member.
	\description Use as byte arrays and additionally use the getStride() method to get the element stride
	***********************************************************************************************************************/
	const std::vector<StridedBuffer>& getVertexData() const
	{
		return m_data.vertexAttributeDataBlocks;
	}

	/*!****************************************************************************************************************
	\brief	Get all face data of this mesh.
	*******************************************************************************************************************/
	const FaceData& getFaces() const { return m_data.faces; }

	/*!****************************************************************************************************************
	\brief	Get all face data of this mesh.
	*******************************************************************************************************************/
	FaceData& getFaces() { return m_data.faces; }

	/*!****************************************************************************************************************
	\brief	Get the information of a VertexAttribute by its SemanticName.
	\return	A VertexAttributeData object with information on this attribute. (layout, index etc.) Null if failed
	\description This method does lookup in O(logN) time. Prefer to call the getVertexAttributeID and then use the
	constant-time O(1) getVertexAttribute(int32) method
	*******************************************************************************************************************/
	uint32 getBoneCount() const
	{
		return m_data.boneCount;
	}
	/*!****************************************************************************************************************
	\brief	Get the information of a VertexAttribute by its SemanticName.
	\return	A VertexAttributeData object with information on this attribute. (layout, index etc.) Null if failed
	\description This method does lookup in O(logN) time. Prefer to call the getVertexAttributeID and then use the
	constant-time O(1) getVertexAttribute(int32) method
	*******************************************************************************************************************/
	const VertexAttributeData* getVertexAttributeByName(const StringHash& semanticName) const
	{
		VertexAttributeContainer::const_index_iterator found = m_data.vertexAttributes.indexed_find(semanticName);
		if (found != m_data.vertexAttributes.indexed_end())
		{
			return &(m_data.vertexAttributes[found->second]);
		}
		return NULL;
	}

	/*!****************************************************************************************************************
	\brief	Get the Index of a VertexAttribute by its SemanticName.
	\return	The Index of the vertexAttribute.
	\description Use this method to get the Index of a vertex attribute in O(logN) time and then be able to retrieve it
	             by index with getVertexAttribute in constant time
	*******************************************************************************************************************/
	int32 getVertexAttributeIndex(const char8* semanticName) const
	{
		return (int32)m_data.vertexAttributes.getIndex(semanticName);
	}

	/*!****************************************************************************************************************
	\brief	Get the information of a VertexAttribute by its SemanticName.
	\return	A VertexAttributeData object with information on this attribute. (layout, data index etc.) Null if failed
	\description This method does lookup in constant O(1) time. Use the getVertexAttributeID to get the index to use
	              this method
	*******************************************************************************************************************/
	const VertexAttributeData* getVertexAttribute(int32 idx) const
	{
		return ((uint32)idx < m_data.vertexAttributes.sizeWithDeleted() ? &m_data.vertexAttributes[idx] : NULL);
	}

	/*!****************************************************************************************************************
	\brief	Get number of vertex attributes.
	*******************************************************************************************************************/
	uint32 getVertexAttributesSize() const {	return (uint32)(m_data.vertexAttributes.size()); }

	/*!****************************************************************************************************************
	\brief	Locate the specified Attribute in a specific position in the vertex attribute array. Can be used to sort the
			vertex attributes according to a specific order.
	\param attributeName The name of an attribute
	\param userIndex The index to put this attribute to. If another attribute is there, indexes will be swapped.
	*******************************************************************************************************************/
	void setVertexAttributeIndex(const char* attributeName, size_t userIndex)
	{
		m_data.vertexAttributes.relocate(attributeName, userIndex);
	}

	/*!****************************************************************************************************************
	\brief	Get all the vertex attributes.
	\return	A reference to the actual container the Vertex Attributes are stored in.
	*******************************************************************************************************************/
	VertexAttributeContainer& getVertexAttributes()
	{
		return m_data.vertexAttributes;
	}

	/*!****************************************************************************************************************
	\brief	Get all the vertex attributes.
	\return	A const reference to the actual container the Vertex Attributes are stored in.
	*******************************************************************************************************************/
	const VertexAttributeContainer& getVertexAttributes() const
	{
		return m_data.vertexAttributes;
	}

	/*!****************************************************************************************************************
	\brief	Get the number of Triangle Strips (if any) that comprise this Mesh.
	\return	The number of Triangle Strips (if any) that comprise this Mesh. 0 if the Mesh is not made of strips
	*******************************************************************************************************************/
	uint32 getNumStrips() const
	{
		return (uint32)m_data.primitiveData.stripLengths.size();
	}

	/*!****************************************************************************************************************
	\brief	Get an array containing the Triangle Strip lengths.
	\return	An array of 32 bit values representing the Triangle Strip lengths. Use getNumStrips for the length of the array.
	*******************************************************************************************************************/
	const uint32* getStripLengths() const
	{
		return m_data.primitiveData.stripLengths.data();
	}

	/*!****************************************************************************************************************
	\brief	Get the length of the specified triangle strip.
	\return	The length of the TriangleStrip with index (strip)
	*******************************************************************************************************************/
	uint32 getStripLength(uint32 strip) const
	{
		return m_data.primitiveData.stripLengths[strip];
	}

	/*!****************************************************************************************************************
	\brief	Set the TriangleStrip number and lengths.
	\param  numStrips The number of TriangleStrips
	\param  lengths An array of size numStrips containing the length of each TriangleStrip, respectively
	*******************************************************************************************************************/
	void setStripData(uint32 numStrips, const uint32* lengths)
	{
		m_data.primitiveData.stripLengths.resize(numStrips);
		m_data.primitiveData.stripLengths.assign(lengths, lengths + numStrips);
	}

	/*!****************************************************************************************************************
	\brief	Set the total number of vertices. Will not change the actual Vertex Data.
	*******************************************************************************************************************/
	void setNumVertices(uint32 numVertices)
	{
		m_data.primitiveData.numVertices = numVertices;
	}

	/*!****************************************************************************************************************
	\brief	Set the total number of faces. Will not change the actual Face Data.
	*******************************************************************************************************************/
	void setNumFaces(uint32 numFaces)
	{
		m_data.primitiveData.numFaces = numFaces;
	}

	/*!****************************************************************************************************************
	\brief	Get a reference to the internal representation and data of this Mesh. Handle with care.
	*******************************************************************************************************************/
	InternalData& getInternalData()
	{
		return m_data;
	}

};
}
}
