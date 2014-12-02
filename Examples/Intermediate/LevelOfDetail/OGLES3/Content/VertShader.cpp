// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"/******************************************************************************\r\n"
	"* Vertex Shader\r\n"
	"******************************************************************************/\r\n"
	"/* \r\n"
	"\tThe vertex and fragment shaders implement two techniques for reflections.\r\n"
	"\tWhich version is used for drawing the object is dependent on the value of \r\n"
	"\tbHighDetail. If bHighDetail is true it uses the method described in \r\n"
	"\tOGLES3PerturbedUVs and for false it uses OGLES3Reflections. \r\n"
	"\t\r\n"
	"\tReason for using 2 methods is that when the object is far away you aren't\r\n"
	"\tgoing to notice all the detail that the PerturbedUVs method adds to\r\n"
	"\tthe mesh so you may as well use a simpler method for creating reflections.\r\n"
	"\tThis way you aren't using valuable resources on something you aren't \r\n"
	"\tgoing to notice.\r\n"
	"\r\n"
	"\tAlso, when the training course is in 'low detail' mode it uses a different mesh.\r\n"
	"\tThe mesh that is being drawn contains only 7% of the original meshes vertices.\r\n"
	"*/\r\n"
	"\r\n"
	"layout (location = 0) in highp vec3\tinVertex;\r\n"
	"layout (location = 1) in mediump vec3 inNormal;\r\n"
	"layout (location = 2) in mediump vec2 inTexCoord;\r\n"
	"layout (location = 3) in mediump vec3 inTangent;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump mat3  ModelWorld;\r\n"
	"uniform mediump vec3  EyePosModel;\r\n"
	"uniform bool          bHighDetail;\r\n"
	"\r\n"
	"out mediump vec3  EyeDirection;\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\r\n"
	"\r\n"
	"\t// Calculate direction from eye position in model space\r\n"
	"\tmediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);\r\n"
	"\r\n"
	"\tif (bHighDetail)\r\n"
	"\t{\t\r\n"
	"\t\t// transform light direction from model space to tangent space\r\n"
	"\t\tmediump vec3 binormal = cross(inNormal,inTangent);\r\n"
	"\t\tmediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);\r\n"
	"\t\tEyeDirection = eyeDirModel * tangentSpaceXform;\t\r\n"
	"\r\n"
	"\t\tTexCoord = inTexCoord;\r\n"
	"\t}\r\n"
	"\telse\r\n"
	"\t{\r\n"
	"\t\t// reflect eye direction over normal and transform to world space\r\n"
	"\t\tmediump vec3 reflectDir = ModelWorld * reflect(-eyeDirModel, inNormal);\r\n"
	"\t\t\r\n"
	"\t\tif(inNormal.z >= 0.0)// facing the viewer\r\n"
	"\t\t{\r\n"
	"\t\t\treflectDir = normalize(reflectDir) * 0.5 + 0.5;\r\n"
	"\t\t}\r\n"
	"\t\telse// facing away from the viewer\r\n"
	"\t\t{\r\n"
	"\t\t\treflectDir = -normalize(reflectDir) * 0.5 + 0.5;\r\n"
	"\t\t}\r\n"
	"\t\tTexCoord = reflectDir.xy;\r\n"
	"\t}\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 2303);

// ******** End: VertShader.vsh ********

