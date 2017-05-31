<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\RenderPassGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES 2/3 implementation of the RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief OpenGL ES 2/3 implementation of the RenderPass.
\file PVRApi/OGLES/RenderPassGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiObjects/RenderPass.h"
namespace pvr{
namespace api{
namespace gles{
class RenderPassGles_ : public impl::RenderPass_
{
public:
	enum BindScope { BindBegin, BindEnd };

<<<<<<< HEAD
    /*!
       \brief Constructor
       \param context The context who owns this resource
     */
	RenderPassGles_(GraphicsContext& context) : impl::RenderPass_(context){}

    /*!
       \brief Initialise this renderpass
       \param Renderpass init param
       \return Return true if success.
     */
	bool init(const RenderPassCreateParam& descriptor);

    /*!
       \brief Begin this renderpass
       \param context
       \param fbo The rendering fbo
       \param drawRegion Draw area
       \param clearColor Clear color for each color attachments (rgba channels)
       \param numClearColor Number of clear colors in the array
       \param clearDepth clear depth value
       \param clearStencil clear stencil value
     */
	void begin(IGraphicsContext& context, const api::Fbo& fbo, const pvr::Rectanglei& drawRegion, glm::vec4* clearColor,
			   pvr::uint32 numClearColor, pvr::float32 clearDepth, pvr::int32 clearStencil)const;

    /*!
       \brief End this renderpass. begin must have been called prior to this function call.
       \param context
     */
	void end(IGraphicsContext& context)const;

    /*!
       \brief Destroy this renderpass, release its resources
     */
	void destroy();

    /*!
       \brief destructor
    */
=======
    /// <summary>Constructor</summary>
    /// <param name="context">The context who owns this resource</param>
	RenderPassGles_(const GraphicsContext& context) : impl::RenderPass_(context){}

    /// <summary>Initialise this renderpass</summary>
    /// <param name="Renderpass">init param</param>
    /// <returns>Return true if success.</returns>
	bool init(const RenderPassCreateParam& descriptor);

    /// <summary>Begin this renderpass</summary>
    /// <param name="context"></param>
    /// <param name="fbo">The rendering fbo</param>
    /// <param name="drawRegion">Draw area</param>
    /// <param name="clearColor">Clear color for each color attachments (rgba channels)</param>
    /// <param name="numClearColor">Number of clear colors in the array</param>
    /// <param name="clearDepth">clear depth value</param>
    /// <param name="clearStencil">clear stencil value</param>
	void begin(IGraphicsContext& context, const api::Fbo& fbo, const pvr::Rectanglei& drawRegion, glm::vec4* clearColor,
			   pvr::uint32 numClearColor, pvr::float32 clearDepth, pvr::int32 clearStencil)const;

    /// <summary>End this renderpass. begin must have been called prior to this function call.</summary>
    /// <param name="context"></param>
	void end(IGraphicsContext& context)const;

    /// <summary>Destroy this renderpass, release its resources</summary>
	void destroy();

    /// <summary>destructor</summary>
>>>>>>> 1776432f... 4.3
	~RenderPassGles_() { destroy(); }
	
	const RenderPassCreateParam& getCreateParam()const{ return _desc; }
private:
	RenderPassCreateParam _desc;
};
}
typedef RefCountedResource<gles::RenderPassGles_> RenderPassGles;
}
}
//!\endcond