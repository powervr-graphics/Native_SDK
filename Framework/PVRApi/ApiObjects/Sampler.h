/*!*********************************************************************************************************************
\file         PVRApi/ApiObjects/Sampler.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the Sampler framework object.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
typedef types::SamplerCreateParam SamplerCreateParam;
class BindDescriptorSets;
namespace impl {

/*!****************************************************************************************************************
\brief The Sampler Framework API object. Wrapped in the pvr::api::Sampler as a Reference counted Framework object.
*******************************************************************************************************************/
class Sampler_
{
	Sampler_& operator=(const Sampler_&);//deleted
public:
	/*!****************************************************************************************************************
	\brief Destructor. Releases the object.
	*******************************************************************************************************************/
	virtual ~Sampler_() { destroy(); }

	/*!****************************************************************************************************************
	\brief Get const reference to the underlying API object of this sampler.
	*******************************************************************************************************************/
	const native::HSampler_& getNativeObject()const;

	/*!****************************************************************************************************************
	\brief Get reference to the underlying API object of this sampler.
	*******************************************************************************************************************/
	native::HSampler_& getNativeObject();

	/*!****************************************************************************************************************
	\brief Get the context who owns this object
	*******************************************************************************************************************/
	GraphicsContext& getContext(){ return m_context; }
protected:
	/*!****************************************************************************************************************
	\brief Create a new sampler object on device from the parameters.
	\param device The device on which to create the sampler on
	\param desc The parameters of the sampler
	*******************************************************************************************************************/
	Sampler_(GraphicsContext& device) : m_context(device){}
protected:
	void destroy();
	GraphicsContext m_context;
};
}// namespace impl
typedef RefCountedResource<impl::Sampler_> Sampler;
}// nampespace api
}// namespace pvr
