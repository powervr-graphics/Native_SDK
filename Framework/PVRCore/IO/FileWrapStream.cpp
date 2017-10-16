/*!
\brief Implementation of the FileWrapStream class methods.
\file PVRCore/FileWrapStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/IO/FileWrapStream.h"
using std::string;
using std::map;
namespace pvr
{
std::map<std::string, SizedPointer<void> >& FileWrapStream::getFileRegistry()
{
	static std::map <std::string, SizedPointer<void> > fileRegistry;
	return fileRegistry;
}

FileWrapStream::FileWrapStream(const string& fileName) : BufferStream(fileName)
{
	_isReadable = true;
	map<string, SizedPointer<void> >::iterator found = getFileRegistry().find(fileName);

	if (found != FileWrapStream::getFileRegistry().end())
	{
		_originalData = found->second;
		_bufferSize = found->second.getSize();
	}
	else
	{
		_originalData = NULL;
	}
}

FileWrapStream::~FileWrapStream()
{
}
}
//!\endcond