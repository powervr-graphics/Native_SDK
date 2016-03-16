/*!****************************************************************************************************************
\file         PVRCamera/CameraInterface.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Common interfaceof the PVRCamera camera streaming interface.
******************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"
/*!****************************************************************************************************************
\brief Main PowerVR Namespace
******************************************************************************************************************/
namespace pvr {
/*!****************************************************************************************************************
\brief Enumeration of the possible hardware cameras present (front, back)
******************************************************************************************************************/
namespace HWCamera {
enum Enum
{
	Front,
	Back,
};
};
/*!****************************************************************************************************************
\brief Contains objects representing the low-level native API objects. For PVRCamera, that is the Texture handle
******************************************************************************************************************/
namespace native {
/*!****************************************************************************************************************
\brief A struct wrapping the native API Texture handle.
******************************************************************************************************************/
struct HTexture_;
}
/*!****************************************************************************************************************
\brief A class design to provide you with a Texture handle to the Camera's image.
******************************************************************************************************************/
class CameraInterface
{
public:
	/*!****************************************************************************************************************
	\brief Constructor
	******************************************************************************************************************/
	CameraInterface();
	
	/*!****************************************************************************************************************
	\brief Destructor
	******************************************************************************************************************/
	~CameraInterface(); 

	/*!****************************************************************************************************************
	\brief      Initializes the capture session using the given hardware camera, if it is available.
	\param[in]  eCamera       The hardware camera to attempt to stream from
	\param[in]  preferredResX,preferredResY If supported by the implementation, set a preferred resolution
	\return     true if successful
	******************************************************************************************************************/
	bool initializeSession(HWCamera::Enum eCamera, int preferredResX = 0, int preferredResY = 0);

	/*!****************************************************************************************************************
	\brief      		Shutdown the AV capture session and release associated objects.
	******************************************************************************************************************/
	void destroySession();

	/*!****************************************************************************************************************
	\brief      Checks to see if the image has been updated.
	******************************************************************************************************************/
	bool updateImage();

	/*!****************************************************************************************************************
	\brief      Checks to see if the projection matrix has changed.
	******************************************************************************************************************/
	bool hasProjectionMatrixChanged();

	/*!****************************************************************************************************************
	\brief      		Retrieves the current texture projection matrix and resets
	the 'changed' flag.
	\return            pointer to 16 float values representing the matrix
	******************************************************************************************************************/
	const glm::mat4& getProjectionMatrix();

	/*!****************************************************************************************************************
	\brief      Retrieves the texture name for the YUV camera texture.
	\return     A native API handle that can be used to get the texture.
	******************************************************************************************************************/
	const native::HTexture_& getRgbTexture();

	/*!****************************************************************************************************************
	\brief    Query if this implementation supports a single RGB texture for the camera streaming interface.
	\return   True if the implementation supports an RGB texture, false otherwise 
	\details  This function will return true if the getRgbTexture() can be used.  In implementations where this 
			  is not supported (e.g. iOS), this function will return false, and the getRgbTexture() function will
			  return an empty (invalid) texture if used. See hasLumaChromaTextures(). In implementations where 
			  RGB textures are supported (e.g. Android)  this function will return true and the getRgbTexture()
			  will return a valid texture handle (if called after this interface was successfully initialized).
	******************************************************************************************************************/
	bool hasRgbTexture();

	/*!****************************************************************************************************************
	\brief    Query if this implementation supports YUV (Luma/Chroma)planar textures.
	\return   True if the implementation supports Luminance/Chrominance textures, false otherwise
	\details  This function will return true if the getLuminanceTexture() and and getChrominanceTexture() can be used.
			  In implementations where this is not supported (e.g. Android), this function will return false, and the 
			  getLuminanceTexture/getChrominanceTexture will return empty (invalid) textures if used. 
			  In implementations where Luminance/Chrominance textures are supported (e.g. iOS) this function will 
			  return true and the getLuminanceTexture(), getChrominanceTexture() will return valid texture handles that
			  can each be used to directly query the Luminance texture (Y channel of the format) and the Chrominance 
			  texture(UV channels of the format)
	******************************************************************************************************************/
	bool hasLumaChromaTextures();

	/*!****************************************************************************************************************
	\brief      		Retrieves the texture name for the YUV camera texture.
	\return            GL texture ID
	******************************************************************************************************************/
	const native::HTexture_& getLuminanceTexture();

	/*!****************************************************************************************************************
	\brief      		Retrieves the texture name for the YUV camera texture.
	\return            GL texture ID
	******************************************************************************************************************/
	const native::HTexture_& getChrominanceTexture();

	/*!****************************************************************************************************************
	\brief      		Returns the resolution of the currently active camera.
	\param[out]        x
	\param[out]        y
	******************************************************************************************************************/
	void getCameraResolution(unsigned int& x, unsigned int& y);

private:
	void* pImpl;
};
}
