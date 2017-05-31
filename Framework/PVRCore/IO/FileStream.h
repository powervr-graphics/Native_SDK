/*!
\brief Streams that are created from files.
\file PVRCore/IO/FileStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Stream.h"

namespace pvr {
/// <summary>A FileStream is a Stream that is used to access a File in the filesystem of the platform.
/// </summary>
class FileStream : public Stream
{
public:
	/// <summary>Create a new filestream of a specified file.</summary>
	/// <param name="filePath">The path of the file. Can be in any format the operating system understands (absolute,
	/// relative etc.)</param>
	/// <param name="flags">fopen-style flags.</param>
	/// <remarks>Possible flags: 'r':read,'w':truncate/write, 'a':append/write, r+: read/write, w+:truncate/read/write,
	/// a+:append/read/write</remarks>
	FileStream(const std::basic_string<char8>& filePath, const std::basic_string<char8>& flags);
	~FileStream() { close(); }

	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="elementCount">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read.</param>
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
	/// <param name="offset">The offset to seec from "origin".</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position.</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool seek(long offset, SeekOrigin origin) const;

	/// <summary>Prepares the stream for read / write / seek operations.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool open()const;

	/// <summary>Closes the stream.</summary>
	virtual void close();

	/// <summary>Query if the stream is open</summary>
	/// <returns>True if the stream is open and ready for other operations.</returns>
	virtual bool isopen() const;

	/// <summary>Query the current position in the stream</summary>
	/// <returns>The current position in the stream.</returns>
	virtual size_t getPosition() const;

	/// <summary>Query the total amount of data in the stream.</summary>
	/// <returns>The total amount of data in the stream.</returns>
	virtual size_t getSize() const;

	/// <summary>Create a new file stream from a filename</summary>
	/// <param name="filename">The filename to create a stream for</param>
	/// <param name="flags">The C++ open flags for the file ("r", "w", "a", "r+", "w+", "a+" and optionally "b"), see
	/// C++ standard.</param>
	/// <returns>Return a valid file stream, else Return null if it fails</returns>
	/// <remarks>Flags correspond the C++ standard for fopen r: Read (read from beginning, preserve contents, fail if
	/// not exist) w: Write (write from beginning, destroy contents, create if not exist) a: Append (write to end only
	/// (regardles of file pointer position), preserve contents, create if not exist) r+: Read+ (read/write from
	/// start, preserve contents, fail if not exist) w+: Write+ (read/write from start, destroy contents, create if
	/// not exist) a+: Append+ (read/write from end (write only happens to end), preserve contents, create if not
	/// exist) b: Additional flag: Do no special handling of line break / form feed characters</remarks>
	static Stream::ptr_type createFileStream(const char* filename, const char* flags)
	{
		Stream::ptr_type stream(new FileStream(filename, flags));
		if (!stream->open()) { stream.reset(); }
		return stream;
	}

protected:
	mutable FILE* _file;
	std::string _flags;
};

}