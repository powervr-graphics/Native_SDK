/*!
\brief Shader.h. Contains a base implementation for a Shader object.
\file PVRApi/ApiObjects/Shader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
namespace impl {

/// <summary>A shader API object.</summary>
class Shader_
{
protected:
	Shader_(const GraphicsContext& context) : _context(context) {}
	virtual ~Shader_() { }
	GraphicsContext _context;
public:
	/// <summary>Return const reference to the graphics context which owns this object</summary>
	const GraphicsContext& getContext()const { return _context; }
	
	/// <summary>Return reference to the graphics context which owns this object</summary>
	GraphicsContext& getContext() { return _context; }

};
}
typedef RefCountedResource<impl::Shader_> Shader;
}
}