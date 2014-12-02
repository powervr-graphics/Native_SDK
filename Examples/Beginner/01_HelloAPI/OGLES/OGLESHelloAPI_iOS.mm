/*******************************************************************************************************************************************

 @File         OGLESHelloAPI_iOS.cpp

 @Title        OpenGL ES 1.x HelloAPI Tutorial

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES 1.x, use it for drawing a triangle and terminate it.
               Entry Point: main

 *******************************************************************************************************************************************/
/*******************************************************************************************************************************************
 Include Files
 *******************************************************************************************************************************************/
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

/*******************************************************************************************************************************************
 Defines
 *******************************************************************************************************************************************/
#define VERTEX_ARRAY	0
#define KFPS			120.0

/*!*****************************************************************************************************************************************
 @Function		TestGLError
 @Input			functionLastCalled          Function which triggered the error
 @Return		True if no GL error was detected
 @Description	Tests for an GL error and prints it in a message box.
 *******************************************************************************************************************************************/
bool TestGLError(const char* functionLastCalled)
{
	/*	glGetError returns the last error that occurred using OpenGL ES, not necessarily the status of the last called function. The user
	 has to check after every single OpenGL ES call or at least once every frame. Usually this would be for debugging only, but for this
	 example it is enabled always
	 */
	GLenum lastError = glGetError();
	if (lastError != GL_NO_ERROR)
	{
		NSLog(@"%s failed (%d).\n", functionLastCalled, lastError);
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 Class EAGLView
 ********************************************************************************************************************************************/
@class EAGLView;

@interface EAGLView : UIView
{
@private
	EAGLContext*        m_context;
	GLuint				m_framebuffer;
	GLuint				m_renderbuffer;
	GLuint				m_depthBuffer;

	GLuint				m_vertexBuffer;
	
	NSTimer*			m_timer;			// timer for rendering our OpenGL content
	id					m_displayLink;		// Prefer using displaylink, if it's available.
	
	BOOL				m_animating;
    BOOL				m_displayLinkSupported;
}

+ (Class) layerClass;
- (void) renderScene;
- (void) dealloc;
- (id)   initWithFrame:(CGRect)frame scale:(CGFloat)scale;
- (BOOL) initialiseBuffer:(GLuint*)vertexBuffer;
- (BOOL) createEAGLContext:(CAEAGLLayer*)eaglLayer frame:(CGRect)frame scale:(CGFloat)scale;
- (BOOL) createRenderbuffer:(CAEAGLLayer*)eaglLayer;
- (void) deInitialiseGLState;
@end

@implementation EAGLView

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

/*!*****************************************************************************************************************************************
 @Function		initialiseBuffer
 @Output		vertexBuffer                Handle to a vertex buffer object
 @Return		Whether the function succeeds or not.
 @Description	Initialises shaders, buffers and other state required to begin rendering with OpenGL ES
 *******************************************************************************************************************************************/
- (BOOL) initialiseBuffer:(GLuint*)vertexBuffer
{
	if(!vertexBuffer)
		return FALSE;

	/*	Concept: Vertices
	 When rendering a polygon or model to screen, OpenGL ES has to be told where to draw the object, and more fundamentally what shape
	 it is. The data used to do this is referred to as vertices, points in 3D space which are usually collected into groups of three
	 to render as triangles. Fundamentally, any advanced 3D shape in OpenGL ES is constructed from a series of these vertices - each
	 vertex representing one corner of a polygon.
	 */

	/*	Concept: Buffer Objects
	 To operate on any data, OpenGL first needs to be able to access it. The GPU maintains a separate pool of memory it uses independent
	 of the CPU. Whilst on many embedded systems these are in the same physical memory, the distinction exists so that they can use and
	 allocate memory without having to worry about synchronising with any other processors in the device.
	 To this end, data needs to be uploaded into buffers, which are essentially a reserved bit of memory for the GPU to use. By creating
	 a buffer and giving it some data we can tell the GPU how to render a triangle.
	 */

	// Vertex data containing the positions of each point of the triangle
	GLfloat vertexData[] = {-0.4f,-0.4f, 0.0f,  // Bottom Left
		0.4f,-0.4f, 0.0f,  // Bottom Right
		0.0f, 0.4f, 0.0f}; // Top Middle

	// Generate a buffer object
	glGenBuffers(1, vertexBuffer);

	// Bind buffer as an vertex buffer so we can fill it with data
	glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);

	/*	Set the buffer's size, data and usage
	 Note the last argument - GL_STATIC_DRAW. This tells the driver that we intend to read from the buffer on the GPU, and don't intend
	 to modify the data until we're done with it.
	 */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	if (!TestGLError("glBufferData"))
	{
		return FALSE;
	}

	return TRUE;
}

/*!*****************************************************************************************************************************************
 @Function		createEAGLContext
 @Return        Whether the function succeeds or not.
 @Description	Initialises EAGL and sets of the context for rendering.
 *******************************************************************************************************************************************/
- (BOOL) createEAGLContext:(CAEAGLLayer*)eaglLayer frame:(CGRect)frame scale:(CGFloat)scale
{
	/*	Create a context.
	 EAGL has to create what is known as a context for OpenGL ES. The concept of a context is OpenGL ES's way of encapsulating any
	 resources and state. What appear to be "global" functions in OpenGL actually only operate on the current context. A context
	 is required for any operations in OpenGL ES.
	 */

	/*
	 Initialise EAGL.
	 */
	[eaglLayer setDrawableProperties: [	NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:NO],
									   kEAGLDrawablePropertyRetainedBacking,
									   kEAGLColorFormatRGBA8,
									   kEAGLDrawablePropertyColorFormat,
									   nil]];

	/*
	 Create a context for rendering with OpenGL ES2.
	 */
	m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

	if((!m_context) || (![EAGLContext setCurrentContext:m_context]))
	{
		return FALSE;
	}

	// Scale the display appropriately.
	if([self respondsToSelector:@selector(contentScaleFactor)])
	{
		self.contentScaleFactor = scale;
	}
	else
	{
		self.bounds    = CGRectMake(0.0, 0.0, frame.size.width * scale, frame.size.height * scale);
		self.transform = CGAffineTransformScale(self.transform, 1 / scale, 1 / scale);
	}

	return TRUE;
}

