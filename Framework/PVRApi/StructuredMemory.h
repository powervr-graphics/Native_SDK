/*!*********************************************************************************************************************
\file         PVRApi\StructuredMemory.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Contains utilities that allow flexible access and setting of memory into buffers and in general
			  in memory objects that are usually accessed as raw data.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"

namespace pvr {
namespace utils {

/*!****************************************************************************************************************
\brief  A structured buffer view is a class that can be used to define an explicit structure to an object that is
usually accessed as raw memory. For example, a GPU-side buffer is mapped to a void pointer, but a StructuredBufferView
can be used to create a runtime structure for it, and set its entries one by one. Normal use:
a) Create a StructuredBufferView.
b) Populate it using the function addEntryPacked(...) which adds information about the variables that will be used
c) When done, call setUpArray or setUpDynamic if it is possible to use this for DynamicUniform, DynamicStorage, or any
   other case where the entries only represent an array member and the buffer is an array of them.
d) Create or Connect to a buffer
	d1) Create a buffer using createConnectedBuffer(...) OR
	d2) Create a buffer using createBufferAsTemplate(...) and use connectWithBuffer(...) OR
	d3) Create a buffer externally and to connect it with connectWithBuffer(...) OR
e) Map the connected buffer or, if you are not using a buffer, point to the memory you wish to set
	e1) Map with map(...), mapArrayIndex(...) or mapMultipleArrayIndices(...)  OR
	e2) Use pointToMemory(...), to set a custom pointer as the destination of your set... operations
f) Set any values you wish to set using the methods: setValue(...), setArrayValue(...). The value you provide will be
   transformed if necessary (adding necessary paddings etc. if such are required), and copied onto the designaded point
   in the buffer
g) Unmap the connected buffer.
*******************************************************************************************************************/
class StructuredBufferView
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
		types::GpuDatatypes::Enum type;
		uint32 arrayElements;
		uint32 offset;
		bool operator<(const StructuredMemoryTableEntry& rhs) const { return offset < rhs.offset; }
		bool operator==(const StructuredMemoryTableEntry& rhs) const { return offset == rhs.offset; }
	};

	std::vector<StructuredMemoryTableEntry> entries;
	Multi<api::BufferView> connectedBuffer;
	void* aliasedMemory;
	uint32 mappedBufferIndex;
	uint32 connectedBufferDefaultOffset;
	uint32 baseSelfAlignedSize;
	uint32 baseUnalignedSize;
	uint32 numArrayElements;
	uint32 minDynamicAlignment;
	types::MapBufferFlags connectedBufferDefaultFlags;
	types::BufferViewTypes connectedBufferTypes;
	void calcAlignedSize()
	{
		baseUnalignedSize = (entries.size() ? types::GpuDatatypes::getTotalSizeAfter(entries.back().type, entries.back().arrayElements, entries.back().offset) : 0);
		//The offset of the first element is the size that the whole structure takes.
		baseSelfAlignedSize = (entries.size() ? types::GpuDatatypes::getOffsetAfter(entries.front().type, baseUnalignedSize) : 0);
		if (minDynamicAlignment)
		{
			uint32 align1 = 0;
			align1 = baseSelfAlignedSize % minDynamicAlignment;
			if (!align1) { align1 += minDynamicAlignment; }
			baseSelfAlignedSize += (minDynamicAlignment - align1);
		}
	}
