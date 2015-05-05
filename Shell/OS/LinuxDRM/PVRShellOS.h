/*!****************************************************************************

 @file         LinuxDRM/PVRShellOS.h
 @ingroup      OS_LinuxDRM
 @copyright    Copyright (c) Imagination Technologies Limited.

 @brief        Initialization for the shell for LinuxDRM.
 
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <xf86drmMode.h>
#include <gbm.h>

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!***********************************************************************
 @struct SDrmFbWrapper
 @brief  Stores DRM FB and associated GBM BO for freeup.
 ************************************************************************/
struct SDrmFbWrapper 
{
	struct gbm_bo *psGbmBo;
	unsigned int ui32FbId;
	int i32Fd;
};

/*!
 @addtogroup OS_LinuxDrm
 @brief      LinuxDrm OS
 @details    The following table illustrates how key codes are mapped in LinuxDrm:
             <table>
             <tr><th> Key code      </th><th> nLastKeyPressed (PVRShell) </th></tr>
             <tr><td> 0             </td><td> PVRShellKeyNameQUIT	     </td></tr>
             <tr><td> Q             </td><td> PVRShellKeyNameQUIT	     </td></tr>
             <tr><td> S             </td><td> PVRShellKeyNameScreenshot	 </td></tr>
             <tr><td> Enter	    </td><td> PVRShellKeyNameSELECT	     </td></tr>
             <tr><td> Space         </td><td> PVRShellKeyNameACTION1	 </td></tr>
             <tr><td> 1             </td><td> PVRShellKeyNameACTION1	 </td></tr>
             <tr><td> 2             </td><td> PVRShellKeyNameACTION2	 </td></tr>
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
	~PVRShellInitOS() 
	{
		// recover tty state
		int retval;
		const char txt[] = {
			27 // the ESCAPE ASCII character
			, '['
			, '?'
			, '2'
			, '5'
			, 'h'
			, 0
		   };
		retval = write(devfd, &txt[0], 7);
		tcsetattr(devfd, TCSANOW, &termio_orig);
	}

public:
	// Key handling
	int devfd;
	struct termios termio;
	struct termios termio_orig;

	timeval 	 m_StartTime;

	unsigned int m_ui32NativeDisplay;
	unsigned int m_ui32NativeWindow;

	unsigned int m_ui32DrmDisplayId;
	int m_i32DrmFile;
	unsigned int m_ui32DrmCrtcId;
	unsigned int m_ui32DrmConnectorId;
	unsigned int m_ui32DrmEncoderId;
	drmModeResPtr m_psDrmResources;
	drmModeCrtcPtr m_psDrmCrtc;
	drmModeEncoderPtr m_psDrmEncoder;
	drmModeModeInfoPtr m_psDrmMode;
	drmModeConnectorPtr m_psDrmConnector;

	struct gbm_device *m_psGbmDev;
	struct gbm_surface *m_psGbmSurface;

	unsigned int m_ui32CurrentFb;

	struct SDrmFbWrapper *DrmFbGetFromBo(struct gbm_bo *bo);
};

/*! @} */

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/

