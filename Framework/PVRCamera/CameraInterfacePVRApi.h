/*!
\brief Function providing PVRApi textures over the textures provided by PVRCamera.
\file PVRCamera/CameraInterfacePVRApi.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Platform independent camera interface API include file.
*/

#include "PVRCamera/CameraInterface.h"
#include "PVRApi/Api.h"

namespace pvr
{
/// <summary>Function providing PVRApi textures over the textures provided by PVRCamera.</summary>
/// <param name="context">A GraphicsContext for the texture creation</param>
/// <param name="cameraTexture">The camera texture handle provided by PVRCamera</param>
/// <returns>A TextureView object wrapping the camera texture handle.</returns>
api::TextureView getTextureFromPVRCameraHandle(pvr::GraphicsContext& context, const native::HTexture_& cameraTexture);

/// <summary>Helper function providing a sampler suitable for sampling the Camera Texture</summary>
/// <param name="context"> A GraphicsContext for the sampler creation</param>
/// <returns> A sampler with Linear sampling, no mipmapping and Clamp to Edge wrapping mode.</returns>
api::Sampler getSamplerForCameraTexture(pvr::GraphicsContext& context)
{
	pvr::assets::SamplerCreateParam desc;
	desc.magnificationFilter = pvr::types::SamplerFilter::Linear;
	desc.minificationFilter = pvr::types::SamplerFilter::Linear;
	desc.mipMappingFilter = pvr::types::SamplerFilter::None;
	desc.wrapModeU = pvr::types::SamplerWrap::Clamp;
	desc.wrapModeV = pvr::types::SamplerWrap::Clamp;
	desc.wrapModeW = pvr::types::SamplerWrap::Clamp;
	return context->createSampler(desc);
}
}
