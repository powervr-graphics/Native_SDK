/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Shader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Shader.h. Contains a base implementation for a Shader object.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
namespace impl {

/*!*********************************************************************************************************************
\brief A shader API object.
***********************************************************************************************************************/
class Shader_
{
protected:
	Shader_(GraphicsContext& context) : m_context(context) {}
	virtual ~Shader_() { }
	GraphicsContext m_context;
public:
	/*!*********************************************************************************************************************
	\brief Return const reference to the graphics context which owns this object
	***********************************************************************************************************************/
	const GraphicsContext& getContext()const { return m_context; }
	
	/*!*********************************************************************************************************************
	\brief Return reference to the graphics context which owns this object
	***********************************************************************************************************************/
	GraphicsContext& getContext() { return m_context; }
	
	/*!*********************************************************************************************************************
	\brief Return reference to native Object
	***********************************************************************************************************************/
	native::HShader_& getNativeObject();
	
	/*!*********************************************************************************************************************
	\brief Return const reference to native Object
	***********************************************************************************************************************/
	const native::HShader_& getNativeObject() const;
};
}
typedef RefCountedResource<impl::Shader_> Shader;
}
}
