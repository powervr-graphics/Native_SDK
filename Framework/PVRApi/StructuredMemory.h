#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace GpuDatatypes {
namespace Standard { enum Enum { std140 }; }

namespace BaseType { enum Enum { Integer = 0, Float = 1 }; };
namespace VectorWidth { enum Enum { Scalar = 0, Vec2 = 1, Vec3 = 2, Vec4 = 3, }; };
namespace MatrixColumns { enum Enum { OneCol = 0, Mat2x = 1, Mat3x = 2, Mat4x = 3 }; }

namespace Bits {
enum Enum
{
	Integer = 0, Float = 1,
	BitScalar = 0, BitVec2 = 2, BitVec3 = 4, BitVec4 = 6,
	BitOneCol = 0, BitMat2x = 8, BitMat3x = 16, BitMat4x = 24,
	ShiftType = 0, MaskType = 1, NotMaskType = ~MaskType,
	ShiftVec = 1, MaskVec = (3 << ShiftVec), NotMaskVec = ~MaskVec,
	ShiftCols = 3, MaskCols = (3 << ShiftCols), NotMaskCols = ~MaskCols
};
};

enum Enum
{
	integer = Bits::Integer | Bits::BitScalar | Bits::BitOneCol, uinteger = integer, boolean = integer,
	ivec2 = Bits::Integer | Bits::BitVec2 | Bits::BitOneCol, uvec2 = ivec2, bvec2 = ivec2,
	ivec3 = Bits::Integer | Bits::BitVec3 | Bits::BitOneCol, uvec3 = ivec3, bvec3 = ivec3,
	ivec4 = Bits::Integer | Bits::BitVec4 | Bits::BitOneCol,
	float32 = Bits::Float | Bits::BitScalar | Bits::BitOneCol,
	vec2 = Bits::Float | Bits::BitVec2 | Bits::BitOneCol,
	vec3 = Bits::Float | Bits::BitVec3 | Bits::BitOneCol,
	vec4 = Bits::Float | Bits::BitVec4 | Bits::BitOneCol,
	mat2x2 = Bits::Float | Bits::BitVec2 | Bits::BitMat2x,
	mat2x3 = Bits::Float | Bits::BitVec3 | Bits::BitMat2x,
	mat2x4 = Bits::Float | Bits::BitVec4 | Bits::BitMat2x,
	mat3x2 = Bits::Float | Bits::BitVec2 | Bits::BitMat3x,
	mat3x3 = Bits::Float | Bits::BitVec3 | Bits::BitMat3x,
	mat3x4 = Bits::Float | Bits::BitVec4 | Bits::BitMat3x,
	mat4x2 = Bits::Float | Bits::BitVec2 | Bits::BitMat4x,
	mat4x3 = Bits::Float | Bits::BitVec3 | Bits::BitMat4x,
	mat4x4 = Bits::Float | Bits::BitVec4 | Bits::BitMat4x,
};

inline uint32 getNumVecElements(GpuDatatypes::Enum type)
{
	return VectorWidth::Enum(((type & Bits::MaskVec) >> Bits::ShiftVec) + 1);
}
inline uint32 getNumMatrixColumns(GpuDatatypes::Enum type)
{
	return MatrixColumns::Enum(((type & Bits::MaskCols) >> Bits::ShiftCols) + 1);
}

inline uint32 getAlignment(GpuDatatypes::Enum type)
{
	uint32 vectype = type & Bits::MaskVec;
	return (vectype == Bits::BitScalar ? 4 : vectype == Bits::BitVec2 ? 8 : 16);
}

inline uint32 getVectorSelfAlignedSize(GpuDatatypes::Enum type)
{
	return getAlignment(type);
}

inline uint32 getVectorUnalignedSize(GpuDatatypes::Enum type)
{
	return 4 * getNumVecElements(type);
}

inline BaseType::Enum getBaseType(GpuDatatypes::Enum type)
{
	return BaseType::Enum(type & 1);
}

