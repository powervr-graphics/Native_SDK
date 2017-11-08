/*!
\brief Represent a Mesh, usually an object (collection of primitives) that use the same transformation (but can be
skinned) and material.
\file PVRAssets/Model/Mesh.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {
/// <summary>Mesh class. Represent a Mesh, a collection of primitives (usually, but not necessarily, triangles)
/// together with their per-vertex information. A mesh's is a grouping where all vertices/primitives will have the
/// same basic transformation (but can then be skinned) and material applied.</summary>
class Mesh
{
public:
	/// <summary>Return the value of a Per-Mesh semantic as a FreeValue, null if it does not exist.</summary>
	/// <param name="semantic">The semantic name to retrieve</param>
	/// <returns>A pointer to a FreeValue containing the value of the semantic. If the semantic does not exist,
	/// return NULL</returns>
	const FreeValue* getMeshSemantic(const StringHash& semantic) const
	{
		auto it = _data.semantics.find(semantic);
		if (it == _data.semantics.end())
		{
			return NULL;
		}
		return &it->second;
	}

	/// <summary>Get the UserData of this mesh, if such user data exist.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	const RefCountedResource<void>& getUserDataPtr() const
	{
		return this->_data.userDataPtr;
	}
	/// <summary>Get the UserData of this mesh, if such user data exist.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	RefCountedResource<void> getUserDataPtr()
	{
		return this->_data.userDataPtr;
	}

	/// <summary>Set the UserData of this mesh (wrap the data into a RefCountedResource and cast to Ref Counted void pointer.</summary>
	/// <param name="ptr">The UserData. Must be wrapped in an appropriate RefCountedResource, and then cast into a RefCountedResource to void</param>
	void setUserDataPtr(const RefCountedResource<void>& ptr)
	{
		_data.userDataPtr = ptr;
	}
	/// <summary>Definition of a single VertexAttribute.</summary>
	class VertexAttributeData // Needs a better name
	{
	private:
		StringHash _semantic;  // Semantic for this element
		VertexAttributeLayout _layout;
		uint16_t _dataIndex;  // Getters, setters and constructors
	public:
		/// <summary>Constructor.</summary>
		VertexAttributeData() : _layout(DataType::None, static_cast<uint8_t>(0), static_cast<uint16_t>(0)), _dataIndex(static_cast<uint16_t>(-1)) { }

		/// <summary>Constructor.</summary>
		/// <param name="semantic">The semantic that this Vertex Attribute represents.</param>
		/// <param name="type">The type of the data of this attribute.</param>
		/// <param name="n">The number of values of DataType that form each attribute</param>
		/// <param name="offset">The offset from the begining of its Buffer that this attribute begins</param>
		/// <param name="dataIndex">The index of this attribute</param>
		VertexAttributeData(const StringHash& semantic, DataType type, uint8_t n, uint16_t offset, uint16_t dataIndex)
			: _semantic(semantic), _layout(type, n, offset), _dataIndex(dataIndex) { }

		/// <summary>Get the semantic of this attribute.</summary>
		/// <returns>The semantic of this attribute</returns>
		const StringHash& getSemantic() const  { return _semantic; }

		/// <summary>Get the offset of this attribute.</summary>
		/// <returns>The offset of this attribute</returns>
		uint32_t getOffset() const { return _layout.offset; }

		/// <summary>Get the layout of this attribute.</summary>
		/// <returns>The layout of this attribute</returns>
		const VertexAttributeLayout& getVertexLayout() const { return _layout; }


		/// <summary>Get number of values per vertex.</summary>
		/// <returns>The number of values (aka width. For example, a vec4 would return 4)</returns>
		uint32_t getN() const { return _layout.width; }

		/// <summary>Get the index of the data for this attribute. Would only be different to 0 if the vertex
		/// attributes were split into different data blocks. Each data block would normally map to a single VBO binding.</summary>
		/// <returns>The index of the data block for this attribute.</returns>
		int32_t getDataIndex() const { return _dataIndex; }

		/// <summary>Set the Semantic Name of this vertex attribute. A semantic name can take ANY value, but forms a "contract"
		/// with users of this class, i.e. the application must be aware of them. There are also, by convention, some semantics
		/// normally used, like "POSITION", "NORMAL", "UV0"-"UV9", "TANGENT", "BINORMAL", "BONEWEIGHT", "BONEINDEX"...</summary>
		/// <param name="semantic">The semantic name to associate to this vertex attribute.</param>
		void setSemantic(const StringHash& semantic)  { _semantic = semantic; }

		/// <summary>Set the DataType of this vertex attribute.</summary>
		/// <param name="type">The data type of this attribute</param>
		void setDataType(DataType type);

		/// <summary>Set the Offset of this vertex attribute.</summary>
		/// <param name="offset">The offset from the beginning of the data block of the first item of this attribute</param>
		void setOffset(uint32_t offset);

		/// <summary>Set the number of values of each entry of this vertex attribute.</summary>
		/// <param name="n">The number of values (width) of this attribute</param>
		void setN(uint8_t n);

		/// <summary>Set the Index of this vertex attribute.</summary>
		/// <param name="dataIndex">The data block (binding id) of this attribute</param>
		void setDataIndex(uint16_t dataIndex);

		/// <summary>Checks if the semantics of the attributes test equal. DOES NOT CHECK ACTUAL DATA</summary>
		/// <param name="rhs">The right hand side</param>
		/// <returns>True if the semantics are equal, otherwise false</returns>
		bool operator==(const VertexAttributeData& rhs) { return _semantic == rhs._semantic; }

		/// <summary>Checks if the semantics of the left attribute test "less than" the right attribute.
		/// USE FOR SORTING AND MAPS - DOES NOT CHECK ACTUAL DATA</summary>
		/// <param name="rhs">The right hand side</param>
		/// <returns>True if the semantic of lhs test less than rhs, otherwise false</returns>
		bool operator<(const VertexAttributeData& rhs) { return _semantic < rhs._semantic; }
	};

	/// <summary>The FaceData class contains the information of the Indices that defines the Faces of a Mesh.
	/// </summary>
	class FaceData
	{
		friend class Mesh;
	protected:
		IndexType _indexType; //!< The index type
		UInt8Buffer _data; //!< The data

	public:
		/// <summary> Constructor </summary>
		FaceData();

		/// <summary>Get the data type of the face data (16-bit or 32 bit integer).</summary>
		/// <returns>The data type of index data</returns>
		IndexType getDataType() const { return _indexType; }

		/// <summary>Get a pointer to the actual face data.</summary>
		/// <returns>A pointer to the actual index data</returns>
		const uint8_t* getData() const  { return _data.data(); }

		/// <summary>Get total size of the face data.</summary>
		/// <returns>The total size of the data</returns>
		uint32_t getDataSize() const { return static_cast<uint32_t>(_data.size()); }

		/// <summary>Get the size of this face data type in Bits.</summary>
		/// <returns>The size of each index, in Bits</returns>
		uint32_t getDataTypeSize() const { return _indexType == IndexType::IndexType16Bit ? 16 : 32; }

		/// <summary>Set all the data of this instance.</summary>
		/// <param name="data">Pointer to the data. Will be copied</param>
		/// <param name="size">The amount of data, in bytes, to copy from the pointer</param>
		/// <param name="indexType">The type of index data (16/32 bit)</param>
		void setData(const uint8_t* data, uint32_t size, const IndexType indexType = IndexType::IndexType16Bit);
	};

	/// <summary>This class is used to break meshes into different batches in order to avoid overflowing the number of
	/// uniforms that would otherwise be required to load all the bones into.</summary>
	struct BoneBatches
	{
		//BATCH STRIDE
		uint32_t  boneBatchStride;      //!< Is the number of bones per batch.
		std::vector<uint32_t> batches;    //!< Space for batchBoneMax bone indices, per batch
		std::vector<uint32_t> numBones;   //!< Actual number of bone indices per batch
		std::vector<uint32_t> offsets;    //!< Offset in triangle array per batch

		/// <summary>Get number of bone indices of the batches.</summary>
		/// <returns>The number of bone indices in the batches</returns>
		uint16_t getNumBones() const { return static_cast<uint16_t>(numBones.size()); }

		/// <summary>Default Constructor.</summary>
		BoneBatches() : boneBatchStride(0) { }
	};

	/// <summary>Contains mesh information.</summary>
	struct MeshInfo
	{
		uint32_t numVertices; //!< Number of vertices in this mesh
		uint32_t numFaces;    //!< Number of faces in this mesh

		std::vector<uint32_t> stripLengths; //!< If triangle strips exist, the length of each. Otherwise empty.

		uint32_t numPatchSubdivisions;      //!< Number of Patch subdivisions
		uint32_t numPatches;                //!< Number of Patches
		uint32_t numControlPointsPerPatch;  //!< Number of Control points per patch

		float units;                    //!< Scaling of the units
		PrimitiveTopology primitiveType; //!< Type of primitive in this Mesh
		bool isIndexed;                   //!< Contains indices (as opposed to being a flat list of vertices)
		bool isSkinned;                   //!< Contains indices (as opposed to being a flat list of vertices)

		/// <summary>Constructor</summary>
		MeshInfo() : numVertices(0), numFaces(0), numPatchSubdivisions(0), numPatches(0), numControlPointsPerPatch(0), units(1.0f),
			primitiveType(PrimitiveTopology::TriangleList), isIndexed(true), isSkinned(0) { }
	};

	/// <summary>This container is automatically kept sorted.</summary>
	typedef IndexedArray<VertexAttributeData, StringHash> VertexAttributeContainer;

	/// <summary>Raw internal structure of the Mesh.</summary>
	struct InternalData
	{
		ContiguousMap<StringHash, FreeValue> semantics;       //!<Container that stores semantic values.
		VertexAttributeContainer vertexAttributes;            //!<Contains information on the vertices, such as semantic names, strides etc.
		std::vector<StridedBuffer> vertexAttributeDataBlocks; //!<Contains the actual raw data (as in, the bytes of information), plus
		uint32_t numBones;          //!< Faces information

		FaceData faces;            //!< Faces information
		MeshInfo primitiveData;    //!< Primitive data information
		BoneBatches boneBatches;   //!< Faces information
		glm::mat4x4 unpackMatrix;  //!< This matrix is used to move from an int16_t representation to a float
		RefCountedResource<void> userDataPtr; //!< This is a pointer that is in complete control of the user, used for per-mesh data.
	};

private:
	InternalData _data;
	class PredicateVertAttribMinOffset
	{
	public:
		const VertexAttributeContainer& container;
		PredicateVertAttribMinOffset(const VertexAttributeContainer& container) : container(container) {}
		bool operator()(uint16_t lhs, uint16_t rhs)
		{ return container[lhs].getOffset() < container[rhs].getOffset(); }
	};


public:

	/// <summary>Set the stride of a Data block.</summary>
	/// <param name="index">The ordinal of the data block (as it was defined by the addData call). If no block exists,
	/// it will be created along with all the ones before it, as blocks are always assumed to be continuous</param>
	/// <param name="stride">The stride that the block (index) will be set to.</param>
	void setStride(uint32_t index, uint32_t stride);  // a size of 0 is supported


	/// <summary>Implicitly append a block of vertex data to the mesh and (optionally) populate it with data.</summary>
	/// <param name="data">A pointer to data that will be copied to the new block. If <paramref name="data"/>is
	/// NULL, the block remains uninitialized.</param>
	/// <param name="size">The ordinal of the data block. If no block exists, it will be created along with all the
	/// ones before it, as</param>
	/// <param name="stride">The stride that the block index will be set to.</param>
	/// <returns>The index of the block that was just created.</returns>
	/// <returns>The index of the block that was just created.</returns>
	/// <remarks>With this call, a new data block will be appended to the end of the mesh, and will be populated with
	/// (size) bytes of data copied from the (data) pointer. (stride) will be saved as metadata with the data of the
	/// block and will be queriable with the (getStride) call with the same index as the data.</remarks>
	int32_t addData(const uint8_t* data, uint32_t size, uint32_t stride);  // a size of 0 is supported

	/// <summary>Add a block of vertex data to the mesh at the specified index and (optionally) populate it with data.
	/// </summary>
	/// <param name="data">A pointer to data that will be copied to the new block. If <paramref name="data"/>is
	/// NULL, the block remains uninitialized.</param>
	/// <param name="size">The ordinal of the data block. If no block exists, it will be created along with all the
	/// ones before it, as</param>
	/// <param name="stride">The stride that the block index will be set to.</param>
	/// <param name="index">The index where this block will be created on.</param>
	/// <returns>The index of the block that was just created.</returns>
	/// <remarks>With this call, a new data block will be added to the specified index of the mesh, and will be
	/// populated with (size) bytes of data copied from the (data) pointer. (stride) will be saved as metadata with
	/// the data of the block and will be queriable with the (getStride) call with the same index as the data.
	/// </remarks>
	int32_t addData(const uint8_t* data, uint32_t size, uint32_t stride, uint32_t index);  // a size of 0 is supported

	/// <summary>Delete a block of data.</summary>
	/// <param name="index">The index of the block to delete</param>
	void removeData(uint32_t index); // Will update Vertex Attributes so they don't point at this data

	/// <summary>Remove all data blocks.</summary>
	void clearAllData()
	{
		_data.vertexAttributeDataBlocks.clear();
	}

	/// <summary>Get a pointer to the data of a specified Data block. Read only overload.</summary>
	/// <param name="index">The index of the data block</param>
	/// <returns>A const pointer to the specified data block.</returns>
	const void* getData(uint32_t index) const
	{
		return static_cast<const void*>(_data.vertexAttributeDataBlocks[index].data());
	}

	/// <summary>Get a pointer to the data of a specified Data block. Read/write overload.</summary>
	/// <param name="index">The index of the data block</param>
	/// <returns>A pointer to the specified data block.</returns>
	uint8_t* getData(uint32_t index)
	{
		return (index >= _data.vertexAttributeDataBlocks.size()) ? NULL : _data.vertexAttributeDataBlocks[index].data();
	}

	/// <summary>Get the size of the specified Data block.</summary>
	/// <param name="index">The index of the data block</param>
	/// <returns>The size in bytes of the specified Data block.</returns>
	size_t getDataSize(uint32_t index) const
	{
		return _data.vertexAttributeDataBlocks[index].size();
	}
	/// <summary>Get distance in bytes from vertex in an array to the next.</summary>
	/// <param name="index">The index of the data block whose stride to get</param>
	/// <returns>The distance in bytes from one array entry to the next.</returns>
	uint32_t getStride(uint32_t index) const
	{
		debug_assertion(index < _data.vertexAttributeDataBlocks.size(), "Stride index out of bound");
		return _data.vertexAttributeDataBlocks[index].stride;
	}

	/// <summary>Add face information to the mesh.</summary>
	/// <param name="data">A pointer to the face data</param>
	/// <param name="size">The size, in bytes, of the face data</param>
	/// <param name="indexType">The actual datatype contained in (data). (16 or 32 bit)</param>
	void addFaces(const uint8_t* data, uint32_t size, const IndexType indexType);

	/// <summary>Add a vertex attribute to the mesh.</summary>
	/// <param name="element">The vertex attribute to add</param>
	/// <param name="forceReplace">If set to true, the element will be replaced if it already exists. Otherwise, the
	/// insertion will fail.</param>
	/// <returns>The index where the element was added (or where the already existing item was)</returns>
	int32_t addVertexAttribute(const VertexAttributeData& element, bool forceReplace = false);

	/// <summary>Add a vertex attribute to the mesh.</summary>
	/// <param name="semanticName">The semantic that the vertex attribute to add represents</param>
	/// <param name="type">The DataType of the Vertex Attribute</param>
	/// <param name="width">The number of (type) values per Vertex Attribute</param>
	/// <param name="offset">The Offset of this Vertex Attribute from the start of its DataBlock</param>
	/// <param name="dataIndex">The DataBlock this Vertex Attribute belongs to</param>
	/// <param name="forceReplace">force replace the attribute</param>
	/// <returns>The index where the element was added (or where the already existing item was)</returns>
	int32_t addVertexAttribute(const StringHash& semanticName, const DataType& type, uint32_t width,
	                           uint32_t offset, uint32_t dataIndex, bool forceReplace = false);

	/// <summary>Remove a vertex attribute to the mesh.</summary>
	/// <param name="semanticName">The semantic that the vertex attribute to remove has</param>
	void removeVertexAttribute(const StringHash& semanticName);

	/// <summary>Remove all vertex attribute to the mesh.</summary>
	void removeAllVertexAttributes();

	/// <summary>Get the number of vertices that comprise this mesh.</summary>
	/// <returns>The number of vertices</returns>
	uint32_t getNumVertices() const
	{
		return _data.primitiveData.numVertices;
	}

	/// <summary>Get the number of faces that comprise this mesh.</summary>
	/// <returns>The number of faces</returns>
	uint32_t getNumFaces() const
	{
		return _data.primitiveData.numFaces;
	}

	/// <summary>Get the number of faces that comprise the designated bonebatch.</summary>
	/// <param name="boneBatch">The bone batch id</param>
	/// <returns>The number of faces</returns>
	uint32_t getNumFaces(uint32_t boneBatch) const;

	/// <summary>Get the number of indices that comprise this mesh. Takes TriangleStrips into consideration.
	/// </summary>
	/// <returns>The number of indexes</returns>
	uint32_t getNumIndices() const
	{
		return static_cast<uint32_t>(_data.primitiveData.stripLengths.size() ?
		                             _data.primitiveData.numFaces + (_data.primitiveData.stripLengths.size() * 2) :
		                             _data.primitiveData.numFaces * 3);
	}

	/// <summary>Get the number of different vertex attributes that this mesh has.</summary>
	/// <returns>The number of vertex attributes</returns>
	uint32_t getNumElements() const
	{
		return static_cast<uint32_t>(_data.vertexAttributes.size());
	}

	/// <summary>Get the number of vertex data blocks that this mesh has.</summary>
	/// <returns>The number of data blocks</returns>
	uint32_t getNumDataElements() const
	{
		return static_cast<uint32_t>(_data.vertexAttributeDataBlocks.size());
	}

	/// <summary>Get the number of BoneBatches the bones of this mesh are organised into.</summary>
	/// <returns>The number of bone batches</returns>
	uint32_t getNumBoneBatches() const
	{
		return _data.primitiveData.isSkinned ? (_data.boneBatches.getNumBones() ? _data.boneBatches.getNumBones() : 1) : 0;
	}

	/// <summary>Get the offset in the Faces data that the specified batch begins at.</summary>
	/// <param name="batch">The index of a BoneBatch</param>
	/// <returns>The offset, in bytes, in the Faces data that the specified batch begins at.</returns>
	uint32_t getBatchFaceOffset(uint32_t batch) const
	{
		return batch < _data.boneBatches.getNumBones() ? _data.boneBatches.offsets[batch] : 0;
	}

	/// <summary>Get the offset in the Faces data that the specified batch begins at.</summary>
	/// <param name="batch">The index of a BoneBatch</param>
	/// <returns>The offset, in bytes, in the Faces data that the specified batch begins at.</returns>
	uint32_t getBatchFaceOffsetBytes(uint32_t batch) const
	{
		return getBatchFaceOffset(batch) * 3 * (_data.faces.getDataType() == IndexType::IndexType16Bit ? 2 : 4);
	}

	/// <summary>Get how many bones the specified bone batch has.</summary>
	/// <param name="batch">The index of a BoneBatch</param>
	/// <returns>The number of bones in the batch with index (batch)</returns>
	uint32_t getNumBatchBones(uint32_t batch) const
	{
		return _data.boneBatches.numBones[batch];
	}

	/// <summary>Get the global index of a bone from its batch and index in the batch.</summary>
	/// <param name="batch">The index of a BoneBatch</param>
	/// <param name="bone">The index in the of a bone in the batch</param>
	/// <returns>The index of the bone</returns>
	uint32_t getBatchBone(uint32_t batch, uint32_t bone) const
	{
		return _data.boneBatches.batches[batch * _data.boneBatches.boneBatchStride + bone];
	}

	/// <summary>Get the primitive topology that the data in this Mesh represent.</summary>
	/// <returns>The primitive topology that the data in this Mesh represent (Triangles, TriangleStrips, TriangleFans,
	/// Patch etc.)</returns>
	PrimitiveTopology getPrimitiveType() const
	{
		return _data.primitiveData.primitiveType;
	}

	/// <summary>Set the primitive topology that the data in this Mesh represent.</summary>
	/// <param name="type">The primitive topology that the data in this Mesh will represent (Triangles,
	/// TriangleStrips, TriangleFans, Patch etc.)</param>
	void setPrimitiveType(const PrimitiveTopology& type)
	{
		_data.primitiveData.primitiveType = type;
	}

	/// <summary>Get information on this Mesh.</summary>
	/// <returns>A Mesh::MeshInfo object containing information on this Mesh</returns>
	const MeshInfo& getMeshInfo() const
	{
		return _data.primitiveData;
	}

	/// <summary>Get the Unpack Matrix of this Mesh. The unpack matrix is used for some exotic types of vertex
	/// position compression.</summary>
	/// <returns>The unpack matrix</returns>
	const glm::mat4x4& getUnpackMatrix() const
	{
		return _data.unpackMatrix;
	}
	/// <summary>Set the Unpack Matrix of this Mesh. The unpack matrix is used for some exotic types of vertex
	/// position compression.</summary>
	/// <param name="unpackMatrix">An unpack matrix</param>
	void setUnpackMatrix(const glm::mat4x4& unpackMatrix)
	{
		_data.unpackMatrix = unpackMatrix;
	}

	/// <summary>Get all DataBlocks of this Mesh.</summary>
	/// <returns>The datablocks, as an std::vector of StridedBuffers that additionally have a stride member.
	/// </returns>
	/// <remarks>Use as char arrays and additionally use the getStride() method to get the element stride</remarks>
	const std::vector<StridedBuffer>& getVertexData() const
	{
		return _data.vertexAttributeDataBlocks;
	}

	/// <summary>Get all face data of this mesh.</summary>
	/// <returns>A reference to the face data object of this mesh</returns>
	const FaceData& getFaces() const { return _data.faces; }

	/// <summary>Get all face data of this mesh.</summary>
	/// <returns>A reference to the face data object of this mesh</returns>
	FaceData& getFaces() { return _data.faces; }

	/// <summary>Get the information of a VertexAttribute by its SemanticName.</summary>
	/// <returns>A VertexAttributeData object with information on this attribute. (layout, index etc.) Null if
	/// failed</returns>
	/// <remarks>This method does lookup in O(logN) time. Prefer to call the getVertexAttributeID and then use the
	/// constant-time O(1) getVertexAttribute(int32_t) method</remarks>
	uint32_t getNumBones() const
	{
		return _data.numBones;
	}
	/// <summary>Get the information of a VertexAttribute by its SemanticName (return NULL if not exist)</summary>
	/// <param name="semanticName">A semantic name with which to look for a vertex attribute.</summary>
	/// <returns>A VertexAttributeData object with information on this attribute. (layout, index etc.) Null if
	/// failed</returns>
	/// <remarks>This method does lookup in O(logN) time. Prefer to call the getVertexAttributeID and then use the
	/// constant-time O(1) getVertexAttribute(int32_t) method</remarks>
	const VertexAttributeData* getVertexAttributeByName(const StringHash& semanticName) const
	{
		VertexAttributeContainer::const_index_iterator found = _data.vertexAttributes.indexed_find(semanticName);
		if (found != _data.vertexAttributes.indexed_end())
		{
			return &(_data.vertexAttributes[found->second]);
		}
		return NULL;
	}

	/// <summary>Get the Index of a VertexAttribute by its SemanticName.</summary>
	/// <param name="semanticName">A semantic name with which to look for a vertex attribute.</summary>
	/// <returns>The Index of the vertexAttribute.</returns>
	/// <remarks>Use this method to get the Index of a vertex attribute in O(logN) time and then be able to retrieve
	/// it by index with getVertexAttribute in constant time</remarks>
	int32_t getVertexAttributeIndex(const char* semanticName) const
	{
		return static_cast<int32_t>(_data.vertexAttributes.getIndex(semanticName));
	}

	/// <summary>Get the information of a VertexAttribute by its SemanticName.</summary>
	/// <param name="idx">A semantic id with which to retrieve a vertex attribute.</summary>
	/// <returns>A VertexAttributeData object with information on this attribute. (layout, data index etc.) Null if
	/// failed</returns>
	/// <remarks>This method does lookup in constant O(1) time. Use the getVertexAttributeID to get the index to use
	/// this method</remarks>
	const VertexAttributeData* getVertexAttribute(int32_t idx) const
	{
		return (static_cast<uint32_t>(idx) < _data.vertexAttributes.sizeWithDeleted() ? &_data.vertexAttributes[idx] : NULL);
	}

	/// <summary>Get number of vertex attributes.</summary>
	/// <returns>The number of vertex attributes</returns>
	uint32_t getVertexAttributesSize() const {  return static_cast<uint32_t>(_data.vertexAttributes.size()); }

	/// <summary>Locate the specified Attribute in a specific position in the vertex attribute array. Can be used to sort
	/// the vertex attributes according to a specific order.</summary>
	/// <param name="attributeName">The name of an attribute</param>
	/// <param name="userIndex">The index to put this attribute to. If another attribute is there, indices will be
	/// swapped.</param>
	void setVertexAttributeIndex(const char* attributeName, size_t userIndex)
	{
		_data.vertexAttributes.relocate(attributeName, userIndex);
	}

	/// <summary>Get all the vertex attributes.</summary>
	/// <returns>A reference to the actual container the Vertex Attributes are stored in.</returns>
	VertexAttributeContainer& getVertexAttributes()
	{
		return _data.vertexAttributes;
	}

	/// <summary>Get all the vertex attributes.</summary>
	/// <returns>A const reference to the actual container the Vertex Attributes are stored in.</returns>
	const VertexAttributeContainer& getVertexAttributes() const
	{
		return _data.vertexAttributes;
	}

	/// <summary>Get the number of Triangle Strips (if any) that comprise this Mesh.</summary>
	/// <returns>The number of Triangle Strips (if any) that comprise this Mesh. 0 if the Mesh is not made of strips
	/// </returns>
	uint32_t getNumStrips() const
	{
		return static_cast<uint32_t>(_data.primitiveData.stripLengths.size());
	}

	/// <summary>Get an array containing the Triangle Strip lengths.</summary>
	/// <returns>An array of 32 bit values representing the Triangle Strip lengths. Use getNumStrips for the length
	/// of the array.</returns>
	const uint32_t* getStripLengths() const
	{
		return _data.primitiveData.stripLengths.data();
	}

	/// <summary>Get the length of the specified triangle strip.</summary>
	/// <param name="strip">The index of the strip of which to return the length</param>
	/// <returns>The length of the TriangleStrip with index (strip)</returns>
	uint32_t getStripLength(uint32_t strip) const
	{
		return _data.primitiveData.stripLengths[strip];
	}

	/// <summary>Set the TriangleStrip number and lengths.</summary>
	/// <param name="numStrips">The number of TriangleStrips</param>
	/// <param name="lengths">An array of size numStrips containing the length of each TriangleStrip, respectively
	/// </param>
	void setStripData(uint32_t numStrips, const uint32_t* lengths)
	{
		_data.primitiveData.stripLengths.resize(numStrips);
		_data.primitiveData.stripLengths.assign(lengths, lengths + numStrips);
	}

	/// <summary>Set the total number of vertices. Will not change the actual Vertex Data.</summary>
	/// <param name="numVertices">Set the number of vertices</param>
	void setNumVertices(uint32_t numVertices)
	{
		_data.primitiveData.numVertices = numVertices;
	}

	/// <summary>Set the total number of faces. Will not change the actual Face Data.</summary>
	/// <param name="numFaces">Set the number of faces</param>
	void setNumFaces(uint32_t numFaces)
	{
		_data.primitiveData.numFaces = numFaces;
	}

	/// <summary>Get a reference to the internal representation and data of this Mesh. Handle with care.</summary>
	/// <returns>The internal representation of this object.</returns>
	InternalData& getInternalData()
	{
		return _data;
	}

	/// <summary>Merge all bone batches into one, effectively re-merging the mesh into one draw call</summary>
	/// <param name="boneIndexAttributeId">The Attribute ID of the Bone Index attribute of this mesh</param>
	void mergeBoneBatches(uint32_t boneIndexAttributeId);
};
}
}