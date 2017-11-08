/*!
\brief Implementations of methods from the Mesh class.
\file PVRAssets/Model/Mesh.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PVRAssets/Model/Mesh.h"
#include<algorithm>
using std::map;
using std::pair;
using pvr::hash;
using std::vector;

namespace pvr {
namespace assets {
class SemanticLessThan
{
	inline bool operator()(const Mesh::VertexAttributeData& lhs, const Mesh::VertexAttributeData rhs) { return lhs.getSemantic() < rhs.getSemantic(); }
};

void Mesh::VertexAttributeData::setDataType(DataType type)
{
	_layout.dataType = type;
}

void Mesh::VertexAttributeData::setOffset(uint32_t offset)
{
	_layout.offset = static_cast<uint16_t>(offset);
}

void Mesh::VertexAttributeData::setN(uint8_t n)
{
	_layout.width = n;
}

void Mesh::VertexAttributeData::setDataIndex(uint16_t dataIndex)
{
	_dataIndex = dataIndex;
}

// CFaceData
Mesh::FaceData::FaceData() : _indexType(IndexType::IndexType16Bit)
{ }

void Mesh::FaceData::setData(const uint8_t* data, uint32_t size, const IndexType indexType)
{
	_indexType = indexType;
	_data.resize(size);
	memcpy(_data.data(), data, size);
}

int32_t Mesh::addData(const uint8_t* data, uint32_t size, uint32_t stride)
{
	_data.vertexAttributeDataBlocks.push_back(StridedBuffer());
	_data.vertexAttributeDataBlocks.back().stride = static_cast<uint16_t>(stride);
	UInt8Buffer& last_element = _data.vertexAttributeDataBlocks.back();
	last_element.resize(size);
	if (data)
	{
		memcpy(last_element.data(), data, size);
	}
	return static_cast<uint32_t>(_data.vertexAttributeDataBlocks.size()) - 1;
}

int32_t Mesh::addData(const uint8_t* data, uint32_t size, uint32_t stride, uint32_t index)
{
	if (_data.vertexAttributeDataBlocks.size() <= index)
	{
		_data.vertexAttributeDataBlocks.resize(index + 1);
	}
	StridedBuffer& last_element = _data.vertexAttributeDataBlocks[index];
	last_element.stride = static_cast<uint16_t>(stride);
	last_element.resize(size);
	if (data)
	{
		memcpy(last_element.data(), data, size);
	}
	return static_cast<uint32_t>(_data.vertexAttributeDataBlocks.size()) - 1;
}

void Mesh::setStride(uint32_t index, uint32_t stride)
{
	if (_data.vertexAttributeDataBlocks.size() <= index)
	{
		_data.vertexAttributeDataBlocks.resize(index + 1);
	}
	_data.vertexAttributeDataBlocks[index].stride = static_cast<uint16_t>(stride);
}


void Mesh::removeData(uint32_t index)
{
	// Remove element
	_data.vertexAttributeDataBlocks.erase(_data.vertexAttributeDataBlocks.begin() + index);

	VertexAttributeContainer::iterator walk = _data.vertexAttributes.begin();


	// Update the indices stored by the Vertex Attributes
	for (; walk != _data.vertexAttributes.end(); ++walk)
	{
		uint32_t idx = walk->value.getDataIndex();

		if (idx > index)
		{
			walk->value.setDataIndex(static_cast<uint16_t>(idx--));
		}
		else if (idx == index)
		{
			walk->value.setDataIndex(static_cast<uint16_t>(-1));
		}
	}
}

// Should this take all the parameters for an element along with data or just pass in an already complete class? or both?
int32_t Mesh::addVertexAttribute(const VertexAttributeData& element, bool forceReplace)
{
	VertexAttributeContainer::index_iterator it = _data.vertexAttributes.indexed_find(element.getSemantic());
	if (it == _data.vertexAttributes.indexed_end())
	{
		return static_cast<int32_t>(_data.vertexAttributes.insert(element.getSemantic(), element));
	}
	else
	{
		if (forceReplace)
		{
			_data.vertexAttributes[it->first] = element;
		}
		else
		{
			return -1;
		}
		return static_cast<int32_t>(it->second);
	}
}

int32_t Mesh::addVertexAttribute(const StringHash& semanticName, const DataType& type, uint32_t n, uint32_t offset, uint32_t dataIndex, bool forceReplace)
{
	int32_t index = static_cast<int32_t>(_data.vertexAttributes.getIndex(semanticName));

	if (index == -1)
	{
		index = static_cast<int32_t>(_data.vertexAttributes.insert(semanticName, VertexAttributeData(semanticName, type, static_cast<uint8_t>(n), static_cast<uint16_t>(offset), static_cast<uint16_t>(dataIndex))));
	}
	else
	{
		if (forceReplace)
		{
			_data.vertexAttributes[index] = VertexAttributeData(semanticName, type, static_cast<uint8_t>(n), static_cast<uint16_t>(offset), static_cast<uint16_t>(dataIndex));
		}
		else
		{
			return -1;
		}
	}
	return index;
}

void Mesh::addFaces(const uint8_t* data, uint32_t size, IndexType indexType)
{
	_data.faces.setData(data, size, indexType);

	if (size)
	{
		_data.primitiveData.numFaces = size / (indexType == IndexType::IndexType32Bit ? 4 : 2) / 3;
	}
	else
	{
		_data.primitiveData.numFaces = 0;
	}
}

void Mesh::removeVertexAttribute(const StringHash& semantic)
{
	_data.vertexAttributes.erase(semantic);
}

void Mesh::removeAllVertexAttributes(void)
{
	_data.vertexAttributes.clear();
}

uint32_t Mesh::getNumFaces(uint32_t batch) const
{
	if (_data.boneBatches.getNumBones() == 0)
	{
		if (batch == 0)
		{
			return getNumFaces();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		assertion(batch < _data.boneBatches.getNumBones());

		if (batch + 1 < _data.boneBatches.getNumBones())
		{
			return _data.boneBatches.offsets[batch + 1] - _data.boneBatches.offsets[batch];
		}
		else
		{
			return _data.primitiveData.numFaces - _data.boneBatches.offsets[batch];
		}
	}
}

template<typename T>
class AddOp
{
	T valueToAdd;
	size_t width;
public:
	AddOp(T valueToAdd, size_t width) : valueToAdd(valueToAdd), width(width) {}
	void operator()(void* destination)
	{
		for (size_t i = 0; i < width; ++i)
		{
			T tmp = 0;
			memcpy(&tmp, destination, sizeof(T));
			tmp += valueToAdd;
			memcpy(destination, &tmp, sizeof(T));
			destination = static_cast<void*>(static_cast<char*>(destination) + sizeof(T));
		}
	}
};

static std::map<int, int> indices;

template<typename OP, typename IndexType>
void processByIndex(OP op, const uint8_t* indexData, size_t totalSize)
{
	const uint8_t* const initialData = indexData;
	while (indexData < initialData + totalSize)
	{
		IndexType index = *reinterpret_cast<const IndexType*>(indexData);
		if (indices.find(index) == indices.end())
		{
			indices[index] = 0;
			op(index);
		}
		indexData += (sizeof(IndexType));
	}

}

template<typename OP>
class ProcessVertexByIndex
{
	OP op;
	uint8_t* vbo;
	size_t stride;
	size_t offset;
public:
	ProcessVertexByIndex(OP op, uint8_t* vbo, size_t stride, size_t offset) : op(op), vbo(vbo), stride(stride), offset(offset) {}

	void operator()(uint32_t index)
	{
		op(vbo + (stride * index + offset));
	}
};

namespace {
struct DataCarrier
{
	uint8_t* indexData;
	uint8_t* vertexData;
	size_t vboStride;
	size_t attribOffset;
	size_t indexDataSize;
	size_t valueToAddToVertices;
};
}

template<typename IndexType, typename ValueType>
void addValueWithIndex(const DataCarrier& data, size_t width)
{
	typedef AddOp<ValueType> OP;
	typedef ProcessVertexByIndex<OP> Process;
	processByIndex<Process, IndexType>(Process(OP((ValueType)data.valueToAddToVertices, width), data.vertexData, data.vboStride, data.attribOffset), data.indexData, data.indexDataSize);
}


template<typename ValueType>
void dispatchByIndex(const DataCarrier& data, bool is16bit, size_t width)
{
	if (is16bit)
	{
		addValueWithIndex<uint16_t, ValueType>(data, width);
	}
	else
	{
		addValueWithIndex<uint32_t, ValueType>(data, width);
	}
}

void addOffsetToVertices(const DataCarrier& data, bool is16bit, DataType dataType, size_t width)
{
	switch (dataType)
	{
	case DataType::Int8: dispatchByIndex<int8_t>(data, is16bit, width); break;
	case DataType::UInt8: dispatchByIndex<uint8_t>(data, is16bit, width); break;
	case DataType::Int16: dispatchByIndex<int16_t>(data, is16bit, width); break;
	case DataType::UInt16: dispatchByIndex<uint16_t>(data, is16bit, width); break;
	case DataType::Int32: dispatchByIndex<int32_t>(data, is16bit, width); break;
	case DataType::UInt32: dispatchByIndex<uint32_t>(data, is16bit, width); break;
	case DataType::Float32: dispatchByIndex<float>(data, is16bit, width); break;
	default: assertion(0); break;
	}
}

void Mesh::mergeBoneBatches(uint32_t boneIndexAttributeId)
{
	DataCarrier data;
	if (_data.boneBatches.numBones.size() < 2) { return; }

	uint32_t numNewBones = 0;
	for (size_t i = 0; i < _data.boneBatches.numBones.size(); ++i)
	{
		numNewBones += _data.boneBatches.numBones[i];
	}

	const auto& attrib = *getVertexAttribute(boneIndexAttributeId);

	indices.clear();
	for (uint32_t i = 0; i < _data.boneBatches.numBones.size(); ++i)
	{
		data.indexData = _data.faces._data.data() + static_cast<uint32_t>(getBatchFaceOffsetBytes(i));
		data.vertexData = _data.vertexAttributeDataBlocks[attrib.getDataIndex()].data();
		data.vboStride = _data.vertexAttributeDataBlocks[attrib.getDataIndex()].stride;
		data.attribOffset = attrib.getOffset();
		if (i + 1 < _data.boneBatches.numBones.size())
		{
			data.indexDataSize = getBatchFaceOffsetBytes(i + 1) - static_cast<uint32_t>(getBatchFaceOffsetBytes(i));
		}
		else
		{
			data.indexDataSize = getFaces().getDataSize() - static_cast<uint32_t>(getBatchFaceOffsetBytes(i));
		}

		data.valueToAddToVertices = i * _data.boneBatches.boneBatchStride;

		bool is16bit = (_data.faces.getDataType() == IndexType::IndexType16Bit);

		addOffsetToVertices(data, is16bit, attrib.getVertexLayout().dataType, attrib.getVertexLayout().width);
	}
	_data.boneBatches.boneBatchStride = numNewBones;
	_data.boneBatches.numBones.resize(1);
	_data.boneBatches.numBones[0] = numNewBones;
	_data.boneBatches.offsets.resize(1);
	_data.boneBatches.offsets[0] = 0;
}
}
}
//!\endcond