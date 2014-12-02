// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: MajHwyShield_text.nav ********

// File data
A32BIT _MajHwyShield_text_nav[] = {
0x1,0xc2af3a17,0x4227140d,0xc2af3a17,0x4227140d,0xfacebeed,0x0,0xfacebeed,0x0,0xfacebeed,0x0,
};

// Register MajHwyShield_text.nav in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_MajHwyShield_text_nav("MajHwyShield_text.nav", _MajHwyShield_text_nav, 40);

// ******** End: MajHwyShield_text.nav ********

