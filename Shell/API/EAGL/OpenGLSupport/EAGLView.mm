/******************************************************************************

 @File         EAGLView.mm

 @Title        

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  

******************************************************************************/


#import <QuartzCore/QuartzCore.h>

#import "EAGLView.h"
#import "OpenGL_Internal.h"

#include "PVRShell.h"
#include "PVRShellOS.h"
#include "PVRShellAPI.h"
#include "PVRShellImpl.h"

//CLASS IMPLEMENTATIONS:

@implementation EAGLView

@synthesize delegate=_delegate, autoresizesSurface=_autoresize, surfaceSize=_size, framebuffer = _framebuffer, pixelFormat = _format, depthFormat = _depthFormat, context = _context;

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (BOOL) _createSurface
{
	CAEAGLLayer*			eaglLayer = (CAEAGLLayer*)[self layer];
	GLuint					oldRenderbuffer;
	GLuint					oldFramebuffer;
	
	if(![EAGLContext setCurrentContext:_context]) {
		return NO;
	}
	
	if([self respondsToSelector:@selector(contentScaleFactor)]) {
		self.contentScaleFactor = _scale;
	}
	else {
		self.bounds = CGRectMake(0,0, self.bounds.size.width * _scale, self.bounds.size.height * _scale);
		self.transform = CGAffineTransformScale(self.transform, 1 / _scale, 1 / _scale);
	}
	
	GLint width, height;
	
#if defined(BUILD_OGLES)
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint *) &oldRenderbuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, (GLint *) &oldFramebuffer);
	
	glGenRenderbuffersOES(1, &_renderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _renderbuffer);
	
	if(![_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer]) {
		glDeleteRenderbuffersOES(1, &_renderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_BINDING_OES, oldRenderbuffer);
		return NO;
	}
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);
	
	glGenFramebuffersOES(1, &_framebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _renderbuffer);
	if (_depthFormat)
	{
		glGenRenderbuffersOES(1, &_depthBuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthBuffer);
		if(_stencilFormat)
		{
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH24_STENCIL8_OES, width, height);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthBuffer);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthBuffer);
		}
		else
		{
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, _depthFormat, width, height);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthBuffer);
		}
	}
	
	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES){
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}
	//MSAA
	const GLubyte *str = glGetString(GL_EXTENSIONS);
	//NSLog(@"%s",str);
	_enableMSAA = (strstr((const char *)str, "GL_APPLE_framebuffer_multisample") != NULL);
	_enableFramebufferDiscard = (strstr((const char *)str, "GL_EXT_discard_framebuffer") != NULL);
	if (_msaaMaxSamples && _enableMSAA)
	{
		GLint maxSamplesAllowed,samplesToUse;
		glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
		samplesToUse = _msaaMaxSamples < maxSamplesAllowed ? _msaaMaxSamples : maxSamplesAllowed;
		if(samplesToUse) {
			glGenFramebuffersOES(1, &_msaaFrameBuffer);
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, _msaaFrameBuffer);
			glGenRenderbuffersOES(1, &_msaaColourBuffer);
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, _msaaColourBuffer);
			glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, samplesToUse, GL_RGBA8_OES, width, height);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _msaaColourBuffer);
			if (_depthFormat)
			{
				glGenRenderbuffersOES(1, &_msaaDepthBuffer);
				glBindRenderbufferOES(GL_RENDERBUFFER_OES, _msaaDepthBuffer);
				if(_stencilFormat)
				{
					glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, samplesToUse, GL_DEPTH24_STENCIL8_OES, width, height);
					glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _msaaDepthBuffer);
					glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _msaaDepthBuffer);
				}
				else
				{
					glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, samplesToUse, _depthFormat, width, height);
					glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _msaaDepthBuffer);
				}
			}
			if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES){
				NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
				return NO;
			}
		}
	}
	_size.width = width;
	_size.height = height;
	if(!_hasBeenCurrent)
	{
		glViewport(0, 0, width, height);
		glScissor(0, 0, width, height);
		_hasBeenCurrent = YES;
	}
	else
	{
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFramebuffer);
	}
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRenderbuffer);
#elif defined(BUILD_OGLES2)
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *) &oldRenderbuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &oldFramebuffer);
	
	glGenRenderbuffers(1, &_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
	
	if(![_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer])
	{
		glDeleteRenderbuffers(1, &_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER_BINDING, oldRenderbuffer);
		return NO;
	}
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
	if (_depthFormat)
	{
		glGenRenderbuffers(1, &_depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		if(_stencilFormat)
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, _depthFormat, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		}
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}

	//MSAA
	const GLubyte *str = glGetString(GL_EXTENSIONS);
	//NSLog(@"%s",str);
	_enableMSAA = (strstr((const char *)str, "GL_APPLE_framebuffer_multisample") != NULL);
	_enableFramebufferDiscard = (strstr((const char *)str, "GL_EXT_discard_framebuffer") != NULL);
	
	if (_msaaMaxSamples && _enableMSAA)
	{
		GLint maxSamplesAllowed,samplesToUse;
		glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
		samplesToUse = _msaaMaxSamples < maxSamplesAllowed ? _msaaMaxSamples : maxSamplesAllowed;
		
		if(samplesToUse) {
			glGenFramebuffers(1, &_msaaFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, _msaaFrameBuffer);
			
			glGenRenderbuffers(1, &_msaaColourBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _msaaColourBuffer);
			glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_RGBA8_OES, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaaColourBuffer);
			
			if (_depthFormat)
			{
				glGenRenderbuffers(1, &_msaaDepthBuffer);
				glBindRenderbuffer(GL_RENDERBUFFER, _msaaDepthBuffer);
				if(_stencilFormat)
				{
					glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_DEPTH24_STENCIL8_OES, width, height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
				}
				else
				{
					glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, _depthFormat, width, height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
				}
			}
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
				return NO;
			}
		}
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}
	
	_size.width = width;
	_size.height = height;
	if(!_hasBeenCurrent) {
		glViewport(0, 0, width, height);
		glScissor(0, 0, width, height);
		_hasBeenCurrent = YES;
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, oldRenderbuffer);
	
