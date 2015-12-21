/*!*********************************************************************************************************************
\file         PVRCore\FileWrapStream.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the FileWrapStream class methods.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/FileWrapStream.h"
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
	m_isReadable = true;
	map<string, SizedPointer<void> >::iterator found = getFileRegistry().find(fileName);

	if (found != FileWrapStream::getFileRegistry().end())
	{
		m_originalData = found->second;
		m_bufferSize = found->second.getSize();
	}
	else
	{
		m_originalData = NULL;
	}
}

FileWrapStream::~FileWrapStream()
{
}
}
//!\endcond