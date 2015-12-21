/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Shader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Shader.h. Contains a base implementation for a Shader object.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/RefCounted.h"

namespace pvr {
namespace api {
namespace impl {

/*!*********************************************************************************************************************
\brief A shader API object.
***********************************************************************************************************************/
class ShaderImpl
{
protected:
	ShaderImpl() {}
};

}
typedef RefCountedResource<impl::ShaderImpl> Shader;
}
}