/*!*****************************************************************************************************************************************
 @Function		createRenderbuffer
 @Return        Whether the function succeeds or not.
 @Description	Creates a render buffer suitable for rendering to.
 *******************************************************************************************************************************************/
- (BOOL) createRenderbuffer:(CAEAGLLayer*)eaglLayer
{
	/* Create a renderbuffer.
	 iOS requires a renderbuffer to be created and attached with a framebuffer. Applications on iOS do not render to the 'default'
	 framebuffer (i.e framebuffer 0). Instead, clients are required to create their own framebuffer and render to this instead. The OS
	 can then composite the user's framebuffer with it's own buffers to create the UI.
	 */

	/*
	 Create a render buffer.
	 */
	GLuint oldRenderbuffer;
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint *) &oldRenderbuffer);
	glGenRenderbuffersOES(1, &m_renderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_renderbuffer);

	if(![m_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer])
	{
		glDeleteRenderbuffersOES(1, &m_renderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_BINDING_OES, oldRenderbuffer);
		return FALSE;
	}

	GLint width, height;
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES,  &width);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);

	/*
	 Create a frame buffer.
	 */
	glGenFramebuffersOES(1, &m_framebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_framebuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_renderbuffer);

	/*
	 Create a depth buffer.
	 */
	glGenRenderbuffersOES(1, &m_depthBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_depthBuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthBuffer);

	/*
	 Attach the render buffer to the framebuffer.
	 */
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		return FALSE;
	}

	/*
	 Set the viewport size to the window size.
	 */
	glViewport(0, 0, width, height);

	return TRUE;
}

/*!*****************************************************************************************************************************************
 @Function		initWithFrame
 @Input			frame
 @Return        self or nil
 @Description	Initialises EAGL and sets of the context for rendering. Also call functions to setup GL resources.
 *******************************************************************************************************************************************/
