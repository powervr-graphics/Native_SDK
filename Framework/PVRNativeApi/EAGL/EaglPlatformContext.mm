/*!
\brief Contains implementation code for the EAGL(iOS) version of the PlatformContext.
\file PVRNativeApi/EAGL/EaglPlatformContext.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
 #include "PVRNativeApi/EAGL/EaglPlatformHandles.h"
#import <QuartzCore/CAEAGLLayer.h>
#include "PVRNativeApi/PlatformContext.h"
#import <UIKit/UIKit.h>
@interface APIView : UIView
{
}

@end

// CLASS IMPLEMENTATION
@implementation APIView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

@end

// TODO: Don't make these globals
//GLint numDiscardAttachments	= 0;
//GLenum discardAttachments[3];
GLuint framebuffer = 0;
GLuint renderbuffer = 0;
GLuint depthBuffer = 0;

GLuint msaaFrameBuffer = 0;
GLuint msaaColorBuffer = 0;
GLuint msaaDepthBuffer = 0;

//TODO: Lots of error checking.
using namespace pvr::platform;

bool PlatformContext::isRayTracingSupported() const { return _supportsRayTracing; }

void PlatformContext::setRayTracingSupported(bool supported) { _supportsRayTracing = supported; }

using namespace pvr;

pvr::Result SharedPlatformContext::init(PlatformContext& context, uint32 id)
{

	std::auto_ptr<ISharedPlatformContext> retval(new SharedPlatformContext());

	// CREATE THE PBUFFER SURFACE FOR THE SHARED CONTEXT
	_parentContext = &context;
	_handles.reset(new NativeSharedPlatformHandles_);
    
    EAGLRenderingAPI api;
    switch (_parentContext->apiType)
    {
        case pvr::Api::OpenGLES2: //GLES2
            Log(Log.Debug, "EAGL context creation: Setting kEAGLRenderingAPIOpenGLES2");
            api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES2);
            break;
        case pvr::Api::OpenGLES3: //GLES2
            Log(Log.Debug, "EGL context creation: kEAGLRenderingAPIOpenGLES3");
            api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES3);
            break;
        default:
            return pvr::Result::UnsupportedRequest;
    }
    _handles->uploadingContext= [[EAGLContext alloc] initWithAPI:api sharegroup:_parentContext->_platformContextHandles->context.sharegroup];
    if(!_handles->uploadingContext)
    {
        Log("Failed to create SharedContext");
        return Result::UnknownError;
    }
    return Result::Success;
}



void PlatformContext::release()
{
    glDeleteFramebuffers(1,&framebuffer);
    glDeleteRenderbuffers(1,&renderbuffer);
    glDeleteRenderbuffers(1,&depthBuffer);
    glDeleteFramebuffers(1,&msaaFrameBuffer);
    glDeleteRenderbuffers(1, &msaaColorBuffer);
    glDeleteRenderbuffers(1,&msaaDepthBuffer);
    framebuffer = renderbuffer = depthBuffer = msaaFrameBuffer = msaaColorBuffer = msaaDepthBuffer = 0;
    
    /*if (_bInitialized)
     {
     //TODO: Too general? Potential of screwing a lot up if you say, set the display to something random before returning. Probably a non-issue really though as OS does a cleanup anyway.
     // Check the current context/surface/display. If they are equal to the handles in this class, remove them from the current context
     if (_apiContextHandles.display == eglGetCurrentDisplay() &&
     _apiContextHandles.drawSurface == eglGetCurrentSurface(EGL_DRAW) &&
     _apiContextHandles.readSurface == eglGetCurrentSurface(EGL_READ) &&
     _apiContextHandles.context == eglGetCurrentContext())
     {
     eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
     }
     
     // These are all refcounted, so these can be safely deleted.
     if (_apiContextHandles.display)
     {
     if (_apiContextHandles.context)
     {
     eglDestroyContext(_apiContextHandles.display, _apiContextHandles.context);
     }
     
     if (_apiContextHandles.drawSurface)
     {
     eglDestroySurface(_apiContextHandles.display, _apiContextHandles.drawSurface);
     }
     
     if (_apiContextHandles.readSurface && _apiContextHandles.readSurface != _apiContextHandles.drawSurface)
     {
     eglDestroySurface(_apiContextHandles.display, _apiContextHandles.readSurface);
     }
     
     eglTerminate(_apiContextHandles.display);
     }
     _bInitialized = false;
     }*/
	 _maxApiVersion = Api::Unspecified;
}

