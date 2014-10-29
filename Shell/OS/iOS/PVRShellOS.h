/*!****************************************************************************

 @file         iOS/PVRShellOS.h
 @ingroup      OS_iOS
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Initialization for the shell for iOS.
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <mach/mach_time.h>

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!
 @addtogroup OS_iOS 
 @brief      iOS
 @{
*/

/*!***************************************************************************
 @class PVRShellInitOS
 @brief Interface with specific Operating System.
*****************************************************************************/
class PVRShellInitOS
{
public:
	char* szTitle;

	float m_vec3Accel[3];
	mach_timebase_info_data_t m_sTimeBaseInfo;

	void BeganTouch(float vec2Location[2], PVRShellInit* init);
	void MovedTouch(float vec2Location[2], PVRShellInit* init);
	void EndedTouch(float vec2Location[2], PVRShellInit* init);
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/