- (id) initWithFrame:(CGRect)frame scale:(CGFloat)scale
{
	if(!(self = [super initWithFrame:frame]))
		return self;
	
	CAEAGLLayer* eaglLayer = (CAEAGLLayer*)[self layer];
	if(![self createEAGLContext:eaglLayer frame:frame scale:scale])
	{
		[self release];
		return nil;
	}

	if(![self createRenderbuffer:eaglLayer])
	{
		[self release];
		return nil;
	}

	if(![self initialiseBuffer:&m_vertexBuffer])
	{
		[self release];
		return nil;
	}
	
	return self;
}

/*!*****************************************************************************************************************************************
 @Function		renderScene
 @Description	Renders the scene to the framebuffer. Usually called within a loop.
 *******************************************************************************************************************************************/
- (void) renderScene
{
	/* Bind the user renderbuffer
	 As previously mentioned, iOS requires users to render to their own renderbuffer, which is later composited by the OS.
	 First though, we take the previously bound render buffer, so we can reset it after rendering.
	 */
	GLuint oldRenderBuffer;
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint*)&oldRenderBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_renderbuffer);

	/*	Set the clear color
	 At the start of a frame, generally you clear the image to tell OpenGL ES that you're done with whatever was there before and want to
	 draw a new frame. In order to do that however, OpenGL ES needs to know what colour to set in the image's place. glClearColor
	 sets this value as 4 floating point values between 0.0 and 1.x, as the Red, Green, Blue and Alpha channels. Each value represents
	 the intensity of the particular channel, with all 0.0 being transparent black, and all 1.x being opaque white. Subsequent calls to
	 glClear with the colour bit will clear the frame buffer to this value.
	 The functions glClearDepth and glClearStencil allow an application to do the same with depth and stencil values respectively.
	 */
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	/*	Clears the color buffer.
	 glClear is used here with the Colour Buffer to clear the colour. It can also be used to clear the depth or stencil buffer using
	 GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, respectively.
	 */
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable the vertex array
	glEnableClientState(GL_VERTEX_ARRAY);

	// Sets the vertex data to this attribute index, with the number of floats in each position
	glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), (GLvoid*)0);
	TestGLError("glVertexAttribPointer");

	// Set a color to render
	glColor4f(1.0f, 1.0f, 0.66f, 1.0f);

	/*	Draw the triangle
	 glDrawArrays is a draw call, and executes the shader program using the vertices and other state set by the user. Draw calls are the
	 functions which tell OpenGL ES when to actually draw something to the framebuffer given the current state.
	 glDrawArrays causes the vertices to be submitted sequentially from the position given by the "first" argument until it has processed
	 "count" vertices. Other draw calls exist, notably glDrawElements which also accepts index data to allow the user to specify that
	 some vertices are accessed multiple times, without copying the vertex multiple times.
	 Others include versions of the above that allow the user to draw the same object multiple times with slightly different data, and
	 a version of glDrawElements which allows a user to restrict the actual indices accessed.
	 */
	glDrawArrays(GL_TRIANGLES, 0, 3);
	TestGLError("glDrawArrays");

	/*	Present the display data to the screen.
	 When rendering to a Window surface, OpenGL ES is double buffered. This means that OpenGL ES renders directly to one frame buffer,
	 known as the back buffer, whilst the display reads from another - the front buffer. eglSwapBuffers signals to the windowing system
	 that OpenGL ES 1.x has finished rendering a scene, and that the display should now draw to the screen from the new data. At the same
	 time, the front buffer is made available for OpenGL ES 1.x to start rendering to. In effect, this call swaps the front and back
	 buffers.
	 */
	if(![m_context presentRenderbuffer:GL_RENDERBUFFER_OES])
	{
		NSLog(@"Failed to swap renderbuffer.\n");
	}

	// Reset the older renderbuffer
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRenderBuffer);
}

/*!*****************************************************************************************************************************************
 @Function		deInitialiseGLState
 @Description	Releases GL resources previously allocated
 *******************************************************************************************************************************************/