static inline pvr::Result preInitialize(OSManager& mgr, NativePlatformHandles& handles)
{
    if (!handles.get())
    {
        handles.reset(new NativePlatformHandles::ElementType);
    }
    return pvr::Result::Success;
}



void PlatformContext::populateMaxApiVersion()
{
	_maxApiVersion = Api::Unspecified;
	Api graphicsapi = Api::OpenGLESMaxVersion;
	while (graphicsapi > Api::Unspecified)
	{
		const char* esversion = (graphicsapi == Api::OpenGLES31 ? "3.1" : graphicsapi == Api::OpenGLES3 ? "3.0" : graphicsapi == Api::OpenGLES2 ?
		                         "2.0" : "UNKNOWN_VERSION");
    
        EAGLContext* context = NULL;
        // create our context
        switch(graphicsapi)
        {
            case Api::OpenGLES31:
                break;
            case Api::OpenGLES2:
                context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
                break;
            case Api::OpenGLES3:
                context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
                break;
            default:assertion(false, "Unsupported Api");
        }
    
        if(context != nil)
        {
            _maxApiVersion = graphicsapi;
            Log(Log.Verbose, "Maximum API level detected: OpenGL ES %s", esversion);
            return;
        }
        else
        {
            Log(Log.Verbose, "OpenGL ES %s NOT supported. Trying lower version...", esversion);
        }
        graphicsapi = Api((uint32)graphicsapi - 1);
	}
	Log(Log.Critical, "=== FATAL: COULD NOT FIND COMPATIBILITY WITH ANY OPENGL ES VERSION ===");

}

pvr::Api PlatformContext::getMaxApiVersion()
{
    if(!_preInitialized)
    {
        if(preInitialize(_OSManager, _platformContextHandles) != Result::Success)
        {
            return Api::Unspecified;
        }
        _preInitialized = true;
        populateMaxApiVersion();
    }
    return _maxApiVersion;
}

bool PlatformContext::isApiSupported(Api apiLevel)
{
	return apiLevel <= getMaxApiVersion();
}

bool SharedPlatformContext::makeSharedContextCurrent()
{
    [EAGLContext setCurrentContext:_handles->uploadingContext];
	return true;
}


