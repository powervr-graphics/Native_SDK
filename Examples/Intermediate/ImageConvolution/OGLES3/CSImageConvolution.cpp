/******************************************************************************

 @File         CSImageConvolution.cpp

 @Title        ImageConvolution

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to apply convolution kernels to images using OpenCL

******************************************************************************/
#include <string.h>
#include "CSImageConvolution.h"

/******************************************************************************
 Content file names
******************************************************************************/
const char c_ImageTextureFile[]	= "Image.pvr";
const char c_ComputeShaderSrcFile[] = "ComputeShader.csh";

/******************************************************************************
 Shader definitions & config
******************************************************************************/
ConvolutionDescription c_FilterDefinitions[] =
{
	//						type, description, maxIterations, iterationStep, workgroupWidth, workgroupHeight
	{ "GAUSSIAN", "Gaussian blur 3x3", 1, 2, 16, 16},
	{ "GAUSSIAN", "Gaussian blur 3x3", 3, 2, 16, 16},
	{ "SHARPEN", "Sharpen 3x3", 1, 2, 16, 16 },
	{ "ERODE", "Erode 3x3", 1, 2, 16, 16 },
	{ "DILATE", "Dilate 3x3", 1, 2, 16, 16 },
	{ "EMBOSS", "Emboss 3x3", 1, 2, 16, 16 },
	{ "GRADIENT_LAPLACE", "Bidirectional Laplace filter", 1, 2, 16, 16 },
	{ "GRADIENT_SOBEL", "Sobel filter", 1, 2, 16, 16 },
	//{ "EDGEDETECT_SOBEL", "Sobel edge detection", 1, 2, 16, 16 },
};

const unsigned int c_NumFilters = sizeof(c_FilterDefinitions) / sizeof(c_FilterDefinitions[0]);

/******************************************************************************
  Convolution Classes
******************************************************************************/

ConvolutionShader::ConvolutionShader()
{
}

/*!****************************************************************************
 @Function		Init
 @Return		bool		true if no error occurred
 @Description	Creates the image processing kernels.
******************************************************************************/
bool CSImageConvolution::init(CPVRTString& errorStr)
{
	char defines_storage[6][128];

	char* defines[6];
	for (size_t i = 0; i < 6; ++i) { defines[i] = defines_storage[i]; }

	sprintf(defines[0], "IMAGE_BINDING_INPUT %d", IMAGE_UNIT_INPUT);
	sprintf(defines[1], "IMAGE_BINDING_OUTPUT %d", IMAGE_UNIT_OUTPUT);

	release();
	for (size_t i = 0; i < c_NumFilters; ++i)
	{
		m_Filters.Append(c_FilterDefinitions[i]);
		//CREATE ALL THE DIFFERENT SHADERS

		sprintf(defines[2], "FILTER_RADIUS %d", m_Filters[i].radius);
		sprintf(defines[3], "WG_WIDTH %d", m_Filters[i].workgroupWidth);
		sprintf(defines[4], "WG_HEIGHT %d", m_Filters[i].workgroupHeight);
		sprintf(defines[5], "%s", m_Filters[i].name);


		if (PVRTShaderLoadFromFile(0, c_ComputeShaderSrcFile, GL_COMPUTE_SHADER, GL_SGX_BINARY_IMG, &m_Filters[i].shader, &errorStr, &m_context, defines, 6) != PVR_SUCCESS)
		{ return false; }
		if (PVRTCreateComputeProgram(&m_Filters[i].program, m_Filters[i].shader, &errorStr) != PVR_SUCCESS)
		{ return false; }
	}


	// Load image file: has to be uncompressed and 4 channel 32bit
	CPVRTResourceFile imgfile(c_ImageTextureFile);
	if (!imgfile.IsOpen())
	{
		errorStr = "Error: Failed to open image file.";
		return false;
	}

	const unsigned char* pData = (const unsigned char*)imgfile.DataPtr();
	const PVRTextureHeaderV3* pHeader = (const PVRTextureHeaderV3*)pData;
	m_imageWidth = (unsigned int)pHeader->u32Width;
	m_imageHeight = (unsigned int)pHeader->u32Height;

	if (pHeader->u64PixelFormat != PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8))
	{
		errorStr = "Error: Only uncompressed 4 channel 32-bit PVR images supported.";
		return false;
	}

	pData += PVRTEX3_HEADERSIZE + pHeader->u32MetaDataSize;

	//Create textures and give them the data
	resetTextures();
	glGenTextures(3, m_textures);
	for (int i = 0; i < 3; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_imageWidth, m_imageHeight);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	imgfile.Close();

	m_initialised = true;
	previousFilter();
	return true;
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependent on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
CSImageConvolution::CSImageConvolution(SPVRTContext& context)
	: m_initialised(false), m_context(context), m_currentFilter(0), m_CurrentInputTexture(0)
{
}


/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
CSImageConvolution::~CSImageConvolution()
{
	release();
}

void CSImageConvolution::release()
{
	if (m_initialised)
	{
		for (size_t i = 0; i < m_Filters.GetSize(); ++i)
		{
			glDeleteProgram(m_Filters[i].program);
			glDeleteShader(m_Filters[i].shader);
		}
		m_Filters.Clear();

		glDeleteTextures(3, m_textures);
		m_initialised = false;
	}
}

bool CSImageConvolution::executeCurrentFilter()
{
	if (m_currentFilter >= m_Filters.GetSize())
	{
		m_CurrentInputTexture = 0;
		return true;
	}
	ConvolutionShader& filter = m_Filters[m_currentFilter];
	const GLuint groups_x = static_cast<GLuint>(ceil((float)m_imageWidth / filter.workgroupWidth));
	const GLuint groups_y = static_cast<GLuint>(ceil((float)m_imageHeight / filter.workgroupHeight));

	executeFilterPasses(filter.program, groups_x, groups_y, filter.iterations);
	
	return true;
}

void CSImageConvolution::executeFilterPasses( GLuint glProgram, int groups_x, int groups_y, unsigned int passes )
{
	for (unsigned int i = 0; i < passes; ++i)
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glUseProgram(glProgram);
		glBindImageTexture(IMAGE_UNIT_INPUT, getCurrentInputTexture(), 0, 0, 0, GL_READ_ONLY, GL_RGBA8);
		glBindImageTexture(IMAGE_UNIT_OUTPUT, getCurrentOutputTexture(), 0, 0, 0, GL_WRITE_ONLY, GL_RGBA8);
		glDispatchCompute(groups_x, groups_y, 1);
		flipTextures();
	}
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}
/******************************************************************************
 End of file (CSImageConvolution.cpp)
******************************************************************************/

