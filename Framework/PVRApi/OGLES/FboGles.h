/*!*********************************************************************************************************************
\file         PVRApi\OGLES\FboGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementation of the PVRApi Default Fbo (Frame Buffer Object).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace api {
namespace gles {

class FboGles_ : public impl::Fbo_, public  native::HFbo_
{
public:
	FboGles_(GraphicsContext& context);
	virtual void bind(IGraphicsContext& m_context, types::FboBindingTarget types)const;
	void destroy();
	const native::HFbo_&  getNativeObject() const;
	native::HFbo_& getNativeObject();
	bool init(const FboCreateParam& desc);
	bool checkFboStatus();
	const RenderPass& getRenderPass()const { return m_desc.renderPass; }
	RenderPass& getRenderPass() { return m_desc.renderPass; }

	virtual ~FboGles_() {}

	mutable types::FboBindingTarget m_target;
	std::vector<TextureView> m_colorAttachments;
	std::vector<TextureView> m_depthStencilAttachment;
};

/*!*********************************************************************************************************************
\brief OpenGL ES Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen.
       Should be used through the Fbo object. If a GLES direct manipulation is required, use through the DefaultFboGles Reference
	   counted Framework object.
***********************************************************************************************************************/
class DefaultFboGles_ : public FboGles_
{
public:
	/*!*********************************************************************************************************************
	\brief  Constructor. Construct a new FBO on the provided context.
	***********************************************************************************************************************/
	DefaultFboGles_(GraphicsContext& context);

	/*!*********************************************************************************************************************
	\brief INTERNAL OGLES: Initialize this fbo with provided parameters.
	\return pvr::Result::Success on success
	***********************************************************************************************************************/
	bool init(const FboCreateParam& desc)
	{
		m_desc = desc;
		handle = 0;
		return true;
	}

	//\brief INTERNAL OGLES: Return true if this is a default fbo (overload).
	bool isDefault()const { return true; }

	//\brief INTERNAL OGLES: Bind this fbo.
	//\param context Bind on this context
	//\param target Bind on this target
	void bind(IGraphicsContext& context, types::FboBindingTarget target)const;

	//\brief INTERNALCheck the status of this fbo.
	//\return return true on success
	bool checkFboStatus();
};
//OpenGL ES Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen. Reference counted.
typedef RefCountedResource<DefaultFboGles_> DefaultFboGles;
typedef RefCountedResource<FboGles_> FboGles;
}
}
}
//!\endcond