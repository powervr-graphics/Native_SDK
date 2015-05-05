/******************************************************************************

 @file         iOS/AppController.mm
 @addtogroup   OS_iOS
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Main controller for iOS apps.  
               Manages the high-level tasks of the application such as 
               bringing the view to the foreground, creating a render loop, 
               and terminating the application properly.

******************************************************************************/

#import "AppController.h"

#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

//CONSTANTS:

const int kFPS = 60.0;
#define kAccelerometerFrequency		30.0 //Hz
#define kFilteringFactor			0.1

// MACROS
#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) / 180.0 * M_PI)

// CLASS IMPLEMENTATION
@implementation AppController

- (void) _renderGLScene
{
	[_glView BeginRender];
	
	// TODO take bool from this and signal exit if appropriate
	if(!m_pPVRShellInit->m_pShell->RenderScene())
	{
		[self doExitFromFunction:@"RenderScene() returned false. Exiting...\n"];
		[_renderTimer invalidate];
		[_renderTimer release];
	}
	
	//Swap framebuffer
	//[_glView swapBuffers];
	[_glView EndRender];
}

- (void) applicationDidFinishLaunching:(UIApplication*)application
{
	UIScreen* screen = [UIScreen mainScreen];
	CGSize  deviceRes;
	CGRect  appBounds = [screen bounds];
	CGFloat scale = 1.0;
	if ([UIScreen instancesRespondToSelector:@selector(scale)])
		scale = [screen scale];

	deviceRes = CGSizeMake(appBounds.size.width * scale, appBounds.size.height * scale);

	m_pPVRShellInit = new PVRShellInit;

	if(!m_pPVRShellInit)
	{
		[self doExitFromFunction:@"Failed to allocate m_pPVRShellInit.\n"];
		return;
	}
	
	if(!m_pPVRShellInit->Init())
	{
		delete m_pPVRShellInit;
		m_pPVRShellInit = 0;
		
		[self doExitFromFunction:@"Failed to initialise m_pPVRShellInit\n"];
		return;
	}
	
	PVRShell* pPVRShell = m_pPVRShellInit->m_pShell;
	pPVRShell->PVRShellSet(prefWidth,  deviceRes.width);
	pPVRShell->PVRShellSet(prefHeight, deviceRes.height);

	// fake command line input
	// if your application is expecting command line input then fake it here
	char pszCL[] = "";
	m_pPVRShellInit->CommandLine(pszCL);

	// set up file paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] bundlePath], @"/"];
	m_pPVRShellInit->SetReadPath([readPath UTF8String]);

  	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
	m_pPVRShellInit->SetWritePath([documentsDirectory UTF8String]);
	
	
	if(pPVRShell->InitApplication())
	{
		printf("InitApplication() succeeded\n");
	}
	else
	{	
		[self doExitFromFunction:@"InitApplication() failed"];
		return;
	}
	
	//Create a full-screen window
	_window = [[UIWindow alloc] initWithFrame:appBounds];
	
	NSString *strColourFormat;
	int iDepthFormat, iStencilFormat;
	
	for(;;)
	{
	
		if(pPVRShell->PVRShellGet(prefColorBPP)==16)
		{
			strColourFormat = kEAGLColorFormatRGB565;
		}
		else
		{
			strColourFormat = kEAGLColorFormatRGBA8;
		}
		
		int iMSAA = pPVRShell->PVRShellGet(prefAASamples);
		
		
#if defined(BUILD_OGLES2)
		//Create the OpenGL ES view and add it to the window
		if(pPVRShell->PVRShellGet(prefStencilBufferContext))
		{
			if(pPVRShell->PVRShellGet(prefZbufferContext))
			{	// need stencil & depth
				iDepthFormat = iStencilFormat = GL_DEPTH24_STENCIL8_OES;
			}
			else
			{	// just stencil
				iDepthFormat = 0;
				iStencilFormat = GL_STENCIL_INDEX8;
			}
		}
		else
		{
			// just depth
			iDepthFormat = GL_DEPTH_COMPONENT24_OES;
			iStencilFormat = 0;
		}
#elif defined(BUILD_OGLES3)
		//Create the OpenGL ES view and add it to the window
		if(pPVRShell->PVRShellGet(prefStencilBufferContext))
		{
			if(pPVRShell->PVRShellGet(prefZbufferContext))
			{	// need stencil & depth
				iDepthFormat = iStencilFormat = GL_DEPTH24_STENCIL8;
			}
			else
			{	// just stencil
				iDepthFormat = 0;
				iStencilFormat = GL_STENCIL_INDEX8;
			}
		}
		else
		{
			// just depth
			iDepthFormat = GL_DEPTH_COMPONENT24;
			iStencilFormat = 0;
		}
#else
		// OGLES 1.1
		//Create the OpenGL ES view and add it to the window
		if(pPVRShell->PVRShellGet(prefStencilBufferContext))
		{
			if(pPVRShell->PVRShellGet(prefZbufferContext))
			{	// stencil and depth
					iDepthFormat = iStencilFormat = GL_DEPTH24_STENCIL8_OES;
			}
			else 
			{	// just stencil
				iDepthFormat = 0;
				iStencilFormat = GL_STENCIL_INDEX8_OES;
			}
		}
		else
		{
			// just depth
			iDepthFormat = GL_DEPTH_COMPONENT24_OES;
			iStencilFormat = 0;
		}

#endif
		// actually make it
		
		_glView = [[EAGLView alloc] initWithFrame:appBounds pixelFormat:strColourFormat depthFormat:iDepthFormat stencilFormat:iStencilFormat preserveBackbuffer:NO scale:scale msaaMaxSamples:iMSAA];	// i.e. 1,2,4 for MSAA

		// check for failure and reduce features of view requested until no failure or complete failure
		if(_glView!=NO)
		{
			break;	// success!!!
		}
		else
		{
			printf("Creating glView failed.\n");
			if(pPVRShell->PVRShellGet(prefStencilBufferContext))
			{
				printf("Dropping stencil buffer\n");
				pPVRShell->PVRShellSet(prefStencilBufferContext,false);
			}
			else if(pPVRShell->PVRShellGet(prefAASamples))
			{
				printf("Dropping FSAA\n");
				pPVRShell->PVRShellSet(prefAASamples,0);
			}
			else
			{
				printf("Couldn't initialise glView\n");
				break;
			}
			
		}
			
		
	}
	
	[_window addSubview:_glView];

	[_glView setPVRShellInit:m_pPVRShellInit];
	
	if(pPVRShell->InitView())
	{
		printf("InitView() succeeded\n");
	}
	else
	{
		[self doExitFromFunction:@"InitView() Failed\n"];
		return;
	}

	//Show the window
	[_window makeKeyAndVisible];
	
	//Configure and start accelerometer
	[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / kAccelerometerFrequency)];
	[[UIAccelerometer sharedAccelerometer] setDelegate:self];

	[UIApplication sharedApplication].idleTimerDisabled = YES;
	
	//Render the initial frame
	[self _renderGLScene];
	
	//Create our rendering timer
	// In later versions of the iPhone SDK there is a dedicated function for this
	// This version is used here for compatiblity
	
	_renderTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(_renderGLScene) userInfo:nil repeats:YES];	
}