inline uint32 getSelfAlignedSize(GpuDatatypes::Enum type)
{
	return getVectorSelfAlignedSize(type) * getNumMatrixColumns(type);
}

inline uint32 getSelfAlignedArraySize(GpuDatatypes::Enum type)
{
	return std::max(getVectorSelfAlignedSize(type) * getNumMatrixColumns(type), 16u);
}

inline uint32 getUnalignedSize(GpuDatatypes::Enum type)
{
	return getVectorSelfAlignedSize(type) * (getNumMatrixColumns(type) - 1) + getVectorUnalignedSize(type);
}

inline uint32 getOffsetAfter(GpuDatatypes::Enum type, uint32 previousTotalSize)
{
	uint32 align = getAlignment(type);
	uint32 diff = previousTotalSize % align;
	diff += (align * (diff == 0)); // REMOVE BRANCHING -- equal to : if(diff==0) { diff+=8 }
	return previousTotalSize - diff + align;
}

inline uint32 getTotalSizeAfter(GpuDatatypes::Enum type, uint32 previousTotalSize)
{
	return getOffsetAfter(type, previousTotalSize) + getUnalignedSize(type);
}

}

namespace BufferViewTypes {
enum Flags
{
	UniformBuffer = 0x1,
	UniformBufferDynamic = 0x2,
	StorageBuffer = 0x4,
	StorageBufferDynamic = 0x8,
};
}

namespace utils {



class StructuredMemoryView
{
	struct StructuredMemoryTableEntry
	{
		struct IsEqual
		{
			const StringHash& hash;
			IsEqual(const StringHash& name) : hash(name) {}
			bool operator()(const StructuredMemoryTableEntry& rhs) const { return hash == rhs.name; }
		};
		StringHash name;
		GpuDatatypes::Enum entry;
		uint32 offset;
		bool operator<(const StructuredMemoryTableEntry& rhs) const { return offset < rhs.offset; }
		bool operator==(const StructuredMemoryTableEntry& rhs) const { return offset == rhs.offset; }
	};

	std::vector<StructuredMemoryTableEntry> entries;
	void* aliasedMemory;
	api::BufferView connectedBuffer;
	uint32 connectedBufferDefaultOffset;
	uint32 baseAlignedSize;
	uint32 baseUnalignedSize;
	uint32 numArrayElements;
	uint32 minDynamicAlignment;
	types::MapBufferFlags::Enum connectedBufferDefaultFlags;
	BufferViewTypes::Flags connectedBufferTypes;
	void calcAlignedSize()
	{
		baseUnalignedSize = (entries.size() ? GpuDatatypes::getTotalSizeAfter(entries.back().entry, entries.back().offset) : 0);
		//The offset of the first element is the size that the whole structure takes.
		baseAlignedSize = (entries.size() ? GpuDatatypes::getOffsetAfter(entries.front().entry, baseUnalignedSize) : 0);
		if (minDynamicAlignment)
		{
			uint32 align1 = 0;
			align1 = baseAlignedSize % minDynamicAlignment;
			if (!align1) { align1 += minDynamicAlignment; }
			baseAlignedSize += (minDynamicAlignment - align1);
		}
	}
public:
	StructuredMemoryView() :
		connectedBufferDefaultFlags(types::MapBufferFlags::Write),
		connectedBufferTypes(BufferViewTypes::Flags(0)),
		minDynamicAlignment(0), baseAlignedSize(0), baseUnalignedSize(0), numArrayElements(1), connectedBufferDefaultOffset(0)
	{
	}

	/*!*******************************************************************************************************************************
	\brief	Get the (unaligned) size of an single element
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getUnalignedElementSize() const
	{
		return baseUnalignedSize;
	}

	/*!*******************************************************************************************************************************
	\brief	Get the aligned size of an single element
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getAlignedElementSize() const { return baseAlignedSize; }

	uint32 getAlignedElementArrayOffset(uint32 array)const
	{
		assertion(array < getNumElements());
		return getAlignedElementSize() * array;
	}

	/*!*******************************************************************************************************************************
	\brief	Get the aligned size of the total element
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getAlignedTotalSize() const { return baseAlignedSize * numArrayElements; }

	/*!*******************************************************************************************************************************
	\brief	Get number elements in the array
	\return	Return number of elements in the array
	**********************************************************************************************************************************/
	uint32 getNumElements()const { return numArrayElements; }

