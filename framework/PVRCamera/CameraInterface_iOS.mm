/*!
\brief An iOS implementation of the PVRCamera camera streaming interface.
\file PVRCamera/CameraInterface_iOS.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#import <AVFoundation/AVFoundation.h>
#include "CameraInterface.h"
#import <OpenGLES/EAGL.h>

struct Texture
{
    GLuint handle;
    GLenum target;
    Texture () : handle(0), target(0){}
};

//Description  Delegate Obj-C class required by AVCaptureVideoDataOutput
@interface CameraInterfaceImpl : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
@public
	// CoreVideo members
	AVCaptureSession*           pAVSession;
	AVCaptureVideoDataOutput*   pAVDataOutput;
	NSString*                   pAVSessionPreset;

	CVOpenGLESTextureCacheRef	videoTextureCache;
	
    CVOpenGLESTextureRef	lumaTexture;
    CVOpenGLESTextureRef	chromaTexture;

    Texture	hLumaTexture;
	Texture	hChromaTexture;
    glm::mat4               cameraTransformation;
}
@end

@implementation CameraInterfaceImpl

//Initializes the capture session
- (BOOL) intialiseCaptureSessionFromCamera:(pvr::HWCamera::Enum)cam withError:(NSString**)error
{
	pAVSessionPreset = AVCaptureSessionPresetHigh;
    cameraTransformation = glm::translate(glm::vec3(1.0f,0.0f,0.0f)) * glm::scale(glm::vec3(-1.0f,-1.0f,1.0f)) * glm::rotate(glm::pi<float>()*-.5f,glm::vec3(0.0f,0.0f,1.0f));
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
        if((cam == pvr::HWCamera::Front && dev.position == AVCaptureDevicePositionFront) ||
		   (cam == pvr::HWCamera::Back && dev.position == AVCaptureDevicePositionBack))
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

//Shutdown the AV capture session and release associated objects.
- (void) destroySession
{
	[self purgeTextures];
	CFRelease(videoTextureCache);

	// Release Obj-C objects
	[pAVDataOutput release];
	[pAVSession release];
}

//Releases textures and flushes the texture cache.
- (void) purgeTextures
{
	if (hLumaTexture.handle)
    {
        CFRelease(lumaTexture);
        hLumaTexture.handle = 0;
    }
    
    if (hChromaTexture.handle)
    {
        CFRelease(chromaTexture);
        chromaTexture = 0;
    }
    
	// Flush
    CVOpenGLESTextureCacheFlush(videoTextureCache, 0);
}

GLenum glTextureBindingName(GLenum glTexTarget)
{
    switch(glTexTarget)
    {
        case GL_TEXTURE_2D: return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_3D: return GL_TEXTURE_BINDING_3D;
        case GL_TEXTURE_CUBE_MAP: return GL_TEXTURE_BINDING_CUBE_MAP;
        default:
            assert(false && "invalid Texture target request");
    }
}

//This function is the Obj-C callback function called by the AVCaptureSession upon a frame being processed.
-(void)captureOutput:(AVCaptureOutput *)captureOutput
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
		fromConnection:(AVCaptureConnection *)connection
{
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    size_t iWidth				 = CVPixelBufferGetWidth(pixelBuffer);
    size_t iHeight				 = CVPixelBufferGetHeight(pixelBuffer);
    GLuint glerr = glGetError();
    if(!videoTextureCache)
		return;		// No video cache. Return.

	// Purge and flush the textures held in the cache.
	[self purgeTextures];

	// Retrieve the current state
	GLint activeTexture;
	GLint boundTexture0, boundTexture1;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture0);
    
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
																	&lumaTexture);        	// Destination texture handle
    if(iLumRet)
    {
		NSLog(@"ERROR: Failed to create luminance texture from CV buffer. Code: %d\n", iLumRet);
    }
	
	hLumaTexture.target = CVOpenGLESTextureGetTarget(lumaTexture);
	hLumaTexture.handle = CVOpenGLESTextureGetName(lumaTexture);
//	glGetIntegerv(glTextureBindingName(hLumaTexture.target), &boundTexture0);
    glBindTexture(hLumaTexture.target, hLumaTexture.handle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
   // UV component
    glActiveTexture(GL_TEXTURE1);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture1);
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
																	&chromaTexture);        // Destination texture handle
    if(iChrRet)
    {
        NSLog(@"ERROR: Failed to create luminance texture from CV buffer. Code: %d\n", iChrRet);
    }
    
	hChromaTexture.target = CVOpenGLESTextureGetTarget(chromaTexture);
	hChromaTexture.handle = CVOpenGLESTextureGetName(chromaTexture);
   // glGetIntegerv(glTextureBindingName(hChromaTexture.target), &boundTexture1);
	glBindTexture(hChromaTexture.target, hChromaTexture.handle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(hChromaTexture.target, boundTexture1);
	// Reset the current state
	glActiveTexture(GL_TEXTURE0);
    glActiveTexture(activeTexture);
	glBindTexture(hLumaTexture.target, boundTexture0);
	
   // glBindTexture(GL_TEXTURE_2D,boundTexture0);
    glerr = glGetError();
    assert((glerr == GL_NO_ERROR) && "GL ERROR");
}

@end


pvr::CameraInterface::CameraInterface()
{
	pImpl = static_cast<void*>([[CameraInterfaceImpl alloc] init]);
}

pvr::CameraInterface::~CameraInterface()
{
	CameraInterfaceImpl* impl = static_cast<CameraInterfaceImpl*>(pImpl);
	[impl release];
}

void pvr::CameraInterface::initializeSession(pvr::HWCamera::Enum eCamera, int preferredResX, int preferredResY)
{
	CameraInterfaceImpl* impl = static_cast<CameraInterfaceImpl*>(pImpl);

	NSString* error;
	if (![impl intialiseCaptureSessionFromCamera:eCamera withError:&error])
	{
		NSLog(@"ERROR: Failed to initialise Camera.\n");
	}
}

bool pvr::CameraInterface::updateImage()
{
	return false;
}

bool pvr::CameraInterface::hasProjectionMatrixChanged()
{
	return false;
}

const glm::mat4& pvr::CameraInterface::getProjectionMatrix()
{
    return static_cast<CameraInterfaceImpl*>(pImpl)->cameraTransformation; 
}

void pvr::CameraInterface::destroySession()
{
	CameraInterfaceImpl* impl = static_cast<CameraInterfaceImpl*>(pImpl);
	[impl destroySession];
}

GLuint pvr::CameraInterface::getRgbTexture()
{
    return 0;
}


bool pvr::CameraInterface::hasRgbTexture()
{
	return false;
}

GLuint pvr::CameraInterface::getLuminanceTexture()
{
    return static_cast<CameraInterfaceImpl*>(pImpl)->hLumaTexture.handle;
}

GLuint pvr::CameraInterface::getChrominanceTexture()
{
	return static_cast<CameraInterfaceImpl*>(pImpl)->hChromaTexture.handle;
}

bool pvr::CameraInterface::hasLumaChromaTextures()
{
	return true;
}

//!\endcond
