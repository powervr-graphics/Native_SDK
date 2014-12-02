/******************************************************************************

 @File         GameOfLife.cpp

 @Title        OGLParticleSystem

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Particle system completely animated with OpenCL. Requires the
               PVRShell.

******************************************************************************/

#include "GameOfLife.h"

/******************************************************************************
 Imprint "Life" to Memory
******************************************************************************/
void ImprintConstruction(unsigned int* pBuffer, const char* pConstruction, int cw, int ch, int xo, int yo, int width)
{
	// imprint construction to memory
	for (int y = 0; y < ch; y++)
		for (int x = 0; x < cw; x++)
			// check index
			if (pConstruction[y * cw + x])
				// set to red for the time being
			{ pBuffer[(y + yo)*width + (x + xo)] = 0xFF; }
}

/******************************************************************************
 Glider
******************************************************************************/
void ConstructGlider(unsigned int* pBuffer, int width, int xo, int yo)
{
	const char aGlider[] =
	{
		0, 1, 0,
		0, 0, 1,
		1, 1, 1
	};

	ImprintConstruction(pBuffer, aGlider, 3, 3, xo, yo, width);
}

/******************************************************************************
 Exploder
******************************************************************************/
void ConstructExploder(unsigned int* pBuffer, int width, int xo, int yo)
{
	const char aExploder[] =
	{
		1, 0, 1, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 1, 0, 1,
	};

	ImprintConstruction(pBuffer, aExploder, 5, 5, xo, yo, width);
}

