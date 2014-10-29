// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FeedbackVertShader.vsh ********

// File data
static const char _FeedbackVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define POSITION_ARRAY\t\t0\r\n"
	"#define VELOCITY_ARRAY\t\t1\r\n"
	"#define ATTRIBUTES_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = POSITION_ARRAY)   in highp  vec3 inPosition;\r\n"
	"layout (location = VELOCITY_ARRAY)   in highp  vec3 inVelocity;\r\n"
	"layout (location = ATTRIBUTES_ARRAY) in highp  vec3 inAttributes; // x = curTimeToLive, y = Damping, z = initialTimeToLive\r\n"
	"\r\n"
	"uniform highp vec3  EmitDirection;\r\n"
	"uniform highp vec3  Force;\r\n"
	"uniform highp float TimeDelta;\r\n"
	"\r\n"
	"out highp vec3  oPosition;\r\n"
	"out highp vec3  oVelocity;\r\n"
	"out highp vec3  oAttributes;\r\n"
	"\r\n"
	"void main() \r\n"
	"{ \t\r\n"
	"\tgl_Position = vec4(inPosition, 1.0);\r\n"
	"\toAttributes.x = inAttributes.x - TimeDelta;\r\n"
	"\toAttributes.y = inAttributes.y;\r\n"
	"\toAttributes.z = inAttributes.z;\r\n"
	"\t\r\n"
	"\t// Spawn at origin if it died\r\n"
	"\tif (oAttributes.x < 0.0)\r\n"
	"\t{\r\n"
	"\t\toPosition = vec3(0.0);\r\n"
	"\t\toVelocity = EmitDirection * inAttributes.y;\r\n"
	"\t\toAttributes.x = inAttributes.z;\r\n"
	"\t}\r\n"
	"\telse\r\n"
	"\t{\r\n"
	"\t\t// not realistic, but works for demo purposes\r\n"
	"\t\toVelocity = inVelocity + Force * TimeDelta;\r\n"
	"\t\toPosition = inPosition + oVelocity * TimeDelta;\r\n"
	"\t}\r\n"
	"} \r\n";

// Register FeedbackVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FeedbackVertShader_vsh("FeedbackVertShader.vsh", _FeedbackVertShader_vsh, 1061);

// ******** End: FeedbackVertShader.vsh ********

