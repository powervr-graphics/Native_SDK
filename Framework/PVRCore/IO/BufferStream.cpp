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
BufferStream::BufferStream(const string& fileName)
	: Stream(fileName), _originalData(0), _currentPointer(0), _bufferSize(0), _bufferPosition(0) { }

BufferStream::BufferStream(const string& fileName, const void* buffer, size_t bufferSize)
	: Stream(fileName), _originalData(buffer), _currentPointer(NULL), _bufferSize(bufferSize), _bufferPosition(0)
{
	_isReadable = true;
}

BufferStream::BufferStream(const string& fileName, void* buffer, size_t bufferSize, bool setWritable/*=true*/,
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
	_currentPointer = NULL;
	_bufferPosition = 0;
}

bool BufferStream::read(size_t size, size_t count, void* const data, size_t& dataRead) const
{
	dataRead = 0;
	if (!_isReadable)
	{
		assertion(0 ,  "Stream not readable");
		Log(Log.Error, "Attempted to read non readable stream");
		return false;
	}
	if (!data || !_currentPointer)
	{
		return false;
	}
	byte* dataCurrent = (byte*)data;
	//Make sure we don't read too much
	for (size_t realcount = 0; realcount < count; ++realcount)
	{
		size_t realsize = (size_t)(std::min)(size, _bufferSize - _bufferPosition);
		memcpy(dataCurrent, _currentPointer, realsize);

		_bufferPosition += realsize;
		_currentPointer = (void*)(((byte*)_currentPointer) + realsize);
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
			byte* dataCurrent = (byte*)data;
			//Make sure we don't read too much
			for (size_t realcount = 0; realcount < count; ++realcount)
			{
				size_t realsize = (size_t)std::min<uint64>((uint64)size, _bufferSize - _bufferPosition);
				memcpy(_currentPointer, dataCurrent, realsize);

				_bufferPosition += realsize;
				_currentPointer = (void*)(((byte*)_currentPointer) + realsize);
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
			Log(Log.Error, "[BufferStream::seek] Attempt to seek from empty stream");
			result = false;
		}
	}
	else
	{
		switch (origin)
		{
		case Stream::SeekOriginFromStart:
		{
			newOffset = (long)glm::clamp<int64>((int64)offset, 0, (int64)_bufferSize);

			_bufferPosition = newOffset;
			_currentPointer = (void*)((byte*)_originalData + _bufferPosition);
			break;
		}
		case Stream::SeekOriginFromCurrent:
		{
			int64 maxOffset = _bufferSize - _bufferPosition;
			int64 minOffset = -1 * (int64)_bufferPosition;

			newOffset = glm::clamp<long>(offset, (long)minOffset, (long)maxOffset);

			_bufferPosition += newOffset;
			_currentPointer = (void*)((byte*)_currentPointer + newOffset);
			break;
		}
		case Stream::SeekOriginFromEnd:
		{
			newOffset = (long)glm::clamp<int64>(offset, (long)(-1 * ((int64)_bufferPosition)), 0);

			_bufferPosition = _bufferSize + newOffset;
			_currentPointer = (void*)((byte*)_originalData + _bufferPosition);
			break;
		}
		}
	}

	if (newOffset != offset)
	{
		Log(Log.Error, "[BufferStream::seek] Attempted to seek past the end of stream");
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