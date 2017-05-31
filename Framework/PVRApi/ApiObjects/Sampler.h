/*!
\brief Contains the Sampler framework object.
\file PVRApi/ApiObjects/Sampler.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
typedef types::SamplerCreateParam SamplerCreateParam;
namespace impl {

/// <summary>The Sampler Framework API object. Wrapped in the pvr::api::Sampler as a Reference counted Framework
/// object.</summary>
class Sampler_
{
	Sampler_& operator=(const Sampler_&);//deleted
public:
	/// <summary>Destructor. Releases the object.</summary>
	virtual ~Sampler_() { }

	/// <summary>Get the context who owns this object</summary>
	GraphicsContext& getContext() { return _context; }
protected:
	/// <summary>Create a new sampler object on device from the parameters.</summary>
	/// <param name="device">The device on which to create the sampler on</param>
	Sampler_(const GraphicsContext& device) : _context(device) {}
protected:
	GraphicsContext _context;
};
}// namespace impl
typedef RefCountedResource<impl::Sampler_> Sampler;
}// nampespace api
}// namespace pvr