static pvr::Result init(PlatformContext& platformContext,const NativeDisplay& nativeDisplay, const NativeWindow& nativeWindow, DisplayAttributes& attributes, const pvr::Api& apiContextType,
                              UIView** outView, EAGLContext** outContext)
{
    pvr::Result result = pvr::Result::Success;
    pvr::Api apiRequest = apiContextType;
    if(apiContextType == pvr::Api::Unspecified || !platformContext.isApiSupported(apiContextType))
    {
        apiRequest = platformContext.getMaxApiVersion();\
        platformContext.getOsManager().setApiTypeRequired(apiRequest);
        pvr::Log(pvr::Log.Information, "Unspecified target API. Setting to max API level, which is %s",
            apiName(apiRequest));
    }
    
    if(!platformContext.isInitialized())
    {
        UIView* view;
        EAGLContext* context;
        UIWindow* nw = (__bridge UIWindow*)nativeWindow;
        // UIWindow* nw = static_cast<UIWindow*>(nativeWindow);
        // Initialize our UIView surface and add it to our view
        view = [[APIView alloc] initWithFrame:[nw bounds]];
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *) [view layer];
        eaglLayer.opaque = YES;
        
        NSString* pixelFormat;
        
		bool supportSRGB = (&kEAGLColorFormatSRGBA8 != NULL);
		bool requestSRGB = attributes.frameBufferSrgb;
		bool request32bitFbo = (attributes.redBits > 5 || attributes.greenBits > 6 || attributes.blueBits > 5 || attributes.alphaBits > 0);
		if (requestSRGB && !supportSRGB) {
            pvr::Log(pvr::Log.Warning, "sRGB window backbuffer requested, but an SRGB backbuffer is not supported. Creating linear RGB backbuffer."); 
            }
		if (requestSRGB && supportSRGB) {
            pixelFormat = kEAGLColorFormatSRGBA8;
                pvr::Log(pvr::Log.Information, "sRGB window backbuffer requested. Creating linear RGB backbuffer (kEAGLColorFormatSRGBA8).");
            }
		else if (request32bitFbo) 
        { 
            pixelFormat = kEAGLColorFormatRGBA8; 
            pvr::Log(pvr::Log.Information, "32-bit window backbuffer requested. Creating linear RGBA backbuffer (kEAGLColorFormatRGBA8).");
        }
		else {
            pvr::Log(pvr::Log.Information, "Invalid backbuffer requested. Creating linear RGB backbuffer (kEAGLColorFormatRGB565).");
            pixelFormat = kEAGLColorFormatRGB565; 
        }

        [eaglLayer setDrawableProperties:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, pixelFormat, kEAGLDrawablePropertyColorFormat, nil]];
        
        [nw addSubview:view];
        [nw makeKeyAndVisible];
            
        EAGLRenderingAPI api;
        // create our context
        switch(apiRequest)
        {
            case pvr::Api::OpenGLES2:
                pvr::Log(pvr::Log.Debug, "EGL context creation: Setting EGL_OPENGL_ES2_BIT");
                api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES2);
                break;
            case pvr::Api::OpenGLES3:
                pvr::Log(pvr::Log.Debug, "EGL context creation: EGL_OPENGL_ES3_BIT");
                api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES3);
                break;
            default:pvr::assertion(0, "Unsupported Api");
        }
        
        context = [[EAGLContext alloc] initWithAPI:api];
        
        if(!context || ![EAGLContext setCurrentContext:context]) // TODO: Save a copy of the current context
        {
            // [context release];
            //  [view release];
            return pvr::Result::UnknownError;
        }
        
        const GLubyte *extensions = glGetString(GL_EXTENSIONS);
        bool hasFramebufferMultisample = (strstr((const char *)extensions, "GL_APPLEframebuffer_multisample") != NULL);
        
        // Create our framebuffers for rendering to
        GLuint oldRenderbuffer;
        GLuint oldFramebuffer;

        glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *) &oldRenderbuffer);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &oldFramebuffer);
        
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        
        if(![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer])
        {
            glDeleteRenderbuffers(1, &renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER_BINDING, oldRenderbuffer);
            
            //  [context release];
            //  [view release];
            return pvr::Result::UnknownError;
        }
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, reinterpret_cast<GLint*>(&attributes.width));
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,reinterpret_cast<GLint*>(&attributes.height));
        
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
        
        
     //   if(hasFramebufferDiscard && attributes.discardFrameColor)
        //    discardAttachments[numDiscardAttachments++] = GL_COLOR_ATTACHMENT0;
        
        if(attributes.depthBPP || attributes.stencilBPP)
        {
            GLuint format;
            
            if(attributes.depthBPP && !attributes.stencilBPP){
                format = GL_DEPTH_COMPONENT24_OES;
                pvr::Log(pvr::Log.Information, "window backbuffer DepthStencil Format: GL_DEPTH_COMPONENT24_OES");
            }
            else if(attributes.depthBPP && attributes.stencilBPP){
                format = GL_DEPTH24_STENCIL8_OES;
                pvr::Log(pvr::Log.Information, "window backbuffer DepthStencil Format: GL_DEPTH24_STENCIL8_OES");
            }
            else if(!attributes.depthBPP && attributes.stencilBPP){
                format = GL_STENCIL_INDEX8;
                pvr::Log(pvr::Log.Information, "window backbuffer DepthStencil Format: GL_STENCIL_INDEX8");
            }
            
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, format, attributes.width, attributes.height);
            
            if(attributes.depthBPP)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
                
            }
            
            if(attributes.stencilBPP)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            }
        }
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            return pvr::Result::UnknownError;
        }
        
        //MSAA
        if (hasFramebufferMultisample && attributes.aaSamples > 0)
        {
            GLint maxSamplesAllowed, samplesToUse;
            glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
            samplesToUse = attributes.aaSamples < maxSamplesAllowed ? attributes.aaSamples : maxSamplesAllowed;
            
            if(samplesToUse)
            {
                glGenFramebuffers(1, &msaaFrameBuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer);
                
                glGenRenderbuffers(1, &msaaColorBuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuffer);
                glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_RGBA8_OES, attributes.width, attributes.height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorBuffer);
                
                if(attributes.depthBPP || attributes.stencilBPP)
                {
                    GLuint format;
                    
                    if(attributes.depthBPP && !attributes.stencilBPP)
                        format = GL_DEPTH_COMPONENT24_OES;
                    else if(attributes.depthBPP && attributes.stencilBPP)
                        format = GL_DEPTH24_STENCIL8_OES;
                    else if(!attributes.depthBPP && attributes.stencilBPP)
                        format = GL_STENCIL_INDEX8;
                    
                    glGenRenderbuffers(1, &msaaDepthBuffer);
                    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
                    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, format, attributes.width, attributes.height);
                    
                    if(attributes.depthBPP)
                    {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuffer);
                    }
                    
                    if(attributes.stencilBPP)
                    {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuffer);
                    }
                }
                
                if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                {
                    NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
                    return pvr::Result::UnknownError;
                }
            }
        }
        
        glViewport(0, 0, attributes.width, attributes.height);
        glScissor(0, 0, attributes.width, attributes.height);
        
        glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderbuffer);
        *outContext = context;
