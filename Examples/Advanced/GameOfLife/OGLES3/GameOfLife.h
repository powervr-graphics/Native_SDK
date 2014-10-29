/******************************************************************************

 @File         OGLES3GameOfLife.h

 @Title        GameOfLife

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Game of life simulation in OpenCL. Requires the PVRShell.

******************************************************************************/
#ifndef _OGLES3GAMEOFLIFE__H_
#define _OGLES3GAMEOFLIFE__H_

#include "OGLES31Tools.h"

/******************************************************************************
 Enumerations
******************************************************************************/

enum eGOLMode
{
  GOL_RANDOM,
  GOL_EXPLODERS,
  GOL_BLINKERS,
  GOL_GLIDERS,
  GOL_GOSPERGLIDERGUN,
  GOL_NUM_MODES
};

/******************************************************************************
 Defines
******************************************************************************/

#define SIZEOF_RGBA 4
#define TEXTURE_UNIT GL_TEXTURE0
#define NUM_RANDOM_ENTITIES             16
#define IMAGE_UNIT_INPUT 0
#define IMAGE_UNIT_OUTPUT 1


/******************************************************************************
 Structure definitions
******************************************************************************/

struct OpenGLObjects
{
	GLuint			program;
	GLuint			shader;
	GLuint			sampler;
	GLuint			textures[2];
	unsigned char	currentTexture;
};

/*!****************************************************************************
 Class implementing the Game of Life
******************************************************************************/
class GameOfLife
{
public:
	GameOfLife(SPVRTContext& m_PvrContext);
	~GameOfLife();

	SPVRTContext& m_PvrContext;

	PVRTuint32 m_Width;
	PVRTuint32 m_Height;
	PVRTuint32 m_WorkGroupHeight;
	PVRTuint32 m_WorkGroupWidth;
	PVRTint32 m_SimulationUpdatePeriodFrames;

	PVRTint32 m_Generation;

	CPVRTArray<unsigned int> m_pStartingState;

	eGOLMode m_uiMode;

	OpenGLObjects m_glObjects;
	bool initialized;

	void CreateTextures()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		if (initialized)
		{
			glDeleteTextures(2, m_glObjects.textures);
		}
		//Create the textures
		glGenTextures(2, m_glObjects.textures);

		glBindTexture(GL_TEXTURE_2D, GetCurrentInputTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, GetCurrentOutputTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

public:

	bool Init(CPVRTString& errorStr, unsigned int width, unsigned int height);
	void Release();

	bool UpdateLife(CPVRTString& errorStr);

	bool SetMode(eGOLMode mode, unsigned int width, unsigned int height, CPVRTString& error);
	const char* GetModeDescription(eGOLMode mode);

	GLuint GetCurrentInputTexture() const { return m_glObjects.textures[m_glObjects.currentTexture]; }
	GLuint GetCurrentOutputTexture() const { return m_glObjects.textures[m_glObjects.currentTexture ^ 1]; }
	PVRTint32 GetGeneration() { return m_Generation; }
	PVRTint32 GetSimulationUpdatePeriod() { return m_SimulationUpdatePeriodFrames; }
	void SetSimulationUpdatePeriod(PVRTint32 period) { m_SimulationUpdatePeriodFrames = period; }
	void FlipTextures() { m_glObjects.currentTexture = m_glObjects.currentTexture ^ 1; }


	inline static bool GetGlErrorString(CPVRTString& errorStr)
	{
#ifdef GOL_DEBUG
		GLuint err = glGetError();
		if (!err) { return 0; }
		switch (err)
		{
			case GL_INVALID_ENUM : errorStr = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE : errorStr = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION : errorStr = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION : errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_OUT_OF_MEMORY : errorStr = "GL_OUT_OF_MEMORY"; break;
			case GL_STACK_UNDERFLOW_KHR : errorStr = "GL_STACK_UNDERFLOW"; break;
			case GL_STACK_OVERFLOW_KHR : errorStr = "GL_STACK_OVERFLOW"; break;
			default : errorStr = "UNKNOWN_ERROR";
		}
		return true;
#else
		return false;
#endif
	}
};

#endif //_OGLES3GAMEOFLIFE__H_

/******************************************************************************
 End of file (OGLGameOfLife.h)
******************************************************************************/