#elif defined(BUILD_OGLES3)
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *) &oldRenderbuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &oldFramebuffer);
	
	glGenRenderbuffers(1, &_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
	
	if(![_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer])
	{
		glDeleteRenderbuffers(1, &_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER_BINDING, oldRenderbuffer);
		return NO;
	}
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
	if (_depthFormat)
	{
		glGenRenderbuffers(1, &_depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		if(_stencilFormat)
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, _depthFormat, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		}
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}

	//MSAA
	const GLubyte *str = glGetString(GL_EXTENSIONS);
	//NSLog(@"%s",str);
	_enableMSAA = (strstr((const char *)str, "GL_APPLE_framebuffer_multisample") != NULL);
	_enableFramebufferDiscard = (strstr((const char *)str, "GL_EXT_discard_framebuffer") != NULL);
	
	if (_msaaMaxSamples && _enableMSAA)
	{
		GLint maxSamplesAllowed,samplesToUse;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamplesAllowed);
		samplesToUse = _msaaMaxSamples < maxSamplesAllowed ? _msaaMaxSamples : maxSamplesAllowed;
		
		if(samplesToUse) {
			glGenFramebuffers(1, &_msaaFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, _msaaFrameBuffer);
			
			glGenRenderbuffers(1, &_msaaColourBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _msaaColourBuffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesToUse, GL_RGBA8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaaColourBuffer);
			
			if (_depthFormat)
			{
				glGenRenderbuffers(1, &_msaaDepthBuffer);
				glBindRenderbuffer(GL_RENDERBUFFER, _msaaDepthBuffer);
				if(_stencilFormat)
				{
					glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesToUse, GL_DEPTH24_STENCIL8, width, height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
				}
				else
				{
					glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesToUse, _depthFormat, width, height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _msaaDepthBuffer);
				}
			}
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
				return NO;
			}
		}
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}
	
	_size.width = width;
	_size.height = height;
	if(!_hasBeenCurrent) {
		glViewport(0, 0, width, height);
		glScissor(0, 0, width, height);
		_hasBeenCurrent = YES;
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, oldRenderbuffer);
	
	
	
	
#endif
	
	PRINT_GL_ERROR();
	
	[_delegate didResizeEAGLSurfaceForView:self];
	
	return YES;
}

