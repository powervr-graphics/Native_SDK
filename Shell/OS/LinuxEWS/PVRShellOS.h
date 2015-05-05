/*!****************************************************************************

 @file         LinuxEWS/PVRShellOS.h
 @ingroup      OS_LinuxEWS
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Initialization for the shell for LinuxEWS.
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <termios.h>
#include <unistd.h>
#include <EWS/ews.h>

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!
 @addtogroup OS_LinuxEWS 
 @brief      LinuxEWS OS
 @details    The following table illustrates how key codes are mapped in LinuxEWS:
             <table>
             <tr><th> Key code      </th><th>   nLastKeyPressed (PVRShell) </th></tr>
             <tr><td> Q             </td><td>   PVRShellKeyNameQUIT        </td></tr>
             <tr><td> ESC           </td><td>   PVRShellKeyNameQUIT        </td></tr>
             <tr><td> S             </td><td>   PVRShellKeyNameScreenshot  </td></tr>
             <tr><td> ENTER         </td><td>   PVRShellKeyNameSELECT      </td></tr>
             <tr><td> SPACE         </td><td>   PVRShellKeyNameACTION1     </td></tr>
             <tr><td> UP            </td><td>   m_eKeyMapUP                </td></tr>
             <tr><td> DOWN          </td><td>   m_eKeyMapDOWN              </td></tr>
             <tr><td> LEFT          </td><td>   m_eKeyMapLEFT              </td></tr>
             <tr><td> RIGHT         </td><td>   m_eKeyMapRIGHT             </td></tr>
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
	EWS_DISPLAY m_EWSDisplay;
	EWS_WINDOW  m_EWSWindow;

	timeval 	 m_StartTime;
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/

