/*!*********************************************************************************************************************
\file         PVRCore\FileWrapStream.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         A stream that is created from an FileWrap object.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/BufferStream.h"
#include "PVRCore/SizedPointer.h"
#include <string>
#include <map>
namespace pvr
{
/*!*********************************************************************************************************************
\brief   Represents a stream that can be created from a file on which the FileWrap utility has been used to create a header from.
\description  The FileWrap utility takes a data file and creates a global variable from its data. It automatically registers
         the class so that it can then be accessed from this class.
***********************************************************************************************************************/
class FileWrapStream : public BufferStream
{
    public:
		/*!*********************************************************************************************************************
		\brief   Construct a filewrap stream.
		\param   fileName The filename with which this resource was registered.
		***********************************************************************************************************************/
        FileWrapStream(const std::string& fileName);
        ~FileWrapStream();

		/*!*********************************************************************************************************************
		\brief   Get the registry of all known (registered) filewrapped entries.
		\return  The registry of all known (registered) filewrapped entries.
		***********************************************************************************************************************/
		static std::map<std::string, SizedPointer<void> > & getFileRegistry();


		/*!*********************************************************************************************************************
		\brief   Automatically used by Filewrap files. They use this class in order to register themselves to the Filewrap utility.
		***********************************************************************************************************************/
        class Register
        {
            public:
                Register(const char* filename, const void * buffer, size_t size)
                {
					SizedPointer<void> ptr(const_cast<void*>(buffer), size);
                    FileWrapStream::getFileRegistry().insert(std::pair<std::string,SizedPointer<void> >(filename, ptr));
                }
        };
};
}

