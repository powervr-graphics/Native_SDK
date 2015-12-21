/*!*********************************************************************************************************************
\file         PVRApi/OGLES/CommandBuffer.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation details CommandBuffer class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/CommandBuffer.h"

namespace pvr {
namespace api {
namespace impl {
void CommandBufferBaseImpl::beginRecording()
{
	if (m_isRecording)
	{
		PVR_ASSERT(!m_isRecording);
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
	}
	clear();
	m_isRecording = true;
}

void CommandBufferBaseImpl::endRecording()
{
	if (!m_isRecording)
	{
		PVR_ASSERT(!m_isRecording);
		Log("Called CommandBuffer::endRecording while a recording was already in progress. Call CommandBuffer::beginRecording first");
	}
	m_isRecording = false;
}

}
}
}
//!\endcond
