/*!
 \brief Contains the declaration of the MVKView class.
 \file PVRShell/OS/AppKit/ViewMVK.h
 \author PowerVR by Imagination, Developer Technology Team
 \copyright Copyright (c) Imagination Technologies Limited.
 */
#pragma once
#include <AppKit/NSView.h>

@interface ViewMVK : NSView
{
    id<MTLDevice>  m_device; // Metal device
}

-(ViewMVK*) initWithFrame:(NSRect) frame;

@end
