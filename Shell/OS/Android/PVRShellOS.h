/*!***************************************************************************

 @file          Android/PVRShellOS.h
 @ingroup       OS_Android
 @copyright     Copyright (c) Imagination Technologies Limited.
 @brief         Initialization for the shell for Android OS using Khronos EGL.
 @details       Makes programming for 3D APIs easier by wrapping surface initialization,
 				Texture allocation and other functions for use by a demo.
 
*****************************************************************************/

#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <android_native_app_glue.h>
#include <time.h>

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!
 @addtogroup OS_Android 
 @brief      Android OS
 @details    The following table illustrates how key codes are mapped in Android:
             <table>
             <tr><th> Android Key Code </th><th> PVRShellKeyPressed  	   </th></tr>
             <tr><td> Q                </td><td> PVRShellKeyNameQUIT	   </td></tr>
             <tr><td> DPAD_CENTER      </td><td> PVRShellKeyNameSELECT	   </td></tr>
             <tr><td> SPACE            </td><td> PVRShellKeyNameACTION1    </td></tr>
             <tr><td> SHIFT_LEFT       </td><td> PVRShellKeyNameACTION2    </td></tr>
             <tr><td> DPAD_UP          </td><td> m_eKeyMapUP     		   </td></tr>
             <tr><td> DPAD_DOWN        </td><td> m_eKeyMapDOWN       	   </td></tr>
             <tr><td> DPAD_LEFT        </td><td> m_eKeyMapLEFT  	       </td></tr>
             <tr><td> DPAD_RIGHT       </td><td> m_eKeyMapRIGHT 	       </td></tr>
             <tr><td> S                </td><td> PVRShellKeyNameScreenshot </td></tr>
             </table>
 @{
*/

/*!***************************************************************************
 @class PVRShellInitOS
 @brief Interface with specific Operating System.
*****************************************************************************/
class PVRShellInitOS
{
public:
	~PVRShellInitOS()
	{
	}

public:
	timeval 	 m_StartTime;

	android_app* m_pAndroidState;
	bool m_bRendering;
	bool m_bError;
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/
