/*!
\brief Implementation of the FileStream class.
\file PVRCore/FileStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/IO/FileStream.h"
#include "PVRCore/Log.h"
using std::string;
namespace pvr {
FileStream::FileStream(const string& filePath, const string& flags)
	: Stream(filePath), _file(NULL), _flags(flags)
{
	if (_flags.find('r') != _flags.npos || _flags.find('+') != _flags.npos)
	{
		_isReadable = true;
	}

	if (_flags.find('w') != _flags.npos || _flags.find('a') != _flags.npos || _flags.find('+') != _flags.npos)
	{
		_isWritable = true;
	}
}

bool FileStream::open()const
{
	if (!_file)
	{
		if (_fileName.length() == 0 || _flags.length() == 0)
		{
			return false;
		}
		else
		{
#ifdef _WIN32
#ifdef _UNICODE
			errno_t error = _wfopen_s(&_file, _fileName.c_str(), _flags.c_str());
#else
			errno_t error = fopen_s(&_file, _fileName.c_str(), _flags.c_str());
#endif
			if (error != 0)
			{
				return false;
			}
#else
			_file = fopen(_fileName.c_str(), _flags.c_str());
#endif

			if (!_file)
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
	if (_file && fclose(_file) == EOF)
	{
		Log(Log.Warning, "[Filestream::close] Failure closing file.");
	}
	_file = 0;
}

bool FileStream::read(size_t elementSize, size_t elementCount, void* const outBuffer, size_t& outDataRead) const
{
	bool result = true;
	outDataRead = 0;
	if (_file)
	{
		if (_isReadable)
		{
			outDataRead = fread(outBuffer, elementSize, elementCount, _file);
			if (outDataRead != elementCount)
			{
				if (feof(_file) != 0)
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
	if (_file)
	{
		if (_isWritable)
		{
			dataWritten = fwrite(data, size, count, _file);
			if (dataWritten != count)
			{
				if (feof(_file) != 0)
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

	if (!_file)
	{
		if (offset)
		{
			Log(Log.Error, "[FileStream::seek] Attempt to seek from empty stream");
			result = false;
		}

	}
	else
	{
		if (fseek(_file, offset, static_cast<int>(origin)) != 0)
		{
			Log(Log.Debug, "[Filestream::read] Was attempting to seek past the end of stream ");
			result = false;
		}
	}

	return result;
}

bool FileStream::isopen() const
{
	return _file != NULL;
}

size_t FileStream::getPosition() const
{
	if (_file)
	{
		return static_cast<size_t>(ftell(_file));
	}
	else
	{
		return 0;
	}
}

size_t FileStream::getSize() const
{
	if (_file)
	{
		long originalPosition = ftell(_file);
		fseek(_file, 0, SeekOriginFromEnd);
		long fileSize = ftell(_file);
		fseek(_file, originalPosition, SeekOriginFromStart);

		return static_cast<size_t>(fileSize);
	}
	else
	{
		return 0;
	}
}
}
//!\endcond