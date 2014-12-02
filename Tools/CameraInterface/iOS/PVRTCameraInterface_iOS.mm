/******************************************************************************

 @File         PVRTCameraInterface_iOS.mm

 @Title        PVRTCameraInterface

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     ANSI compatible

 @Description  iOS implementation of the camera streaming interface.

 ******************************************************************************/

/*****************************************************************************
 ** Includes
 ******************************************************************************/
#import <AVFoundation/AVFoundation.h>
#include "PVRTCameraInterface_iOS.h"
#import <OpenGLES/EAGL.h>


/*!****************************************************************************
 Class        CPVRTCameraInterfaceiOSImpl
 Description  Delegate Obj-C class required by AVCaptureVideoDataOutput
 ******************************************************************************/
@interface CPVRTCameraInterfaceiOSImpl : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
@public
	// CoreVideo members
	AVCaptureSession*           pAVSession;
	AVCaptureVideoDataOutput*   pAVDataOutput;
	NSString*                   pAVSessionPreset;

	CVOpenGLESTextureCacheRef	videoTextureCache;
	CVOpenGLESTextureRef		lumaTexture;
    CVOpenGLESTextureRef		chromaTexture;
}
@end

@implementation CPVRTCameraInterfaceiOSImpl

/*!***************************************************************************
 @Function		intialiseCaptureSessionFromCamera
 @Input         cam       The HW camera to use
 @Output        error     An error description
 @Returns       true if successful.
 @Description	Initialises the capture session
 *****************************************************************************/
- (BOOL) intialiseCaptureSessionFromCamera:(EPVRTHWCamera)cam withError:(NSString**)error
{
	pAVSessionPreset = AVCaptureSessionPresetHigh;
	
	// Get the current context.
	EAGLContext* pContext = [EAGLContext currentContext];

	// Create the CV texture cache which allows for the quick imager buffer conversion.
    if(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, pContext, NULL, &videoTextureCache))
    {
		*error = [*error stringByAppendingString:@"ERROR: CVOpenGLESTextureCacheCreate failed.\n"];
        return FALSE;
    }
    
    // Setup session and set preset.
    pAVSession = [[AVCaptureSession alloc] init];
    [pAVSession beginConfiguration];
    [pAVSession setSessionPreset:pAVSessionPreset];
	
	// Try and get the front facing camera if possible
	NSArray* pAvailableDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
	
	AVCaptureDevice* pVideoDevice = NULL;
	for(AVCaptureDevice* dev in pAvailableDevices)
	{
		if((cam == ePVRTHWCamera_Front && dev.position == AVCaptureDevicePositionFront) ||
		   (cam == ePVRTHWCamera_Back && dev.position == AVCaptureDevicePositionBack))
		{
			pVideoDevice = dev;
			break;
		}
	}

	if(pVideoDevice == NULL)
	{
		pVideoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
	}
	    
	// Create a capture device based on video type.
    if(!pVideoDevice)
	{
		*error = [*error stringByAppendingString:@"ERROR: Failed to create a capture device.\n"];
		return FALSE;
	}
    
    // Add the device to the session.
    NSError *captureError;
    AVCaptureDeviceInput* pInput = [AVCaptureDeviceInput deviceInputWithDevice:pVideoDevice error:&captureError];
    if(captureError)
	{
		*error = [*error stringByAppendingString:[captureError localizedDescription]];
		return FALSE;
	}
    [pAVSession addInput:pInput];
    
    // Create the output for the capture session.
    pAVDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [pAVDataOutput setAlwaysDiscardsLateVideoFrames:YES]; // Don't care about late frames as we are simply streaming.
    
    // Set output to YUV420 format.
    [pAVDataOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    
	// We're reqired to dispatch to the main thread (this) as OpenGL can only handle data for the thread
	// the context is created upon.
    [pAVDataOutput setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
    
    [pAVSession addOutput:pAVDataOutput];
    [pAVSession commitConfiguration];
    
    [pAVSession startRunning];
	
	return TRUE;
}

/*!***************************************************************************
 @Function		destroySession
 @Description	Shutdown the AV capture session and release associated objects.
 *****************************************************************************/
- (void) destroySession
{
	[self purgeTextures];
	CFRelease(videoTextureCache);

	// Release Obj-C objects
	[pAVDataOutput release];
	[pAVSession release];
}

/*!***************************************************************************
 @Function		purgeTextures
 @Description	Releases textures and flushes the texture cache.
 *****************************************************************************/
- (void) purgeTextures
{
	if (lumaTexture)
    {
        CFRelease(lumaTexture);
        lumaTexture = NULL;
    }
    
    if (chromaTexture)
    {
        CFRelease(chromaTexture);
        chromaTexture = NULL;
    }
    
	// Flush
    CVOpenGLESTextureCacheFlush(videoTextureCache, 0);
}

/*!***************************************************************************
 @Function		<Objective-C delegate callback>
 @Input         captureOutput
 @Input			sampleBuffer
 @Input			connection
 @Description	This function is the Obj-C callback function called by
				the AVCaptureSession upon a frame being processed.
 *****************************************************************************/
- (void)captureOutput:(AVCaptureOutput *)captureOutput
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
		fromConnection:(AVCaptureConnection *)connection
{
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    size_t iWidth				 = CVPixelBufferGetWidth(pixelBuffer);
    size_t iHeight				 = CVPixelBufferGetHeight(pixelBuffer);
    
    if(!videoTextureCache)
		return;		// No video cache. Return.

	// Purge and flush the textures held in the cache.
	[self purgeTextures];

	// Retrieve the current state
	GLint activeTexture;
	GLint boundTexture;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    
	// The following functions create OpenGL:ES textures optimally from
	// CoreVideo buffers.
    // Y component
    glActiveTexture(GL_TEXTURE0);
    CVReturn iLumRet = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,			// Default allocator
																	videoTextureCache,	        	// Cache manager object
																	pixelBuffer,					// Pixel data
																	NULL,							// Texture attribute dictionary
																	GL_TEXTURE_2D,					// Texture target
																	GL_LUMINANCE,					// Texture internal format
																	(GLsizei)iWidth,				// Source width
																	(GLsizei)iHeight,       		// Source height
																	GL_LUMINANCE,					// Texture format
																	GL_UNSIGNED_BYTE,				// Data type
																	0,								// CV plane index - Y in this case
																	&lumaTexture);		        	// Destination texture handle
    if(iLumRet)
    {
		NSLog(@"ERROR: Failed to create luminance texture from CV buffer. Code: %d\n", iLumRet);
    }
    glBindTexture(CVOpenGLESTextureGetTarget(lumaTexture), CVOpenGLESTextureGetName(lumaTexture));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // UV component
    glActiveTexture(GL_TEXTURE1);
    CVReturn iChrRet = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,			// Default allocator
																	videoTextureCache,	        	// Cache manager object
																	pixelBuffer,					// Pixel data
																	NULL,							// Texture attribute dictionary
																	GL_TEXTURE_2D,					// Texture target
																	GL_LUMINANCE_ALPHA,				// Texture internal format
																	(GLsizei)iWidth/2,				// Source width
																	(GLsizei)iHeight/2,				// Source height
																	GL_LUMINANCE_ALPHA,				// Texture format
																	GL_UNSIGNED_BYTE,				// Data type
																	1,								// CV plane index - UV in this case.
																	&chromaTexture);	  	        // Destination texture handle
    if(iChrRet)
    {
        NSLog(@"ERROR: Failed to create luminance texture from CV buffer. Code: %d\n", iChrRet);
    }
    glBindTexture(CVOpenGLESTextureGetTarget(chromaTexture), CVOpenGLESTextureGetName(chromaTexture));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Reset the current state
	glActiveTexture(activeTexture);
	glBindTexture(GL_TEXTURE_2D, boundTexture);

}

