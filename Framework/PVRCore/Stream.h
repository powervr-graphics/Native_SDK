/*!*********************************************************************************************************************
\file         PVRCore\Stream.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains a class used to abstract streams of data (files, blocks of memory, resources etc.).
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
/*!********************************************************************************************************
\brief   This class is used to abstract streams of data (files, blocks of memory, resources etc.). In general
         a stream is considered something that can be read or written from. Specializations for many
		 different types of streams are provided by the PowerVR Framework, the most commonly used ones
		 being Files and Memory. The common interface and pointer types allow the Stream to abstract data
		 in a very useful manner.
		 Use the Stream::ptr_type to pass abstract streams around (it is actually an std::auto_ptr)
**********************************************************************************************************/
class Stream
{
public:
	typedef std::auto_ptr<Stream> ptr_type;
	typedef std::auto_ptr<const Stream> const_ptr_type;
	/*!********************************************************************************************************
	\brief   When seeking, select if your offset should be considered to be from the Start of the stream,
			the Current point in the stream or the End of the stream.
	**********************************************************************************************************/
	enum SeekOrigin
	{
		SeekOriginFromStart,
		SeekOriginFromCurrent,
		SeekOriginFromEnd
	};

	virtual ~Stream() {}

	/*!********************************************************************************************************
	\return   True if this stream can be read from.
	**********************************************************************************************************/
	bool isReadable() const { return m_isReadable; }

	/*!********************************************************************************************************
	\return   True if this stream can be written to.
	**********************************************************************************************************/
	bool isWritable() const { return m_isWritable; }

	/*!********************************************************************************************************
	\return   The filename of the file that this string represents, if such exists. Otherwise, empty string.
	**********************************************************************************************************/
	const std::string& getFileName() const { return m_fileName; }

public:
	/*!********************************************************************************************************
	\brief    Main read function. Read up to a specified amount of items into the provided buffer.
	\param[in]  elementSize  The size of each element that will be read.
	\param[in]  elementCount  The maximum number of elements to read.
	\param[in]  buffer  The buffer into which to write the data.
	\param[out]  dataRead  After returning, will contain the number of items that were actually read
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool read(size_t elementSize, size_t elementCount, void* buffer, size_t& dataRead) const = 0;

	/*!********************************************************************************************************
	\brief    Main write function. Write into the stream the specified amount of items from a provided buffer.
	\param[in]  elementSize  The size of each element that will be written.
	\param[in]  elementCount  The number of elements to write.
	\param[in]  buffer  The buffer from which to read the data. If the buffer is smaller than
	            elementSize * elementCount bytes, result is undefined.
	\param[out]  dataWritten  After returning, will contain the number of items that were actually written. Will
	            contain elementCount unless an error has occured.
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool write(size_t elementSize, size_t elementCount, const void* buffer, size_t& dataWritten) = 0;

	/*!********************************************************************************************************
	\brief    Seek a specific point for random access streams. After successful call, subsequent operation will
	          happen in the specified point.
	\param[in]  offset  The offset to seec from "origin"
	\param[in]  origin  Beginning of stream, End of stream or Current position
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool seek(long offset, SeekOrigin origin) const = 0;

	/*!********************************************************************************************************
	\brief    Prepares the stream for read / write / seek operations.
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool open()const = 0;

	/*!********************************************************************************************************
	\brief    Closes the stream.
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual void close() = 0;

	/*!********************************************************************************************************
	\return   True if the stream is open and ready for other operations.
	**********************************************************************************************************/
	virtual bool isopen() const = 0;

	/*!********************************************************************************************************
	\return   If suppored, returns the current position in the stream.
	**********************************************************************************************************/
	virtual size_t getPosition() const = 0;

	/*!********************************************************************************************************
	\return   If suppored, returns the total amount of data in the stream.
	**********************************************************************************************************/
	virtual size_t getSize() const = 0;

	/*!********************************************************************************************************
	\brief    Convenience functions that reads all data in the stream into a contiguous block of memory of
	          a specified element type.
	          Requires random-access stream.
	\tparam   Type_ The type of item that will be read into.
	\return   A std::vector<Type_> containing all data from the current point to the end of the stream.
	**********************************************************************************************************/
	template<typename Type_> std::vector<Type_> readToEnd() const
	{
		std::vector<Type_> ret;
		size_t mySize = getSize() - getPosition();
		size_t myElements = mySize / sizeof(Type_);
		ret.resize(mySize);
		size_t actuallyRead;
		read(sizeof(Type_), myElements, ret.data(), actuallyRead);
		return ret;
	}

	/*!********************************************************************************************************
	\brief    Convenience function that reads all data in the stream into a raw, contiguous block of memory.
	Requires random-access stream.
	\param[out] outString  A std::vector<char> that will contain all data in the stream.
	\return   true if successful, false otherwise.
	**********************************************************************************************************/
	bool readIntoCharBuffer(std::vector<char>& outString) const
	{
		if (!isopen()) { return false; }
		outString.resize(getSize() + 1);

		size_t dataRead;
		return read(1, getSize(), outString.data(), dataRead);
	}

	/*!********************************************************************************************************
	\brief    Convenience function that reads all data in the stream into a raw, contiguous block of memory.
	Requires random-access stream.
	\param[out] outString  A std::vector<char> that will contain all data in the stream.
	\return   true if successful, false otherwise.
	**********************************************************************************************************/
	template<typename T_>
	bool readIntoBuffer(std::vector<T_>& outString) const
	{
		if (!isopen()) { return false; }
		size_t sz = getSize();
		assertion(sz < (std::numeric_limits<size_t>::max)());
		size_t initial_size = outString.size();
		outString.resize(initial_size + getSize());

		size_t dataRead;
		return read(sizeof(T_), getSize(), outString.data() + initial_size, dataRead);
	}

	/*!********************************************************************************************************
	\brief    Convenience function that reads all data in the stream into a raw, contiguous block of memory.
	          Requires random-access stream.
	\return   A std::vector<char> that will contain all data in the stream.
	**********************************************************************************************************/
	std::vector<char8> readChars() const
	{
		std::vector<char8> pData;
		readIntoCharBuffer(pData);
		return pData;
	}

	/*!********************************************************************************************************
	\brief    Convenience function that reads all data in the stream into a std::string
	\return   A std::string that will contain all data in the stream.
	**********************************************************************************************************/
	bool readIntoString(std::string& outString) const
	{
		std::vector<char8> pData;
		bool rslt = readIntoCharBuffer(pData);
		outString.clear();
		outString.append(pData.data());
		return rslt;
	}

protected:
	Stream(const string& fileName) : m_isReadable(false), m_isWritable(false), m_fileName(fileName) {}
	bool m_isReadable;
	bool m_isWritable;
	string m_fileName;

private:
	// Disable copying and assign.
	void operator=(const Stream&);
	Stream(const Stream&);
};
}