- (void) dealloc {
	// In ordinary circumstances this isn't called
	
	m_pPVRShellInit->m_eState = ePVRShellReleaseView;
	
	delete(m_pPVRShellInit);
		

	[_glView release];
	[_window release];
	

	[super dealloc];
}

// throws up a warning dialog
- (void) doExitFromFunction:(NSString*)reason
{
	// Exiting an iPhone application programmatically is not possible from public APIs (at time of writing)
	// So this just puts up a message and lets the user press the button and quit
	printf("%s\n",[reason UTF8String]);
	UIAlertView *myExitWindow = [[UIAlertView alloc]
								 initWithFrame: [[UIScreen mainScreen] bounds]];
	[myExitWindow setTitle:reason];
	if(m_pPVRShell->PVRShellGet(prefExitMessage))
	{	// if this message is unset then this avoids a crash
		[myExitWindow setMessage:[NSString stringWithUTF8String:(const char*)m_pPVRShell->PVRShellGet(prefExitMessage)]];
	}
	else
	{
		[myExitWindow setMessage:@"Exit message is unset"];
	}
	[myExitWindow show];
	
}


- (void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration {
	// A basic low-pass filter on the accelerometer values could be added here
	
	m_pPVRShellInit->m_vec3Accel[0] = acceleration.x;
	m_pPVRShellInit->m_vec3Accel[1] = acceleration.y;
	m_pPVRShellInit->m_vec3Accel[2] = acceleration.z;
	
}

@end

