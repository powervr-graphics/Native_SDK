=============
Surround View
=============

.. figure:: ./Surroundview.png

This sample shows an Automotive surround view example with four camera feed.

API
---
* OpenGL ES 3.0+

OS
---
* Linux Embedded (Yocto)

Description
-----------
This sample shows a basic implementation on how an Automotive surround view application can be implemented. In this example four physical cameras are considered, using zero-copy approach allowing maximum performance when streaming information from physical cameras. The physical camera management code is encapsulated in the CameraManager class, where the device detection and initialization is done, together with the commands to start and stop streaming (startCapturing and stopCapturing), and update the images streamed by each camera (updateCameraFrame). The image update is done in the following way: Each camera has a set of buffers where it will write the latest image captured. This information is provided in the method readCameraFrame. On the OpenGL ES side, in the method initTexturesForCameras one OpenGL ES texture is generated per camera buffer (meaning if we have four cameras and each camera has four buffers, then sixteen textures will be generated). Also, for each one of those OpenGL ES textures generated, one EGL image is generated, binding both through the API EGLImageTargetTexture2DOES so the image provided by the camera can be sampled and used in a format understood by OpenGL ES. Support for the extension GL_OES_EGL_image providing this API is required for this sample to work ( https://registry.khronos.org/OpenGL/extensions/OES/OES_EGL_image.txt ). EGL acts as a bridge between the physical camera device information and OpenGL ES, providing a way to share information without the need to do a copy of the information. The camera color format is RGB565, which is understood by OpenGL ES, allowing as well a low bandwidth consumption.

This sample uses four physical cameras, with the device paths hardcoded at source code level. The variable vectorCameraDeviceName has the values { "/dev/video1", "/dev/video3", "/dev/video3" and "/dev/video0" } mapping to the parts {front, right, back, left} of our car setup. You might need to change the values or the order in vectorCameraDeviceName to match your setup. One of the most delicate parts of surround view applications is the projection of each one of the images streamed from each physical camera onto a 3D mesh in a way that can be stitched together. For this sample we are using a feature developed by our Demo Team at Imagination Technologies, where some per-vertex attributes are hardcoded in a 3D mesh which represents a quarter of a hemisphere, which help properly distribute the physical camera image in the 3D mesh (see the code in SurroundVertShader.vsh and SurroundFragShader.fsh).

This demo does not have some features like pixel harmonization, which is based on postprocessing steps to unify the look of all cameras streaming information, since the amount of light arriving at each one of them might be different, generating darker / lighter areas. There is research publicly available on how this can be done.

This demo is targeted for Automotive devices, which operate in a quite specific environment. In this case, we used a non-Safety Critical embedded Linux OS, Yocto Linux 2.4.3 (rocko). We used four automotive-grade cameras RDACM21 with resolution up to 1280x1080 in a board equipped with a Renesas RCar-H3 featuring a PowerVR GX6650 GPU. This means you will only be able to use this sample with physical camera streaming if you have a compatible device (a regular Linux setup with USB cameras might be possible but it might not support the require EGLImageTargetTexture2DOES API). In case you do not have this setup, you can at least run the sample with no command line parameters, which defaults to using four static images taken from the physical cameras mentioned which are projected in the surround view 3D meshes (on windows you will also need to use GLES emulation, you can copy the files libEGL.dll, libGLES_CM.dll and libGLESv2.dll from lib/Windows_x86_64 to the executable folder). This sample is optimized for PowerVR and allows a performance of over 60 FPS with four physical cameras streaming at resolution of 1280x1080 in the hardware setup specified when using manual mip map generation.

The camera streaming functionality is actually not the default option when running this sample. Below is an explanation of the few command-line options you can use with this sample:

Command line option: -useCameraStreaming
	Use camera streaming (only available on Linux).
	Example ./OpenGLSCSurroundView -useCameraStreaming

Command line option: -cameraResolutionWidth and -cameraResolutionHeight
	Specify the physical camera resolution (if supported by hardware) when using physical cameras.
	Example ./OpenGLSCSurroundView -useCameraStreaming -cameraResolutionWidth=320 -cameraResolutionHeight=240

Command line option: -avoidCubemap
	Avoid using a cubemap for improved car 3D model rendering (this cubemap is updated dynamically when streaming images from physical cameras), meaning no environment mapping will be applied to the car 3D model.
	Example: ./OpenGLSCSurroundView -avoidCubemap

Command line option: -cubemapTextureSize
	Specify the texture resolution of the cubemap used for improved 3D car model rendering (this cubemap is updated dynamically when streaming images from physical cameras)
	Example: ./OpenGLSCSurroundView -cubemapTextureSize	

Command line option: -useManualMipMapGeneration
	Manually generate the mipmap levels of the physical camera images streamed for improved performance
	Example: ./OpenGLSCSurroundView -useCameraStreaming -useManualMipMapGeneration	

Other general commands which might be helpful when working with a Linux environment with physical cameras are below:
	Take a picture from a specific camera device: ffmpeg -f v4l2 -video_size 1280x720 -i /dev/video0 -frames 1 out.jpg
	Record a video from a specific camera device: ffmpeg -f v4l2 -framerate 30 -video_size 1280x720 -input_format mjpeg -i /dev/video0 -c copy out.mkv
