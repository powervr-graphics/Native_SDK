// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SecHwyShield_text.nav ********

// File data
A32BIT _SecHwyShield_text_nav[] = {
0x1,0xc2af2803,0x4227109c,0xc2af201c,0x4227281c,0xfacebeed,0x0,0xfacebeed,0x0,0xfacebeed,0x0,
};

// Register SecHwyShield_text.nav in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SecHwyShield_text_nav("SecHwyShield_text.nav", _SecHwyShield_text_nav, 40);

// ******** End: SecHwyShield_text.nav ********

