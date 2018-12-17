/*!
\brief A Stream wrapping a block of memory.
\file PVRCore/io/BufferStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"
#include <algorithm>

namespace pvr {
/// <summary>This class is used to access a block of memory as a Stream.</summary>
class BufferStream : public Stream
{
public:
	/// <summary>The pointer type normally used to wrap a Stream interface.</summary>
	typedef std::unique_ptr<BufferStream> ptr_type;
	/// <summary>Create a BufferStream from a buffer and associate it with an (arbitrary) filename.</summary>
	/// <param name="fileName">The created stream will have this filename. Arbitrary - not used to access anything.
	/// </param>
	/// <param name="buffer">Pointer to the memory that this stream will be used to access. Must be kept live from the
	/// point the stream is opened until the stream is closed</param>
	/// <param name="bufferSize">The size, in bytes, of the buffer</param>
	/// <param name="setWritable">Allow writing for this stream. Default true.</param>
	/// <param name="setReadable">Allow reading for this stream. Default true.</param>
	BufferStream(const std::string& fileName, void* buffer, size_t bufferSize, bool setWritable = true, bool setReadable = true)
		: Stream(fileName), _originalData(buffer), _currentPointer(NULL), _bufferSize(bufferSize), _bufferPosition(0)
	{
		_isWritable = setWritable;
		_isReadable = setReadable;
	}

	~BufferStream()
	{
		close();
	}

	/// <summary>Create a BufferStream from a read only buffer and associate it with an (arbitrary) filename. Read only.
	/// </summary>
	/// <param name="fileName">The created stream will have this filename. Arbitrary - not used to access anything.
	/// </param>
	/// <param name="buffer">Pointer to the memory that this stream will be used to access. Must be kept live from the
	/// point the stream is opened until the stream is closed</param>
	/// <param name="bufferSize">The size, in bytes, of the buffer</param>
	BufferStream(const std::string& fileName, const void* buffer, size_t bufferSize)
		: Stream(fileName), _originalData(buffer), _currentPointer(NULL), _bufferSize(bufferSize), _bufferPosition(0)
	{
		_isReadable = true;
	}

	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="numElements">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read</param>
	virtual void read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const
	{
		dataRead = 0;
		if (!_isReadable)
		{
			throw InvalidOperationError("Attempted to read non readable stream");
		}
		if (!buffer || !_currentPointer)
		{
			throw InvalidOperationError("Attempted to read a null BufferStream");
		}
		char* dataCurrent = static_cast<char*>(buffer);
		// Make sure we don't read too much
		for (size_t realcount = 0; realcount < numElements; ++realcount)
		{
			size_t realsize = static_cast<size_t>(std::min(elementSize, _bufferSize - _bufferPosition));
			memcpy(dataCurrent, _currentPointer, realsize);

			_bufferPosition += realsize;
			_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + realsize);
			dataCurrent += realsize;

			if (realsize == elementSize)
			{
				++dataRead;
			}
		}
		if (dataRead != numElements && _bufferPosition != _bufferSize)
		{
			throw FileIOError("Unknown error while reading BufferStream.");
		}
	}

	/// <summary>Main write function. Write into the stream the specified amount of items from a provided buffer.
	/// </summary>
	/// <param name="elementSize">The size of each element that will be written.</param>
	/// <param name="numElements">The number of elements to write.</param>
	/// <param name="buffer">The buffer from which to read the data. If the buffer is smaller than elementSize *
	/// numElements bytes, result is undefined.</param>
	/// <param name="dataWritten">After returning, will contain the number of items that were actually written. Will
	/// contain numElements unless an error has occured.</param>
	virtual void write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten)
	{
		dataWritten = 0;
		if (!_isWritable)
		{
			throw InvalidOperationError("BufferStream::write: Attempt to write to non-writable stream");
		}

		if (!buffer || !_currentPointer)
		{
			throw FileIOError("BufferStream::write: UnknownError: No data / Memory Pointer was NULL");
		}
		const unsigned char* dataCurrent = static_cast<const unsigned char*>(buffer);
		// Make sure we don't read too much
		for (size_t realcount = 0; realcount < numElements; ++realcount)
		{
			size_t realsize = static_cast<size_t>(std::min<uint64_t>(static_cast<uint64_t>(elementSize), _bufferSize - _bufferPosition));
			memcpy(_currentPointer, dataCurrent, realsize);

			_bufferPosition += realsize;
			_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + realsize);
			dataCurrent += realsize;

			if (realsize == elementSize)
			{
				++dataWritten;
			}
		}
		if (dataWritten != numElements)
		{
			throw FileIOError("BufferStream::write: Unknown error trying to write stream");
		}
	}

#define CLAMP(val, minim, maxim) ((val < minim) ? minim : ((val > maxim) ? maxim : val))

	/// <summary>Seek a specific point for random access streams. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin"</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position</param>
	virtual void seek(long offset, SeekOrigin origin) const
	{
		long newOffset = 0;

		if (!_currentPointer || !_originalData)
		{
			if (offset)
			{
				throw FileIOError("BufferStream::seek: Attempt to seek from empty stream");
			}
		}
		else
		{
			switch (origin)
			{
			case Stream::SeekOriginFromStart:
			{
				newOffset = static_cast<long>(CLAMP(static_cast<int64_t>(offset), 0, static_cast<int64_t>(_bufferSize)));

				_bufferPosition = newOffset;
				_currentPointer = const_cast<void*>(static_cast<const void*>(static_cast<const unsigned char*>(_originalData) + _bufferPosition));
				break;
			}
			case Stream::SeekOriginFromCurrent:
			{
				int64_t maxOffset = _bufferSize - _bufferPosition;
				int64_t minOffset = -1 * static_cast<int64_t>(_bufferPosition);

				newOffset = static_cast<long>(CLAMP(offset, static_cast<long>(minOffset), static_cast<long>(maxOffset)));

				_bufferPosition += newOffset;
				_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + newOffset);
				break;
			}
			case Stream::SeekOriginFromEnd:
			{
				newOffset = static_cast<long>(CLAMP(offset, static_cast<long>(-1 * (static_cast<int64_t>(_bufferPosition))), 0));

				_bufferPosition = _bufferSize + newOffset;
				_currentPointer = const_cast<void*>(static_cast<const void*>(static_cast<const unsigned char*>(_originalData) + _bufferPosition));
				break;
			}
			}
		}

		if (newOffset != offset)
		{
			throw FileIOError("[BufferStream::seek] Attempted to seek past the end of stream");
		}
	}

	/// <summary>Prepares the stream for read / write / seek operations.</summary>
	virtual void open() const
	{
		if (!isopen())
		{
			_currentPointer = const_cast<void*>(_originalData);
			_bufferPosition = 0;

			if (_currentPointer == 0)
			{
				throw FileIOError("Could not open BufferStream: Pointer was null");
			}
		}
	}

	/// <summary>Closes the stream.</summary>
	virtual void close()
	{
		_currentPointer = nullptr;
		_bufferPosition = 0;
	}

	/// <summary>Check if the stream is open and ready for operations</summary>
	/// <returns>True if the stream is open and ready for other operations.</returns>
	virtual bool isopen() const
	{
		return _currentPointer != NULL;
	}

	/// <summary>If suppored, check the current position in the stream.</summary>
	/// <returns>If suppored, returns the current position in the stream.</returns>
	virtual size_t getPosition() const
	{
		return _bufferPosition;
	}

	/// <summary>If suppored, get the total data in the stream.</summary>
	/// <returns>If suppored, return the total amount of data in the stream.</returns>
	virtual size_t getSize() const
	{
		return _bufferSize;
	}

protected:
	/// <summary>Constructor. Constructs with a specified resource identifier.</summary>
	/// <param name="resourceName">A resource identifier (conceptually, a "Filename" without a file)</param>
	explicit BufferStream(const std::string& resourceName) : Stream(resourceName), _originalData(0), _currentPointer(0), _bufferSize(0), _bufferPosition(0) {}
	const void* _originalData; //!< The original pointer of the memory this stream accesses
	mutable void* _currentPointer; //!< Pointer to the current position in the stream
	mutable size_t _bufferSize; //!< The size of this stream
	mutable size_t _bufferPosition; //!< Offset of the current position in the stream

private:
	// Disable copy and assign.
	void operator=(const BufferStream&);
	BufferStream(const BufferStream&);
};
} // namespace pvr
