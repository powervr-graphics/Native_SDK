/*!****************************************************************************

 @file         iOS/AppController.h
 @ingroup      OS_iOS
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Main controller for iOS apps. 
               Manages the high-level tasks of the application such as 
               bringing the view to the foreground, creating a render loop, 
               and terminating the application properly.

******************************************************************************/

#ifndef _APPCONTROLLER_H_
#define _APPCONTROLLER_H_

#include "PVRShell.h"
#import "EAGLView.h"

//CLASS INTERFACES:

/*!****************************************************************************
 @interface    AppController
 @brief        Main controller class for iOS apps.  
 @addtogroup   OS_iOS
******************************************************************************/

@interface AppController : NSObject <UIAccelerometerDelegate>
{
	UIWindow*				_window;
	EAGLView*				_glView;  // A view for OpenGL ES rendering
	NSTimer*				_renderTimer;	// timer for render loop
	UIAccelerationValue		_accelerometer[3];
	
	PVRShell*			m_pPVRShell;
	PVRShellInit*		m_pPVRShellInit ;
}

- (void) doExitFromFunction:(NSString*)reason;

@end

#endif /*_APPCONTROLLER_H_*/

