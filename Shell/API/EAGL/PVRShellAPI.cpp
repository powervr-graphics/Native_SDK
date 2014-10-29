/******************************************************************************

 @File         EAGL/PVRShellAPI.cpp

 @Title        EAGL/PVRShellAPI

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/

// No Doxygen for CPP files, due to documentation duplication
/// @cond NO_DOXYGEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "PVRShell.h"
#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"



/*****************************************************************************
	Declarations
*****************************************************************************/

/****************************************************************************
** Class: PVRShellInitAPI
****************************************************************************/

/*****************************************************************************
* Function Name  : ActivatePreferences
* Description    : Activates the user set preferences (like v-sync)
*****************************************************************************/
void PVRShellInit::ApiActivatePreferences()
{
}

/*****************************************************************************
* Function Name  : ApiInitAPI
* Returns        : true for success
* Description    : would initialise the 3D API except this is done automatically frm the nib here
*****************************************************************************/
bool PVRShellInit::ApiInitAPI()
{
	return true;
}

void PVRShellInit::OutputAPIInfo()
{

}

/*******************************************************************************
 * Function Name  : ReleaseAPI
 * Inputs		  : None
 * Returns        : Nothing
 * Description    : Clean up when we're done
 *******************************************************************************/
void PVRShellInit::ApiReleaseAPI()
{

}

bool PVRShellInit::ApiScreenCaptureBuffer(int Width, int Height, unsigned char* pBuf)
{
	unsigned char*	pLines2;
	int				i, j;
	bool			bRet = true;



	/* Allocate memory for line */
	pLines2 = (unsigned char*)calloc(4 * Width * Height, sizeof(unsigned char));
	if (!pLines2) { return false; }

	while (glGetError());
	/* Read line from frame buffer */
	glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, pLines2);

	if (glGetError())
	{
		bRet = false;
	}
	else
	{
		/* Convert RGB to BGR in line */
		for (j = 0, i = 0; j < 4 * Width * Height; j += 4, i += 3)
		{
			pBuf[i] = pLines2[j + 2];
			pBuf[i + 1] = pLines2[j + 1];
			pBuf[i + 2] = pLines2[j];
		}
	}

	free(pLines2);
	return bRet;

}


void PVRShellInit::ApiRenderComplete()
{
}

/*!***********************************************************************
 @Function		ApiSet
 @Input			prefName	Name of value to set
 @Modified		i32Value	Value to set it to
 @description	Set parameters which are specific to the API.
*************************************************************************/
bool PVRShellInit::ApiSet(const prefNameIntEnum prefName, const int i32Value)
{
	switch (prefName)
	{
		case prefApiMajorVersion: m_ApiMajorVersion = i32Value; return true;
		case prefApiMinorVersion: m_ApiMinorVersion = i32Value; return true;
		default: break;
	}
	return false;
}

/****************************************************************************
** Get
****************************************************************************/
bool PVRShellInit::ApiGet(const prefNameIntEnum prefName, int* pn)
{
	switch (prefName)
	{
		case prefApiMajorVersion: *pn = (int) m_ApiMajorVersion; return true;
		case prefApiMinorVersion: *pn = (int) m_ApiMinorVersion; return true;
		default:;
	}
	return false;
}

bool PVRShellInit::ApiGet(const prefNamePtrEnum prefName, void** pp)
{
	return false;
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellAPI.cpp)
*****************************************************************************/

