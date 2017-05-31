#pragma once
// NO INCLUDES AS THIS FILE IS ONLY A PART OF SHELL.H - IT ONLY EXISTS SO THAT THE DEBUGGER IS NOT GETTING CONFUSED

// The functions provided here serves the sole purpose that if an application tries to link without PVRApi and 
// PVRNativeApi, expecting to use them as DLL's, including these two functions will allow the application to 
// avoid linker error as PVRShell requires their presense. Additionally, they allow PVRShell to runtime detect
// this condition, i.e. if PVRNativeApi / PVRApi are statically or dynamically linked.

#ifdef PVRAPI_DYNAMIC_LIBRARY
// DYMMY IMPLEMENTATION - AVOID LINKER ERROR DUE TO SHELL.CPP HAVING A BRANCHING PATH BASED ON THE BUILD
pvr::GraphicsContextStrongReference PVR_API_FUNC createGraphicsContext()
{ return GraphicsContextStrongReference(); }
std::auto_ptr<IPlatformContext> PVR_API_FUNC createNativePlatformContext(OSManager& osManager)
{ return std::auto_ptr<IPlatformContext>(); }
#endif