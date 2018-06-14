/*!
\brief Implementation of the FileStream class.
\file PVRCore/IO/FileStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/IO/FileStream.h"
#include "PVRCore/Log.h"
using std::string;
namespace pvr {
FileStream::FileStream(const std::string& filePath, const std::string& flags, bool errorOnFileNotFound) : Stream(filePath), _file(NULL), _flags(flags), _errorOnFileNotFound(errorOnFileNotFound)
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

void FileStream::open() const
{
	if (_file) // If file exists, just reset it.
	{
		return seek(0, SeekOriginFromStart);
	}

	// open:
	if (_fileName.length() == 0 || _flags.length() == 0)
	{
		throw InvalidOperationError("[FileStream::open] Attempted to open a nonexistent file");
	}

#ifdef _WIN32
#ifdef _UNICODE
	errno_t error = _wfopen_s(&_file, _fileName.c_str(), _flags.c_str());
#else
	errno_t error = fopen_s(&_file, _fileName.c_str(), _flags.c_str());
#endif
	if (error != 0)
	{
		if (_errorOnFileNotFound)
		{
			throw FileNotFoundError(_fileName, "[FileStream::open] Failed to open file.");
		}
		else
		{
			_file = NULL;
			return;
		}
	}
#else
	_file = fopen(_fileName.c_str(), _flags.c_str());
#endif

	if (!_file)
	{
		if (_errorOnFileNotFound)
		{
			throw FileNotFoundError(_fileName, "[FileStream::open] Failed to open file.");
		}
		else
		{
			return;
		}
	}
}

void FileStream::close()
{
	if (_file && fclose(_file) == EOF)
	{
		Log(LogLevel::Warning, "[Filestream::close] Failure closing file.");
	}
	_file = 0;
}

void FileStream::read(size_t elementSize, size_t numElements, void* const outBuffer, size_t& outDataRead) const
{
	outDataRead = 0;
	if (!_file)
	{
		throw FileIOError(getFileName(), "[Filestream::read] Attempted to read empty stream.");
	}
	if (!_isReadable)
	{
		throw FileIOError(getFileName(), "[Filestream::read] Attempted to read non-readable stream.");
	}

	outDataRead = fread(outBuffer, elementSize, numElements, _file);
	if (outDataRead != numElements)
	{
		if (feof(_file) != 0)
		{
			Log(LogLevel::Debug, "[Filestream::read] Was attempting to read past the end of stream ");
		}
		else
		{
			throw FileIOError(getFileName(), "[Filestream::read] Unknown Error.");
		}
	}
}

void FileStream::write(size_t size, size_t count, const void* data, size_t& dataWritten)
{
	dataWritten = 0;
	if (!_file)
	{
		throw FileIOError(getFileName(), "[Filestream::read] Attempted to write an empty string.");
	}
	if (!_isWritable)
	{
		throw FileIOError(getFileName(), "[Filestream::read] Attempted to write a non-writable stream.");
	}
	dataWritten = fwrite(data, size, count, _file);
	if (dataWritten != count)
	{
		if (feof(_file) != 0)
		{
			Log(LogLevel::Debug, ("[Filestream::write]" + getFileName() + ": Attempted to write past the end of stream").c_str());
		}
		else
		{
			throw FileIOError(getFileName(), "[Filestream::write] Unknown error");
		}
	}
}

void FileStream::seek(long offset, SeekOrigin origin) const
{
	if (!_file)
	{
		if (offset)
		{
			throw FileIOError(getFileName(), "[FileStream::seek] Attempt to seek in empty stream.");
		}
	}
	else
	{
		if (fseek(_file, offset, static_cast<int>(origin)) != 0)
		{
			throw FileIOError(getFileName(), "[FileStream::seek] Attempt to seek  past the end of stream.");
		}
	}
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
} // namespace pvr
//!\endcond