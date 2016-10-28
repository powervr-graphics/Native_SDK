/*!*********************************************************************************************************************
\file         PVRCore\FileStream.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Streams that are created from files.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Stream.h"

namespace pvr {
/*!*********************************************************************************************************************
\brief     A FileStream is a Stream that is used to access a File in the filesystem of the platform.
***********************************************************************************************************************/
class FileStream : public Stream
{
public:
	/*!*********************************************************************************************************************
	\brief  Create a new filestream of a specified file.
	\param  filePath The path of the file. Can be in any format the operating system understands (absolute, relative etc.)
	\param  flags   fopen-style flags.
	\description Possible flags: 'r':read,'w':truncate/write, 'a':append/write, r+: read/write, w+:truncate/read/write,
	        a+:append/read/write
	***********************************************************************************************************************/
	FileStream(const std::basic_string<char8>& filePath, const std::basic_string<char8>& flags);
	~FileStream() { close(); }

	/*!********************************************************************************************************
	\brief    Main read function. Read up to a specified amount of items into the provided buffer.
	\param[in]  elementSize  The size of each element that will be read.
	\param[in]  elementCount  The maximum number of elements to read.
	\param[in]  buffer  The buffer into which to write the data.
	\param[out]  dataRead  After returning, will contain the number of items that were actually read.
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
	\param[in]  offset  The offset to seec from "origin".
	\param[in]  origin  Beginning of stream, End of stream or Current position.
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
	**********************************************************************************************************/
	virtual void close();

	/*!********************************************************************************************************
	\brief		Query if the stream is open
	\return   True if the stream is open and ready for other operations.
	**********************************************************************************************************/
	virtual bool isopen() const;

	/*!********************************************************************************************************
	\brief		Query the current position in the stream
	\return   The current position in the stream.
	**********************************************************************************************************/
	virtual size_t getPosition() const;

	/*!********************************************************************************************************
	\brief		Query the total amount of data in the stream.
	\return   The total amount of data in the stream.
	**********************************************************************************************************/
	virtual size_t getSize() const;

	/*!
	   \brief Create a new file stream
	   \param file
	   \param flag
	   \return Return a valid file stream, else Return null if it fails
	 */
	static Stream::ptr_type createFileStream(const char* file, const char* flag)
	{
		Stream::ptr_type stream(new FileStream(file, flag));
		if (!stream->open()) { stream.reset(); }
		return stream;
	}

protected:
	mutable FILE* m_file;
	std::string m_flags;
};

}
