/*!*********************************************************************************************************************
\file         PVRApi\ApiCommand.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the ApiCommand interface used by commands that can be enqueued in a CommandBuffer.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiErrors.h"
namespace pvr {
namespace api {
/*!*********************************************************************************************************************
\brief Interface for Dynamic commands which can be queued into a CommandBuffer. In specific 
       implementations, contains a stacktrace for the actual submission of the command into the
       commandbuffer, which can greatly assist debugging.	   
***********************************************************************************************************************/
class ApiCommand
{
public:
#ifdef DEBUG
	std::string debug_commandCallSiteStackTrace;
#endif
	typedef void* isCommand;//!< Used in SFINAE dispatch of classes when submitting into the CommandBuffer.
	virtual ~ApiCommand() {}
	
	void execute(impl::CommandBufferBaseImpl& commandBuffer)
	{
		execute_private(commandBuffer);
#ifdef DEBUG
		debugLogApiError(("Error logged for API command. Stacktrace:\n" + debug_commandCallSiteStackTrace).c_str());
#endif
	}
private:
	virtual void execute_private(impl::CommandBufferBaseImpl&) = 0;
};
}
}