- (void) _destroySurface
{
	EAGLContext *oldContext = [EAGLContext currentContext];
	
	if (oldContext != _context)
		[EAGLContext setCurrentContext:_context];

#if defined(BUILD_OGLES)
	if(_depthFormat) {
		glDeleteRenderbuffersOES(1, &_depthBuffer);
		_depthBuffer = 0;
	}
	
	glDeleteRenderbuffersOES(1, &_renderbuffer);
	_renderbuffer = 0;

	glDeleteFramebuffersOES(1, &_framebuffer);
#elif defined(BUILD_OGLES2) || defined(BUILD_OGLES3)
	if(_depthFormat) {
		glDeleteRenderbuffers(1, &_depthBuffer);
		_depthBuffer = 0;
	}
	
	glDeleteRenderbuffers(1, &_renderbuffer);
	_renderbuffer = 0;
	
	glDeleteFramebuffers(1, &_framebuffer);	
#endif
	_framebuffer = 0;
	if (oldContext != _context)
		[EAGLContext setCurrentContext:oldContext];
}

- (id) initWithFrame:(CGRect)frame
{
	return [self initWithFrame:frame pixelFormat:kEAGLColorFormatRGB565 depthFormat:0 stencilFormat:0 preserveBackbuffer:NO scale:1.0f msaaMaxSamples:0];
}

- (id) initWithFrame:(CGRect)frame pixelFormat:(NSString*)format 
{
	return [self initWithFrame:frame pixelFormat:format depthFormat:0 stencilFormat:0 preserveBackbuffer:NO scale:1.0f msaaMaxSamples:0];
}

- (id) initWithFrame:(CGRect)frame
		 pixelFormat:(NSString*)format
		 depthFormat:(GLuint)depth
	   stencilFormat:(GLuint)stencil
  preserveBackbuffer:(BOOL)retained
			   scale:(float)fscale
	  msaaMaxSamples:(GLuint)maxSamples
{
	if((self = [super initWithFrame:frame])) {
		CAEAGLLayer*			eaglLayer = (CAEAGLLayer*)[self layer];

		[eaglLayer setDrawableProperties:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:retained], kEAGLDrawablePropertyRetainedBacking, format, kEAGLDrawablePropertyColorFormat, nil]];
		_format = format;
		_depthFormat = depth;
		_stencilFormat = stencil;
		_enableMSAA = false;
		_msaaMaxSamples = maxSamples;
		_scale = fscale;
		
#if defined(BUILD_OGLES)
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#elif defined(BUILD_OGLES2)
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#elif defined(BUILD_OGLES3)
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
#endif
		
		if(_context == nil) {
			[self release];
			return nil;
		}
		
		if(![self _createSurface]) {
			[self release];
			return nil;
		}
		
		[self setMultipleTouchEnabled:NO];
	}

	return self;
}

- (void) dealloc
{
	[self _destroySurface];
	
	[_context release];
	_context = nil;
	
	[super dealloc];
}

- (void) layoutSubviews
{
	CGRect				bounds = [self bounds];
	
	if(_autoresize && ((roundf(bounds.size.width) != _size.width) || (roundf(bounds.size.height) != _size.height))) {
		[self _destroySurface];
#if __DEBUG__
		REPORT_ERROR(@"Resizing surface from %fx%f to %fx%f", _size.width, _size.height, roundf(bounds.size.width), roundf(bounds.size.height));
#endif
		[self _createSurface];
	}
}

