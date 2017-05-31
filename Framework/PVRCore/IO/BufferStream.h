/*!
\brief A Stream wrapping a block of memory.
\file PVRCore/IO/BufferStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Stream.h"

namespace pvr {
/// <summary>This class is used to access a block of memory as a Stream.</summary>
class BufferStream : public Stream
{
public:
	typedef std::auto_ptr<BufferStream> ptr_type;
	/// <summary>Create a BufferStream from a buffer and associate it with an (arbitrary) filename.</summary>
	/// <param name="fileName">The created stream will have this filename. Arbitrary - not used to access anything.
	/// </param>
	/// <param name="buffer">Pointer to the memory that this stream will be used to access. Must be kept live from the
	/// point the stream is opened until the stream is closed</param>
	/// <param name="bufferSize">The size, in bytes, of the buffer</param>
	/// <param name="setWritable">Allow writing for this stream. Default true.</param>
	/// <param name="setReadable">Allow reading for this stream. Default true.</param>
	BufferStream(const std::basic_string<char8>& fileName, void* buffer, size_t bufferSize, bool setWritable = true,
	             bool setReadable = true);
	/// <summary>Create a BufferStream from a read only buffer and associate it with an (arbitrary) filename. Read only.
	/// </summary>
	/// <param name="fileName">The created stream will have this filename. Arbitrary - not used to access anything.
	/// </param>
	/// <param name="buffer">Pointer to the memory that this stream will be used to access. Must be kept live from the
	/// point the stream is opened until the stream is closed</param>
	/// <param name="bufferSize">The size, in bytes, of the buffer</param>
	BufferStream(const std::basic_string<char8>& fileName, const void* buffer, size_t bufferSize);

	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="elementCount">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool read(size_t elementSize, size_t elementCount, void* buffer, size_t& dataRead) const;

	/// <summary>Main write function. Write into the stream the specified amount of items from a provided buffer.
	/// </summary>
	/// <param name="elementSize">The size of each element that will be written.</param>
	/// <param name="elementCount">The number of elements to write.</param>
	/// <param name="buffer">The buffer from which to read the data. If the buffer is smaller than elementSize *
	/// elementCount bytes, result is undefined.</param>
	/// <param name="dataWritten">After returning, will contain the number of items that were actually written. Will
	/// contain elementCount unless an error has occured.</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool write(size_t elementSize, size_t elementCount, const void* buffer, size_t& dataWritten);

	/// <summary>Seek a specific point for random access streams. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin"</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool seek(long offset, SeekOrigin origin) const;

	/// <summary>Prepares the stream for read / write / seek operations.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool open()const;

	/// <summary>Closes the stream.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void close();

	/// <summary>Check if the stream is open and ready for operations</summary>
	/// <returns>True if the stream is open and ready for other operations.</returns>
	virtual bool isopen() const;

	/// <summary>If suppored, check the current position in the stream.</summary>
	/// <returns>If suppored, returns the current position in the stream.</returns>
	virtual size_t getPosition() const;

	/// <summary>If suppored, get the total data in the stream.</summary>
	/// <returns>If suppored, return the total amount of data in the stream.</returns>
	virtual size_t getSize() const;

protected:
	BufferStream(const std::basic_string<char8>& fileName);
	const void* _originalData; //!<The original pointer of the memory this stream accesses
	mutable void* _currentPointer; //!<Pointer to the current position in the stream
	mutable size_t _bufferSize; //!<The size of this stream
	mutable size_t _bufferPosition;//!<Offset of the current position in the stream

private:
	// Disable copy and assign.
	void operator=(const BufferStream&);
	BufferStream(const BufferStream&);
};
}