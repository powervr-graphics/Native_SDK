/*!*********************************************************************************************************************
\file         PVRAssets\Model\Mesh.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods from the Mesh class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PVRAssets/Model/Mesh.h"
#include<algorithm>
using std::map;
using std::pair;
using pvr::hash;
using std::vector;

namespace pvr {
using namespace types;
namespace assets {
class SemanticLessThan
{
	inline bool operator()(const Mesh::VertexAttributeData& lhs, const Mesh::VertexAttributeData rhs) { return lhs.getSemantic() < rhs.getSemantic(); }
};

void Mesh::VertexAttributeData::setDataType(DataType type)
{
	m_layout.dataType = type;
}

void Mesh::VertexAttributeData::setOffset(uint32 offset)
{
	m_layout.offset = offset;
}

void Mesh::VertexAttributeData::setN(uint8 n)
{
	m_layout.width = n;
}

void Mesh::VertexAttributeData::setDataIndex(uint16 dataIndex)
{
	m_dataIndex = dataIndex;
}

// CFaceData
Mesh::FaceData::FaceData() : m_indexType(IndexType::IndexType16Bit)
{ }

void Mesh::FaceData::setData(const uint8* data, uint32 size, const IndexType indexType)
{
	m_indexType = indexType;
	m_data.resize(size);
	memcpy(m_data.data(), data, size);
}

int32 Mesh::addData(const byte* data, uint32 size, uint32 stride)
{
	m_data.vertexAttributeDataBlocks.push_back(StridedBuffer());
	m_data.vertexAttributeDataBlocks.back().stride = stride;;
	UCharBuffer& last_element = m_data.vertexAttributeDataBlocks.back();
	last_element.resize(size);
	if (data)
	{
		memcpy(last_element.data(), data, size);
	}
	return (uint32)m_data.vertexAttributeDataBlocks.size() - 1;
}

int32 Mesh::addData(const byte* data, uint32 size, uint32 stride, uint32 index)
{
	if (m_data.vertexAttributeDataBlocks.size() <= index)
	{
		m_data.vertexAttributeDataBlocks.resize(index + 1);
	}
	StridedBuffer& last_element = m_data.vertexAttributeDataBlocks[index];
	last_element.stride = stride;;
	last_element.resize(size);
	if (data)
	{
		memcpy(last_element.data(), data, size);
	}
	return (uint32)m_data.vertexAttributeDataBlocks.size() - 1;
}

void Mesh::setStride(uint32 index, uint32 stride)
{
	if (m_data.vertexAttributeDataBlocks.size() <= index)
	{
		m_data.vertexAttributeDataBlocks.resize(index + 1);
	}
	m_data.vertexAttributeDataBlocks[index].stride = stride;
}


void Mesh::removeData(uint32 index)
{
	// Remove element
	m_data.vertexAttributeDataBlocks.erase(m_data.vertexAttributeDataBlocks.begin() + index);

	VertexAttributeContainer::iterator walk = m_data.vertexAttributes.begin();


	// Update the indices stored by the Vertex Attributes
	for (; walk != m_data.vertexAttributes.end(); ++walk)
	{
		uint32 idx = walk->value.getDataIndex();

		if (idx > index)
		{
			walk->value.setDataIndex(idx--);
		}
		else if (idx == index)
		{
			walk->value.setDataIndex(-1);
		}
	}
}

// Should this take all the parameters for an element along with data or just pass in an already complete class? or both?
int32 Mesh::addVertexAttribute(const VertexAttributeData& element, bool forceReplace)
{
	VertexAttributeContainer::index_iterator it = m_data.vertexAttributes.indexed_find(element.getSemantic());
	if (it == m_data.vertexAttributes.indexed_end())
	{
		return (int32)m_data.vertexAttributes.insert(element.getSemantic(), element);
	}
	else
	{
		if (forceReplace)
		{
			m_data.vertexAttributes[it->first] = element;
		}
		else
		{
			return -1;
		}
		return (int32)it->second;
	}
}

int32 Mesh::addVertexAttribute(const StringHash& semanticName, const DataType& type, uint32 n, uint32 offset, uint32 dataIndex, bool forceReplace)
{
	int32 index = (int32)m_data.vertexAttributes.getIndex(semanticName);

	if (index == -1)
	{
		index = (int32)m_data.vertexAttributes.insert(semanticName, VertexAttributeData(semanticName, type, n, offset, dataIndex));
	}
	else
	{
		if (forceReplace)
		{
			m_data.vertexAttributes[index] = VertexAttributeData(semanticName, type, n, offset, dataIndex);
		}
		else
		{
			return -1;
		}
	}
	return index;
}

void Mesh::addFaces(const byte* data, uint32 size, IndexType indexType)
{
	m_data.faces.setData(data, size, indexType);

	if (size)
	{
		m_data.primitiveData.numFaces = size / (indexType == IndexType::IndexType32Bit ? 4 : 2) / 3;
	}
	else
	{
		m_data.primitiveData.numFaces = 0;
	}
}

void Mesh::removeVertexAttribute(const StringHash& semantic)
{
	m_data.vertexAttributes.erase(semantic);
}

void Mesh::removeAllVertexAttributes(void)
{
	m_data.vertexAttributes.clear();
}

uint32 Mesh::getNumFaces(uint32 batch) const
{
	if (m_data.boneBatches.getCount() == 0)
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
		assertion(batch < m_data.boneBatches.getCount());

		if (batch + 1 < m_data.boneBatches.getCount())
		{
			return m_data.boneBatches.offsets[batch + 1] - m_data.boneBatches.offsets[batch];
		}
		else
		{
			return m_data.primitiveData.numFaces - m_data.boneBatches.offsets[batch];
		}
	}
}

}
}
//!\endcond