public:
	/*!*******************************************************************************************************************************
	\brief	Ctor. Creates an empty StructuredBufferView.
	**********************************************************************************************************************************/
	StructuredBufferView() :
		aliasedMemory(0), connectedBufferDefaultOffset(0), baseSelfAlignedSize(0), baseUnalignedSize(0), numArrayElements(1), minDynamicAlignment(0),
		connectedBufferDefaultFlags(types::MapBufferFlags::Write), connectedBufferTypes(types::BufferViewTypes(0))
	{
	}

	/*!*******************************************************************************************************************************
	\return Return true if the connected buffer is a multibuffered object.
	**********************************************************************************************************************************/
	bool isMultiBuffered() { return connectedBuffer.size() > 1; }

	/*!*******************************************************************************************************************************
	\brief Set the connected buffers as a multibuffered object
	\param size The number of buffers to set
	**********************************************************************************************************************************/
	void setMultibufferSize(uint32 size) { assertion(size > 0); connectedBuffer.resize(size); }

	/*!*******************************************************************************************************************************
	\return Get the number of connected buffers.
	**********************************************************************************************************************************/
	uint32 getMultibufferSize() { return (uint32)connectedBuffer.size(); }

	/*!*******************************************************************************************************************************
	\brief	Get the (unaligned) size of an single element. "Element" here means an entire definition of the buffer, but if the buffer
	is a Dynamic buffer or an array of structures, it contains multiple "slices" of that definition. Otherwise, the element size is
	the size of the entire buffer.
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getUnalignedElementSize() const
	{
		return baseUnalignedSize;
	}

	/*!*******************************************************************************************************************************
	\brief	Get the aligned size of an single element. "Aligned" here means the space that each element of this buffer would take if it
	was a member of an array or dynamic buffer. If the buffer is not dynamic or an array, it is the same as the unaligned size. If the
	buffer is dynamic, it is the unaligned size rounded up to an integer multiple of the minimum alignment for a dynamic buffer of the
	correct type the platform allows.
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getAlignedElementSize() const { return baseSelfAlignedSize; }

	/*!*******************************************************************************************************************************
	\brief	Gets the offset of an element (a dynamic "slice" or array element) of the buffer. Same as getAlignedElementSize() * index.
	\return The offset in a buffer of an array/dynamic element.
	**********************************************************************************************************************************/
	uint32 getAlignedElementArrayOffset(uint32 index)const
	{
		return getAlignedElementSize() * index;
	}

	/*!*******************************************************************************************************************************
	\brief	Get the total size of the buffer, padded for alignement.
	\return	Return The Aligned Size
	**********************************************************************************************************************************/
	uint32 getAlignedTotalSize() const { return baseSelfAlignedSize * numArrayElements; }

	/*!*******************************************************************************************************************************
	\brief	Get number of array or dynamic buffer elements
	\return	Return number of array or dynamic buffer elements
	**********************************************************************************************************************************/
	uint32 getNumElements()const { return numArrayElements; }

