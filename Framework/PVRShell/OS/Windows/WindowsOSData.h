/*!*********************************************************************************************************************
\file         PVRShell\OS\Windows\WindowsOSData.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains implementation details required by the Microsoft Windows version of PVRShell.
***********************************************************************************************************************/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace pvr {
namespace platform {
/*!**********************************************************************************************************
\brief OS specific data for windows
************************************************************************************************************/
struct WindowsOSData
{
	int cmdShow;

	WindowsOSData() : cmdShow(SW_SHOW)
	{
	}
};
}
}