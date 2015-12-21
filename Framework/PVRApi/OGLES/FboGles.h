/*!*********************************************************************************************************************
\file         PVRApi\OGLES\FboGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementation of the PVRApi Default Fbo (Frame Buffer Object).
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace api {
namespace impl {
/*!*********************************************************************************************************************
\brief OpenGL ES Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen.
       Should be used through the Fbo object. If a GLES direct manipulation is required, use through the DefaultFboGles Reference 
	   counted Framework object.
***********************************************************************************************************************/
class DefaultFboGlesImpl : public FboImpl
{
public:
	/*!*********************************************************************************************************************
	\brief  Constructor. Construct a new FBO on the provided context.
	***********************************************************************************************************************/
	DefaultFboGlesImpl(GraphicsContext& context);

	/*!*********************************************************************************************************************
	\brief INTERNAL OGLES: Initialize this fbo with provided parameters.
	\return pvr::Result::Success on success
	***********************************************************************************************************************/
	virtual Result::Enum init(const FboCreateParam& desc){ 
		m_desc = desc;	
		m_fbo.construct(0);
		return pvr::Result::Success;
	}

	//\brief INTERNAL OGLES: Return true if this is a default fbo (overload).
	bool isDefault()const { return true; }

	//\brief INTERNAL OGLES: Bind this fbo.
	//\param context Bind on this context
	//\param target Bind on this target
	void bind(IGraphicsContext& context, api::FboBindingTarget::Enum target)const;

	//\brief INTERNALCheck the status of this fbo.
	//\return return true on success
	bool checkFboStatus();
};
}

//OpenGL ES Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen. Reference counted.
typedef RefCountedResource<impl::DefaultFboGlesImpl> DefaultFboGles;
}
}
