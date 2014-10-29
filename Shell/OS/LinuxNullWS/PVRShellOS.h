/*!****************************************************************************

 @file         LinuxNullWS/PVRShellOS.h
 @ingroup      OS_LinuxNullWS
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Initialization for the shell for LinuxNullWS.
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <termios.h>
#include <unistd.h>

#ifdef PVRSHELL_OMAP3_TS_SUPPORT
// Touchscreen include file
#include "tslib.h"
#define PVRSHELL_TS_SAMPLES 15
#endif

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

#if defined(USE_GDL_PLANE)
#include "libgdl.h"
#endif

/*!
 @addtogroup OS_LinuxNullWS 
 @brief      LinuxNullWS OS
 @details    The following table illustrates how key codes are mapped in LinuxNullWS:
             <table>
             <tr><th> Key code      </th><th> nLastKeyPressed (PVRShell) </th></tr>
             <tr><td> 0             </td><td> PVRShellKeyNameQUIT	     </td></tr>
             <tr><td> Q             </td><td> PVRShellKeyNameQUIT	     </td></tr>
             <tr><td> F11           </td><td> PVRShellKeyNameScreenshot	 </td></tr>
             <tr><td> S             </td><td> PVRShellKeyNameScreenshot	 </td></tr>
             <tr><td> 13            </td><td> PVRShellKeyNameSELECT	     </td></tr>
             <tr><td> Space         </td><td> PVRShellKeyNameACTION1	 </td></tr>
             <tr><td> 49            </td><td> PVRShellKeyNameACTION1	 </td></tr>
             <tr><td> 50            </td><td> PVRShellKeyNameACTION2	 </td></tr>
             <tr><td> Up arrow      </td><td> m_eKeyMapUP	             </td></tr>
             <tr><td> Down arrow    </td><td> m_eKeyMapDOWN	             </td></tr>
             <tr><td> Left arrow    </td><td> m_eKeyMapLEFT	             </td></tr>
             <tr><td> Right arrow   </td><td> m_eKeyMapRIGHT	         </td></tr>
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
	~PVRShellInitOS() {
		/* recover tty state */
		tcsetattr(devfd, TCSANOW, &termio_orig);

		if(remote_fd > 0)
		{
			tcsetattr(remote_fd, TCSANOW, &remios_orig);
			close(remote_fd);
		}
	}

public:
	// Pixmap support: variables for the pixmap

	// Keypad handling.
	int keypad_fd;

	// Remote control handling.
	int remote_fd;
    struct termios remios;
    struct termios remios_orig;

	// Key handling
	int devfd;
	struct termios termio;
    struct termios termio_orig;

#ifdef PVRSHELL_OMAP3_TS_SUPPORT
	// Touchscreen
	struct tsdev *ts;
	struct ts_sample samples[PVRSHELL_TS_SAMPLES];
	float m_vec2PointerLocation[2];
#endif

#if defined(USE_GDL_PLANE)
	gdl_plane_id_t m_Plane; // A gdl plane
#endif

	timeval 	 m_StartTime;

	unsigned int m_ui32NativeDisplay;
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/