#if !defined(TARGET_OS_IPHONE)
        *outView = view;
#endif
    }
    
    return result;
}

pvr::Result PlatformContext::init(){
    preInitialize(_OSManager,_platformContextHandles);
    populateMaxApiVersion();
    
    if (_OSManager.getApiTypeRequired() == Api::Unspecified)
    {
        if (_OSManager.getMinApiTypeRequired() == Api::Unspecified)
        {
            apiType = getMaxApiVersion();
            _OSManager.setApiTypeRequired(apiType);
            Log(Log.Information, "Unspecified target API -- Setting to max API level : %s", apiName(apiType));
        }
        else
        {
            apiType = std::max(_OSManager.getMinApiTypeRequired(), getMaxApiVersion());
            Log(Log.Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
                apiName(_OSManager.getMinApiTypeRequired()), apiName(getMaxApiVersion()));
            _OSManager.setApiTypeRequired(apiType);
        }
    }
    else
    {
        Log(Log.Information, "Forcing specific API level: %s", apiName(_OSManager.getApiTypeRequired()));
    }
    
    if (_OSManager.getApiTypeRequired() > getMaxApiVersion())
    {
        Log(Log.Error, "================================================================================\n"
            "API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
            "**** APPLICATION WILL EXIT ****\n"
            "================================================================================",
            apiName(_OSManager.getApiTypeRequired()), apiName(getMaxApiVersion()));
        return Result::UnsupportedRequest;
    }
    EAGLContext* context;
    UIView* view;
    if(::init(*this, reinterpret_cast<const NativeDisplay>(_OSManager.getDisplay()),
                reinterpret_cast<const NativeWindow>(_OSManager.getWindow()),
            _OSManager.getDisplayAttributes(),_OSManager.getApiTypeRequired(),&view,&context) == Result::Success)
    {
        _platformContextHandles->context = context;
        _platformContextHandles->view = (__bridge VoidUIView*)view;
        _initialized = true;
        return pvr::Result::Success;
    }
    return pvr::Result::NotInitialized;
}

