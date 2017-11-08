/*!
\brief Contains a class used to abstract streams of data (files, blocks of memory, resources etc.).
\file PVRCore/Stream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
/// <summary>This class is used to abstract streams of data (files, blocks of memory, resources etc.). In general a
/// stream is considered something that can be read or written from. Specializations for many different types of
/// streams are provided by the PowerVR Framework, the most commonly used ones being Files and Memory. The common
/// interface and pointer types allow the Stream to abstract data in a very useful manner. Use the Stream::ptr_type
/// to pass abstract streams around (it is actually an std::unique_ptr)</summary>
class Stream
{
public:
	/// <summary>The pointer type of the stream. This is the main handle used to pass nullable streams around.</summary>
	typedef std::unique_ptr<Stream> ptr_type;
	/// <summary>When seeking, select if your offset should be considered to be from the Start of the stream, the
	/// Current point in the stream or the End of the stream.</summary>
	enum SeekOrigin
	{
		SeekOriginFromStart,
		SeekOriginFromCurrent,
		SeekOriginFromEnd
	};

	/// <summary>Destructor (virtual, this class can and should be used polymorphically).</summary>
	virtual ~Stream() {}

	/// <summary>Return true if this stream can be read from.</summary>
	/// <returns>True if this stream can be read from.</returns>
	bool isReadable() const { return _isReadable; }

	/// <summary>Return true if this stream can be written from.</summary>
	/// <returns>True if this stream can be written to.</returns>
	bool isWritable() const { return _isWritable; }

	/// <summary>Get the filename of the file that this std::string represents, if such exists. Otherwise, empty std::string.
	/// </summary>
	/// <returns>The filename of the file that this std::string represents, if such exists. Otherwise, empty std::string.
	/// </returns>
	const std::string& getFileName() const { return _fileName; }

public:
	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="numElements">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const = 0;

	/// <summary>Main write function. Write into the stream the specified amount of items from a provided buffer.
	/// </summary>
	/// <param name="elementSize">The size of each element that will be written.</param>
	/// <param name="numElements">The number of elements to write.</param>
	/// <param name="buffer">The buffer from which to read the data. If the buffer is smaller than elementSize *
	/// numElements bytes, result is undefined.</param>
	/// <param name="dataWritten">After returning, will contain the number of items that were actually written. Will
	/// contain numElements unless an error has occured.</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten) = 0;

	/// <summary>Seek a specific point for random access streams. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin"</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position</param>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool seek(long offset, SeekOrigin origin) const = 0;

	/// <summary>Prepares the stream for read / write / seek operations.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual bool open()const = 0;

	/// <summary>Closes the stream.</summary>
	/// <returns>Success if successful, error code otherwise.</returns>
	virtual void close() = 0;

	/// <summary>Return true if the stream is open and ready for other operations.</summary>
	/// <returns>True if the stream is open and ready for other operations.</returns>
	virtual bool isopen() const = 0;

	/// <summary>If supported, returns the current position in the stream.</summary>
	/// <returns>If suppored, returns the current position in the stream. Otherwise, returns 0.</returns>
	virtual size_t getPosition() const = 0;

	/// <summary>If supported, returns the total size of the stream.</summary>
	/// <returns>If suppored, returns the total amount of data in the stream. Otherwise, returns 0.</returns>
	virtual size_t getSize() const = 0;

	/// <summary>Convenience functions that reads all data in the stream into a contiguous block of memory of a specified
	/// element type. Requires random-access stream.</summary>
	/// <typeparam name="Type_">The type of item that will be read into.</typeparam>
	/// <returns>A std::vector<Type_> containing all data from the current point to the end of the stream.
	/// </returns>
	template<typename Type_> std::vector<Type_> readToEnd() const
	{
		std::vector<Type_> ret;
		if (!open())
		{
			return ret;
		}
		size_t mySize = getSize() - getPosition();
		size_t myElements = mySize / sizeof(Type_);
		ret.resize(mySize);
		size_t actuallyRead;
		read(sizeof(Type_), myElements, ret.data(), actuallyRead);
		return ret;
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <param name="outString">A std::vector<char> that will contain all data in the stream.</param>
	/// <returns>true if successful, false otherwise.</returns>
	bool readIntoCharBuffer(std::vector<char>& outString) const
	{
		if (!isopen()) { return false; }
		outString.resize(getSize() + 1);

		size_t dataRead;
		return read(1, getSize(), outString.data(), dataRead);
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <param name="outString">A std::vector<char> that will contain all data in the stream.</param>
	/// <returns>true if successful, false otherwise.</returns>
	template<typename T_>
	bool readIntoBuffer(std::vector<T_>& outString) const
	{
		if (!isopen()) { return false; }
		size_t sz = getSize();
		assertion(sz < (std::numeric_limits<size_t>::max)());
		size_t initial_size = outString.size();
		outString.resize(initial_size + getSize());

		size_t dataRead;
		return read(sizeof(T_), getSize(), outString.data() + initial_size, dataRead);
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <returns>A std::vector<char> that will contain all data in the stream.</returns>
	std::vector<char> readChars() const
	{
		std::vector<char> pData;
		readIntoCharBuffer(pData);
		return pData;
	}

	/// <summary>Convenience function that reads all data in the stream into a std::string</summary>
	/// <param name="outString">The string where the stream's data will all be saved</param>
	/// <returns>A std::string that will contain all data in the stream.</returns>
	bool readIntoString(std::string& outString) const
	{
		std::vector<char> pData;
		bool rslt = readIntoCharBuffer(pData);
		outString.clear();
		outString.append(pData.data());
		return rslt;
	}

protected:
	/// <summary>Constructor. Open a stream to a new filename. Must be overriden as it only sets the filename.</summary>
	/// <param name="fileName">Commmonly the filename, but may be any type of resource identifier (such as Windows
	/// Embedded Resource id)</param>
	Stream(const std::string& fileName) : _isReadable(false), _isWritable(false), _fileName(fileName) {}
	bool _isReadable; ///< True if the stream can be read
	bool _isWritable; ///< True if the stream can be written
	std::string _fileName; ///< The filename (conceptually, a resource identifier as there may be other sources for the stream)

private:
	// Disable copying and assign.
	void operator=(const Stream&);
	Stream(const Stream&);
};
}