	template<typename ParamType>
	StructuredMemoryView& setValue(const StringHash& name, const ParamType& value)
	{
		return setValue(getIndex(name), value);
	}

	template<typename ParamType>
	StructuredMemoryView& setValue(uint32 variableIndex, const ParamType& value)
	{
		return setArrayValue(variableIndex, 0, value);
	}

	template<typename ParamType>
	StructuredMemoryView& setArrayValue(const StringHash& name, uint32 arrayIndex, const ParamType& value)
	{
		return setArrayValue(getIndex(name), arrayIndex, value); return *this;
	}
	template<typename ParamType>
	StructuredMemoryView& setArrayValue(uint32 index, uint32 arrayIndex, const ParamType& value)
	{
		uint32 myoffset = entries[index].offset;
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();

		memcpy((char*)aliasedMemory + myoffset + arrayOffset, (const void*)&value, sizeof(value));
		return *this;
	}
	template<typename ParamType>
	ParamType getValue(const StringHash& name) const
	{
		ParamType value;
		return getValue<ParamType>(getIndex(name));
	}
	template<typename ParamType>
	ParamType getValue(uint32 variableIndex) const
	{
		return getArrayValue<ParamType>(variableIndex, 0);
	}

	template<typename ParamType>
	ParamType getArrayValue(const StringHash& name, uint32 arrayIndex) const
	{
		return getArrayValue<ParamType>(getIndex(name), arrayIndex);
	}

	template<typename ParamType>
	ParamType getArrayValue(uint32 variableIndex, uint32 arrayIndex) const
	{
		ParamType value;
		memcpy((void*)&value, (char*)aliasedMemory + getArrayOffset(variableIndex, arrayIndex), sizeof(value));
		return value;
	}

	const std::vector<StructuredMemoryTableEntry>& getVariableList() const { return entries; }
	std::vector<StructuredMemoryTableEntry>& getVariableList() { return entries; }

	/*!****************************************************************************************************
	\brief Add a value to the structured buffer. The value is "memcopied" as-is to the specified offset of
	the buffer. The size is calculated by the size/width of the value, as and
	*******************************************************************************************************/
	uint32 addEntryAtOffset(const StringHash& name, GpuDatatypes::Enum type, uint32 offset)
	{
		StructuredMemoryTableEntry entry;
		entry.name = name;
		entry.offset = offset;
		entry.entry = type;
		uint32 entryOffset = (uint32)insertSorted(entries, entry);
		calcAlignedSize();
		return entryOffset;
	}

	template<GpuDatatypes::Standard::Enum _standard_ = GpuDatatypes::Standard::std140>
	uint32 addEntryPacked(const StringHash& name, GpuDatatypes::Enum type)
	{

		return addEntryAtOffset(name, type, GpuDatatypes::getOffsetAfter(type, getUnalignedElementSize()));
	}

	template<GpuDatatypes::Standard::Enum _standard_ = GpuDatatypes::Standard::std140>
	void addEntriesPacked(const std::pair<StringHash, GpuDatatypes::Enum>* const entries, pvr::uint32 numEntries)
	{
		for (pvr::uint32 i = 0; i < numEntries; ++i){	addEntryPacked(entries[i].first, entries[i].second);	}
	}


	uint32 getOffset(const StringHash& name) const { return getOffset(getIndex(name)); }

	uint32 getOffset(uint32 variableIndex) const
	{
		return entries[variableIndex].offset;
	}

	uint32 getArrayOffset(uint32 variableIndex, uint32 arrayIndex) const
	{
		return entries[variableIndex].offset + getAlignedElementSize() * arrayIndex;
	}
	uint32 getArrayOffset(const StringHash& variableName, uint32 arrayIndex) const
	{
		return getArrayOffset(getIndex(variableName), arrayIndex);
	}

