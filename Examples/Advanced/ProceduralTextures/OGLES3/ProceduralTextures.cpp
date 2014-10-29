/******************************************************************************

 @File         ProceduralTextures.cpp

 @Title        ProceduralTextures

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Procedural texture example based on Steven Worley's  Cellular
               Texture Basis Functions.

******************************************************************************/

#include "ProceduralTextures.h"

/******************************************************************************
 Defines
******************************************************************************/

#define KERNEL_WORKGROUP_SIZE_X    8
#define KERNEL_WORKGROUP_SIZE_Y    4
#define NUM_SAMPLEPOINTS           4

/******************************************************************************
 Content file names
******************************************************************************/

const char  g_cszComputeShaderFile[] = "ComputeShader.csh";
const char* g_cszModeNames[] = { "Euclid", "Manhattan", "Chessboard" };

/*!****************************************************************************
 @Function		Constructor
 @Description	Constructor
******************************************************************************/
ProceduralTextures::ProceduralTextures()
{
	for (unsigned int i = 0; i < NUM_GENERATORS; i++)
	{ m_Programs[i] = 0; }
}

/*!****************************************************************************
 @Function		Destructor
 @Description	Destructor
******************************************************************************/
ProceduralTextures::~ProceduralTextures()
{ }

/*!****************************************************************************
 @Function		Init
 @Return		bool		true if no error occured
 @Description	Initializes context and acquires all required
                resources.
******************************************************************************/
bool ProceduralTextures::Init(CPVRTString* pErrorStr)
{
	const unsigned int numDefines = 4;
	char buffer[numDefines][64];
	char* pszDefines[numDefines];
	for (unsigned int i = 0; i < numDefines; i++)
	{ pszDefines[i] = buffer[i]; }
	sprintf(pszDefines[0], "NUM_SAMPLEPOINTS %d", NUM_SAMPLEPOINTS);
	sprintf(pszDefines[1], "WORKGROUPSIZE_X %d", KERNEL_WORKGROUP_SIZE_X);
	sprintf(pszDefines[2], "WORKGROUPSIZE_Y %d", KERNEL_WORKGROUP_SIZE_Y);

	GLuint tmpShader;

	for (unsigned int i = 0; i < NUM_GENERATORS; i++)
	{
		sprintf(pszDefines[3], "%s", g_cszModeNames[i]);
		if (PVRTShaderLoadFromFile(0, g_cszComputeShaderFile, GL_COMPUTE_SHADER, GL_SGX_BINARY_IMG, &tmpShader, pErrorStr, 0, pszDefines, numDefines) != PVR_SUCCESS)
		{
			*pErrorStr = ("Error: Failed to create shader " + CPVRTString(g_cszModeNames[i]) + (" \n" + *pErrorStr));
			return false;
		}

		if (PVRTCreateComputeProgram(&m_Programs[i], tmpShader, pErrorStr) != PVR_SUCCESS)
		{
			*pErrorStr = ("Error: Failed to link shader " + CPVRTString(g_cszModeNames[i]) + (" \n" + *pErrorStr));
			return false;
		}

		glDeleteShader(tmpShader);
	}
	return true;
}

/*!****************************************************************************
 @Function		Release
 @Description	Initializes context and acquires all required
******************************************************************************/
void ProceduralTextures::Release()
{
	glUseProgram(0);
	for (unsigned int i = 0; i < NUM_GENERATORS; i++) { if (m_Programs[i]) { glDeleteProgram(m_Programs[i]); m_Programs[i] = 0; } }
}


/*!****************************************************************************
 @Function		GenerateTexture
 @Return		bool		true if no error occurred
 @Description	Generates the procedural texture.
******************************************************************************/
bool ProceduralTextures::GenerateIntoTexture(const eGenerator generator, GLuint texture, float width, float height, float scalar/*=1.0f*/)
{
	const GLuint wg_x = (GLuint)ceil((float)width / (float)KERNEL_WORKGROUP_SIZE_X);
	const GLuint wg_y = (GLuint)ceil((float)height / (float)KERNEL_WORKGROUP_SIZE_Y);

	glBindImageTexture(0, texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUseProgram(m_Programs[generator]);
	GLint scale_idx = glGetUniformLocation(m_Programs[generator], "uniform_input_scale");
	glUniform1f(scale_idx, scalar);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glDispatchCompute(wg_x, wg_y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	return true;
}

/*!****************************************************************************
 @Function		GetModeDescription
 @Return		const char *	A textual description of the specified generator
 *****************************************************************************/
const char* ProceduralTextures::GetModeDescription(eGenerator generator) const
{
	return g_cszModeNames[generator];
}

/******************************************************************************
 End of file (ProceduralTextures.cpp)
******************************************************************************/