- (void) setAutoresizesEAGLSurface:(BOOL)autoresizesEAGLSurface;
{
	_autoresize = autoresizesEAGLSurface;
	if(_autoresize)
	[self layoutSubviews];
}

- (void) setCurrentContext
{
	if(![EAGLContext setCurrentContext:_context]) {
		printf("Failed to set current context %p in %s\n", _context, __FUNCTION__);
	}
}

- (BOOL) isCurrentContext
{
	return ([EAGLContext currentContext] == _context ? YES : NO);
}

- (void) clearCurrentContext
{
	if(![EAGLContext setCurrentContext:nil])
		printf("Failed to clear current context in %s\n", __FUNCTION__);
}

- (void) BeginRender
{
#if defined(BUILD_OGLES2) || defined(BUILD_OGLES3)
	if(_msaaMaxSamples && _enableMSAA){		
		glBindFramebuffer(GL_FRAMEBUFFER, _msaaFrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _msaaColourBuffer);
	}else{
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	}
#elif defined(BUILD_OGLES)
	if(_msaaMaxSamples && _enableMSAA){		
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _msaaFrameBuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, _msaaColourBuffer);
	}else{
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);
	}
#endif
}

- (void) EndRender
{
	const GLint numAttachments	= 3;
	GLint currentAttachment		= 0;
	GLenum attachments[numAttachments];
	
#if defined(BUILD_OGLES) || defined(BUILD_OGLES2) || defined(BUILD_OGLES3)
	if(_enableFramebufferDiscard)
	{
		//Discard the framebuffer if set.
		PVRShell* pShell = m_pPVRShellInit->m_pShell;
		if (pShell->PVRShellGet(prefDiscardColor))
		{
#ifdef BUILD_OGLES
			attachments[currentAttachment]=GL_COLOR_ATTACHMENT0_OES;
#else
			attachments[currentAttachment]=GL_COLOR_ATTACHMENT0;
#endif
			currentAttachment++;
		}
		if (pShell->PVRShellGet(prefDiscardDepth))
		{
#ifdef BUILD_OGLES
			attachments[currentAttachment]=GL_DEPTH_ATTACHMENT_OES;
#else
			attachments[currentAttachment]=GL_DEPTH_ATTACHMENT;
#endif
			currentAttachment++;
		}
		if (pShell->PVRShellGet(prefDiscardStencil))
		{
#ifdef BUILD_OGLES
			attachments[currentAttachment]=GL_STENCIL_ATTACHMENT_OES;
#else
			attachments[currentAttachment]=GL_STENCIL_ATTACHMENT;
#endif
			currentAttachment++;
		}
	}
#endif	
	
#if defined(BUILD_OGLES2)
	//MSAA
	if(_msaaMaxSamples && _enableMSAA)
	{
		glDisable(GL_SCISSOR_TEST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, _msaaFrameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, _framebuffer);
		glResolveMultisampleFramebufferAPPLE();
		
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE,currentAttachment,attachments);
		}
	}
	else
	{
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glDiscardFramebufferEXT(GL_FRAMEBUFFER,currentAttachment,attachments);
		}
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
	if(![_context presentRenderbuffer:GL_RENDERBUFFER])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
#elif defined(BUILD_OGLES3)
//MSAA
	if(_msaaMaxSamples && _enableMSAA)
	{
		glDisable(GL_SCISSOR_TEST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaaFrameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _framebuffer);
		//glResolveMultisampleFramebufferAPPLE();
		
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER,currentAttachment,attachments);
		}
	}
	else
	{
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glInvalidateFramebuffer(GL_FRAMEBUFFER,currentAttachment,attachments);
		}
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
	if(![_context presentRenderbuffer:GL_RENDERBUFFER])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
