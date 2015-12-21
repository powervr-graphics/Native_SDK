/*!******************************************************************************************************************
\file         PVRCamera\CameraInterfacePVRApi.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Platform independent camera interface API include file.
\brief         Function providing PVRApi textures over the textures provided by PVRCamera.
********************************************************************************************************************/

#include "PVRCamera/CameraInterface.h"
#include "PVRApi/Api.h"

namespace pvr
{
	/*!******************************************************************************************************************
	\brief         Function providing PVRApi textures over the textures provided by PVRCamera.
	\param context A GraphicsContext for the texture creation
	\param cameraTexture The camera texture handle provided by PVRCamera
	\return A TextureView object wrapping the camera texture handle.
	********************************************************************************************************************/
	api::TextureView getTextureFromPVRCameraHandle(pvr::GraphicsContext& context, const native::HTexture_& cameraTexture);
}
