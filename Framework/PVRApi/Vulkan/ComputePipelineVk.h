/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ComputePipelineVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Vulkan implementation of the all important GraphicsPipeline class
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {
class ComputePipelineImplVk : public impl::ComputePipelineImplBase, public native::HPipeline_
{
public:
	ComputePipelineCreateParam m_desc;
	vulkan::ContextVkWeakRef m_context;
	bool m_initialized;

	/*!
	 \brief Constructor
	 \param context Context who owns this object
	*/
	ComputePipelineImplVk(GraphicsContext context) : m_context(context), m_initialized(false) { }

	/*!
	 \brief Destructor
	*/
	~ComputePipelineImplVk() { destroy(); }

	/*!
	  \brief Destroy this object, Release its resources.
	 */
	void destroy();

	/*!
	   \brief Initialize this object
	   \param desc Initialise description
	   \return Return true on success.
	 */
	bool init(const ComputePipelineCreateParam& desc);

	/*!
	   \brief Get native object handle (const)
	 */
	const native::HPipeline_& getNativeObject() const { return *this; }

	/*!
	   \brief Get native object handle
	 */
	native::HPipeline_& getNativeObject() { return *this; }

	/*!
	   \brief Get this pipeline layout.
	 */
	const PipelineLayout& getPipelineLayout() const { return m_desc.pipelineLayout;  }


	///// Following functions are dummy
	void getUniformLocation(const char8**, uint32, int32*)
	{
		assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	}

	int32 getUniformLocation(const char8*)
	{
		assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
		return -1;
	}

	void bind() {}
};

}
}
}
//!\endcond
