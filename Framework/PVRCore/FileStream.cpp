/*!*********************************************************************************************************************
\file         PVRCore\FileStream.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the FileStream class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/FileStream.h"
#include "PVRCore/Log.h"
using std::string;
namespace pvr {
FileStream::FileStream(const string& filePath, const string& flags)
	: Stream(filePath), m_file(NULL), m_flags(flags)
{
	if (m_flags.find('r') != m_flags.npos || m_flags.find('+') != m_flags.npos)
	{
		m_isReadable = true;
	}

	if (m_flags.find('w') != m_flags.npos || m_flags.find('a') != m_flags.npos || m_flags.find('+') != m_flags.npos)
	{
		m_isWritable = true;
	}
}

bool FileStream::open()const
{
	if (!m_file)
	{
		if (m_fileName.length() == 0 || m_flags.length() == 0)
		{
			return false;
		}
		else
		{
#ifdef _WIN32
#ifdef _UNICODE
			errno_t error = _wfopen_s(&m_file, m_fileName.c_str(), m_flags.c_str());
#else
			errno_t error = fopen_s(&m_file, m_fileName.c_str(), m_flags.c_str());
#endif
			if (error != 0)
			{
				return false;
			}
#else
			m_file = fopen(m_fileName.c_str(), m_flags.c_str());
#endif

			if (!m_file)
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return seek(0, SeekOriginFromStart);
	}
}

void FileStream::close()
{
	if (m_file && fclose(m_file) == EOF)
	{
		Log(Log.Warning, "[Filestream::close] Failure closing file.");
	}
	m_file = 0;
}

bool FileStream::read(size_t elementSize, size_t elementCount, void* const outBuffer, size_t& outDataRead) const
{
	bool result = true;
	outDataRead = 0;
	if (m_file)
	{
		if (m_isReadable)
		{
			outDataRead = fread(outBuffer, elementSize, elementCount, m_file);
			if (outDataRead != elementCount)
			{
				if (feof(m_file) != 0)
				{
					Log(Log.Debug, "[Filestream::read] Was attempting to read past the end of stream ");
					result = true;
				}
				else
				{
					Log("[Filestream::read] Unknown Error.");
					assertion(false, "[Filestream::read] Unknown Error.");
					result = false;
				}
			}
		}
		else
		{
			Log("[Filestream::read] Attempted to read non-readable stream.");
			assertion(false, "[Filestream::read] Attempted to read non-readable stream.");
			result = false;
		}
	}
	else
	{
		Log("[Filestream::read] Attempted to read empty stream.");
		assertion(false, "[Filestream::read] Attempted to read empty stream.");
		result = false;
	}

	return result;
}

bool FileStream::write(size_t size, size_t count, const void* data, size_t& dataWritten)
{
	bool result = true;
	dataWritten = 0;
	if (m_file)
	{
		if (m_isWritable)
		{
			dataWritten = fwrite(data, size, count, m_file);
			if (dataWritten != count)
			{
				if (feof(m_file) != 0)
				{
					Log(Log.Debug, "[Filestream::read] Was attempting to write past the end of stream ");
					result = false;
				}
				else
				{
					Log("[Filestream::read] Unknown Error.");
					assertion(false, "[Filestream::read] Unknown Error.");
					result = false;
				}
			}
		}
		else
		{
			Log("[Filestream::read] Attempted to write a non-writable stream.");
			assertion(false, "[Filestream::read] Attempted to write a non-writable stream.");
			result = false;
		}
	}
	else
	{
		Log("[Filestream::read] Attempted to write an empty stream.");
		assertion(false, "[Filestream::read] Attempted to write an empty stream.");
		result = false;
	}
	return result;
}

bool FileStream::seek(long offset, SeekOrigin origin) const
{
	bool result = true;

	if (!m_file)
	{
		if (offset)
		{
			Log(Log.Error, "[FileStream::seek] Attempt to seek from empty stream");
			result = false;
		}

	}
	else
	{
		if (fseek(m_file, offset, static_cast<int>(origin)) != 0)
		{
			Log(Log.Debug, "[Filestream::read] Was attempting to seek past the end of stream ");
			result = false;
		}
	}

	return result;
}

bool FileStream::isopen() const
{
	return m_file != NULL;
}

size_t FileStream::getPosition() const
{
	if (m_file)
	{
		return static_cast<size_t>(ftell(m_file));
	}
	else
	{
		return 0;
	}
}

size_t FileStream::getSize() const
{
	if (m_file)
	{
		long originalPosition = ftell(m_file);
		fseek(m_file, 0, SeekOriginFromEnd);
		long fileSize = ftell(m_file);
		fseek(m_file, originalPosition, SeekOriginFromStart);

		return static_cast<size_t>(fileSize);
	}
	else
	{
		return 0;
	}
}
}
//!\endcond