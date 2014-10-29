// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: WaterSeg_meshes.nav ********

// File data
A32BIT _WaterSeg_meshes_nav[] = {
0x1,0x0,0x0,0x0,0x0,0xfacebeed,0x0,0xfacebeed,0x0,0xfacebeed,0x0,
};

// Register WaterSeg_meshes.nav in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_WaterSeg_meshes_nav("WaterSeg_meshes.nav", _WaterSeg_meshes_nav, 40);

// ******** End: WaterSeg_meshes.nav ********