#elif defined(BUILD_OGLES)
	//MSAA
	if(_msaaMaxSamples && _enableMSAA){
		glDisable(GL_SCISSOR_TEST);
		glBindFramebufferOES(GL_READ_FRAMEBUFFER_APPLE, _msaaFrameBuffer);
		glBindFramebufferOES(GL_DRAW_FRAMEBUFFER_APPLE, _framebuffer);
		glResolveMultisampleFramebufferAPPLE();
		
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE,currentAttachment,attachments);
		}
	}
	else
	{
		// Assuming some attachments have been chosen, discard them.
		if (currentAttachment!=0)
		{
			glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES,currentAttachment,attachments);
		}
	}
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _renderbuffer);
	if(![_context presentRenderbuffer:GL_RENDERBUFFER_OES])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
#endif
}

- (void) swapBuffers
{
	EAGLContext *oldContext = [EAGLContext currentContext];
	GLuint oldRenderbuffer;
	
	if(oldContext != _context)
		[EAGLContext setCurrentContext:_context];
	
	PRINT_GL_ERROR();
	
#if defined(BUILD_OGLES)
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint *) &oldRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _renderbuffer);
	
	if(![_context presentRenderbuffer:GL_RENDERBUFFER_OES])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
#elif defined(BUILD_OGLES2) || defined(BUILD_OGLES3)
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *) &oldRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
	
	if(![_context presentRenderbuffer:GL_RENDERBUFFER])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
#endif
	
	if(oldContext != _context)
		[EAGLContext setCurrentContext:oldContext];
}

- (CGPoint) convertPointFromViewToSurface:(CGPoint)point
{
	CGRect				bounds = [self bounds];
	
	return CGPointMake((point.x - bounds.origin.x) / bounds.size.width * _size.width, (point.y - bounds.origin.y) / bounds.size.height * _size.height);
}

- (CGRect) convertRectFromViewToSurface:(CGRect)rect
{
	CGRect				bounds = [self bounds];
	
	return CGRectMake((rect.origin.x - bounds.origin.x) / bounds.size.width * _size.width, (rect.origin.y - bounds.origin.y) / bounds.size.height * _size.height, rect.size.width / bounds.size.width * _size.width, rect.size.height / bounds.size.height * _size.height);
}

// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	if ([touches count] != 1)
		return;
		
	for( UITouch *touch in touches )
	{
		// Grab the location of the touch
		CGPoint touchLocation = [touch locationInView:self];
		if(_scale != 1.0)
			touchLocation.x*=_scale;touchLocation.y*=_scale;
		
		float Position[2] = { touchLocation.x, touchLocation.y };
		m_pPVRShellInit->BeganTouch(Position, m_pPVRShellInit);
	}
}

// Handles the continuation of a touch.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	if ([touches count] != 1)
		return;
		
	for( UITouch *touch in touches )
	{
		// Grab the location of the touch
		CGPoint touchLocation = [touch locationInView:self];
		if(_scale != 1.0)
			touchLocation.x*=_scale;touchLocation.y*=_scale;
		
		float Position[2] = { touchLocation.x, touchLocation.y };
		m_pPVRShellInit->MovedTouch(Position, m_pPVRShellInit);
	}
}

// Handles the end of a touch event.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	if ([touches count] != 1)
		return;
	
	for( UITouch *touch in touches )
	{
		// Grab the location of the touch
		// look at start coords and finish coords and decide what action to set
		CGPoint touchLocation = [touch locationInView:self];
		if(_scale != 1.0)
			touchLocation.x*=_scale;touchLocation.y*=_scale;
		
		float Position[2] = { touchLocation.x, touchLocation.y };
		m_pPVRShellInit->EndedTouch(Position, m_pPVRShellInit);
	}
}

- (void) setPVRShellInit: (PVRShellInit *)pPVRShellInit
{
	m_pPVRShellInit = pPVRShellInit;
}

@end

