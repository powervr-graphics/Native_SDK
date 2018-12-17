/*!
\brief Streams that are created from files.
\file PVRCore/stream/FileStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"

namespace pvr {
/// <summary>A FileStream is a Stream that is used to access a File in the filesystem of the platform.</summary>
class FileStream : public Stream
{
public:
	/// <summary>Create a new filestream of a specified file.</summary>
	/// <param name="filePath">The path of the file. Can be in any format the operating system understands (absolute,
	/// relative etc.)</param>
	/// <param name="flags">fopen-style flags.</param>
	/// <param name="errorOnFileNotFound">OPTIONAL. Set this to false to avoid an error when the file is not found.</param>
	/// <remarks>Possible flags: 'r':read,'w':truncate/write, 'a':append/write, r+: read/write, w+:truncate/read/write,
	/// a+:append/read/write</remarks>
	FileStream(const std::string& filePath, const std::string& flags, bool errorOnFileNotFound = true)
		: Stream(filePath), _file(NULL), _flags(flags), _errorOnFileNotFound(errorOnFileNotFound)
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

	~FileStream()
	{
		close();
	}

	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="numElements">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read.</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const
	{
		dataRead = 0;
		if (!_file)
		{
			throw FileIOError(getFileName(), "[Filestream::read] Attempted to read empty stream.");
		}
		if (!_isReadable)
		{
			throw FileIOError(getFileName(), "[Filestream::read] Attempted to read non-readable stream.");
		}

		dataRead = fread(buffer, elementSize, numElements, _file);
		if (dataRead != numElements)
		{
			if (feof(_file) != 0)
			{
				throw FileIOError(getFileName(), "[Filestream::read] Was attempting to read past the end of stream.");
			}
			else
			{
				throw FileIOError(getFileName(), "[Filestream::read] Unknown Error.");
			}
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
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten)
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
		dataWritten = fwrite(buffer, elementSize, numElements, _file);
		if (dataWritten != numElements)
		{
			if (feof(_file) != 0)
			{
				throw FileIOError(getFileName(), "[Filestream::write] Was attempting to write past the end of stream.");
			}
			else
			{
				throw FileIOError(getFileName(), "[Filestream::write] Unknown error");
			}
		}
	}

	/// <summary>Seek a specific point for random access streams. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin".</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position.</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void seek(long offset, SeekOrigin origin) const
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

	/// <summary>Prepares the stream for read / write / seek operations.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void open() const
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

	/// <summary>Closes the stream.</summary>
	virtual void close()
	{
		if (_file && fclose(_file) == EOF)
		{
			throw FileIOError(getFileName(), "[FileStream::close] Failure closing file.");
		}
		_file = 0;
	}

	/// <summary>Query if the stream is open</summary>
	/// <returns>True if the stream is open and ready for other operations.</returns>
	virtual bool isopen() const
	{
		return _file != NULL;
	}

	/// <summary>Query the current position in the stream</summary>
	/// <returns>The current position in the stream.</returns>
	virtual size_t getPosition() const
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

	/// <summary>Query the total amount of data in the stream.</summary>
	/// <returns>The total amount of data in the stream.</returns>
	virtual size_t getSize() const
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

	/// <summary>Create a new file stream from a filename</summary>
	/// <param name="filename">The filename to create a stream for</param>
	/// <param name="flags">The C++ open flags for the file ("r", "w", "a", "r+", "w+", "a+" and optionally "b"), see
	/// C++ standard.</param>
	/// <param name="errorOnFileNotFound">OPTIONAL. Set this to false to avoid an error when the file is not found.</param>
	/// <returns>Return a valid file stream, else Return null if it fails</returns>
	/// <remarks>Flags correspond the C++ standard for fopen r: Read (read from beginning, preserve contents, fail if
	/// not exist) w: Write (write from beginning, destroy contents, create if not exist) a: Append (write to end only
	/// (regardles of file pointer position), preserve contents, create if not exist) r+: Read+ (read/write from
	/// start, preserve contents, fail if not exist) w+: Write+ (read/write from start, destroy contents, create if
	/// not exist) a+: Append+ (read/write from end (write only happens to end), preserve contents, create if not
	/// exist) b: Additional flag: Do no special handling of line break / form feed characters</remarks>
	static Stream::ptr_type createFileStream(const char* filename, const char* flags, bool errorOnFileNotFound = true)
	{
		Stream::ptr_type stream(new FileStream(filename, flags, errorOnFileNotFound));
		stream->open();
		return stream;
	}

protected:
	/// <summary>The underlying C FILE object</summary>
	mutable FILE* _file;
	/// <summary>The C flags used when opening the file</summary>
	std::string _flags;
	/// <summary>True to error when files are not found otherwise False to avoid an error when the file is not found.</summary>
	bool _errorOnFileNotFound;
};

} // namespace pvr
