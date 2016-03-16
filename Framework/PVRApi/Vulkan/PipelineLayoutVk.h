/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/PipelineLayoutVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the PipelineLayout class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object PipelineLayout to the underlying Vulkan PipelineLayout.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
namespace pvr{
namespace api{
namespace vulkan{
/*!*********************************************************************************************************************
\brief Vulkan implementation of the PipelineLayout class.
***********************************************************************************************************************/
class PipelineLayoutVk_ : public impl::PipelineLayout_, public native::HPipelineLayout_
{
public:
	/*!*********************************************************************************************************************
	\brief Initialize this PipelineLayout
	\param createParam PipelineLayout create parameters
	\return Return true on success, false in case of error
	***********************************************************************************************************************/
    bool init(const PipelineLayoutCreateParam &createParam);
	
	/*!*********************************************************************************************************************
	\brief ctor, Construct a PipelineLayout
	\param context The GraphicsContext this pipeline-layout will be constructed from.
	***********************************************************************************************************************/
    PipelineLayoutVk_(GraphicsContext& device) : PipelineLayout_(device){}
};
typedef RefCountedResource<PipelineLayoutVk_> PipelineLayoutVk;
}// namespace impl
}
}

PVR_DECLARE_NATIVE_CAST(PipelineLayout)