/*!*********************************************************************************************************************
\file         PVRCore\BufferStream.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         A Stream wrapping a block of memory.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Stream.h"

namespace pvr {
/*!*********************************************************************************************************************
\brief      This class is used to access a block of memory as a Stream.
***********************************************************************************************************************/
class BufferStream : public Stream
{
public:
	typedef std::auto_ptr<BufferStream> ptr_type;
	/*!*********************************************************************************************************************
	\brief      Create a BufferStream from a buffer and associate it with an (arbitrary) filename.
	\param      fileName The created stream will have this filename. Arbitrary - not used to access anything.
	\param      buffer Pointer to the memory that this stream will be used to access. Must be kept live from the point the 
	            stream is opened until the stream is closed
	\param      bufferSize The size, in bytes, of the buffer
	\param      setWritable Allow writing for this stream. Default true.
	\param      setReadable Allow reading for this stream. Default true.
	***********************************************************************************************************************/
	BufferStream(const std::basic_string<char8>& fileName, void* buffer, size_t bufferSize, bool setWritable = true,
	             bool setReadable = true);
	/*!*********************************************************************************************************************
	\brief      Create a BufferStream from a read only buffer and associate it with an (arbitrary) filename. Read only.
	\param      fileName The created stream will have this filename. Arbitrary - not used to access anything.
	\param      buffer Pointer to the memory that this stream will be used to access. Must be kept live from the point the 
	            stream is opened until the stream is closed
	\param      bufferSize The size, in bytes, of the buffer
	***********************************************************************************************************************/
	BufferStream(const std::basic_string<char8>& fileName, const void* buffer, size_t bufferSize);

	/*!********************************************************************************************************
	\brief    Main read function. Read up to a specified amount of items into the provided buffer.
	\param[in]  elementSize  The size of each element that will be read.
	\param[in]  elementCount  The maximum number of elements to read.
	\param[in]  buffer  The buffer into which to write the data.
	\param[out]  dataRead  After returning, will contain the number of items that were actually read
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool read(size_t elementSize, size_t elementCount, void* buffer, size_t& dataRead) const;

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
	virtual bool write(size_t elementSize, size_t elementCount, const void* buffer, size_t& dataWritten);

	/*!********************************************************************************************************
	\brief    Seek a specific point for random access streams. After successful call, subsequent operation will
	          happen in the specified point.
	\param[in]  offset  The offset to seec from "origin"
	\param[in]  origin  Beginning of stream, End of stream or Current position
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool seek(long offset, SeekOrigin origin) const;

	/*!********************************************************************************************************
	\brief    Prepares the stream for read / write / seek operations.
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual bool open()const;

	/*!********************************************************************************************************
	\brief    Closes the stream.
	\return   Success if successful, error code otherwise.
	**********************************************************************************************************/
	virtual void close();

	/*!********************************************************************************************************
	\return   True if the stream is open and ready for other operations.
	**********************************************************************************************************/
	virtual bool isopen() const;

	/*!********************************************************************************************************
	\return   If suppored, returns the current position in the stream.
	**********************************************************************************************************/
	virtual size_t getPosition() const;

	/*!********************************************************************************************************
	\return   If suppored, returns the total amount of data in the stream.
	**********************************************************************************************************/
	virtual size_t getSize() const;

protected:
	/*!*********************************************************************************************************************
	\brief      Create a BufferStream without setting the internal store.
	\param      fileName The created stream will have this filename. Arbitrary - not used to access anything.
	***********************************************************************************************************************/
	BufferStream(const std::basic_string<char8>& fileName);
	const void* m_originalData; //!<The original pointer of the memory this stream accesses
	mutable void* m_currentPointer; //!<Pointer to the current position in the stream
	mutable size_t m_bufferSize; //!<The size of this stream
	mutable size_t m_bufferPosition;//!<Offset of the current position in the stream

private:
	// Disable copy and assign.
	void operator=(const BufferStream&);
	BufferStream(const BufferStream&);
};
}