- (void) deInitialiseGLState
{
	// Delete the VBO as it is no longer needed
	glDeleteBuffers(1, &m_vertexBuffer);

	/*
	 Release renderbuffers
	 */
	EAGLContext *oldContext = [EAGLContext currentContext];

	if (oldContext != m_context)
		[EAGLContext setCurrentContext:m_context];

	glDeleteRenderbuffersOES(1, &m_depthBuffer);
	m_depthBuffer = 0;

	glDeleteRenderbuffersOES(1, &m_renderbuffer);
	m_renderbuffer = 0;

	glDeleteFramebuffersOES(1, &m_framebuffer);
	m_framebuffer = 0;

	if (oldContext != m_context)
		[EAGLContext setCurrentContext:oldContext];

	/*
	 EAGLContext deinitialisation.
	 */
	[m_context release];
	m_context = nil;
}

/*!*****************************************************************************************************************************************
 @Function		dealloc
 @Description	Class destructor. Calls function to destroy the GL state.
 *******************************************************************************************************************************************/
- (void) dealloc
{
	[self deInitialiseGLState];
	[super dealloc];
}

/*!*****************************************************************************************************************************************
 @Function		startAnimation
 @Description	Starts rendering.
 *******************************************************************************************************************************************/
- (void)startAnimation
{
    if(!m_animating)
    {
        if(m_displayLinkSupported)
        {
            // CADisplayLink is an API new to iOS SDK 3.1. We prefer to use it for animation rather than a timer, but fallback to the timer if it's
            // not available.
            m_displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(renderScene)];
            [m_displayLink setFrameInterval:1];		// Fire at 60fps on a 60Hz display
            [m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }
        else
		{
			m_timer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / KFPS) target:self selector:@selector(renderScene) userInfo:nil repeats:YES];
		}
        
        m_animating = TRUE;
    }
}

/*!*****************************************************************************************************************************************
 @Function		stopAnimation
 @Description	Stops rendering.
 *******************************************************************************************************************************************/
- (void)stopAnimation
{
    if(m_animating)
    {
        if(m_displayLinkSupported)
        {
            [m_displayLink invalidate];
            m_displayLink = nil;
        }
        else
        {
            [m_timer invalidate];
            m_timer = nil;
        }
        
        m_animating = FALSE;
    }
}

@end

/*!*****************************************************************************************************************************************
 Class AppController
 *******************************************************************************************************************************************/
@interface AppController : NSObject <UIApplicationDelegate>
{
	UIWindow*			window; 	// Our window
	EAGLView*			view;		// View
}
@end

@implementation AppController

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	/*
	 Create a fullscreen window that we can use for OpenGL ES output.
	 */
	CGRect frameSize;
	UIScreen* screen = [UIScreen mainScreen];
	CGFloat    scale = 1.0;
	if ([UIScreen instancesRespondToSelector:@selector(scale)])
	{
		scale = [screen scale];
	}
	CGRect appFrame = [screen bounds];
	frameSize       = CGRectMake(appFrame.origin.x, appFrame.origin.y,
								 appFrame.size.width, appFrame.size.height);

	window = [[UIWindow alloc] initWithFrame:frameSize];
	view   = [[EAGLView alloc] initWithFrame:frameSize scale:scale];
	if(view)
	{
		/*
		 Add this view to the window and show
		 */
		[window addSubview: view];
		[window makeKeyAndVisible];

		// Setup a timer to redraw the view at a regular interval
		[view startAnimation];
	}
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	[view stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
    [view startAnimation];
}

- (void) applicationWillTerminate:(UIApplication *)application
{
	[view stopAnimation];
}

- (void) dealloc
{
    [window release];
    [view release];
    
    [super dealloc];
}

@end

/*!*****************************************************************************************************************************************
 @Function		main
 @Description	Runs the application.
 *******************************************************************************************************************************************/
int main(int argc, char **argv)
{
	NSAutoreleasePool* pool = [NSAutoreleasePool new];
	UIApplicationMain(argc, argv, nil, @"AppController");
	[pool release];
	return 0;
}