#define DEFINE_SETVALUE_FOR_TYPE(ParamType)\
	/*!*******************************************************************************************************************************\
	\fn StructuredBufferView& setValue(const StringHash& name, const ParamType& value, uint32 entryArrayIndex = 0)
	\brief Set the value to the entry by name. Buffer must be mapped.
	\param name The name of the entry to set.
	\param value The value. Overloads for all supported types (as defined in pvr::types::GpuDatatypes) exist.
	\param entryArrayIndex If the value is an array, which index to set
	\return	this (allow chaining)
	**********************************************************************************************************************************/\
  StructuredBufferView& setValue(const StringHash& name, const ParamType& value, uint32 entryArrayIndex = 0)\
  {\
     return setValue(getIndex(name), value, entryArrayIndex);\
  }\
  \
	/*!*******************************************************************************************************************************\
	\fn StructuredBufferView& setValue(uint32 variableIndex, const ParamType& value, uint32 entryArrayIndex = 0)
	\brief Set the value to the entry by index. Buffer must be mapped.
	\param variableIndex The index of the entry (its order in the buffer).
	\param value The value. Overloads for all supported types (as defined in pvr::types::GpuDatatypes) exist.
	\param entryArrayIndex If the variable entry is an array, which index to set. Default 0.
	\return	this (allow chaining)
	**********************************************************************************************************************************/\
  StructuredBufferView& setValue(uint32 variableIndex, const ParamType& value, uint32 entryArrayIndex = 0)\
  {\
    return setArrayValue(variableIndex, 0, value, entryArrayIndex);\
  }\
	/*!*******************************************************************************************************************************\
	\fn StructuredBufferView& setArrayValue(const StringHash& name, const ParamType& value, uint32 entryArrayIndex = 0)
	\brief Set the value of an entry for a dynamic or array buffer, by entry name. Buffer must be mapped.
	\param name The name of the entry to set.
	\param arrayIndex The dynamic or array index of the 'slice' of the buffer to set to.
	\param value The value. Overloads for all supported types (as defined in pvr::types::GpuDatatypes) exist.
	\param entryArrayIndex If the variable entry is an array, which index to set. Default 0.
	\return	this (allow chaining)
	**********************************************************************************************************************************/\
  StructuredBufferView& setArrayValue(const StringHash& name, uint32 arrayIndex, const ParamType& value, uint32 entryArrayIndex = 0)\
  {\
    return setArrayValue(getIndex(name), arrayIndex, value, entryArrayIndex); return *this;\
  }\
  \
	/*!*******************************************************************************************************************************\
	\fn StructuredBufferView& setArrayValue(uint32 variableIndex, const ParamType& value, uint32 entryArrayIndex = 0)
	\brief Set the value of an entry for a dynamic or array buffer, by entry index. Buffer must be mapped.
	\param variableIndex The index of the entry (its order in the buffer).
	\param arrayIndex The dynamic or array index of the 'slice' of the buffer to set to.
	\param value The value. Overloads for all supported types (as defined in pvr::types::GpuDatatypes) exist.
	\param entryArrayIndex If the variable entry is an array, which index to set. Default 0.
	\return	this (allow chaining)
	**********************************************************************************************************************************/\
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const ParamType& value, uint32 entryArrayIndex = 0)\
	{\
		uint32 myoffset = getOffset(index, entryArrayIndex); \
		uint32 arrayOffset = arrayIndex * getAlignedElementSize(); \
	\
	memcpy((char*)aliasedMemory + myoffset + arrayOffset, (const void*)&value, sizeof(value)); \
	return *this; \
}



	DEFINE_SETVALUE_FOR_TYPE(float32)
	DEFINE_SETVALUE_FOR_TYPE(int32)
	DEFINE_SETVALUE_FOR_TYPE(float64)
	DEFINE_SETVALUE_FOR_TYPE(int64)
	DEFINE_SETVALUE_FOR_TYPE(glm::vec2)
	DEFINE_SETVALUE_FOR_TYPE(glm::vec3)
	DEFINE_SETVALUE_FOR_TYPE(glm::vec4)
	DEFINE_SETVALUE_FOR_TYPE(glm::ivec2)
	DEFINE_SETVALUE_FOR_TYPE(glm::ivec3)
	DEFINE_SETVALUE_FOR_TYPE(glm::ivec4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat2x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat2x4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat3x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat3x4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat4x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat4x4)


