/*!
\brief Implementations of methods of the BufferStream class.
\file PVRCore/IO/BufferStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include <algorithm>

#include "PVRCore/Maths.h"
#include "PVRCore/IO/BufferStream.h"

namespace pvr {
BufferStream::BufferStream(const std::string& fileName)
	: Stream(fileName), _originalData(0), _currentPointer(0), _bufferSize(0), _bufferPosition(0) { }

BufferStream::BufferStream(const std::string& fileName, const void* buffer, size_t bufferSize)
	: Stream(fileName), _originalData(buffer), _currentPointer(NULL), _bufferSize(bufferSize), _bufferPosition(0)
{
	_isReadable = true;
}

BufferStream::BufferStream(const std::string& fileName, void* buffer, size_t bufferSize, bool setWritable/*=true*/,
                           bool setReadable/*=true*/)
	: Stream(fileName), _originalData(buffer), _currentPointer(NULL), _bufferSize(bufferSize), _bufferPosition(0)
{
	_isWritable = setWritable;
	_isReadable = setReadable;
}

bool BufferStream::open()const
{
	_currentPointer = const_cast<void*>(_originalData);
	_bufferPosition = 0;

	return _currentPointer != 0;
}

void BufferStream::close()
{
	_currentPointer = nullptr;
	_bufferPosition = 0;
}

bool BufferStream::read(size_t size, size_t count, void* const data, size_t& dataRead) const
{
	dataRead = 0;
	if (!_isReadable)
	{
		assertion(0,  "Stream not readable");
		Log(LogLevel::Error, "Attempted to read non readable stream");
		return false;
	}
	if (!data || !_currentPointer)
	{
		return false;
	}
	char* dataCurrent = static_cast<char*>(data);
	//Make sure we don't read too much
	for (size_t realcount = 0; realcount < count; ++realcount)
	{
		size_t realsize = static_cast<size_t>(std::min(size, _bufferSize - _bufferPosition));
		memcpy(dataCurrent, _currentPointer, realsize);

		_bufferPosition += realsize;
		_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + realsize);
		dataCurrent += realsize;

		if (realsize == size)
		{
			++dataRead;
		}
	}
	if (dataRead != count)
	{
		if (_bufferPosition == _bufferSize)
		{
			return true;
		}
		else
		{
			Log("Unknown error while reading stream.");
			assertion(0);
			return false;
		}
	}
	return true;
}

bool BufferStream::write(size_t size, size_t count, const void* data, size_t& dataWritten)
{
	dataWritten = 0;
	if (_isWritable)
	{
		if (data && _currentPointer)
		{
			const unsigned char* dataCurrent = static_cast<const unsigned char*>(data);
			//Make sure we don't read too much
			for (size_t realcount = 0; realcount < count; ++realcount)
			{
				size_t realsize = static_cast<size_t>(std::min<uint64_t>(static_cast<uint64_t>(size), _bufferSize - _bufferPosition));
				memcpy(_currentPointer, dataCurrent, realsize);

				_bufferPosition += realsize;
				_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + realsize);
				dataCurrent += realsize;

				if (realsize == size)
				{
					++dataWritten;
				}
			}
			if (dataWritten != count)
			{
				if (_bufferPosition == _bufferSize)
				{
					assertion(0, "END OF STREAM?!");
					Log("BufferStream::write: Unknown error trying to write");
					return false;
				}
				else
				{
					assertion(0, "UNKNOWN ERROR");
					Log("BufferStream::write: Unknown error trying to write");
					return false;
				}
			}
		}
		else
		{
			assertion(false, "UNKNOWN ERROR");
			return false;
		}
	}
	else
	{
		Log("BufferStream::write: Attempt to write to non-writable stream");
		assertion(false, "ATTEMPT TO WRITE TO NON-WRITABLE STREAM");
		return false;
	}

	return true;
}

bool BufferStream::seek(long offset, SeekOrigin origin) const
{
	bool result = true;
	long newOffset = 0;

	if (!_currentPointer || !_originalData)
	{
		if (offset)
		{
			Log(LogLevel::Error, "[BufferStream::seek] Attempt to seek from empty stream");
			result = false;
		}
	}
	else
	{
		switch (origin)
		{
		case Stream::SeekOriginFromStart:
		{
			newOffset = static_cast<long>(glm::clamp<int64_t>(static_cast<int64_t>(offset), 0, static_cast<int64_t>(_bufferSize)));

			_bufferPosition = newOffset;
			_currentPointer = const_cast<void*>(static_cast<const void*>(static_cast<const unsigned char*>(_originalData) + _bufferPosition));
			break;
		}
		case Stream::SeekOriginFromCurrent:
		{
			int64_t maxOffset = _bufferSize - _bufferPosition;
			int64_t minOffset = -1 * static_cast<int64_t>(_bufferPosition);

			newOffset = glm::clamp<long>(offset, static_cast<long>(minOffset), static_cast<long>(maxOffset));

			_bufferPosition += newOffset;
			_currentPointer = static_cast<void*>(static_cast<char*>(_currentPointer) + newOffset);
			break;
		}
		case Stream::SeekOriginFromEnd:
		{
			newOffset = static_cast<long>(glm::clamp<int64_t>(offset, static_cast<long>(-1 * (static_cast<int64_t>(_bufferPosition))), 0));

			_bufferPosition = _bufferSize + newOffset;
			_currentPointer = const_cast<void*>(static_cast<const void*>(static_cast<const unsigned char*>(_originalData) + _bufferPosition));
			break;
		}
		}
	}

	if (newOffset != offset)
	{
		Log(LogLevel::Error, "[BufferStream::seek] Attempted to seek past the end of stream");
		result = false;;
	}

	return result;
}

bool BufferStream::isopen() const
{
	return _currentPointer != NULL;
}

size_t BufferStream::getPosition() const
{
	return _bufferPosition;
}

size_t BufferStream::getSize() const
{
	return _bufferSize;
}
}
//!\endcond