/******************************************************************************
 Gosper Glider Gun
******************************************************************************/
void ConstructGosperGliderGun(unsigned int* pBuffer, int width, int xo, int yo)
{
	const char aGun[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	ImprintConstruction(pBuffer, aGun, 38, 15, xo, yo, width);
}


/******************************************************************************
 Blinker
******************************************************************************/
void ConstructBlinker(unsigned int* pBuffer, int width, int xo, int yo)
{
	const char aBlinker[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
	};

	ImprintConstruction(pBuffer, aBlinker, 3, 3, xo, yo, width);
}


/******************************************************************************
 Content file names
******************************************************************************/

const char g_cszComputeShaderFile[]     = "ComputeShader.csh";

/*!****************************************************************************
 @Function		Constructor
 @Description	Constructor
******************************************************************************/
GameOfLife::GameOfLife(SPVRTContext& context)
	: m_PvrContext(context), m_WorkGroupWidth(8), m_WorkGroupHeight(4),
	  m_Generation(0), m_SimulationUpdatePeriodFrames(0), initialized(false)
{
	memset(&m_glObjects, 0, sizeof(m_glObjects));
}

/*!****************************************************************************
 @Function		Destructor
 @Description	Destructor
******************************************************************************/
GameOfLife::~GameOfLife()
{
	Release();
}

/*!****************************************************************************
 @Function		Release
 @Description	Release all non-automatically managed resources
******************************************************************************/
void GameOfLife::Release()
{
	if (initialized)
	{
		glDeleteShader(m_glObjects.shader);
		glDeleteProgram(m_glObjects.program);
		glDeleteTextures(2, m_glObjects.textures);
		initialized = false;
	}
}

/*!****************************************************************************
 @Function		Init
 @Return		bool		true if no error occured
 @Description	Initializes the required buffers and other OpenGL objects
				required. Assumes a bound OpenGL context.
******************************************************************************/
bool GameOfLife::Init(CPVRTString& errorStr, unsigned int width, unsigned int height)
{
	m_Width = width; m_Height = height;

	glGetError();
	char defines_storage[6][128];

	char* defines[6];
	for (size_t i = 0; i < 6; ++i) { defines[i] = defines_storage[i]; }

	sprintf(defines[0], "IMAGE_BINDING_INPUT %d", IMAGE_UNIT_INPUT);
	sprintf(defines[1], "IMAGE_BINDING_OUTPUT %d", IMAGE_UNIT_OUTPUT);
	sprintf(defines[2], "WG_WIDTH %d", m_WorkGroupWidth);
	sprintf(defines[3], "WG_HEIGHT %d", m_WorkGroupHeight);
	sprintf(defines[4], "TOTAL_WIDTH %d", m_Width);
	sprintf(defines[5], "TOTAL_HEIGHT %d", m_Height);

	if (PVRTShaderLoadFromFile(0, g_cszComputeShaderFile, GL_COMPUTE_SHADER, GL_SGX_BINARY_IMG, &m_glObjects.shader, &errorStr, &m_PvrContext, defines, 6) != PVR_SUCCESS)
	{ return false; }

	m_glObjects.program = glCreateProgram();
	glAttachShader(m_glObjects.program, m_glObjects.shader);
	glLinkProgram(m_glObjects.program);

	GLint Linked;
	glGetProgramiv(m_glObjects.program, GL_LINK_STATUS, &Linked);

	if (!Linked)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(m_glObjects.program, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(m_glObjects.program, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		errorStr = CPVRTString("Failed to link: ") + pszInfoLog + "\n";
		delete [] pszInfoLog;
		return false;
	}

	if (!SetMode(GOL_RANDOM, m_Width, m_Height, errorStr)) { errorStr = "ChangeMode failed:\n" + errorStr; return false;	}
	m_Generation = 0;
	initialized = true;

	return true;
}


/*!****************************************************************************
 @Function		UpdateLife
 @Description	Updates particle positions and attributes, e.g. lifespan.
******************************************************************************/
bool GameOfLife::UpdateLife(CPVRTString& errorStr)
{
	static int step = 0;
	
	if (!initialized) { return false; }
	if (step-- > 0)
	{
		return true;
	}
	step = m_SimulationUpdatePeriodFrames;

	const char* errcode = 0;

	++m_Generation;
	if (GetGlErrorString(errorStr)) { errorStr = "Had error:" + errorStr; return false;	}

	glBindImageTexture(IMAGE_UNIT_INPUT, GetCurrentInputTexture(), 0, GL_FALSE, 0,  GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(IMAGE_UNIT_OUTPUT, GetCurrentOutputTexture(), 0, GL_FALSE, 0,  GL_WRITE_ONLY, GL_RGBA8);
	if (GetGlErrorString(errorStr)) { errorStr = "BindImageTexture :" + errorStr; return false;	}

	glUseProgram(m_glObjects.program);
	//The "ceil" shows the less desirable but more generic way to dispatch compute shaders: When they are not an exact multiple
	//of workgroup size. In that case, the edges of the domain will have some underutilized threads, and the compute shader itself will
	//probably need some kind of bounds checking. It is generally desirable, when we can, to have a domain that is an exact multiple of
	//workgroup size in all dimensions.
	glDispatchCompute((GLuint)ceil(m_Width / (float)m_WorkGroupWidth), (GLuint)ceil(m_Height / (float)m_WorkGroupHeight), 1);
	if (GetGlErrorString(errorStr)) { errorStr = "DispatchCompute :" + errorStr; return false;	}

	FlipTextures();
	if (GetGlErrorString(errorStr)) { errorStr = "FlipTextures :" + errorStr; return false;	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	if (GetGlErrorString(errorStr)) { errorStr = "Barrier :" + errorStr; return false;	}

	return true;
}


/*!****************************************************************************
  @Function		ChangeMode
  @Description	Changes the life simulation mode and initializes with a certain
                preset.
******************************************************************************/
bool GameOfLife::SetMode(eGOLMode mode, unsigned int width, unsigned int height, CPVRTString& errorStr)
{
	m_Generation = 0;
	// Clear screen
	m_Width = width; m_Height = height;
	m_pStartingState.Resize(m_Width * m_Height * sizeof(unsigned int));
	memset(&m_pStartingState[0], 0, m_Width * m_Height * sizeof(unsigned int));
	switch (mode)
	{
		case GOL_EXPLODERS:
		{
			// Randomly generate exploders
			for (unsigned int i = 0; i < NUM_RANDOM_ENTITIES * 5; i++)
			{
				float rx = rand() / (float)RAND_MAX;
				float ry = rand() / (float)RAND_MAX;
				int xo = 15 + (int)((m_Width - 30) * rx);
				int yo = 15 + (int)((m_Height - 30) * ry);
				ConstructExploder(&m_pStartingState[0], m_Width, xo, yo);
			}

			break;
		}

		case GOL_BLINKERS:
		{
			// Randomly generate blinkers
			for (unsigned int i = 0; i < NUM_RANDOM_ENTITIES * 14; i++)
			{
				float rx = rand() / (float)RAND_MAX;
				float ry = rand() / (float)RAND_MAX;
				int xo = 15 + (int)((m_Width - 30) * rx);
				int yo = 15 + (int)((m_Height - 30) * ry);
				ConstructBlinker(&m_pStartingState[0], m_Width, xo, yo);
			}

			break;
		}

		case GOL_GLIDERS:
		{
			// Randomly generate gliders
			for (unsigned int i = 0; i < NUM_RANDOM_ENTITIES * 8; i++)
			{
				float rx = rand() / (float)RAND_MAX;
				float ry = rand() / (float)RAND_MAX;
				int xo = 15 + (int)((m_Width - 30) * rx);
				int yo = 15 + (int)((m_Height - 30) * ry);
				ConstructGlider(&m_pStartingState[0], m_Width, xo, yo);
			}

			break;
		}

		case GOL_GOSPERGLIDERGUN:
		{
			// Randomly generate some gosper guns
			for (unsigned int i = 0; i < NUM_RANDOM_ENTITIES; i++)
			{
				float rx = rand() / (float)RAND_MAX;
				float ry = rand() / (float)RAND_MAX;
				int xo = 45 + (int)((m_Width - 45) * rx);
				int yo = 20 + (int)((m_Height - 20) * ry);
				ConstructGosperGliderGun(&m_pStartingState[0], m_Width, xo, yo);
			}

			break;
		}

		default://GOL_RANDOM
		{
			// Randomly generate population
			for (unsigned int y = 0; y < m_Height; y++)
				for (unsigned int x = 0; x < m_Width; x++)

				{
					m_pStartingState[y * m_Width + x] = (rand() / (float)RAND_MAX) > 0.5f ? 0x000000FF : 0;
				}
			break;
		}
	}

	CreateTextures();
	if (GetGlErrorString(errorStr))  { errorStr = "Texture generation failed:\n" + errorStr; return false;	}

	glActiveTexture(TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, GetCurrentInputTexture());
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to bind texture 0:\n" + errorStr; return false; }
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_Width, m_Height);
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to set texture storage 0:\n" + errorStr; return false; }
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA,  GL_UNSIGNED_BYTE, &m_pStartingState[0]);
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to set texture data 0:\n" + errorStr; return false; }

	glBindTexture(GL_TEXTURE_2D, GetCurrentOutputTexture());
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to bind texture 1:\n" + errorStr; return false; }
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_Width, m_Height);
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to set texture storage 1:\n" + errorStr; return false; }
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA,  GL_UNSIGNED_BYTE, &m_pStartingState[0]);
	if (GameOfLife::GetGlErrorString(errorStr))	{ errorStr = "Failed to set texture data 1:\n" + errorStr; return false; }
	return true;
}

/*!****************************************************************************
 @Function		InitOpenCL
 @Return		bool		true if no error occured
 @Description	Initializes an OpenCL context and acquires all required
                resources.
******************************************************************************/
const char* GameOfLife::GetModeDescription(eGOLMode mode)
{
	// Different presets to start evolving from
	static const char* cszModeNames[] = { "Random population", "Exploders", "Blinkers", "Gliders", "Gosper Glider Gun" };
	static char buffer[64];

	sprintf(buffer, "%s - Generation %d", cszModeNames[mode], GetGeneration());
	return buffer;
}

/******************************************************************************
 End of file (OGLGameOfLife.cpp)
******************************************************************************/

