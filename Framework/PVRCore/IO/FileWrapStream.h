/*!
\brief A stream that is created from an FileWrap object.
\file PVRCore/IO/FileWrapStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/IO/BufferStream.h"
#include "PVRCore/Base/SizedPointer.h"
#include <string>
#include <map>
namespace pvr {
/// <summary>Represents a stream that can be created from a file on which the FileWrap utility has been used to create a
/// header from.</summary>
/// <remarks>The FileWrap utility takes a data file and creates a global variable from its data. It automatically
/// registers the class so that it can then be accessed from this class.</remarks>
class FileWrapStream : public BufferStream
{
public:
	/// <summary>Construct a filewrap stream.</summary>
	/// <param name="fileName">The filename with which this resource was registered.</param>
	FileWrapStream(const std::string& fileName);
	~FileWrapStream() {}

	/// <summary>Get the registry of all known (registered) filewrapped entries.</summary>
	/// <returns>The registry of all known (registered) filewrapped entries.</returns>
	static std::map<std::string, SizedPointer<void> >& getFileRegistry()
	{
		static std::map <std::string, SizedPointer<void> > fileRegistry;
		return fileRegistry;
	}



	/// <summary>Automatically used by Filewrap files. They use this class in order to register themselves to the Filewrap
	/// utility. Uses a registry associating data with a resource identifier (filename)</summary>
	class Register
	{
	public:
		/// <summary>Constructor. Adds an entry to the globla file registry</summary>
		/// <param name="filename">The filename - or resource identifier - used to identify the stream</param>
		/// <param name="buffer">The data of the filewrapstream. Must be valid throughout the usage. No attempt made to manage</param>
		/// <param name="size">The amount of data contained in buffer</param>
		Register(const char* filename, const void* buffer, size_t size)
		{
			SizedPointer<void> ptr(const_cast<void*>(buffer), size);
			FileWrapStream::getFileRegistry().insert(std::pair<std::string, SizedPointer<void> >(filename, ptr));
		}
	};
};
}