bool PlatformContext::makeCurrent()
{
    if(msaaFrameBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuffer);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }
    
    if(_platformContextHandles->context != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:_platformContextHandles->context];
    
    return true;
}

bool PlatformContext::presentBackbuffer()
{
    bool result;
    EAGLContext* oldContext = [EAGLContext currentContext];
    
    if(oldContext != _platformContextHandles->context)
        [EAGLContext setCurrentContext:_platformContextHandles->context]; // TODO: If we're switching context then we should also be saving the renderbuffer/scissor test state
    
    //MSAA
    if(msaaFrameBuffer)
    {
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, msaaFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, framebuffer);
        glResolveMultisampleFramebufferAPPLE();
    }
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    
    result = [_platformContextHandles->context presentRenderbuffer:GL_RENDERBUFFER] != NO ? true : false;
    
    if(oldContext != _platformContextHandles->context)
        [EAGLContext setCurrentContext:oldContext];
    
    return result;
}

pvr::string PlatformContext::getInfo()
{
    //	TODO:
    string out,tmp;
    /*	EGLint i32Values[5];
     
     out.Reserve(2048);
     tmp.Reserve(1024);
     
     out.Append("\nGL:\n");
     
     tmp.Format("\tVendor:   %s\n", (char*) glGetString(GL_VENDOR));
     out.Append(tmp);
     
     tmp.Format("\tRenderer: %s\n", (char*) glGetString(GL_RENDERER));
     out.Append(tmp);
     
     tmp.Format("\tVersion:  %s\n", (char*) glGetString(GL_VERSION));
     out.Append(tmp);
     
     tmp.Format("\tExtensions:  %s\n", (char*) glGetString(GL_EXTENSIONS));
     out.Append(tmp);
     
     out.Append("\nEGL:\n");
     
     tmp.Format("\tVendor:   %s\n" , (char*) eglQueryString(_apiContextHandles.display, EGL_VENDOR));
     out.Append(tmp);
     
     tmp.Format("\tVersion:  %s\n" , (char*) eglQueryString(_apiContextHandles.display, EGL_VERSION));
     out.Append(tmp);
     
     tmp.Format("\tExtensions:  %s\n" , (char*) eglQueryString(_apiContextHandles.display, EGL_EXTENSIONS));
     out.Append(tmp);
     
     if(eglQueryContext(_apiContextHandles.display, _apiContextHandles.context, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &i32Values[0]))
     {
     switch(i32Values[0])
     {
     case EGL_CONTEXT_PRIORITY_HIGH_IMG:   out.Append("\tContext priority: High\n");  break;
     case EGL_CONTEXT_PRIORITY_MEDIUM_IMG: out.Append("\tContext priority: Medium\n");break;
     case EGL_CONTEXT_PRIORITY_LOW_IMG:    out.Append("\tContext priority: Low\n");   break;
     default: out.Append("\tContext priority: Unrecognised.\n");
     }
     }
     else
     {
     eglGetError(); // Clear error
     out.Append("\tContext priority: Unsupported\n");
     }
     
     #if defined(EGL_VERSION_1_2)
     tmp.Format("\tClient APIs:  %s\n" , (char*) eglQueryString(_apiContextHandles.display, EGL_CLIENT_APIS));
     out.Append(tmp);
     #endif
     
     eglQuerySurface(_apiContextHandles.display, _apiContextHandles.drawSurface, EGL_WIDTH,  &i32Values[0]);
     tmp.Format("\nSurface Width:  %i\n" , i32Values[0]);
     out.Append(tmp);
     
     eglQuerySurface(_apiContextHandles.display, _apiContextHandles.drawSurface, EGL_HEIGHT, &i32Values[0]);
     tmp.Format("Surface Height: %i\n\n" , i32Values[0]);
     out.Append(tmp);
     
     // EGLSurface details
     
     // Get current config
     EGLConfig config;
     eglQueryContext(_apiContextHandles.display, _apiContextHandles.context, EGL_CONFIG_ID, &i32Values[0]);
     const EGLint attributes[] = { EGL_CONFIG_ID, i32Values[0], EGL_NONE };
     eglChooseConfig(_apiContextHandles.display, attributes, &config, 1, &i32Values[1]);
     
     out.Append("EGL Surface:\n");
     
     tmp.Format("\tConfig ID:      %i\n", i32Values[0]);
     out.Append(tmp);
     
     // Color buffer
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_BUFFER_SIZE, &i32Values[0]);
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_RED_SIZE   , &i32Values[1]);
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_GREEN_SIZE , &i32Values[2]);
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_BLUE_SIZE  , &i32Values[3]);
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_ALPHA_SIZE , &i32Values[4]);
     tmp.Format("\tColor Buffer:  %i bits (R%i G%i B%i A%i)\n", i32Values[0],i32Values[1],i32Values[2],i32Values[3],i32Values[4]);
     
     // Depth buffer
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_DEPTH_SIZE , &i32Values[0]);
     tmp.Format("\tDepth Buffer:   %i bits\n", i32Values[0]);
     out.Append(tmp);
     
     // Stencil Buffer
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_STENCIL_SIZE , &i32Values[0]);
     tmp.Format("\tStencil Buffer: %i bits\n", i32Values[0]);
     out.Append(tmp);
     
     // EGL surface bits support
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_SURFACE_TYPE , &i32Values[0]);
     tmp.Format("\tSurface type:   %s%s%s\n",	i32Values[0] & EGL_WINDOW_BIT  ? "WINDOW " : "",
     i32Values[1] & EGL_PBUFFER_BIT ? "PBUFFER " : "",
     i32Values[2] & EGL_PIXMAP_BIT  ? "PIXMAP " : "");
     out.Append(tmp);
     
     // EGL renderable type
     #if defined(EGL_VERSION_1_2)
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_RENDERABLE_TYPE , &i32Values[0]);
     tmp.Format("\tRenderable type: %s%s%s%s\n",
     i32Values[0] & EGL_OPENVG_BIT ? "OPENVG " : "",
     i32Values[0] & EGL_OPENGL_ES_BIT ? "OPENGL_ES " : "",
     #if defined(EGL_OPENGL_BIT)
     i32Values[0] & EGL_OPENGL_BIT ? "OPENGL " : "",
     #endif
     i32Values[0] & EGL_OPENGL_ES2_BIT ? "OPENGL_ES2 " : "");
     out.Append(tmp);
     #endif
     
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_SAMPLE_BUFFERS , &i32Values[0]);
     eglGetConfigAttrib(_apiContextHandles.display, config, EGL_SAMPLES , &i32Values[1]);
     tmp.Format("\tSample buffer No.: %i\n", i32Values[0]);
     out.Append(tmp);
     tmp.Format("\tSamples per pixel: %i", i32Values[1]);
     out.Append(tmp);
     */
    return out;
}

std::auto_ptr<ISharedPlatformContext> PlatformContext::createSharedPlatformContext(uint32 id)
{
	auto retval = std::auto_ptr<ISharedPlatformContext>(new SharedPlatformContext());
	SharedPlatformContext& shared = static_cast<SharedPlatformContext&>(*retval);
	shared.init(*this, id);
	return retval;
}


//Creates an instance of a graphics context
std::auto_ptr<pvr::IPlatformContext> pvr::createNativePlatformContext(OSManager& mgr)
{
    auto ptr =  std::auto_ptr<pvr::IPlatformContext>(new pvr::platform::PlatformContext(mgr));
    ptr->baseApi = BaseApi::OpenGLES;
	ptr->swapChainLength = 1;
	ptr->swapIndex = 0;
	ptr->lastPresentedSwapIndex = 0;
    return ptr;
}
//!\endcond 
