/*!*********************************************************************************************************************
\file         PVRApi/ApiObjects/Sampler.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the Sampler framework object.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/PipelineStateCreateParam.h"
#include "PVRAssets/SamplerDescription.h"
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRApi/Bindables.h"

namespace pvr {
namespace api {
class BindDescriptorSets;
namespace impl {

/*!****************************************************************************************************************
\brief The Sampler Framework API object. Wrapped in the pvr::api::Sampler as a Reference counted Framework object.
*******************************************************************************************************************/
class SamplerImpl : public IIndexBindable
{
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::api::impl::DescriptorSetGlesImpl;
	template<typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
	friend class ::pvr::IGraphicsContext;
	SamplerImpl& operator=(const SamplerImpl&);//deleted
	void bind(IGraphicsContext& context, uint32 index) const;
public:
	/*!****************************************************************************************************
	\brief Initialize with sampler create param.
	*******************************************************************************************************/
	void init(const assets::SamplerCreateParam& _desc) const;

	/*!****************************************************************************************************************
	\brief Create a new sampler object on device from the parameters.
	\param device The device on which to create the sampler on
	\param desc The parameters of the sampler
	*******************************************************************************************************************/
	SamplerImpl(pvr::RefCountedWeakReference<pvr::IGraphicsContext> device, const assets::SamplerCreateParam& desc) : m_desc(desc),
		m_context(device), m_initialised(false) {}

	/*!****************************************************************************************************************
	\brief Destructor. Releases the object.
	*******************************************************************************************************************/
	~SamplerImpl() { destroy(); }

	/*!****************************************************************************************************************
	\brief Query if the object is ready to use.
	*******************************************************************************************************************/
	bool isInitialized()const { return m_initialised; }

	/*!****************************************************************************************************************
	\brief Get the underlying API object of this sampler.
	*******************************************************************************************************************/
	const native::HSampler& getNativeHandle()const { return m_sampler; }
protected:
	void destroy();
	assets::SamplerCreateParam m_desc;//< required for ES2
	pvr::RefCountedWeakReference<pvr::IGraphicsContext> m_context;
	mutable native::HSampler m_sampler;
	mutable bool m_initialised;
};
}// namespace impl

}//nampespace api
}//namespace pvr
