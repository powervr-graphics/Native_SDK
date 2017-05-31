/*!
\brief Two classes designed to carry values of arbitrary datatypes along with their "reflective" data (datatypes
etc.) FreeValue is statically allocated but has a fixed (max) size of 64 bytes, while TypedMem stores arbitrary
sized data.
\file PVRCore/DataStructures/FreeValue.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"
#include "PVRCore/Maths.h"
namespace pvr {
namespace types {
namespace GpuDatatypes {
template<> struct Metadata<char*> { typedef std::array<char, 64> storagetype; static const GpuDatatypes::Enum dataType = GpuDatatypes::float32; static const size_t gpuSize = 1; };
template<> struct Metadata<unsigned char*> { typedef std::array<char, 64> storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::float32; } static size_t gpuSizeOf() { return 1; } };
template<> struct Metadata<const char*> { typedef std::array<char, 64> storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::float32; } static size_t gpuSizeOf() { return 1; } };
template<> struct Metadata<const unsigned char*> { typedef std::array<char, 64> storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::float32; } static size_t gpuSizeOf() { return 1; } };
template<> struct Metadata<pvr::float64> { typedef pvr::float32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::float32; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<pvr::float32> { typedef pvr::float32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::float32; } static size_t gpuSizeOf() { return 4; } };
template<> struct Metadata<pvr::int64> { typedef pvr::int32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::integer; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<pvr::int32> { typedef pvr::int32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::integer; } static size_t gpuSizeOf() { return 4; } };
template<> struct Metadata<pvr::int16> { typedef pvr::int32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::integer; } static size_t gpuSizeOf() { return 2; } };
template<> struct Metadata<pvr::int8> { typedef pvr::int32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::integer; } static size_t gpuSizeOf() { return 1; } };
template<> struct Metadata<pvr::uint64> { typedef pvr::uint32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uinteger; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<pvr::uint32> { typedef pvr::uint32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uinteger; } static size_t gpuSizeOf() { return 4; } };
template<> struct Metadata<pvr::uint16> { typedef pvr::uint32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uinteger; } static size_t gpuSizeOf() { return 2; } };
template<> struct Metadata<pvr::uint8> { typedef pvr::uint32 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uinteger; } static size_t gpuSizeOf() { return 1; } };
template<> struct Metadata<glm::vec2> { typedef glm::vec2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::vec2; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<glm::vec3> { typedef glm::vec3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::vec3; } static size_t gpuSizeOf() { return 12; } };
template<> struct Metadata<glm::vec4> { typedef glm::vec4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::vec4; } static size_t gpuSizeOf() { return 16; } };
template<> struct Metadata<glm::ivec2> { typedef glm::ivec2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::ivec2; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<glm::ivec3> { typedef glm::ivec3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::ivec3; } static size_t gpuSizeOf() { return 12; } };
template<> struct Metadata<glm::ivec4> { typedef glm::ivec4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::ivec4; } static size_t gpuSizeOf() { return 16; } };
template<> struct Metadata<glm::uvec2> { typedef glm::uvec2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uvec2; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<glm::uvec3> { typedef glm::uvec3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uvec3; } static size_t gpuSizeOf() { return 12; } };
template<> struct Metadata<glm::uvec4> { typedef glm::uvec4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::uvec4; } static size_t gpuSizeOf() { return 16; } };
template<> struct Metadata<glm::bvec2> { typedef glm::bvec2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::bvec2; } static size_t gpuSizeOf() { return 8; } };
template<> struct Metadata<glm::bvec3> { typedef glm::bvec3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::bvec3; } static size_t gpuSizeOf() { return 12; } };
template<> struct Metadata<glm::bvec4> { typedef glm::bvec4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::bvec4; } static size_t gpuSizeOf() { return 16; } };
template<> struct Metadata<glm::mat2x2> { typedef glm::mat2x2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat2x2; } static size_t gpuSizeOf() { return 32; } };
template<> struct Metadata<glm::mat2x3> { typedef glm::mat2x3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat2x3; } static size_t gpuSizeOf() { return 32; } };
template<> struct Metadata<glm::mat2x4> { typedef glm::mat2x4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat2x4; } static size_t gpuSizeOf() { return 32; } };
template<> struct Metadata<glm::mat3x2> { typedef glm::mat3x2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat3x2; } static size_t gpuSizeOf() { return 48; } };
template<> struct Metadata<glm::mat3x3> { typedef glm::mat3x3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat3x3; } static size_t gpuSizeOf() { return 48; } };
template<> struct Metadata<glm::mat3x4> { typedef glm::mat3x4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat3x4; } static size_t gpuSizeOf() { return 48; } };
template<> struct Metadata<glm::mat4x2> { typedef glm::mat4x2 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat4x2; } static size_t gpuSizeOf() { return 64; } };
template<> struct Metadata<glm::mat4x3> { typedef glm::mat4x3 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat4x3; } static size_t gpuSizeOf() { return 64; } };
template<> struct Metadata<glm::mat4x4> { typedef glm::mat4x4 storagetype; static GpuDatatypes::Enum dataTypeOf() { return GpuDatatypes::mat4x4; } static size_t gpuSizeOf() { return 64; } };
}
}


class FreeValueView
{
protected:
	unsigned char* value_;
	uint32 arrayElements_; //!< Semantic data type
	types::GpuDatatypes::Enum dataType_; //!< Semantic data type
public:
	FreeValueView() : value_(0), arrayElements_(0), dataType_(types::GpuDatatypes::none)
	{
	}
	types::GpuDatatypes::Enum dataType() const { return dataType_; }

	bool isDataCompatible(const FreeValueView& rhs) const
	{
		return (dataType_ == rhs.dataType_) && (arrayElements_ == rhs.arrayElements_);
	}
	uint32 dataSize() const
	{
		uint32 isnone = (dataType_ == types::GpuDatatypes::none);
		return isnone * arrayElements_ + (1 - isnone) * types::GpuDatatypes::getCpuPackedSize(dataType_, arrayElements_);
	}
	uint32 arrayElements() const
	{
		return arrayElements_;
	}
	void* raw(uint32 arrayIndex)
	{
		size_t offset = (arrayIndex * types::GpuDatatypes::getCpuPackedSize(dataType_));
		return value_ + offset;
	}
	const void* raw(uint32 arrayIndex) const
	{
		size_t offset = (arrayIndex * types::GpuDatatypes::getCpuPackedSize(dataType_));
		return value_ + offset;
	}

	void* raw() { return value_; }
	const void* raw() const { return value_; }
	template<typename Type_> Type_* rawAs() { return reinterpret_cast<Type_*>(value_); }
	template<typename Type_> const Type_* rawAs() const { return reinterpret_cast<Type_*>(value_); }

	float32* rawFloats() { return reinterpret_cast<float32*>(value_); }
	const float32* rawFloats() const { return reinterpret_cast<const float32*>(value_); }
	int32* rawInts() { return reinterpret_cast<int32*>(value_); }
	const int32* rawInts() const { return reinterpret_cast<const int32*>(value_); }
	unsigned char* rawChars() { return value_; }
	const unsigned char* rawChars() const { return value_; }

	template<typename Type_> Type_& interpretValueAs(uint32 entryIndex = 0) { return ((Type_*)value_)[entryIndex]; }
	template<typename Type_> const Type_& interpretValueAs(uint32 entryIndex = 0) const { return ((Type_*)value_)[entryIndex]; }

};

struct TypedMem : public FreeValueView
{
private:
	uint32 currentSize_;
public:
	TypedMem() : currentSize_(0)
	{
	}
	~TypedMem()
	{
		free(FreeValueView::value_);
	}

	void assign(const TypedMem& rhs)
	{
		uint32 dataTypeSize = rhs.dataSize();
		allocate(rhs.dataType_, rhs.arrayElements_);
		memcpy(value_, rhs.value_, dataTypeSize);
	}

	const TypedMem& operator=(const TypedMem& rhs) const
	{
		uint32 dataTypeSize = rhs.dataSize();
		debug_assertion(dataTypeSize <= dataSize(), "TypedMem operator=: MISMATCHED SIZE");
		memcpy(value_, rhs.value_, dataTypeSize);
		return *this;
	}

	TypedMem(const TypedMem& rhs) : currentSize_(0)
	{
		uint32 dataTypeSize = rhs.dataSize();
		allocate(rhs.dataType_, rhs.arrayElements_);
		memcpy(value_, rhs.value_, dataTypeSize);
	}


	uint32 totalSize()const
	{
		return currentSize_;
	}

	void shrink(uint32 arrayElements)
	{
		uint32 dataTypeSize = (dataType_ == types::GpuDatatypes::none ? arrayElements : types::GpuDatatypes::getCpuPackedSize(dataType_) * arrayElements);
		arrayElements_ = arrayElements;
		if (!arrayElements)
		{
			free(value_);
			value_ = 0;
		}
		else if (dataTypeSize != currentSize_)
		{
			value_ = (unsigned char*)realloc(value_, dataTypeSize);
		}
		currentSize_ = dataTypeSize;
	}
	void clear()
	{
		dataType_ = types::GpuDatatypes::none;
		arrayElements_ = 0;
	}
	//grows only
	void allocate(types::GpuDatatypes::Enum dataType, uint32 arrayElements = 1)
	{
		uint32 isnone = (dataType == types::GpuDatatypes::none);
		uint32 dataTypeSize = isnone * arrayElements + (1 - isnone) * types::GpuDatatypes::getCpuPackedSize(dataType, arrayElements);

		dataType_ = dataType;
		arrayElements_ = arrayElements;
		if (dataTypeSize > currentSize_)
		{
			value_ = (unsigned char*)realloc(value_, dataTypeSize);
			currentSize_ = dataTypeSize;
		}
	}


	template<typename Type_> void allocAndSetValue(const Type_& rawvalue)
	{
		assertion(currentSize_ >= sizeof(rawvalue), "TypedMemory:: UnIf array values are used with this class, they must be pre-allocated");
		memcpy(value_, &rawvalue, sizeof(Type_));
	}

	template<typename Type_> void allocAndSetValue(const Type_& rawvalue, uint32 arrayIndex)
	{
		assertion(arrayElements_ > arrayIndex, "TypedMemory:: If array values are used with this class, they must be pre-allocated");
		memcpy(value_ + (arrayIndex * sizeof(Type_)), &rawvalue, sizeof(Type_));
	}

	template<typename Type_> void setValue(const Type_& rawvalue)
	{
		allocate(types::GpuDatatypes::Metadata<Type_>::dataTypeOf(), 1);
		memcpy(value_, &rawvalue, sizeof(Type_));
	}

	template<typename Type_> void setValue(const Type_& rawvalue, uint32 arrayIndex)
	{
		assertion(arrayElements_ > arrayIndex, "TypedMemory:: If array values are used with this class, they must be pre-allocated");
		memcpy(value_ + (arrayIndex * sizeof(Type_)), &rawvalue, sizeof(Type_));
	}

	void setValue(const char* c_string_value)
	{
		this->dataType_ = types::GpuDatatypes::none;
		pvr::uint32 sz = (uint32)strlen(c_string_value);
		allocate(types::GpuDatatypes::none, sz + 1);
		memcpy(value_, c_string_value, sz);
		value_[sz] = 0;
	}
	void setValue(const std::string& rawvalue)
	{
		setValue(rawvalue.c_str());
	}
};


struct FreeValue : public FreeValueView
{
private:
	union
	{
		double _alignment_[8];
		unsigned char value[64];
		int32 integer32_[16];
		float32 float32_[16];
		int16 int16_[16];
		int64 int64_[8];
	};
public:
	FreeValue() { value_ = value; }
	
	void setDataType(types::GpuDatatypes::Enum datatype) { dataType_ = datatype; }

	template<typename Type_> void setValue(const Type_& rawvalue)
	{
		this->dataType_ = types::GpuDatatypes::Metadata<Type_>::dataTypeOf();

		memcpy(this->value, &rawvalue, sizeof(Type_));
	}

	template<typename Type_> void setValue(TypedMem& rawvalue)
	{
		this->dataType_ = types::GpuDatatypes::Metadata<Type_>::dataTypeOf();

		memcpy(this->value, &rawvalue, sizeof(Type_));
	}

	void setValue(const char* c_string_value)
	{
		this->dataType_ = types::GpuDatatypes::none;
		size_t sz = strlen(c_string_value);

		memcpy(value, c_string_value, std::min(sz, sizeof(value)));
		value[63] = 0;
	}
	void setValue(const std::string& rawvalue)
	{
		this->dataType_ = types::GpuDatatypes::none;

		memcpy(value, rawvalue.c_str(), std::min(sizeof(value), rawvalue.length()));
		value[63] = 0;
	}

	void fastSet(types::GpuDatatypes::Enum type, char* value)
	{
		this->dataType_ = type;
		memcpy(this->value, value, 64);
	}


	template<typename Type_> Type_& interpretValueAs() { return *(Type_*)value; }

	template<typename Type_> const Type_& interpretValueAs() const { return *(Type_*)value; }

	template<typename Type_> Type_ castValueScalarToScalar() const
	{
		switch (dataType_)
		{
		case types::GpuDatatypes::float32: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<float32>::storagetype>());
		case types::GpuDatatypes::integer: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<int32>::storagetype>());
		default: Log("FreeValue: Tried to interpret matrix, string or vector value as scalar."); return Type_();
		}
	}

	template<typename Type_> Type_ castValueVectorToVector() const
	{
		switch (dataType_)
		{
		case types::GpuDatatypes::vec2: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::vec2>::storagetype>());
		case types::GpuDatatypes::vec3: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::vec3>::storagetype>());
		case types::GpuDatatypes::vec4: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::vec4>::storagetype>());
		case types::GpuDatatypes::ivec2: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::ivec2>::storagetype>());
		case types::GpuDatatypes::ivec3: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::ivec3>::storagetype>());
		case types::GpuDatatypes::ivec4: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::ivec4>::storagetype>());
		default: Log("FreeValue: Tried to interpret matrix, string or scalar value as vector."); return Type_();
		}
	}

	template<typename Type_> Type_ castValueMatrixToMatrix() const
	{
		switch (dataType_)
		{
		case types::GpuDatatypes::mat2x2: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat2x2>::storagetype>());
		case types::GpuDatatypes::mat2x3: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat2x3>::storagetype>());
		case types::GpuDatatypes::mat2x4: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat2x4>::storagetype>());
		case types::GpuDatatypes::mat3x2: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat3x2>::storagetype>());
		case types::GpuDatatypes::mat3x3: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat3x3>::storagetype>());
		case types::GpuDatatypes::mat3x4: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat3x4>::storagetype>());
		case types::GpuDatatypes::mat4x2: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat4x2>::storagetype>());
		case types::GpuDatatypes::mat4x3: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat4x3>::storagetype>());
		case types::GpuDatatypes::mat4x4: return Type_(interpretValueAs<types::GpuDatatypes::Metadata<glm::mat4x4>::storagetype>());
		default: Log("FreeValue: Tried to interpret vector, string or scalar value as matrix."); return Type_();
		}
	}

	const char* getValueAsString() const
	{
		switch (dataType_)
		{
		case types::GpuDatatypes::none: return (const char*)value;
		default: Log("FreeValue: Tried to interpret vector, matrix or scalar value as string."); return "";
		}
	}
};

}