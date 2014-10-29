/*!****************************************************************************

 @file         OSX/PVRShellOS.h
 @ingroup      OS_OSX
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Initialization for the shell for OSX.
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <mach/mach_time.h>

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!
 @addtogroup OS_OSX 
 @brief      OSX OS
 @{
*/

class PVRShellInit;

// Objective C variable typedefs
typedef void VoidNSWindow;
typedef void VoidNSView;
typedef void VoidNSApplicationDelegate;

// C to Objective-C interface functions
void ObjC_OSInit(PVRShellInit* pInit);
bool ObjC_OSInitOS(PVRShellInit* pOS);
bool ObjC_OSReleaseOS(PVRShellInit* pOS);
bool ObjC_ExitMessage(PVRShellInit* pInit, const char * const pExitMessage);
bool ObjC_OSGet(PVRShellInit* pInit, const prefNameIntEnum prefName, int *pn);

/*!***************************************************************************
 @class PVRShellInitOS
 @brief Interface with specific Operating System.
*****************************************************************************/
class PVRShellInitOS
{
public:
	PVRShellInitOS() : m_pWindow(0), m_pAppController(0)
	{
	}
	
public:
	struct mach_timebase_info m_sTimeBaseInfo;
	
	// Objective C variables
	VoidNSWindow* m_pWindow;
    VoidNSView* m_pView;
	VoidNSApplicationDelegate* m_pAppController;
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/

