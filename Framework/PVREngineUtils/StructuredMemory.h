/*!
\brief Contains utilities that allow flexible access and setting of memory into buffers and in general in memory
objects that are usually accessed as raw data.
\file PVREngineUtils/StructuredMemory.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"

namespace pvr {
namespace utils {

class StructuredMemoryTableEntry
{
private:
	StringHash _name;
	types::GpuDatatypes::Enum _type;
	uint32 _arrayElementCount;
	uint32 _offset;

public:
	struct IsEqual
	{
		const StringHash& _hash;
		IsEqual(const StringHash& name) : _hash(name) {}
		bool operator()(const StructuredMemoryTableEntry& rhs) const { return _hash == rhs._name; }
	};

	bool operator<(const StructuredMemoryTableEntry& rhs) const { return _offset < rhs._offset; }
	bool operator==(const StructuredMemoryTableEntry& rhs) const { return _offset == rhs._offset; }

	StringHash getName() { return _name; }
	const StringHash getName() const { return _name; }

	types::GpuDatatypes::Enum getType() { return _type; }
	const types::GpuDatatypes::Enum getType() const { return _type; }

	uint32 getArrayElementCount() { return _arrayElementCount; }
	const uint32 getArrayElementCount() const { return _arrayElementCount; }

	uint32 getOffset() { return _offset; }
	const uint32 getOffset() const { return _offset; }

	/// <summary>Ctor.</summary>
	StructuredMemoryTableEntry(StringHash entryName, uint32 offset, types::GpuDatatypes::Enum entryType, uint32 arrayElementCount) :
		_name(entryName), _offset(offset), _type(entryType), _arrayElementCount(arrayElementCount)
	{
	}
};

/// <summary>A structured buffer view is a class that can be used to define an explicit structure to an object
/// that is usually accessed as raw memory. For example, a GPU-side buffer is mapped to a void pointer, but a
/// StructuredBufferView can be used to create a runtime structure for it, and set its entries one by one. Normal
/// use: a) Create a StructuredBufferView. b) Populate it using the function addEntryPacked(...) which adds
/// information about the variables that will be used c) When done, call setUpArray or setUpDynamic if it is
/// possible to use this for DynamicUniform, DynamicStorage, or any other case where the entries only represent an
/// array member and the buffer is an array of them. d) Create or Connect to a buffer d1) Create a buffer using
/// createConnectedBuffer(...) OR d2) Create a buffer using createBufferAsTemplate(...) and use
/// connectWithBuffer(...) OR d3) Create a buffer externally and to connect it with connectWithBuffer(...) OR e)
/// Map the connected buffer or, if you are not using a buffer, point to the memory you wish to set e1) Map with
/// map(...), mapArrayIndex(...) or mapMultipleArrayIndices(...) OR e2) Use pointToMemory(...), to set a custom
/// pointer as the destination of your set... operations f) Set any values you wish to set using the methods:
/// setValue(...), setArrayValue(...). The value you provide will be transformed if necessary (adding necessary
/// paddings etc. if such are required), and copied onto the designaded point in the buffer g) Unmap the connected
/// buffer.</summary>
class StructuredBufferView
{
private:
	std::vector<StructuredMemoryTableEntry> _entries;
	Multi<api::BufferView> _connectedBuffers;
	types::BufferBindingUse _bufferBindingUse;
	void* _aliasedMemory;
	uint32 _connectedBufferDefaultOffset;
	uint32 _baseSelfAlignedSize;
	uint32 _baseUnalignedSize;
	uint32 _elementCount;
	uint32 _minUboDynamicAlignment;
	uint32 _minSsboDynamicAlignment;
	bool _finalized;
	types::MapBufferFlags _connectedBufferDefaultFlags;

	uint32 calculateBufferDynamicAlignment(types::BufferBindingUse bufferAllowedUses)
	{
		uint32 minDynamicAlignment = 0;

		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::UniformBuffer) != 0 && _minUboDynamicAlignment)
		{
			minDynamicAlignment += _minUboDynamicAlignment;
		}
		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::StorageBuffer) != 0 && _minSsboDynamicAlignment)
		{
			minDynamicAlignment += _minSsboDynamicAlignment;
		}

		return minDynamicAlignment;
	}

	void calculateStructureSizes(uint32 minDynamicAlignment)
	{
		StructuredMemoryTableEntry& front = _entries.front();
		StructuredMemoryTableEntry& back = _entries.back();
		_baseUnalignedSize = (_entries.size() ? types::GpuDatatypes::getTotalSizeAfter(back.getType(), back.getArrayElementCount(), back.getOffset()) : 0);
		//The offset of the first element is the size that the whole structure takes.
		_baseSelfAlignedSize = (_entries.size() ? types::GpuDatatypes::getOffsetAfter(front.getType(), _baseUnalignedSize) : 0);

		if (minDynamicAlignment)
		{
			uint32 align1 = 0;
			align1 = _baseSelfAlignedSize % minDynamicAlignment;
			if (!align1) { align1 += minDynamicAlignment; }
			_baseSelfAlignedSize += (minDynamicAlignment - align1);
		}
	}
public:
	/// <summary>Ctor. Creates an empty StructuredBufferView.</summary>
	StructuredBufferView() :
		_aliasedMemory(0), _connectedBufferDefaultOffset(0), _baseSelfAlignedSize(0), _baseUnalignedSize(0),
		_elementCount(0), _minUboDynamicAlignment(0), _minSsboDynamicAlignment(0), _connectedBufferDefaultFlags(types::MapBufferFlags::Write), _finalized(false)
	{
	}

	/// <summary>Check if the connected buffer is a multibuffered object.</summary>
	/// <returns>Return true if the connected buffer is a multibuffered object.</returns>
	bool isMultiBuffered() { return _connectedBuffers.size() > 1; }

	/// <summary>Set the connected buffers as a multibuffered object</summary>
	/// <param name="size">The number of buffers to set</param>
	void setMultibufferCount(uint32 size)
	{
		assertion(!isFinalized(), "Structured memory view must not be finalized.");
		assertion(size > 0);
		_connectedBuffers.resize(size);
	}

	/// <summary>Get the number of connected buffers.</summary>
	/// <returns>The number of connected buffers.</returns>
	uint32 getMultibufferCount() { return (uint32)_connectedBuffers.size(); }

	/// <summary>Get the (unaligned) size of an single element. "Element" here means an entire definition of the
	/// buffer, but if the buffer is a Dynamic buffer or an array of structures, it contains multiple "slices" of that
	/// definition. Otherwise, the element size is the size of the entire buffer.</summary>
	/// <returns>Return The Aligned Size</returns>
	uint32 getUnalignedElementSize() const
	{
		return _baseUnalignedSize;
	}

	/// <summary>Get the aligned size of an single element. "Aligned" here means the space that each element of this
	/// buffer would take if it was a member of an array or dynamic buffer. If the buffer is not dynamic or an array,
	/// it is the same as the unaligned size. If the buffer is dynamic, it is the unaligned size rounded up to an
	/// integer multiple of the minimum alignment for a dynamic buffer of the correct type the platform allows.
	/// </summary>
	/// <returns>Return The Aligned Size</returns>
	uint32 getAlignedElementSize() const
	{
		return _baseSelfAlignedSize;
	}

	/// <summary>Gets the offset of an element (a dynamic "slice" or array element) of the buffer. Same as
	/// getAlignedElementSize() * index.</summary>
	/// <returns>The offset in a buffer of an array/dynamic element.</returns>
	uint32 getAlignedElementArrayOffset(uint32 index) const
	{
		return getAlignedElementSize() * index;
	}

	/// <summary>Get the total size of the buffer, padded for alignement.</summary>
	/// <returns>Return The Aligned Size</returns>
	uint32 getAlignedTotalSize() const
	{
		return _baseSelfAlignedSize * _elementCount;
	}

	/// <summary>Get number of array or dynamic buffer elements</summary>
	/// <returns>Return number of array or dynamic buffer elements</returns>
	uint32 getElementCount()const { return _elementCount; }

#define DEFINE_SETVALUE_FOR_TYPE(ParamType)\
	/*!*******************************************************************************************************************************\
	\fn StructuredBufferView& setValue(const StringHash& name, const ParamType& value, uint32 entryArrayIndex = 0)
	\brief Set the value to the entry by name. Buffer must be mapped.
	\param name The name of the entry to set.
	\param value The value. Overloads for all supported types (as defined in pvr::types::GpuDatatypes) exist.
	\param entryArrayIndex If the value is an array, which index to set
	\return this (allow chaining)
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
	\return this (allow chaining)
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
	\return this (allow chaining)
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
	\return this (allow chaining)
	**********************************************************************************************************************************/\
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const ParamType& value, uint32 entryArrayIndex = 0)\
	{\
		uint32 myoffset = getOffset(index, entryArrayIndex); \
		uint32 arrayOffset = arrayIndex * getAlignedElementSize(); \
		\
		memcpy((char*)_aliasedMemory + myoffset + arrayOffset, (const void*)&value, sizeof(value)); \
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

		memcpy((char*)_aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const glm::mat3x3& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		glm::mat3x4 newvalue(value);

		memcpy((char*)_aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const glm::mat4x3& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		glm::mat4x4 newvalue(value);

		memcpy((char*)_aliasedMemory + myoffset + arrayOffset, (const void*)&newvalue, sizeof(newvalue));
		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const FreeValue& value, uint32 entryArrayIndex = 0)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();

		debug_assertion(_entries[index].getType() == value.dataType() || value.dataType() == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");

		if (value.dataType() == types::GpuDatatypes::mat3x3)
		{
			glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>());
			memcpy((char*)_aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(_entries[index].getType()));
		}
		else
		{
			memcpy((char*)_aliasedMemory + myoffset + arrayOffset, value.raw(), types::GpuDatatypes::getSize(_entries[index].getType()));
		}

		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const TypedMem& value)
	{
		uint32 myoffset = getOffset(index);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();
		uint32 dataTypeOffset = types::GpuDatatypes::getSelfAlignedArraySize(value.dataType());

		debug_assertion(_entries[index].getType() == value.dataType() || value.dataType() == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");

		assertion(value.arrayElements() == _entries[index].getArrayElementCount());

		for (uint32 i = 0; i < value.arrayElements(); ++i)
		{
			if (value.dataType() == types::GpuDatatypes::mat3x3)
			{
				glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>(i));
				memcpy((char*)_aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(_entries[index].getType()));
			}
			else
			{
				memcpy((char*)_aliasedMemory + myoffset + arrayOffset, value.raw(i), types::GpuDatatypes::getSize(_entries[index].getType()));
			}
			myoffset += dataTypeOffset;
		}

		return *this;
	}
	StructuredBufferView& setArrayValue(uint32 index, uint32 arrayIndex, const TypedMem& value_as_single_value, uint32 entryArrayIndex)
	{
		uint32 myoffset = getOffset(index, entryArrayIndex);
		uint32 arrayOffset = arrayIndex * getAlignedElementSize();

		debug_assertion(_entries[index].getType() == value_as_single_value.dataType() || value_as_single_value.dataType() == types::GpuDatatypes::mat3x3, "StructuredBufferView: Mismatched FreeValue datatype");

		if (value_as_single_value.dataType() == types::GpuDatatypes::mat3x3)
		{
			glm::mat3x4 tmp(value_as_single_value.interpretValueAs<glm::mat3x3>(entryArrayIndex));
			memcpy((char*)_aliasedMemory + myoffset + arrayOffset, &tmp[0][0], types::GpuDatatypes::getSize(_entries[index].getType()));
		}
		else
		{
			memcpy((char*)_aliasedMemory + myoffset + arrayOffset, value_as_single_value.raw(entryArrayIndex), types::GpuDatatypes::getSize(_entries[index].getType()));
		}

		return *this;
	}

	/// <summary>Get the full variable entry list in raw format. Const overload.</summary>
	/// <returns>The full variable entry list, as a std::vector of entries.</returns>
	const std::vector<StructuredMemoryTableEntry>& getVariableList() const { return _entries; }
	/// <summary>Get the full variable entry list in raw format. Non-const overload.</summary>
	/// <returns>The full variable entry list, as a std::vector of entries.</returns>
	std::vector<StructuredMemoryTableEntry>& getVariableList() { return _entries; }

	/// <summary>Add a variable entry to the specified byte offset of the buffer. Order is implicit based on the
	/// offset.</summary>
	/// <param name="name">The name of the new variable</param>
	/// <param name="type">The datatype of the new variable</param>
	/// <param name="offset">The byte offset (number of bytes from the beginning of the buffer) to which to add the
	/// new variable</param>
	/// <param name="arrayElements">If greater than one, the entry will be an array (type[arrayElements])</param>
	/// <returns>The index into which the array was inserted. If it is not the last element, will change the indexes
	/// of other array elements.</returns>
	uint32 addEntryAtOffset(const StringHash& name, types::GpuDatatypes::Enum type, uint32 offset, uint32 arrayElements = 1)
	{
		assertion(!isFinalized(), "Structured memory view must not be finalized.");
		assertion(_connectedBuffers[0].isNull(), "StructuredBufferView: Attempting to add entries to the object, but buffers have "
		          "already been connected. This is invalid, because it would cause future connected buffers to have the wrong sizes.");

		StructuredMemoryTableEntry entry(name, offset, type, arrayElements);
		uint32 entryOffset = (uint32)insertSorted(_entries, entry);
		calculateStructureSizes(0);
		return entryOffset;
	}

	/// <summary>Add an entry to the end of the list, packed on the minimum valid offset that the specified packing
	/// standard allows.</summary>
	/// <param name="name">The name of the new variable</param>
	/// <param name="type">The datatype of the new variable</param>
	/// <param name="arrayElements">If greater than one, the entry will be an array (type[arrayElements])</param>
	/// <typeparam name="standard">The packing standard to use. Currently, only std140 is supported.</typeparam>
	/// <returns>The index into which the array was inserted. Since the entry is added to the end of the list, it does
	/// not affect the indexes of any array elements that are already added.</returns>
	template<types::GpuDatatypes::Standard _standard_ = types::GpuDatatypes::Standard::std140>
	uint32 addEntryPacked(const StringHash& name, types::GpuDatatypes::Enum type, uint32 arrayElements = 1)
	{
		assertion(!isFinalized(), "Structured memory view must not be finalized.");
		return addEntryAtOffset(name, type, types::GpuDatatypes::getOffsetAfter(type, getUnalignedElementSize()), arrayElements);
	}

	/// <summary>Add multiple entries to the end of the list, in order, each packed on the minimum valid offset that
	/// the specified packing standard allows.</summary>
	/// <param name="entries">A c-style array of pairs of Names and Datatypes, that will be added (As if
	/// addEntryPacked was called for each sequentially</param>
	/// <param name="numEntries">The number of entries contained in the array 'entries'</param>
	/// <typeparam name="standard">The packing standard to use. Currently, only std140 is supported.</typeparam>
	template<types::GpuDatatypes::Standard _standard_ = types::GpuDatatypes::Standard::std140>
	void addEntriesPacked(const std::pair<StringHash, types::GpuDatatypes::Enum>* const entries, pvr::uint32 numEntries)
	{
		for (pvr::uint32 i = 0; i < numEntries; ++i) { addEntryPacked(entries[i].first, entries[i].second, 1); }
	}

	/// <summary>Get the byte offset of the specified variable by name</summary>
	/// <param name="name">The name of the entry variable</param>
	/// <param name="entryArrayIndex">If the variable is an array, which member of the array to return the offset to
	/// </param>
	/// <returns>The byte offset of the entry with name 'name'</returns>
	uint32 getOffset(const StringHash& name, uint32 entryArrayIndex = 0) const { return getOffset(getIndex(name), entryArrayIndex); }

	/// <summary>Get the byte offset of the specified variable by entry index</summary>
	/// <param name="variableIndex">The index of the entry to get the byte offset of</param>
	/// <param name="entryArrayIndex">If the variable is an array, which member of the array to return the offset to
	/// </param>
	/// <returns>The byte offset of the entry at position 'variableIndex'</returns>
	uint32 getOffset(uint32 variableIndex, uint32 entryArrayIndex = 0) const
	{
		return _entries[variableIndex].getOffset() + types::GpuDatatypes::getSelfAlignedArraySize(_entries[variableIndex].getType()) * entryArrayIndex;
	}

	/// <summary>For a dynamic buffer or an array of structs buffer, get the byte offset from the start of the buffer,
	/// of the specified variable by name, for a specified dynamic or array 'slice' of the buffer</summary>
	/// <param name="variableName">The name of the variable entry to get the byte offset of</param>
	/// <param name="dynamicIndex">The index of the buffer dynamic or array slice for which to perform the
	/// calculations</param>
	/// <param name="entryArrayIndex">If the variable is an array, which member of the array to return the offset to
	/// </param>
	/// <returns>The byte offset of the entry at position 'variableIndex' of the slice 'dynamicIndex'</returns>
	uint32 getDynamicOffset(const StringHash& variableName, uint32 dynamicIndex, uint32 entryArrayIndex = 0) const
	{
		return getDynamicOffset(getIndex(variableName), dynamicIndex, entryArrayIndex);
	}

	/// <summary>For a dynamic buffer or an array of structs buffer, get the byte offset from the start of the buffer,
	/// of the specified variable by entry index, for a specified dynamic or array 'slice' of the buffer</summary>
	/// <param name="variableIndex">The index of the entry to get the byte offset of</param>
	/// <param name="dynamicIndex">The index of the buffer dynamic or array slice for which to perform the
	/// calculations</param>
	/// <param name="entryArrayIndex">If the variable is an array, which member of the array to return the offset to
	/// </param>
	/// <returns>The byte offset of the entry at position 'variableIndex' of the slice 'dynamicIndex'</returns>
	uint32 getDynamicOffset(uint32 variableIndex, uint32 dynamicIndex, uint32 entryArrayIndex = 0) const
	{
		return getOffset(variableIndex, entryArrayIndex) + getAlignedElementSize() * dynamicIndex;
	}

	/// <summary>Retrieve the index of a variable entry by its name</summary>
	/// <param name="name">The name of a variable entry</param>
	/// <returns>The index of a variable entry</returns>
	uint32 getIndex(const StringHash& name) const
	{
		auto entry = std::find_if(_entries.begin(), _entries.end(), StructuredMemoryTableEntry::IsEqual(name));
		assertion(entry != _entries.end());
		return uint32(entry - _entries.begin());
	}

	/// <summary>Instead of connecting this object to an actual buffer, directly provide a pointer to some kind of memory.
	/// This memory will be used when the setValue(...), setArrayValue(...) methods are called</summary>
	/// <param name="memoryToPointTo">A pointer to memory that is assumed to follow the structure defined by this
	/// object.</param>
	void pointToMemory(void* memoryToPointTo) { _aliasedMemory = memoryToPointTo; }

	inline void validateBufferUsage(types::BufferBindingUse bufferBindingUse)
	{
		// the buffer binding uses for buffer must exist within the set of buffer uses specified at finalize time
		// check whether buffer includes uses the structured memory view does not
		assertion(((~static_cast<pvr::uint32>(_bufferBindingUse)) & static_cast<pvr::uint32>(bufferBindingUse)) == 0,
		          "Buffer usage must be compatible with structured memory view.");
	}

	/// <summary>Connect a buffer to this object, so that the methods map... setValue... unMap... be called directly. The
	/// buffer must be mappable and (obviously) be large enough to contain the data written. Using this function and
	/// calling map/set/unmap would be similar to calling pointToMemory(buffer->map), setValue, buffer->unmap,
	/// pointToMemory(null). Swap indexes can be set separately to facilitate multibuffering setups.</summary>
	/// <param name="swapIdx">Set a buffer for this swap index. For non-multibuffering setups, use 0. Only one buffer
	/// will be set per index. Setting another buffer discards the reference to the previous one.</param>
	/// <param name="buffer">The buffer to connect to this object.</param>
	/// <param name="mapDefaultFlags">The flags to use by default when mapping. Can be overriden in the mapping
	/// command. Default is Write.</param>
	/// <param name="mapDefaultOffset">A custom offset to apply by default to any map() methods called for this
	/// buffer. Default is 0.</param>
	void connectWithBuffer(uint32 swapIdx, const api::BufferView& buffer, types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write, uint32 mapDefaultOffset = 0)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");

		if (buffer->getResource()->getBufferUsage() != _bufferBindingUse)
		{
			validateBufferUsage(buffer->getResource()->getBufferUsage());
			calculateStructureSizes(calculateBufferDynamicAlignment(buffer->getResource()->getBufferUsage()));
		}

		_connectedBuffers[swapIdx] = buffer;
		_connectedBufferDefaultFlags = mapDefaultFlags;
		_connectedBufferDefaultOffset = mapDefaultOffset;
		debug_assertion(buffer->getRange() >= _baseUnalignedSize, "Buffer to connect is too small");
	}

	/// <summary>Using the structure of this object as a template, create a set of buffers suitable for its contents.
	/// The buffer will be created with exactly the correct size and with the flags passed. Same as calling
	/// createConnectedBuffers once for each number from zero to numberOfSwapIdxs-1</summary>
	/// <param name="numberOfSwapIdxs">Set up multibuffering: one buffer will be created per swapIdx.</param>
	/// <param name="ctx">The graphics context to create the buffers on</param>
	/// <param name="mapDefaultFlags">The mapping flags used as default when calling map(). Default: Write</param>
	void createConnectedBuffers(uint32 numberOfSwapIdxs, GraphicsContext& ctx, types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		createConnectedBuffers(numberOfSwapIdxs, ctx, _bufferBindingUse, mapDefaultFlags);
	}

	/// <summary>Using the structure of this object as a template, create a set of buffers suitable for its contents.
	/// The buffer will be created with exactly the correct size and with the flags passed. Same as calling
	/// createConnectedBuffers once for each number from zero to numberOfSwapIdxs-1</summary>
	/// <param name="numberOfSwapIdxs">Set up multibuffering: one buffer will be created per swapIdx.</param>
	/// <param name="ctx">The graphics context to create the buffers on</param>
	/// <param name="bufferAllowedUses">The uses the specific set of buffers can be used for. Will
	/// affect the alignment of each element of this object, so must be respected. This usage must be a subset of
	/// the uses set out in the call made to finalize.</param>
	/// <param name="mapDefaultFlags">The mapping flags used as default when calling map(). Default: Write</param>
	void createConnectedBuffers(uint32 numberOfSwapIdxs, GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses,
	                            types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		_connectedBufferDefaultFlags = mapDefaultFlags;
		_connectedBufferDefaultOffset = 0;

		for (uint32 i = 0; i < numberOfSwapIdxs; ++i)
		{
			createConnectedBuffer(i, ctx, mapDefaultFlags);
		}
	}

	/// <summary>Using the structure of this object as a template, create a buffer suitable for its contents. The buffer
	/// will be created with exactly the correct size and with the flags passed. It is in all respects the same as
	/// calling createBufferAsTemplate and then connectWithBuffer.</summary>
	/// <param name="swapIdx">The swap index to assign this buffer to (i.e. the index that will be used in
	/// mapConnectedBuffer to map this buffer).</param>
	/// <param name="ctx">The graphics context to create the buffer on</param>
	/// <param name="mapDefaultFlags">The mapping flags used as default when calling map(). Default: Write</param>
	void createConnectedBuffer(uint32 swapIdx, GraphicsContext& ctx, types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		createConnectedBuffer(swapIdx, ctx, _bufferBindingUse, mapDefaultFlags);
	}

	/// <summary>Using the structure of this object as a template, create a buffer suitable for its contents. The buffer
	/// will be created with exactly the correct size and with the flags passed. It is in all respects the same as
	/// calling createBufferAsTemplate and then connectWithBuffer.</summary>
	/// <param name="swapIdx">The swap index to assign this buffer to (i.e. the index that will be used in
	/// mapConnectedBuffer to map this buffer).</param>
	/// <param name="ctx">The graphics context to create the buffer on</param>
	/// <param name="bufferAllowedUses">The uses the buffer can be used for. Will
	/// affect the alignment of each element of this object, so must be respected. This usage must be a subset of
	/// the uses set out in the call made to finalize.</param>
	/// <param name="mapDefaultFlags">The mapping flags used as default when calling map(). Default: Write</param>
	void createConnectedBuffer(uint32 swapIdx, GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses,
	                           types::MapBufferFlags mapDefaultFlags = types::MapBufferFlags::Write)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		_connectedBufferDefaultFlags = mapDefaultFlags;
		_connectedBufferDefaultOffset = 0;
		bool isMappable = mapDefaultFlags != types::MapBufferFlags::None;
		_connectedBuffers[swapIdx] = createBufferAsTemplate(ctx, bufferAllowedUses, isMappable);
	}

	/// <summary>Using this object as a template, create a buffer suitable for exactly holding the information
	/// represented by this object.</summary>
	/// <param name="ctx">The graphics context to create the buffer on</param>
	/// <param name="mappable">The created buffer can be mapped. Default true.</param>
	api::BufferView createBufferAsTemplate(GraphicsContext& ctx, bool mappable = true)
	{
		return createBufferAsTemplate(ctx, _bufferBindingUse, mappable);
	}

	/// <summary>Using this object as a template, create a buffer suitable for exactly holding the information
	/// represented by this object.</summary>
	/// <param name="ctx">The graphics context to create the buffer on</param>
	/// <param name="bufferAllowedUses">The uses the buffer can be used for. Will
	/// affect the alignment of each element of this object, so must be respected. This usage must be a subset of
	/// the uses set out in the call made to finalize.</param>
	/// <param name="mappable">The created buffer can be mapped. Default true.</param>
	api::BufferView createBufferAsTemplate(GraphicsContext& ctx, types::BufferBindingUse bufferAllowedUses, bool mappable = true)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");

		if (bufferAllowedUses != _bufferBindingUse)
		{
			validateBufferUsage(bufferAllowedUses);
			calculateStructureSizes(calculateBufferDynamicAlignment(bufferAllowedUses));
		}

		auto buffer = ctx->createBuffer(getAlignedTotalSize(), _bufferBindingUse, mappable);
		return ctx->createBufferView(buffer, 0, getAlignedElementSize());
	}

	/// <summary>Call this function in order to set this object up properly to represent an array of elements, i.e. the
	/// information that has been added represents only an array member or dynamic slice of the buffer and not its
	/// entire contents. This function must be called for dynamic uniform/storage buffers before calling any of the
	/// createConnectedBuffer and similar functions as it affects the total size AND alignment of items.</summary>
	/// <param name="context">The graphics context to query for alignment restrictions</param>
	/// <param name="elementCount">The number of dynamic elements or array elements (i.e. the number of array members)
	/// of the buffer</param>
	/// <param name="bufferAllowedUses">The types of buffers that this object will be able to be used with. Will affect
	/// the alignment/padding of each element of this object, so must be respected. If in doubt, add both UniformBuffer
	/// and StorageBufer in order to ensure the strictest alignment requirements are enforced.</param>
	/// <param name="allowedUboDynamic">Allow this object to be bound as a Dynamic Ubo. WARNING: If this flag is not set
	/// and the object is used as a DynamicUbo, errors may occur as the buffer may be more packed than the API may 
	/// support</param>
	/// <param name="allowedSssboDynamic">Allow this object to be bound as a Dynamic SSBO. WARNING: If this flag is not 
	/// set and the object is later used as a DynamicUbo, errors may occur as the buffer may be more packed than the API 
	/// may support</param>
	void finalize(const GraphicsContext& context, uint32 elementCount, types::BufferBindingUse bufferAllowedUses,
		bool allowedUboDynamic = false, bool allowedSsboDynamic = false)
	{
		assertion(!isFinalized(), "Structured memory view must not already be finalized.");
		assertion(elementCount != 0, "Element count must not be 0");
		_elementCount = elementCount;
		_minUboDynamicAlignment = 0;
		_minSsboDynamicAlignment = 0;

		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::UniformBuffer) != 0 && allowedUboDynamic)
		{
			_minUboDynamicAlignment = context->getApiCapabilities().uboDynamicOffsetAlignment();
		}
		if (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::StorageBuffer) != 0 && allowedSsboDynamic)
		{
			_minSsboDynamicAlignment = context->getApiCapabilities().ssboDynamicOffsetAlignment();
		}

		// check whether allow dynamic ubo and ubo
		assertion(!allowedUboDynamic || (allowedUboDynamic && (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::UniformBuffer) != 0)),
		          "A dynamic buffer can only be a Uniform or Storage buffer.");

		// check whether allow dynamic ubo and ssbo
		assertion(!allowedSsboDynamic || (allowedSsboDynamic && (static_cast<pvr::uint32>(bufferAllowedUses & types::BufferBindingUse::StorageBuffer) != 0)),
		          "A dynamic buffer can only be a Uniform or Storage buffer.");

		calculateStructureSizes(calculateBufferDynamicAlignment(bufferAllowedUses));
		_bufferBindingUse = bufferAllowedUses;

		_finalized = true;
	}

	/// <summary>Gets whether the structured memory view has been finalized.</summary>
	/// <returns>Whether finalize has been called for the structured memory view.</returns>
	bool isFinalized() const
	{
		return _finalized;
	}

	/// <summary>Get the connected buffer for the specified swap index.</summary>
	/// <param name="swapIdx">A swap index</param>
	/// <returns>The buffer that has been created or connected to the specified swap index</returns>
	api::BufferView getConnectedBuffer(uint32 swapIdx) const
	{
		return _connectedBuffers[swapIdx];
	}

	/// <summary>Map the buffer connected to the specified swap index. After performing this operation, the
	/// setValue(...) setArrayValue(...) and similar commands become valid and actually set values on the buffer. Maps
	/// the entire buffer.</summary>
	/// <param name="swapIdx">A swap index</param>
	/// <param name="flags">The Mapping flags to use. Default:Write.</param>
	/// <param name="offset">A custom offset, from the beginneing of this buffer, to apply to the mapping operation.
	/// This offset overrides the default offset defined when calling connectWithBuffer. Default: The default offset
	/// </param>
	void map(uint32 swapIdx, types::MapBufferFlags flags = types::MapBufferFlags::Write, uint32 offset = 0xFFFFFFFFu)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		mapMultipleArrayElements(swapIdx, 0, _elementCount, flags, offset);
	}

	/// <summary>Return true if it is already mapped for a given swapchain index.</summary>
	/// <returns>true if the buffer for <paramref name="swapIdx"/>is mapped, otherwise false</returns>
	bool isMapped(uint32 swapIdx)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		return _connectedBuffers[swapIdx]->isMapped();
	}

	/// <summary>Map multiple consecutive array/dynamic elements of the buffer connected to the specified swap index.
	/// After performing this operation, the setValue(...) setArrayValue(...) and similar commands become valid and
	/// actually set values on the buffer for that swap index.</summary>
	/// <param name="swapIdx">A swap index</param>
	/// <param name="arrayStartIndex">The first element (array slice) to map</param>
	/// <param name="numElementsToMap">The number of elements to map.</param>
	/// <param name="flags">The Mapping flags to use. Default:Write.</param>
	/// <param name="offset">A custom offset, from the beginneing of this buffer, to apply to the mapping operation.
	/// This offset overrides the default offset defined when calling connectWithBuffer. Default: The default offset
	/// </param>
	void mapMultipleArrayElements(uint32 swapIdx, uint32 arrayStartIndex, uint32 numElementsToMap,
	                              types::MapBufferFlags flags = types::MapBufferFlags::Write, uint32 offset = 0xFFFFFFFFu)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		flags = (flags == types::MapBufferFlags(0xFFFFFFFFu) ? _connectedBufferDefaultFlags : flags);
		offset = (offset == 0xFFFFFFFFu ? _connectedBufferDefaultOffset : offset);
		_aliasedMemory = _connectedBuffers[swapIdx]->map(flags, offset + arrayStartIndex * getAlignedElementSize(),
		                 getAlignedElementSize() * numElementsToMap);
	}

	/// <summary>Map multiple a single array/dynamic element of the buffer connected to the specified swap index.
	/// Identical as calling mapMultipleArrayElements(index, 1,...) After performing this operation, the setValue(...)
	/// setArrayValue(...) and similar commands become valid and actually set values on the buffer for this swap
	/// index.</summary>
	/// <param name="swapIdx">A swap index</param>
	/// <param name="arrayIndex">The element (array slice) to map</param>
	/// <param name="flags">The Mapping flags to use. Default:Write.</param>
	void mapArrayIndex(uint32 swapIdx, uint32 arrayIndex = 0, types::MapBufferFlags flags = types::MapBufferFlags::Write)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		mapMultipleArrayElements(swapIdx, arrayIndex, 1, flags);
	}

	/// <summary>Unmap the mapped buffer at a specified swap index. After calling this function, calling setValue() is
	/// no longer valid for that swap index.</summary>
	/// <param name="swapIdx">A swap index</param>
	void unmap(uint32 swapIdx)
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		_connectedBuffers[swapIdx]->unmap();
		_aliasedMemory = NULL;
	}

	/// <summary>Returns the pointer to which any setValue...(...) operation called will be writing to. Normally set when
	/// performing the map...(...) operations, or directly by the user with pointToMemory(...)</summary>
	void* getMemoryPointer()
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		return _aliasedMemory;
	}

	/// <summary>Returns the pointer to which any setValue...(...) operation called will be writing to. Normally set when
	/// performing the map...(...) operations, or directly by the user with pointToMemory(...)</summary>
	const void* getMemoryPointer() const
	{
		assertion(isFinalized(), "Structured memory view must be finalized.");
		return _aliasedMemory;
	}
};

typedef StructuredBufferView StructuredMemoryView;

}
}