#undef DEFINE_SETVALUE_FOR_TYPE

	StructuredBufferView& setValue(const StringHash& name, const glm::mat2x3& value, uint32 entryArrayIndex = 0)
	{
		return setValue(getIndex(name), value, entryArrayIndex);
	}
	StructuredBufferView& setValue(const StringHash& name, const glm::mat3x3& value, uint32 entryArrayIndex = 0)
	{
		return setValue(getIndex(name), value, entryArrayIndex);
	}
	StructuredBufferView& setValue(const StringHash& name, const glm::mat4x3& value, uint32 entryArrayIndex = 0)
	{
		return setValue(getIndex(name), value, entryArrayIndex);
	}


	StructuredBufferView& setValue(uint32 variableIndex, const glm::mat2x3& value, uint32 entryArrayIndex = 0)
	{
		return setArrayValue(variableIndex, 0, value, entryArrayIndex);
	}
	StructuredBufferView& setValue(uint32 variableIndex, const glm::mat3x3& value, uint32 entryArrayIndex = 0)
	{
		return setArrayValue(variableIndex, 0, value, entryArrayIndex);
	}
	StructuredBufferView& setValue(uint32 variableIndex, const glm::mat4x3& value, uint32 entryArrayIndex = 0)
	{
		return setArrayValue(variableIndex, 0, value, entryArrayIndex);
	}


	StructuredBufferView& setValue(uint32 variableIndex, const FreeValue& value, uint32 entryArrayIndex = 0)
	{
		return setArrayValue(variableIndex, 0, value, entryArrayIndex);
	}

	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const glm::mat2x3& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		glm::mat2x4 newvalue(value);

		memcpy((char*)aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const glm::mat3x3& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		glm::mat3x4 newvalue(value);

		memcpy((char*)aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const glm::mat4x3& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		glm::mat4x4 newvalue(value);

		memcpy((char*)aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}

	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const FreeValue& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();

		debug_assertion(entries[index].type == value.dataType || value.dataType == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");

		if (value.dataType == types::GpuDatatypes::mat3x3)
		{
			glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>());
			memcpy((char*)aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(entries[index].type));
		}
		else
		{
			memcpy((char*)aliasedMemory + myoffset + arrayOffset, value.raw(), types::GpuDatatypes::getSize(entries[index].type));
		}

		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const TypedMem& value)
	{
		uint32 myoffset = getOffset(index);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		uint32 dataTypeOffset = types::GpuDatatypes::getSelfAlignedArraySize(value.dataType());

		debug_assertion(entries[index].type == value.dataType() || value.dataType() == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");

		assertion(value.arrayElements() == entries[index].arrayElements);

		for (uint32 i = 0; i < value.arrayElements(); ++i)
		{
			if (value.dataType() == types::GpuDatatypes::mat3x3)
			{
				glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>(i));
				memcpy((char*)aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(entries[index].type));
			}
			else
			{
				memcpy((char*)aliasedMemory + myoffset + arrayOffset, value.raw(i), types::GpuDatatypes::getSize(entries[index].type));
			}
			myoffset += dataTypeOffset;
		}

		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const TypedMem& value_as_single_value, uint32 entryArrayIndex)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();

		debug_assertion(entries[index].type == value_as_single_value.dataType() || value_as_single_value.dataType() == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");


		if (value_as_single_value.dataType() == types::GpuDatatypes::mat3x3)
		{
			glm::mat3x4 tmp(value_as_single_value.interpretValueAs<glm::mat3x3>(entryArrayIndex));
			memcpy((char*)aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(entries[index].type));
		}
		else
		{
			memcpy((char*)aliasedMemory + myoffset + arrayOffset, value_as_single_value.raw(entryArrayIndex), types::GpuDatatypes::getSize(entries[index].type));
		}

		return *this;
	}


	/*!****************************************************************************************************
	\brief Get the full variable entry list in raw format. Const overload.
	\return The full variable entry list, as a std::vector of entries.
	*******************************************************************************************************/
	const std::vector<StructuredMemoryTableEntry>& getVariableList() const { return entries; }
	/*!****************************************************************************************************
	\brief Get the full variable entry list in raw format. Non-const overload.
	\return The full variable entry list, as a std::vector of entries.
	*******************************************************************************************************/
	std::vector<StructuredMemoryTableEntry>& getVariableList() { return entries; }

	/*!****************************************************************************************************
	\brief Add a variable entry to the specified byte offset of the buffer. Order is implicit based on the offset.
	\param name The name of the new variable
	\param type The datatype of the new variable
	\param offset The byte offset (number of bytes from the beginning of the buffer) to which to add the new variable
	\param arrayElements If greater than one, the entry will be an array (type[arrayElements])
	\return The index into which the array was inserted. If it is not the last element, will change the
	indexes of other array elements.
	*******************************************************************************************************/
	uint32 addEntryAtOffset(const StringHash& name, types::GpuDatatypes::Enum type, uint32 offset, uint32 arrayElements = 1)
	{
		StructuredMemoryTableEntry entry;
		entry.name = name;
		entry.offset = offset;
		entry.type = type;
		entry.arrayElements = arrayElements;
		uint32 entryOffset = (uint32)insertSorted(entries, entry);
		calcAlignedSize();
		return entryOffset;
	}

	/*!****************************************************************************************************
	\brief Add an entry to the end of the list, packed on the minimum valid offset that the specified packing
	standard allows.
	\tparam standard The packing standard to use. Currently, only std140 is supported.
	\param name The name of the new variable
	\param type The datatype of the new variable
	\param arrayElements If greater than one, the entry will be an array (type[arrayElements])
	\return The index into which the array was inserted. Since the entry is added to the end of the list, it
	does not affect the indexes of any array elements that are already added.
	*******************************************************************************************************/
	template<types::GpuDatatypes::Standard _standard_ = types::GpuDatatypes::Standard::std140>
	uint32 addEntryPacked(const StringHash& name, types::GpuDatatypes::Enum type, uint32 arrayElements = 1)
	{
		return addEntryAtOffset(name, type, types::GpuDatatypes::getOffsetAfter(type, getUnalignedElementSize()), arrayElements);
	}

	/*!****************************************************************************************************
	\brief Add multiple entries to the end of the list, in order, each packed on the minimum valid offset
	that the specified packing standard allows.
	\tparam standard The packing standard to use. Currently, only std140 is supported.
	\param entries A c-style array of pairs of Names and Datatypes, that will be added (As if addEntryPacked
	was called for each sequentially
	\param numEntries The number of entries contained in the array 'entries'
	*******************************************************************************************************/
	template<types::GpuDatatypes::Standard _standard_ = types::GpuDatatypes::Standard::std140>
	void addEntriesPacked(const std::pair<StringHash, types::GpuDatatypes::Enum>* const entries, pvr::uint32 numEntries)
	{
		for (pvr::uint32 i = 0; i < numEntries; ++i) { addEntryPacked(entries[i].first, entries[i].second, 1); }
	}

	/*!****************************************************************************************************
	\brief Get the byte offset of the specified variable by name
	\param name The name of the entry variable
	\param entryArrayIndex If the variable is an array, which member of the array to return the offset to
	\return The byte offset of the entry with name 'name'
	*******************************************************************************************************/
	uint32 getOffset(const StringHash& name, uint32 entryArrayIndex = 0) const { return getOffset(getIndex(name), entryArrayIndex); }

	/*!****************************************************************************************************
	\brief Get the byte offset of the specified variable by entry index
	\param variableIndex The index of the entry to get the byte offset of
	\param entryArrayIndex If the variable is an array, which member of the array to return the offset to
	\return The byte offset of the entry at position 'variableIndex'
	*******************************************************************************************************/
	uint32 getOffset(uint32 variableIndex, uint32 entryArrayIndex = 0) const
	{
		return entries[variableIndex].offset + types::GpuDatatypes::getSelfAlignedArraySize(entries[variableIndex].type) * entryArrayIndex;
	}

	/*!****************************************************************************************************
	\brief For a dynamic buffer or an array of structs buffer, get the byte offset from the start of the
	buffer, of the specified variable by name, for a specified dynamic or array 'slice' of the buffer
	\param variableName The name of the variable entry to get the byte offset of
	\param dynamicIndex The index of the buffer dynamic or array slice for which to perform the calculations
	\param entryArrayIndex If the variable is an array, which member of the array to return the offset to
	\return The byte offset of the entry at position 'variableIndex' of the slice 'dynamicIndex'
	*******************************************************************************************************/
	uint32 getDynamicOffset(const StringHash& variableName, uint32 dynamicIndex, uint32 entryArrayIndex = 0) const
	{
		return getDynamicOffset(getIndex(variableName), dynamicIndex, entryArrayIndex);
	}

	/*!****************************************************************************************************
	\brief For a dynamic buffer or an array of structs buffer, get the byte offset from the start of the
	buffer, of the specified variable by entry index, for a specified dynamic or array 'slice' of the buffer
	\param variableIndex The index of the entry to get the byte offset of
	\param dynamicIndex The index of the buffer dynamic or array slice for which to perform the calculations
	\param entryArrayIndex If the variable is an array, which member of the array to return the offset to
	\return The byte offset of the entry at position 'variableIndex' of the slice 'dynamicIndex'
	*******************************************************************************************************/
	uint32 getDynamicOffset(uint32 variableIndex, uint32 dynamicIndex, uint32 entryArrayIndex = 0) const
	{
		return getOffset(variableIndex, entryArrayIndex) + getAlignedElementSize() * dynamicIndex;
	}

	/*!****************************************************************************************************
	\brief Retrieve the index of a variable entry by its name
	\param name The name of a variable entry
	\return The index of a variable entry
	*******************************************************************************************************/
	uint32 getIndex(const StringHash& name) const
	{
		auto entry = std::find_if(entries.begin(), entries.end(), StructuredMemoryTableEntry::IsEqual(name));
		assertion(entry != entries.end());
		return uint32(entry - entries.begin());
	}

	/*!****************************************************************************************************
	\brief Instead of connecting this object to an actual buffer, directly provide a pointer to some kind of
	memory. This memory will be used when the setValue(...), setArrayValue(...) methods are called
	\param memoryToPointTo A pointer to memory that is assumed to follow the structure defined by this object.
	*******************************************************************************************************/
	void pointToMemory(void* memoryToPointTo) { aliasedMemory = memoryToPointTo; }

	/*!****************************************************************************************************
	\brief Connect a buffer to this object, so that the methods map... setValue... unMap... be called directly.
	The buffer must be mappable and (obviously) be large enough to contain the data written.
	Using this function and calling map/set/unmap would be similar to calling pointToMemory(buffer->map),
	setValue, buffer->unmap, pointToMemory(null). Swap indexes can be set separately to facilitate multibuffering setups.
	\param swapIdx Set a buffer for this swap index. For non-multibuffering setups, use 0. Only one buffer will be
	set per index. Setting another buffer discards the reference to the previous one.
	\param buffer The buffer to connect to this object.
	\param bufferAllowedUses The uses that this buffer may be used at. This information is necessary because
	it affects the minimum alignment of dynamic /array slices of the buffer.
	\param mapDefaultFlags The flags to use by default when mapping. Can be overriden in the mapping command. Default is Write.
	\param mapDefaultOffset A custom offset to apply by default to any map() methods called for this buffer. Default is 0.
	*******************************************************************************************************/
	void connectWithBuffer(uint32 swapIdx, const api::BufferView& buffer, types::BufferViewTypes bufferAllowedUses,
	                       types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write,
	                       uint32 mapDefaultOffset = 0)
	{
		connectedBuffer[swapIdx] = buffer;
		connectedBufferDefaultFlags = mapDefaultFlags;
		connectedBufferDefaultOffset = mapDefaultOffset;
		setupArray(buffer->getContext(), numArrayElements, bufferAllowedUses);
	}

	/*!****************************************************************************************************
	\brief Using the structure of this object as a template, create a set of buffers suitable for its contents.
	The buffer will be created with exactly the correct size and with the flags passed. Same as calling
	createConnectedBuffers once for each number from zero to numberOfSwapIdxs-1
	\param numberOfSwapIdxs Set up multibuffering: one buffer will be created per swapIdx.
	\param ctx The graphics context to create the buffers on
	\param bufferAllowedUses The uses that this buffer may be used at. This information affects the minimum alignment
	so must be respected.
	\param allowDynamicBuffers Allow this buffer to be used as a Dynamic Uniform or Dynamic Storage. This
	information affects the minimum alignment of array slices of this buffer so must be respected.
	\param mapDefaultFlags The mapping flags used as default when calling map(). Default: Write
	*******************************************************************************************************/
	void createConnectedBuffers(uint32 numberOfSwapIdxs, GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses,
	                            bool allowDynamicBuffers,types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		connectedBufferDefaultFlags = mapDefaultFlags;
		connectedBufferDefaultOffset = 0;
		for (uint32 i = 0; i < numberOfSwapIdxs; ++i)
		{
			createConnectedBuffer(i, ctx, bufferAllowedUses, allowDynamicBuffers, mapDefaultFlags);
		}
	}

	/*!****************************************************************************************************
	\brief Using the structure of this object as a template, create a buffer suitable for its contents.
	The buffer will be created with exactly the correct size and with the flags passed. It is in all respects
	the same as calling createBufferAsTemplate and then connectWithBuffer.
	\param swapIdx The swap index to assign this buffer to (i.e. the index that will be used in mapConnectedBuffer
	to  map this buffer).
	\param ctx The graphics context to create the buffer on
	\param bufferAllowedUses The uses that this buffer may be used at. This information affects the minimum
	slice alignment so must be respected.
	\param allowDynamicBuffers Allow this buffer to be used as a Dynamic Uniform or Dynamic Storage. This
	information affects the minimum alignment of array slices of this buffer so must be respected.
	\param mapDefaultFlags The mapping flags used as default when calling map(). Default: Write
	*******************************************************************************************************/
	void createConnectedBuffer(uint32 swapIdx, GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses, bool allowDynamicBuffers,
	                           types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		connectedBufferDefaultFlags = mapDefaultFlags;
		connectedBufferDefaultOffset = 0;
		connectedBuffer[swapIdx] = createBufferAsTemplate(ctx, bufferAllowedUses, true);
		types::BufferViewTypes bindingTypes =
		  types::BufferViewTypes(
		    (uint32(bufferAllowedUses & types::BufferBindingUse::UniformBuffer) != 0 ?
		     (allowDynamicBuffers ? types::BufferViewTypes::UniformBufferDynamic : types::BufferViewTypes::UniformBuffer) : types::BufferViewTypes(0)) |
		    (uint32(bufferAllowedUses & types::BufferBindingUse::StorageBuffer) != 0 ?
		     (allowDynamicBuffers ? types::BufferViewTypes::StorageBufferDynamic : types::BufferViewTypes::StorageBuffer) : types::BufferViewTypes(0)));
		setupArray(ctx, numArrayElements, bindingTypes);
	}

	/*!****************************************************************************************************
	\brief Using this object as a template, create a buffer suitable for exactly holding the information
	represented by this object.
	\param ctx The graphics context to create the buffer on
	\param bufferAllowedUses The uses that this buffer may be used at. This information affects the minimum
	slice alignment so must be respected.
	\param mappable The created buffer can be mapped. Default true.
	*******************************************************************************************************/
	api::BufferView createBufferAsTemplate(GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses, bool mappable = true)
	{
		auto buffer = ctx->createBuffer(getAlignedTotalSize(), bufferAllowedUses, mappable);
		return ctx->createBufferView(buffer, 0, getAlignedElementSize());
	}

	/*!****************************************************************************************************
	\brief Call this function in order to set this object up properly to represent an array of elements, i.e.
	the information that has been added represents only an array member or dynamic slice of the buffer and
	not its entire contents. This function must be called for dynamic uniform/storage buffers before calling
	any of the createConnectedBuffer and similar functions as it affects the total size AND alignment of items.
	\param context The graphics context to query for alignment restrictions
	\param numElements The number of dynamic elements or array elements (i.e. the number of array members) of the buffer
	\param bufferAllowedUses The uses that any buffers that are connected objects may be used for. Will affect
	the alignment of each element of this object, so must be respected. If in doubt, add both UniformBufferDynamic
	and StorageBufferDynamic as flags in order to ensure the strictest alignment requirements are enforced.
	*******************************************************************************************************/
	void setupArray(const GraphicsContext& context, uint32 numElements, types::BufferViewTypes bufferAllowedUses)
	{
		numArrayElements = numElements;
		minDynamicAlignment = 0;
		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferViewTypes::UniformBufferDynamic) != 0)
		{
			minDynamicAlignment = std::max<uint32>(minDynamicAlignment, context->getApiCapabilities().uboDynamicOffsetAlignment());
		}
		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferViewTypes::StorageBufferDynamic) != 0)
		{
			minDynamicAlignment = std::max<uint32>(minDynamicAlignment, context->getApiCapabilities().ssboDynamicOffsetAlignment());
		}
		calcAlignedSize();
	}

	/*!****************************************************************************************************
	\brief Call this function in order to set this object up properly to represent an array of elements, i.e.
	the information that has been added represents only an array member or dynamic slice of the buffer and
	not its entire contents. This function must be called for dynamic uniform/storage buffers before calling
	any of the createConnectedBuffer and similar functions as it affects the total size AND alignment of items.
	\param context The graphics context to query for alignment restrictions
	\param numElements The number of dynamic elements or array elements (i.e. the number of array members) of the buffer
	\param bufferAllowedUses The uses that any buffers that are connected objects may be used for. Will affect
	the alignment of each element of this object, so must be respected. If in doubt, add both UniformBufferDynamic
	and StorageBufferDynamic as flags in order to ensure the strictest alignment requirements are enforced.
	*******************************************************************************************************/
	void setupDynamic(const GraphicsContext& context, uint32 numElements, types::BufferViewTypes bufferAllowedUses)
	{
		setupArray(context, numElements, bufferAllowedUses);
	}

	/*!****************************************************************************************************
	\brief Get the connected buffer for the specified swap index.
	\param swapIdx A swap index
	\returns The buffer that has been created or connected to the specified swap index
	*******************************************************************************************************/
	api::BufferView getConnectedBuffer(uint32 swapIdx) const { return connectedBuffer[swapIdx]; }

	/*!****************************************************************************************************
	\brief Get the connected buffer for the specified swap index.
	\param swapIdx A swap index
	\returns The buffer that has been created or connected to the specified swap index
	*******************************************************************************************************/
