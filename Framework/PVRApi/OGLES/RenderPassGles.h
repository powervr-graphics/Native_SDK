#pragma once
#include "PVRApi/ApiObjects/RenderPass.h"
namespace pvr{
namespace api{
namespace gles{
class RenderPassGles_ : public impl::RenderPass_
{
public:
	enum BindScope { BindBegin, BindEnd };

	RenderPassGles_(GraphicsContext& context) : impl::RenderPass_(context){}
	bool init(const RenderPassCreateParam& descriptor);

	void begin(IGraphicsContext& context, const api::Fbo& fbo, const pvr::Rectanglei& drawRegion, glm::vec4* clearColor,
			   pvr::uint32 numClearColor, pvr::float32 clearDepth, pvr::int32 clearStencil)const;

	void end(IGraphicsContext& context)const;

	const RenderPassCreateParam& getCreateParam()const{ return m_desc; }
private:
	RenderPassCreateParam m_desc;
};
}
typedef RefCountedResource<gles::RenderPassGles_> RenderPassGles;
}
}