	uint32 getIndex(const StringHash& name) const
	{
		auto entry = std::find_if(entries.begin(), entries.end(), StructuredMemoryTableEntry::IsEqual(name));
		assertion(entry != entries.end());
		return uint32(entry - entries.begin());
	}

	void pointToMemory(void* memoryToPointTo) { aliasedMemory = memoryToPointTo; }

	void connectWithBuffer(const api::BufferView& buffer, BufferViewTypes::Flags bufferIntendedUses, types::MapBufferFlags::Enum mapDefaultFlags = types::MapBufferFlags::Write,
	                       uint32 mapDefaultOffset = 0)
	{
		connectedBuffer = buffer;
		connectedBufferDefaultFlags = mapDefaultFlags;
		connectedBufferDefaultOffset = mapDefaultOffset;
		setupArray(buffer->getContext(), numArrayElements, bufferIntendedUses);
	}

	void setupArray(const GraphicsContext& context, uint32 numElements, BufferViewTypes::Flags bufferIntendedUses)
	{
		numArrayElements = numElements;
		minDynamicAlignment = 0;
		if (bufferIntendedUses & BufferViewTypes::Flags::UniformBufferDynamic)
		{
			minDynamicAlignment = std::max<uint32>(minDynamicAlignment, context->getApiCapabilities().uboDynamicOffsetAlignment());
		}
		if (bufferIntendedUses & BufferViewTypes::Flags::StorageBufferDynamic)
		{
			minDynamicAlignment = std::max<uint32>(minDynamicAlignment, context->getApiCapabilities().ssboDynamicOffsetAlignment());
		}
		calcAlignedSize();
	}

	void setupDynamic(const GraphicsContext& context, uint32 numElements, BufferViewTypes::Flags bufferIntendedUses)
	{
		setupArray(context, numElements, bufferIntendedUses);
	}

	const api::BufferView& getConnectedBuffer() const { return connectedBuffer; }
	api::BufferView& getConnectedBuffer() { return connectedBuffer; }

	void map(types::MapBufferFlags::Enum flags = types::MapBufferFlags::Enum(0xFFFFFFFFu), uint32 offset = 0xFFFFFFFu)
	{
		mapArray(flags, 1, 0, offset);
	}

	void mapArray(types::MapBufferFlags::Enum flags = types::MapBufferFlags::Enum(0xFFFFFFFFu), uint32 arraySize = 1, uint32 arrayOffset = 0, uint32 offset = 0xFFFFFFFu)
	{
		flags = (flags == types::MapBufferFlags::Enum(0xFFFFFFFFu) ? connectedBufferDefaultFlags : flags);
		offset = (offset = 0xFFFFFFFFu ? connectedBufferDefaultOffset : offset);
		aliasedMemory = connectedBuffer->map(flags, offset + arrayOffset * getAlignedElementSize(), getAlignedElementSize() * arraySize);
	}

	void mapArrayIndex(uint32 arrayIndex, types::MapBufferFlags::Enum flags = types::MapBufferFlags::Enum(0xFFFFFFFFu))
	{
		mapMultipleArrayIndices(arrayIndex, 1, flags);
	}

	void mapMultipleArrayIndices(uint32 arrayIndex, uint32 numElementsToMap, types::MapBufferFlags::Enum flags = types::MapBufferFlags::Enum(0xFFFFFFFFu))
	{
		flags = (flags == types::MapBufferFlags::Enum(0xFFFFFFFFu) ? connectedBufferDefaultFlags : flags);
		aliasedMemory = connectedBuffer->map(flags, arrayIndex * getAlignedElementSize(), getAlignedElementSize() * numElementsToMap);
	}

	void unmap() { connectedBuffer->unmap(); aliasedMemory = NULL; }
	void* getMemoryPointer() { return aliasedMemory; }
};

}
}