@end

/****************************************************************************/

CPVRTCameraInterfaceiOS::CPVRTCameraInterfaceiOS()
{
	m_pImpl = [[CPVRTCameraInterfaceiOSImpl alloc] init];
}

/****************************************************************************/

CPVRTCameraInterfaceiOS::~CPVRTCameraInterfaceiOS()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	[pImpl release];
}

/****************************************************************************/

bool CPVRTCameraInterfaceiOS::InitialiseSession(EPVRTHWCamera eCamera)
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);

	NSString* error;
	return [pImpl intialiseCaptureSessionFromCamera:eCamera withError:&error];
}

/****************************************************************************/

void CPVRTCameraInterfaceiOS::DestroySession()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	[pImpl destroySession];
}

/****************************************************************************/

GLuint CPVRTCameraInterfaceiOS::GetLuminanceTexture()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	return CVOpenGLESTextureGetName(pImpl->lumaTexture);
}

/****************************************************************************/

GLuint CPVRTCameraInterfaceiOS::GetChrominanceTexture()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	return CVOpenGLESTextureGetName(pImpl->chromaTexture);
}

/****************************************************************************/

GLenum CPVRTCameraInterfaceiOS::GetLuminanceTextureTarget()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	return CVOpenGLESTextureGetTarget(pImpl->lumaTexture);
}

/****************************************************************************/

GLenum CPVRTCameraInterfaceiOS::GetChrominanceTextureTarget()
{
	CPVRTCameraInterfaceiOSImpl* pImpl = static_cast<CPVRTCameraInterfaceiOSImpl*>(m_pImpl);
	return CVOpenGLESTextureGetTarget(pImpl->chromaTexture);
}

/*****************************************************************************
 End of file (PVRTCameraInterface_iOS.mm)
 *****************************************************************************/
