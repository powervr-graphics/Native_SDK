/******************************************************************************

 @File         CSImageConvolution.h

 @Title        ImageConvolution

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to apply convolution kernels to images using Compute shaders

******************************************************************************/

#include "PVRShell.h"
#include "OGLES31Tools.h"

/******************************************************************************
 Defines
******************************************************************************/

// Default workgroup size
enum CONFIGURATION
{
	IMAGE_UNIT_INPUT = 0,
	IMAGE_UNIT_OUTPUT = 1,
	TEXTURE_UNIT = 0,
	SAMPLER_UNIT = 0,
	KERNEL_BENCHMARK_ITERATIONS = 10
};

/******************************************************************************
  Convolution Classes
******************************************************************************/
struct ConvolutionDescription
{
	const char* name;
	const char* description;
	unsigned int iterations;
	//unsigned int iterationStep;
	unsigned int radius;
	GLuint workgroupWidth;
	GLuint workgroupHeight;
};

/*
 *  Base class for all kinds of convolution kernels.
 */
class ConvolutionShader : public ConvolutionDescription
{
public:
	ConvolutionShader();
	ConvolutionShader(const ConvolutionDescription& description) : ConvolutionDescription(description), iteration(0) { }

	const char* getDescription() const {return description; }

	void imprintDescription(char* buffer)
	{
		if (iterations > 1)
		{ sprintf(buffer, "%s (%d passes)", description, iterations); }
		else { sprintf(buffer, "%s", description); }
	}

	//bool dispatch(unsigned int u32ImageWidth, unsigned int u32ImageHeight, unsigned int u32NumIterations);
	
	GLuint program;
	GLuint shader;
	GLuint buffer;
	GLuint sampler;
	unsigned int iteration;
};



/*!****************************************************************************
 Class implementing image processing functionality
******************************************************************************/
class CSImageConvolution
{
	//CPVRTArray<unsigned char > m_imageData;
	unsigned int   m_imageWidth;
	unsigned int   m_imageHeight;
	SPVRTContext & m_context;
	//Original image
	GLuint m_textures[3];
	size_t m_CurrentInputTexture;

	size_t m_currentFilter;
	bool m_initialised;

	CPVRTArray<ConvolutionShader> m_Filters;

public:

	CSImageConvolution(SPVRTContext & context);
	~CSImageConvolution();

	bool init(CPVRTString& errorStr);
	void release();

	void nextFilter() { ++m_currentFilter; m_currentFilter %= (m_Filters.GetSize()+1); }
	void previousFilter() { m_currentFilter+=m_Filters.GetSize(); m_currentFilter %= (m_Filters.GetSize()+1); }

	//unsigned int getNumFilters() const { return m_Filters.GetSize(); }
	//unsigned int getFilterMaxIterations() const { return m_Filters[m_currentFilter].iterations; }

	void imprintCurrentFilterDescription(char* buffer)
	{
		if (m_initialised)
		{
			if (m_currentFilter>=m_Filters.GetSize())
			{
				sprintf(buffer, "Original Image");
			}
			else
			{
				m_Filters[m_currentFilter].imprintDescription(buffer);
			}
		}else 
		{
			buffer[0]='\0';
		}
	}

	GLuint getOriginalImage() { 
		return m_textures[0]; }
	GLuint getCurrentInputTexture() { 
		return m_textures[m_CurrentInputTexture]; }
	GLuint getCurrentOutputTexture() { 
		return m_CurrentInputTexture ? m_textures[m_CurrentInputTexture ^ 3] : m_textures[1]; } //current texture : in 0 => out 1, in 1 => out 2, in 2=> out 1 (avoid 0!)

	/*flipTextures binds the output temporary image as input, and a suitable temp texture as output.
	  It never binds the original as output. Xor 3 there turns 1 to 2 and 2 to 1*/
	void flipTextures() { m_CurrentInputTexture = m_CurrentInputTexture ? m_CurrentInputTexture ^= 3 : 1; }
	/*resetTextures binds the original image as input and a suitable temp texture as output.*/
	void resetTextures() { m_CurrentInputTexture = 0; }

	bool executeCurrentFilter();
	void executeFilterPasses(GLuint glProgram, int groups_x, int groups_y, unsigned int passes);
};


/******************************************************************************
 End of file (CSImageConvolution.h)
******************************************************************************/

