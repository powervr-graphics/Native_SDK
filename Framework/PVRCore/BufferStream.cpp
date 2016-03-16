/*!*********************************************************************************************************************
\file         PVRCore\BufferStream.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods of the BufferStream class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>
#include <algorithm>

#include "PVRCore/Maths.h"
#include "PVRCore/BufferStream.h"

namespace pvr {
BufferStream::BufferStream(const string& fileName)
	: Stream(fileName), m_originalData(0), m_currentPointer(0), m_bufferSize(0), m_bufferPosition(0) { }

BufferStream::BufferStream(const string& fileName, const void* buffer, size_t bufferSize)
	: Stream(fileName), m_originalData(buffer), m_currentPointer(NULL), m_bufferSize(bufferSize), m_bufferPosition(0)
{
	m_isReadable = true;
}

BufferStream::BufferStream(const string& fileName, void* buffer, size_t bufferSize, bool setWritable/*=true*/,
                           bool setReadable/*=true*/)
	: Stream(fileName), m_originalData(buffer), m_currentPointer(NULL), m_bufferSize(bufferSize), m_bufferPosition(0)
{
	m_isWritable = setWritable;
	m_isReadable = setReadable;
}

bool BufferStream::open()const
{
	m_currentPointer = const_cast<void*>(m_originalData);
	m_bufferPosition = 0;

	return m_currentPointer != 0;
}

void BufferStream::close()
{
	m_currentPointer = NULL;
	m_bufferPosition = 0;
}

bool BufferStream::read(size_t size, size_t count, void* const data, size_t& dataRead) const
{
	dataRead = 0;
	if (!m_isReadable)
	{
		assertion(0 ,  "Stream not readable");
		Log(Log.Error, "Attempted to read non readable stream");
		return false;
	}
	if (!data || !m_currentPointer)
	{
		return false;
	}
	byte* dataCurrent = (byte*)data;
	//Make sure we don't read too much
	for (size_t realcount = 0; realcount < count; ++realcount)
	{
		size_t realsize = (size_t)(std::min)(size, m_bufferSize - m_bufferPosition);
		memcpy(dataCurrent, m_currentPointer, realsize);

		m_bufferPosition += realsize;
		m_currentPointer = (void*)(((byte*)m_currentPointer) + realsize);
		dataCurrent += realsize;

		if (realsize == size)
		{
			++dataRead;
		}
	}
	if (dataRead != count)
	{
		if (m_bufferPosition == m_bufferSize)
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
	if (m_isWritable)
	{
		if (data && m_currentPointer)
		{
			byte* dataCurrent = (byte*)data;
			//Make sure we don't read too much
			for (size_t realcount = 0; realcount < count; ++realcount)
			{
				size_t realsize = (size_t)std::min<uint64>((uint64)size, m_bufferSize - m_bufferPosition);
				memcpy(m_currentPointer, dataCurrent, realsize);

				m_bufferPosition += realsize;
				m_currentPointer = (void*)(((byte*)m_currentPointer) + realsize);
				dataCurrent += realsize;

				if (realsize == size)
				{
					++dataWritten;
				}
			}
			if (dataWritten != count)
			{
				if (m_bufferPosition == m_bufferSize)
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

	if (!m_currentPointer || !m_originalData)
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
			newOffset = (long)glm::clamp<int64>((int64)offset, 0, (int64)m_bufferSize);

			m_bufferPosition = newOffset;
			m_currentPointer = (void*)((byte*)m_originalData + m_bufferPosition);
			break;
		}
		case Stream::SeekOriginFromCurrent:
		{
			int64 maxOffset = m_bufferSize - m_bufferPosition;
			int64 minOffset = -1 * (int64)m_bufferPosition;

			newOffset = glm::clamp<long>(offset, (long)minOffset, (long)maxOffset);

			m_bufferPosition += newOffset;
			m_currentPointer = (void*)((byte*)m_currentPointer + newOffset);
			break;
		}
		case Stream::SeekOriginFromEnd:
		{
			newOffset = (long)glm::clamp<int64>(offset, (long)(-1 * ((int64)m_bufferPosition)), 0);

			m_bufferPosition = m_bufferSize + newOffset;
			m_currentPointer = (void*)((byte*)m_originalData + m_bufferPosition);
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
	return m_currentPointer != NULL;
}

size_t BufferStream::getPosition() const
{
	return m_bufferPosition;
}

size_t BufferStream::getSize() const
{
	return m_bufferSize;
}
}
//!\endcond