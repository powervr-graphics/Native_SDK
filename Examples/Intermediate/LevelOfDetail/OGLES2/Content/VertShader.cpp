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
	"/******************************************************************************\n"
	"* Vertex Shader\n"
	"******************************************************************************/\n"
	"/* \n"
	"\tThe vertex and fragment shaders implement two techniques for reflections.\n"
	"\tWhich version is used for drawing the object is dependent on the value of \n"
	"\tbHighDetail. If bHighDetail is true it uses the method described in \n"
	"\tOGLES2PerturbedUVs and for false it uses OGLES2Reflections. \n"
	"\t\n"
	"\tReason for using 2 methods is that when the object is far away you aren't\n"
	"\tgoing to notice all the detail that the PerturbedUVs method adds to\n"
	"\tthe mesh so you may as well use a simpler method for creating reflections.\n"
	"\tThis way you aren't using valuable resources on something you aren't \n"
	"\tgoing to notice.\n"
	"\n"
	"\tAlso, when the training course is in 'low detail' mode it uses a different mesh.\n"
	"\tThe mesh that is being drawn contains only 7% of the original meshes vertices.\n"
	"*/\n"
	"\n"
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"attribute mediump vec3  inTangent;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump mat3  ModelWorld;\n"
	"uniform mediump vec3  EyePosModel;\n"
	"uniform bool          bHighDetail;\n"
	"\n"
	"varying mediump vec3  EyeDirection;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\n"
	"\n"
	"\t// Calculate direction from eye position in model space\n"
	"\tmediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);\n"
	"\n"
	"\tif (bHighDetail)\n"
	"\t{\t\n"
	"\t\t// transform light direction from model space to tangent space\n"
	"\t\tmediump vec3 binormal = cross(inNormal,inTangent);\n"
	"\t\tmediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);\n"
	"\t\tEyeDirection = eyeDirModel * tangentSpaceXform;\t\n"
	"\n"
	"\t\tTexCoord = inTexCoord;\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\t// reflect eye direction over normal and transform to world space\n"
	"\t\tmediump vec3 reflectDir = ModelWorld * reflect(-eyeDirModel, inNormal);\n"
	"\t\t\n"
	"\t\tif(inNormal.z >= 0.0)// facing the viewer\n"
	"\t\t{\n"
	"\t\t\treflectDir = normalize(reflectDir) * 0.5 + 0.5;\n"
	"\t\t}\n"
	"\t\telse// facing away from the viewer\n"
	"\t\t{\n"
	"\t\t\treflectDir = -normalize(reflectDir) * 0.5 + 0.5;\n"
	"\t\t}\n"
	"\t\tTexCoord = reflectDir.xy;\n"
	"\t}\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 2173);

// ******** End: VertShader.vsh ********