// api::BufferView& getConnectedBuffer(uint32 swapIdx) { return connectedBuffer[swapIdx]; }


	/*!****************************************************************************************************
	\brief Map the buffer connected to the specified swap index. After performing this operation, the
	setValue(...) setArrayValue(...) and similar commands become valid and actually set values on the buffer.
	\param swapIdx A swap index
	\param flags The Mapping flags to use. Default:Write.
	\param offset A custom offset, from the beginneing of this buffer, to apply to the mapping operation.
	This offset overrides the default offset defined when calling connectWithBuffer. Default: The default offset
	*******************************************************************************************************/
	void map(uint32 swapIdx, types::MapBufferFlags flags = types::MapBufferFlags::Write, uint32 offset = 0xFFFFFFFFu)
	{
		mapMultipleArrayElements(swapIdx, 0, numArrayElements, flags, offset);
	}

    /*!
       \brief Return true if it is already mapped for a given swapchain index.
       \param swapIdx
     */
	bool isMapped(uint32 swapIdx)
	{
		return connectedBuffer[swapIdx]->isMapped();
	}


	/*!****************************************************************************************************
	\brief Map multiple consecutive array/dynamic elements of the buffer connected to the specified swap index.
	After performing this operation, the setValue(...) setArrayValue(...) and similar commands become valid
	and actually set values on the buffer for that swap index.
	\param swapIdx A swap index
	\param arrayStartIndex The first element (array slice) to map
	\param numElementsToMap The number of elements to map.
	\param flags The Mapping flags to use. Default:Write.
	\param offset A custom offset, from the beginneing of this buffer, to apply to the mapping operation.
	This offset overrides the default offset defined when calling connectWithBuffer. Default: The default offset
	*******************************************************************************************************/
	void mapMultipleArrayElements(uint32 swapIdx, uint32 arrayStartIndex, uint32 numElementsToMap,
	                              types::MapBufferFlags flags = types::MapBufferFlags::Write, uint32 offset = 0xFFFFFFFFu)
	{
		flags = (flags == types::MapBufferFlags(0xFFFFFFFFu) ? connectedBufferDefaultFlags : flags);
		offset = (offset == 0xFFFFFFFFu ? connectedBufferDefaultOffset : offset);
		aliasedMemory = connectedBuffer[swapIdx]->map(flags, offset + arrayStartIndex * getAlignedElementSize(),
		                getAlignedElementSize() * numElementsToMap);
	}


	/*!****************************************************************************************************
	\brief Map multiple a single array/dynamic element of the buffer connected to the specified swap index.
	Identical as calling mapMultipleArrayElements(index, 1,...)
	After performing this operation, the setValue(...) setArrayValue(...) and similar commands become valid
	and actually set values on the buffer for this swap index.
	\param swapIdx A swap index
	\param arrayIndex The element (array slice) to map
	\param flags The Mapping flags to use. Default:Write.
	*******************************************************************************************************/
	void mapArrayIndex(uint32 swapIdx, uint32 arrayIndex, types::MapBufferFlags flags = types::MapBufferFlags::Write)
	{
		mapMultipleArrayElements(swapIdx, arrayIndex, 1, flags);
	}

	/*!****************************************************************************************************
	\brief Unmap the mapped buffer at a specified swap index. After calling this function, calling setValue()
	is no longer valid for that swap index.
	\param swapIdx A swap index
	*******************************************************************************************************/
	void unmap(uint32 swapIdx) { connectedBuffer[swapIdx]->unmap(); aliasedMemory = NULL; }

	/*!****************************************************************************************************
	\brief Returns the pointer to which any setValue...(...) operation called will be writing to. Normally
	set when performing the map...(...) operations, or directly by the user with pointToMemory(...)
	*******************************************************************************************************/
	void* getMemoryPointer() { return aliasedMemory; }
};

typedef StructuredBufferView StructuredMemoryView;

}
}
