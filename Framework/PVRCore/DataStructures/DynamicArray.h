/*!
\brief A slightly lighter version of std::vector, that is 64 bits smaller on 64 bit machines by only supporting
32bit indexes. Stub.
\file PVRCore/DataStructures/DynamicArray.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
namespace pvr{

	template<class _type, uint32 _arraySize>
	class DynamicArray
	{
		_type typeArray[_arraySize];
		_type* dynPtr;

		uint32 size;
		uint32 capacity;
		uint32 count;
	public:
		DynamicArray() : dynPtr(typeArray), size(_arraySize){}
		
		_type& operator[](uint32 index)	
		{	
			assertion(index < capacity());
			return dynPtr[index];	
		}
		
		const _type& operator[](uint32 index) const
		{
			assertion(index < getCapacity());
			return dynPtr[index];
		}
		void clear(){ size = 0; }
		void reserve(uint32 size)
		{
			if (size > capcity()){	reserveImpl(size);	}
		}

		uint32 size()const { return size; }
		uint32 capacity()const { return capacity; }
	private:
		void reserveImpl(uint32 size)
		{
			_type* tmp = dynPtr;
			dynPtr = new _type[size];
			copyElements<false>(tmp, 0, this->size);
			capacity = size;
			delete []tmp;
		}

		template<bool deleteSrc>
		void copyElements(_type* src,uint32 count)
			void DynamicArray::copyElements(_type* src, uint32 dstOffset, uint32 count)
		{
			for (pvr::uint32 i = 0; i < count; ++i)
			{
				dynPtr[dstOffset + i] = src[i];
				if (deleteSrc){	src[i].~_type(); }
			}
		}
	};
}//namespace pvr