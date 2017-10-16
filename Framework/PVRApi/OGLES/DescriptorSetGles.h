/*!
\brief Definition of the OpenGL ES implementation of the DescriptorSet and supporting classes
\file PVRApi/OGLES/DescriptorSetGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/SamplerGles.h"
#include "PVRApi/OGLES/TextureGles.h"
namespace pvr {
namespace api {
namespace gles {


/// <summary>ctor. OpenGL ES implementation of a DescriptorSet.</summary>
class DescriptorSetLayoutGles_ : public native::HDescriptorSetLayout_, public impl::DescriptorSetLayout_
{
public:
	/// <summary>ctor. Do not use directly, use context->createDescriptorSet.</summary>
	DescriptorSetLayoutGles_(const GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayout_(context, desc) {}

	/// <summary>Initialize this descriptor-set-layout</summary>
	/// <returns>Return true on success.</returns>
	bool init();

	/// <summary>Free all the resources held by this object</summary>
	void destroy();

	/// <summary>dtor.</summary>
	virtual ~DescriptorSetLayoutGles_();
};

/// <summary>OpenGL ES implementation of a DescriptorSet.</summary>
class DescriptorSetGles_ : public impl::DescriptorSet_, public native::HDescriptorSet_
{
public:
	/// <summary>ctor.</summary>
	DescriptorSetGles_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSet_(descSetLayout, pool) {}

	void bind(IGraphicsContext& device, uint32*& dynamicOffsets)const;

	/// <summary>Initialize this descriptor-set</summary>
	/// <returns>Return true on success</returns>
	bool init();

	/// <summary>Free all the resources held by this object.</summary>
	void destroy();

	/// <summary>dtor.</summary>
	virtual ~DescriptorSetGles_();
private:
	bool update_(const DescriptorSetUpdate& descSet);
};

class DescriptorPoolGles_ : public impl::DescriptorPool_, public native::HDescriptorPool_
{
public:
	DescriptorPoolGles_(const GraphicsContext& device) : DescriptorPool_(device) {}

	/// <summary>dtor</summary>
	virtual ~DescriptorPoolGles_();

	/// <summary>Initialize this descriptor-pool</summary>
	/// <returns>Return true on success</returns>
	bool init(const DescriptorPoolCreateParam& createParam);

	/// <summary>Destroy this descriptor-pool</summary>
	void destroy();

private:
	DescriptorSet allocateDescriptorSet_(const DescriptorSetLayout& layout);
	DescriptorPoolCreateParam _createParam;
};

typedef RefCountedResource<gles::DescriptorSetGles_> DescriptorSetGles;
typedef RefCountedResource<gles::DescriptorPoolGles_> DescriptorPoolGles;
typedef RefCountedResource<gles::DescriptorSetLayoutGles_> DescriptorSetLayoutGles;
}

}
}
PVR_DECLARE_NATIVE_CAST(DescriptorSet);
PVR_DECLARE_NATIVE_CAST(DescriptorPool);
