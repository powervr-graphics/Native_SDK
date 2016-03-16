/*!*********************************************************************************************************************
\File         OGLESIntroducingPVRAssets.cpp
\Title        Introducing the POD 3D file format
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to load POD files and play the animation with basic  lighting
***********************************************************************************************************************/
//Main include file for the PVRShell library. Use this file when you will not be linking the PVRApi library.
#include "PVRShell/PVRShellNoPVRApi.h"
//Main include file for the PVRAssets library.
#include "PVRAssets/PVRAssets.h"

//Used to manually decompress PVRTC compressed textures on platforms that do not support them.
#include "PVRAssets/Texture/PVRTDecompress.h"

//The OpenGL ES bindings used throughout this SDK. Use by calling gl::initGL and then using all the OpenGL ES functions from the gl::namespace.
// (So, glTextImage2D becomes gl::TexImage2D)
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

using namespace pvr;
using namespace pvr::types;
// Index to bind the attributes to vertex shaders
const pvr::uint32 VertexArray = 0;
const pvr::uint32 NormalArray = 1;
const pvr::uint32 TexCoordArray = 2;

//Shader files
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

//POD scene file
const char SceneFile[] = "GnomeToy.pod";

const pvr::StringHash AttribNames[] =
{
	"POSITION",
	"NORMAL",
	"UV0",
};

/*!*********************************************************************************************************************
\brief Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESIntroducingPVRAssets : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle	scene;

	// OpenGL handles for shaders, textures and VBOs
	pvr::uint32 vertShader;
	pvr::uint32 fragShader;
	std::vector<GLuint> vbo;
	std::vector<GLuint> indexVbo;
	std::vector<GLuint> texDiffuse;
	// Group shader programs and their uniform locations together
	struct
	{
		GLuint handle;
		pvr::uint32 uiMVPMatrixLoc;
		pvr::uint32 uiLightDirLoc;
		pvr::uint32 uiWorldViewITLoc;
	} ShaderProgram;

	// Variables to handle the animation in a time-based manner
	pvr::float32 frame;
	glm::mat4 projection;
public:
	//pvr::Shell implementation.
	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

	bool loadTextures();
	bool loadShaders();
	bool LoadVbos();

	Result::Enum loadTexturePVR(const StringHash& filename, GLuint& outTexHandle,
	                            pvr::assets::Texture* outTexture, assets::TextureHeader* outDescriptor);

	void drawMesh(int i32NodeIndex);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS EXTRACTED FROM THE REST OF THE POWERVR FRAMEWORK
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool getOpenGLFormat(PixelFormat pixelFormat, ColorSpace::Enum colorSpace, VariableType::Enum dataType,
                     uint32& glInternalFormat, uint32& glFormat, uint32& glType, uint32& glTypeSize,
                     bool& isCompressedFormat);

/*!*********************************************************************************************************************
\brief	Validate if required extension is supported
\return	Return true if the the extension is supported
\param	extension Extension to validate
***********************************************************************************************************************/
bool isExtensionSupported(const char* extension)
{
	static const GLubyte* extensionString = gl::GetString(GL_EXTENSIONS);
	// from http://opengl.org/resources/features/OGLextensions/
	const unsigned char* start = extensionString;
	char* position;

	// Extension names should not have spaces.
	position = (char*)strchr(extension, ' ');

	if (position || *extension == '\0') { return true; }
	return false;
}


/*!*********************************************************************************************************************
\brief	Get an OpenGl format from the description of a Texture format in PVR Assets (Code extracted from PVRApi)
\return	Return true on success
\param	pixelFormat The pixel/texel format, as a pvr::PixelFormat (RGBA 8888 etc.)
\param	colorSpace	The colorspace as a pvr::Colorspaces (linear RGB or SRGB)
\param	dataType	The variable type (Float, Unsigned Byte etc.)
\param[out] glInternalFormat	Output: The GLenum that should be passed as the "internalFormat" parameter in GLES calls
\param[out] glFormat			Output: The GLenum that should be passed as the "format" parameter in GLES calls
\param[out] glType				Output: The GLenum that should be passed as the "glType" parameter in GLES calls
\param[out] glTypeSize			Output: The GLenum that should be passed as the "glTypeSize" parameter in GLES calls
\param[out] isCompressedFormat	Output: True is the format is compressed, false otherwise.
***********************************************************************************************************************/
bool getOpenGLFormat(PixelFormat pixelFormat, ColorSpace::Enum colorSpace, VariableType::Enum dataType,
                     uint32& glInternalFormat, uint32& glFormat, uint32& glType, uint32& glTypeSize,
                     bool& isCompressedFormat)
{
	isCompressedFormat = (pixelFormat.getPart().High == 0)
	                     && (pixelFormat.getPixelTypeId() != CompressedPixelFormat::SharedExponentR9G9B9E5);
	if (pixelFormat.getPart().High == 0)
	{
		//Format and type == 0 for compressed textures.
		glFormat = 0;
		glType = 0;
		glTypeSize = 1;
		switch (pixelFormat.getPixelTypeId())
		{
		case CompressedPixelFormat::PVRTCI_2bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			return true;
		}
		case CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			return true;
		}
		case CompressedPixelFormat::PVRTCI_4bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			return true;
		}
		case CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			return true;
		}
#if !defined(TARGET_OS_IPHONE)
		case CompressedPixelFormat::PVRTCII_2bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
			return true;
		}
		case CompressedPixelFormat::PVRTCII_4bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
			return true;
		}
#endif
		case CompressedPixelFormat::SharedExponentR9G9B9E5:
		{
			//Not technically a compressed format by OpenGL ES standards.
			glType = GL_UNSIGNED_INT_5_9_9_9_REV;
			glTypeSize = 4;
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB9_E5;
			return true;
		}
		case CompressedPixelFormat::ETC2_RGB:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
			};
			return true;
		}
		case CompressedPixelFormat::ETC2_RGBA:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			return true;
		}
		case CompressedPixelFormat::ETC2_RGB_A1:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			return true;
		}
		case CompressedPixelFormat::EAC_R11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_R11_EAC;
			}
			return true;
		}
		case CompressedPixelFormat::EAC_RG11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RG11_EAC;
			}
			return true;
		}
		//Formats not supported by opengl/opengles
		case CompressedPixelFormat::BC4:
		case CompressedPixelFormat::BC5:
		case CompressedPixelFormat::BC6:
		case CompressedPixelFormat::BC7:
		case CompressedPixelFormat::RGBG8888:
		case CompressedPixelFormat::GRGB8888:
		case CompressedPixelFormat::UYVY:
		case CompressedPixelFormat::YUY2:
		case CompressedPixelFormat::BW1bpp:
		case CompressedPixelFormat::NumCompressedPFs:
			return false;
		}
	}
	else
	{
		switch (dataType)
		{
		case VariableType::UnsignedFloat:
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID)
			{
				glTypeSize = 4;
				glType = GL_UNSIGNED_INT_10F_11F_11F_REV;
				glFormat = GL_RGB;
				glInternalFormat = GL_R11F_G11F_B10F;
				return true;
			}
			break;
		case VariableType::SignedFloat:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			//HALF_FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG16F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R16F;
				return true;
			}
			//FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA32F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB32F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG32F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R32F;
				return true;
			}
			case assets::GeneratePixelType1<'d', 24>::ID:
			{
				glType = GL_UNSIGNED_INT;
				glTypeSize = 3;
				glInternalFormat = GL_DEPTH_COMPONENT24_OES;
				glFormat = GL_DEPTH_COMPONENT;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8_ALPHA8;
				}
				else
				{
					glInternalFormat = GL_RGBA8;
				}
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = glInternalFormat = GL_RGB;
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8;
				}
				else
				{
					glInternalFormat = GL_RGB8;
				}
				return true;
			}

			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 8>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA8_SNORM;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8_SNORM;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8UI;
				//glInternalFormat = GL_RGBA;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8UI;
				//glInternalFormat = GL_RGB;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8UI;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				glType = GL_UNSIGNED_SHORT_4_4_4_4;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA4;
				return true;
			}
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_5_5_1;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB5_A1;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_6_5;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB565;
				return true;
			}

			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			glTypeSize = 2;
			glType = GL_SHORT;
			switch (pixelFormat.getPixelTypeId())
			{

			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16UI;
				return true;
			}
			case assets::GeneratePixelType1<'d', 16>::ID:
			{
				glFormat = GL_DEPTH_COMPONENT;
				glInternalFormat = GL_DEPTH_COMPONENT16;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			glType = GL_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			glTypeSize = 4;
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB10_A2;
				return true;
			}
			break;
		}
		case VariableType::UnsignedInteger:
		{
			glType = GL_UNSIGNED_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32UI;
				return true;
			}
			case assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGB10_A2UI;
				return true;
			}
			case assets::GeneratePixelType1<'d', 24>::ID:
			{
				glFormat = GL_DEPTH_COMPONENT;
				glInternalFormat = GL_DEPTH_COMPONENT24_OES;
				return true;
			}
			case assets::GeneratePixelType2<'d', 's', 24, 8>::ID:
			{
				glFormat = GL_DEPTH_STENCIL_OES;
				glInternalFormat = GL_DEPTH24_STENCIL8_OES;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			glType = GL_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32I;
				return true;
			}
			}
			break;
		}
		default: {}
		}
	}
	//Default (erroneous) return values.
	glTypeSize = glType = glFormat = glInternalFormat = 0;
	return false;
}

/*!*********************************************************************************************************************
\brief	Create Shader program from vertex and fragment shader. Logs error if one occurs. (Code Extracted from PVRApi.)
\return	Return pvr::Result::Success on success
\param	shaders[] An array of OpenGL ES compiled shaders that will be combined into a shader program
\param	count The number of shaders into shaders
\param	attribs An array of attribute names, which will each be assigned successive attribute locations.
\param	attribCount The number of attributes in attribs
\param[out]	shaderProg The shader program, if successful.
***********************************************************************************************************************/
Result::Enum createShaderProgram(GLuint shaders[], uint32 count, const char** attribs, uint32 attribCount, GLuint& shaderProg)
{
	shaderProg = gl::CreateProgram();
	for (uint32 i = 0; i < count; ++i) { gl::AttachShader(shaderProg, shaders[i]); }

	if (attribs && attribCount)
	{
		for (uint32 i = 0; i < attribCount; ++i) { gl::BindAttribLocation(shaderProg, i, attribs[i]); }
	}
	gl::LinkProgram(shaderProg);
	//check for link success
	GLint glStatus;
	gl::GetProgramiv(shaderProg, GL_LINK_STATUS, &glStatus);
	if (!glStatus)
	{
		std::string infolog;
		int32 infoLogLength, charWriten;
		gl::GetProgramiv(shaderProg, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog.resize(infoLogLength);
		if (infoLogLength)
		{
			gl::GetProgramInfoLog(shaderProg, infoLogLength, &charWriten, &(infolog)[0]);
			Log(Log.Debug, infolog.c_str());
		}
		return Result::InvalidData;
	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\brief	Load shader from stream
\return	Return pvr::Result::Success on success
\param	shaderSource Shader source to load
\param	shaderType Type of the shader, e.g VertexShader, FragmentShader
\param	defines Shader Defines
\param	defineCount Number of defines in \p defines
\param	outShader Returned Opengles shader
***********************************************************************************************************************/
pvr::Result::Enum loadShader(const Stream& shaderSource, ShaderType::Enum shaderType, const char* const* defines, uint32 defineCount, GLuint& outShader)
{
	if (!shaderSource.isopen())
	{
		if (!shaderSource.open()) { return Result::UnableToOpen; }
	}

	string shaderSrc;
	if (!shaderSource.readIntoString(shaderSrc)) { return Result::UnableToOpen; };

	switch (shaderType)
	{
	case ShaderType::VertexShader:
		outShader = gl::CreateShader(GL_VERTEX_SHADER);
		break;
	case ShaderType::FragmentShader:
		outShader = gl::CreateShader(GL_FRAGMENT_SHADER);
		break;
	default:
		Log("loadShader: Unknown shader type requested.");
		return Result::InvalidArgument;
	}
	string::size_type versBegin = shaderSrc.find("#version");
	string::size_type versEnd = 0;
	string sourceDataStr;
	if (versBegin != string::npos)
	{
		versEnd = shaderSrc.find("\n", versBegin);
		sourceDataStr.append(shaderSrc.begin() + versBegin, shaderSrc.begin() + versBegin + versEnd);
		sourceDataStr.append("\n");
	}
	else
	{
		versBegin = 0;
	}
	// insert the defines
	for (uint32 i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc.begin() + versBegin + versEnd, shaderSrc.end());
	const char* pSource = sourceDataStr.c_str();
	gl::ShaderSource(outShader, 1, &pSource, NULL);
	gl::CompileShader(outShader);
	// error checking
	GLint glRslt;
	gl::GetShaderiv(outShader, GL_COMPILE_STATUS, &glRslt);
	if (!glRslt)
	{
		int infoLogLength, charsWritten;
		// get the length of the log
		gl::GetShaderiv(outShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> pLog;
		pLog.resize(infoLogLength);
		gl::GetShaderInfoLog(outShader, infoLogLength, &charsWritten, pLog.data());
		const char* const typestring =
		  (shaderType == ShaderType::VertexShader ? "Vertex" :
		   shaderType == ShaderType::FragmentShader ? "Fragment" :
		   shaderType == ShaderType::ComputeShader ? "Compute" :
		   shaderType == ShaderType::FrameShader ? "Frame" :
		   shaderType == ShaderType::RayShader ? "Ray" : "Unknown");
		Log(Log.Error,
		    strings::createFormatted("Failed to compile %s shader.\n ==========Infolog:==========\n%s\n============================",
		                             typestring, pLog.data()).c_str());
		return Result::InvalidData;
	}
	return Result::Success;
}


/*!*********************************************************************************************************************
\brief	Upload the texture (extracted from PVRApi)
\return	Return pvr::Result::Success on success
\param	texture Texture to upload
\param	outTextureName Return a handle to uploaded opengles texture
\param	apiType What api used to upload the texture to
\param	allowDecompress Allow decompress the texture if it is a compressed texture when uploading
***********************************************************************************************************************/
Result::Enum textureUpload(const assets::Texture& texture, GLuint& outTextureName, pvr::Api::Enum apiType, bool allowDecompress/*=true*/)
{
	using namespace assets;
	/*****************************************************************************
	* Initial error checks
	*****************************************************************************/
	// Check for any glError occurring prior to loading the texture, and warn the user.
	assertion(&texture != NULL, "TextureUtils.h:textureUpload:: Invalid Texture");

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		return Result::UnsupportedRequest;
	}

	/*****************************************************************************
	* Setup code to get various state
	*****************************************************************************/
	// Generic error strings for textures being unsupported.
	const char8* cszUnsupportedFormat =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";
	const char8* cszUnsupportedFormatDecompressionAvailable =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation. Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	// Get the texture format for the API.
	GLenum glInternalFormat = 0;
	GLenum glFormat = 0;
	GLenum glType = 0;
	GLenum glTypeSize = 0;
	bool unused;

	// Check that the format is a valid format for this API - Doesn't check specifically between
	// OpenGL/ES, it simply gets the values that would be set for a KTX file.
	if (!getOpenGLFormat(texture.getPixelFormat(), texture.getColorSpace(), texture.getChannelType(),
	                     glInternalFormat, glFormat, glType, glTypeSize, unused))
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		return Result::UnsupportedRequest;
	}

	// Is the texture compressed? RGB9E5 is treated as an uncompressed texture in OpenGL/ES so
	// is a special case.
	bool isCompressedFormat = (texture.getPixelFormat().getPart().High == 0)
	                          && (texture.getPixelFormat().getPixelTypeId() != CompressedPixelFormat::SharedExponentR9G9B9E5);

	//Whether we should use TexStorage or not.

	bool isEs2 = apiType < Api::OpenGLES3;
	bool useTexStorage = !isEs2;
	bool needsSwizzling = false;
	GLenum swizzle_r, swizzle_g, swizzle_b, swizzle_a;

	//Texture to use if we decompress in software.
	assets::Texture cDecompressedTexture;

	// Texture pointer which points at the texture we should use for the function. Allows
	// switching to, for example, a decompressed version of the texture.
	const assets::Texture* textureToUse = &texture;

	//Default texture target, modified as necessary as the texture type is determined.
	GLenum texTarget = GL_TEXTURE_2D;

	/*****************************************************************************
	* Check that extension support exists for formats supported in this way.
	*****************************************************************************/
	{
		//Check for formats that cannot be supported by this context version
		switch (glFormat)
		{
		case GL_LUMINANCE:
			if (!isEs2)
			{
				Log(Log.Information, "LUMINANCE texture format detected in OpenGL ES 3+ context. "
				    "Remapping to RED texture with swizzling (r,r,r,1) enabled.");
				needsSwizzling = true;
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				swizzle_r = GL_RED;
				swizzle_g = GL_RED;
				swizzle_b = GL_RED;
				swizzle_a = GL_ONE;
			}
			break;
		case GL_ALPHA:
			if (!isEs2)
			{
				Log(Log.Information, "ALPHA format texture detected in OpenGL ES 3+ context. "
				    "Remapping to RED texture with swizzling (0,0,0,r) enabled in order to allow"
				    " Texture Storage.");
				needsSwizzling = true;
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				swizzle_r = GL_ZERO;
				swizzle_g = GL_ZERO;
				swizzle_b = GL_ZERO;
				swizzle_a = GL_RED;
			}
			break;
		case GL_LUMINANCE_ALPHA:
			if (!isEs2)
			{
				Log(Log.Information, "LUMINANCE/ALPHA format texture detected in OpenGL ES 3+ "
				    "context. Remapping to RED texture with swizzling (r,r,r,g) enabled in order"
				    " to allow Texture Storage.");
				needsSwizzling = true;
				glFormat = GL_RG;
				glInternalFormat = GL_RG8;
				swizzle_r = GL_RED;
				swizzle_g = GL_RED;
				swizzle_b = GL_RED;
				swizzle_a = GL_GREEN;
			} break;
		case GL_RED:
			if (isEs2)
			{
				Log(Log.Warning, "RED channel texture format texture detected in OpenGL ES 2+ "
				    "context. Remapping to LUMINANCE texture to avoid errors. Ensure shaders "
				    "are compatible with a LUMINANCE swizzle (r,r,r,1)");
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
			} break;
		case GL_RG:
			if (isEs2)
			{
				Log(Log.Warning, "RED/GREEN channel texture format texture detected in OpenGL ES"
				    " 2+ context. Remapping to LUMINANCE_ALPJA texture to avoid errors. Ensure "
				    "shaders are compatible with a LUMINANCE/ALPHA swizzle (r,r,r,g)");
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
			} break;
		}

		// Check for formats only supported by extensions.
		switch (glInternalFormat)
		{
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
		{
			//useTexStorage = false;
			if (!isExtensionSupported("GL_IMG_texture_compression_pvrtc"))
			{
				if (allowDecompress)
				{
					//No longer compressed if this is the case.
					isCompressedFormat = false;

					//Set up the new texture and header.
					assets::TextureHeader cDecompressedHeader(texture);
					// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
					cDecompressedHeader.setPixelFormat(assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

					cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
					cDecompressedTexture = assets::Texture(cDecompressedHeader);

					//Update the texture format.
					getOpenGLFormat(cDecompressedTexture.getPixelFormat(), cDecompressedTexture.getColorSpace(),
					                cDecompressedTexture.getChannelType(), glInternalFormat, glFormat, glType,
					                glTypeSize, unused);

					//Do decompression, one surface at a time.
					for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
					{
						for (uint32 uiArray = 0; uiArray < textureToUse->getNumberOfArrayMembers(); ++uiArray)
						{
							for (uint32 uiFace = 0; uiFace < textureToUse->getNumberOfFaces(); ++uiFace)
							{
								PVRTDecompressPVRTC(textureToUse->getDataPointer(uiMIPLevel, uiArray, uiFace),
								                    (textureToUse->getBitsPerPixel() == 2 ? 1 : 0),
								                    textureToUse->getWidth(uiMIPLevel),
								                    textureToUse->getHeight(uiMIPLevel),
								                    cDecompressedTexture.getDataPointer(uiMIPLevel, uiArray, uiFace));
							}
						}
					}

					//Make sure the function knows to use a decompressed texture instead.
					textureToUse = &cDecompressedTexture;
				}
				else
				{
					Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC1");
					return Result::UnsupportedRequest;
				}
			}
			break;
		}
#if !defined(TARGET_OS_IPHONE)
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
		{
			//useTexStorage = false;
			if (!isExtensionSupported("GL_IMG_texture_compression_pvrtc2"))
			{
				Log(Log.Error, cszUnsupportedFormat, "PVRTC2");
				return Result::UnsupportedRequest;
			}
			break;
		}


		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		{
			//useTexStorage = false;
			if (!isExtensionSupported("GL_EXT_texture_compression_dxt1"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT1");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
		{
			//useTexStorage = false;
			if (!isExtensionSupported("GL_ANGLE_texture_compression_dxt3"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT3");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
		{
			//useTexStorage = false;
			if (!isExtensionSupported("GL_ANGLE_texture_compression_dxt5"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT5");
				return Result::UnsupportedRequest;
			}
			break;
		}
#endif

		default:
		{}
		}
	}

	/*****************************************************************************
	* Check the type of texture (e.g. 3D textures).
	*****************************************************************************/
	{
		// Only 2D Arrays are supported in this API.
		if (textureToUse->getNumberOfArrayMembers() > 1)
		{
			Log(Log.Error,
			    "TextureUtils.h:textureUpload:: Texture arrays are not supported by this implementation.\n");
			return Result::UnsupportedRequest;
		}

		// 3D Cubemaps aren't supported
		if (textureToUse->getDepth() > 1)
		{
			Log(Log.Error,
			    "TextureUtils.h:textureUpload:: 3-Dimensional textures are not supported by this implementation.\n");
			return Result::UnsupportedRequest;
		}

		//Check if it's a Cube Map.
		if (textureToUse->getNumberOfFaces() > 1)
		{
			//Make sure it's a complete cube, otherwise warn the user. We've already checked if it's a 3D texture or a texture array as well.
			if (textureToUse->getNumberOfFaces() < 6)
			{
				Log(Log.Warning,
				    "TextureUtils.h:textureUpload:: Textures with between 2 and 5 faces are unsupported. Faces up to 6 will be allocated in a cube map as undefined surfaces.\n");
			}
			else if (textureToUse->getNumberOfFaces() > 6)
			{
				Log(Log.Warning,
				    "TextureUtils.h:textureUpload:: Textures with more than 6 faces are unsupported. Only the first 6 faces will be loaded into the API.\n");
			}
			texTarget = GL_TEXTURE_CUBE_MAP;
		}
	}

	/*****************************************************************************
	* Setup the texture object.
	*****************************************************************************/
	{
		// Check the error here, in case the extension loader or anything else raised any errors.
		//Generate a new texture name.
		gl::GenTextures(1, &outTextureName);

		//Bind the texture to edit it.
		gl::BindTexture(texTarget, outTextureName);

		//Set the unpack alignment to 1 - PVR textures are not stored as padded.
		gl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (needsSwizzling)
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzle_r);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzle_g);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzle_b);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzle_a);
		}


	}

	/*****************************************************************************
	* Load the texture.
	*****************************************************************************/
	{
		// 2D textures.
		if (texTarget == GL_TEXTURE_2D)
		{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage2D(texTarget, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), 0,
						                         textureToUse->getDataSize(uiMIPLevel, false, false),
						                         textureToUse->getDataPointer(uiMIPLevel, 0, 0));
					}
					else
					{
                        if (isEs2) {glInternalFormat = glFormat;}
						gl::TexImage2D(texTarget, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0, 0));

					}
				}
		}
		// Cube maps.
		else if (texTarget == GL_TEXTURE_CUBE_MAP)
		{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					//Iterate through 6 faces regardless, as these should always be iterated through. We fill in the blanks with uninitialized data for uncompressed textures, or repeat faces for compressed data.
					for (uint32 uiFace = 0; uiFace < 6; ++uiFace)
					{
						GLenum eTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						if (isCompressedFormat)
						{
							//Make sure to wrap texture faces around if there are fewer faces than 6 in a compressed texture.
							gl::CompressedTexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							                         textureToUse->getWidth(uiMIPLevel),
							                         textureToUse->getHeight(uiMIPLevel), 0, textureToUse->getDataSize(uiMIPLevel, false, false),
							                         textureToUse->getDataPointer(uiMIPLevel, 0, uiFace % textureToUse->getNumberOfFaces()));
						}
						else
						{
							//No need to wrap faces for uncompressed textures, as gl will handle a NULL pointer, which Texture::getDataPtr will do when requesting a non-existant face.
							gl::TexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							               textureToUse->getWidth(uiMIPLevel),
							               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
							                   uiFace % textureToUse->getNumberOfFaces()));
						}
					}
				}
		}
		else
		{
			Log(Log.Debug,
			    "TextureUtilsGLES3 : TextureUpload : File corrupted or suspected bug : unknown texture target type.");
		}
	}

	return Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Example specific methods
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*!*********************************************************************************************************************
\brief	load pvr texture
\return	Result::Success on success
\param	const StringHash & filename
\param	GLuint & outTexHandle
\param	pvr::assets::Texture * outTexture
\param	assets::TextureHeader * outDescriptor
***********************************************************************************************************************/
Result::Enum OGLESIntroducingPVRAssets::loadTexturePVR(const StringHash& filename, GLuint& outTexHandle, pvr::assets::Texture* outTexture,
    assets::TextureHeader* outDescriptor)
{
	assets::Texture tempTexture;
	Result::Enum result;
	GLuint textureHandle;
	Stream::ptr_type assetStream = this->getAssetStream(filename);

	if (!assetStream.get())
	{
		Log(Log.Error, "AssetStore.loadTexture error for filename %s : File not found", filename.c_str());
		return Result::NotFound;
	}
	result = assets::textureLoad(assetStream, assets::TextureFileFormat::PVR, tempTexture);
	if (result == Result::Success)
	{
		textureUpload(tempTexture, textureHandle, pvr::Api::OpenGLES2, true);
	}
	if (result != Result::Success)
	{
		Log(Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
		    filename.c_str(), Log.getResultCodeString(result));
		return result;
	}
	if (outTexture) { *outTexture = tempTexture; }
	outTexHandle = textureHandle;
	return result;
}

/*!*********************************************************************************************************************
\brief	Load model
\return	Return Result::Success on success
\param	assetProvider Assets stream provider
\param	filename Name of the model file
\param	outModel Returned loaded model
***********************************************************************************************************************/
Result::Enum loadModel(pvr::IAssetProvider* assetProvider, const char* filename, assets::ModelHandle& outModel)
{
	Stream::ptr_type assetStream = assetProvider->getAssetStream(filename);
	if (!assetStream.get())
	{
		Log(Log.Error, "AssetStore.loadModel  error for filename %s : File not found", filename);
		return Result::NotFound;
	}

	assets::PODReader reader(assetStream);
	assets::ModelHandle handle = assets::Model::createWithReader(reader);

	if (handle.isNull())
	{
		Log(Log.Error, "AssetStore.loadModel error : Failed to load model %s.", filename);
		return pvr::Result::UnableToOpen;
	}
	else
	{
		outModel = handle;
	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\brief	Load the material's textures
\return	Return true if success
***********************************************************************************************************************/
bool OGLESIntroducingPVRAssets::loadTextures()
{
	pvr::uint32 numMaterials = scene->getNumMaterials();
	texDiffuse.resize(numMaterials);
	for (pvr::uint32 i = 0; i < numMaterials; ++i)
	{
		const pvr::assets::Model::Material& material = scene->getMaterial(i);
		if (material.getDiffuseTextureIndex() != -1)
		{
			// Load the diffuse texture map
			if (loadTexturePVR(scene->getTexture(material.getDiffuseTextureIndex()).getName(),
			                   texDiffuse[i], NULL, 0) != pvr::Result::Success)
			{
				Log("Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
				return false;
			}
			gl::BindTexture(GL_TEXTURE_2D, texDiffuse[i]);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
	}// next material
	return true;
}

/*!*********************************************************************************************************************
\brief	Loads and compiles the shaders and links the shader programs required for this training course
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESIntroducingPVRAssets::loadShaders()
{
	// Load and compile the shaders from files.

	const char* attributes[] = { "inVertex", "inNormal", "inTexCoord" };
	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(VertShaderSrcFile, *this);

	GLuint shaders[2];
	if (loadShader(*fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES2), ShaderType::VertexShader, 0, 0,
	               shaders[0]) != pvr::Result::Success)
	{
		return false;
	}

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	if (loadShader(*fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES2), ShaderType::FragmentShader, 0,
	               0, shaders[1]) != pvr::Result::Success)
	{
		return false;
	}
	if (createShaderProgram(shaders, 2, attributes, sizeof(attributes) / sizeof(attributes[0]),
	                        ShaderProgram.handle) != pvr::Result::Success)
	{
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(ShaderProgram.handle);
	gl::Uniform1i(gl::GetUniformLocation(ShaderProgram.handle, "sTexture"), 0);

	// Store the location of uniforms for later use
	ShaderProgram.uiMVPMatrixLoc = gl::GetUniformLocation(ShaderProgram.handle, "MVPMatrix");
	ShaderProgram.uiLightDirLoc = gl::GetUniformLocation(ShaderProgram.handle, "LightDirection");
	ShaderProgram.uiWorldViewITLoc = gl::GetUniformLocation(ShaderProgram.handle, "WorldViewIT");

	return true;
}

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESIntroducingPVRAssets::LoadVbos()
{
	vbo.resize(scene->getNumMeshes());
	indexVbo.resize(scene->getNumMeshes());
	gl::GenBuffers(scene->getNumMeshes(), &vbo[0]);

	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.

	for (unsigned int i = 0; i < scene->getNumMeshes(); ++i)
	{
		// Load vertex data into buffer object
		const pvr::assets::Mesh& mesh = scene->getMesh(i);
		pvr::uint32 size = mesh.getDataSize(0);
		//PVRTuint32 uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		gl::BindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		gl::BufferData(GL_ARRAY_BUFFER, size, mesh.getData(0), GL_STATIC_DRAW);

		// Load index data into buffer object if available
		indexVbo[i] = 0;
		if (mesh.getFaces().getData())
		{
			gl::GenBuffers(1, &indexVbo[i]);
			size = mesh.getFaces().getDataSize();
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo[i]);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, size, mesh.getFaces().getData(), GL_STATIC_DRAW);
		}
	}
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.).
		If the rendering context is lost, InitApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRAssets::initApplication()
{
	// Load the scene
	pvr::Result::Enum rslt = pvr::Result::Success;
	if ((rslt = loadModel(this, SceneFile, scene)) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return rslt;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The scene does not contain a camera. Please add one and re-export.\n");
		return pvr::Result::InvalidData;
	}

	// We also check that the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		this->setExitMessage("ERROR: The scene does not contain a light. Please add one and re-export.\n");
		return pvr::Result::InvalidData;
	}

	// Initialize variables used for the animation
	frame = 0;
	return rslt;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
        If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRAssets::quitApplication() {	return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRAssets::initView()
{
	pvr::string ErrorStr;
	//Initialize the PowerVR OpenGL bindings. Must be called before using any of the gl:: commands.
	gl::initGl();
	//	Initialize VBO data
	if (!LoadVbos())
	{
		this->setExitMessage(ErrorStr.c_str());
		return pvr::Result::UnknownError;
	}

	//	Load textures
	if (!loadTextures())
	{
		this->setExitMessage(ErrorStr.c_str());
		return pvr::Result::UnknownError;
	}

	//	Load and compile the shaders & link programs
	if (!loadShaders())
	{
		this->setExitMessage(ErrorStr.c_str());
		return pvr::Result::UnknownError;
	}

	//	Set OpenGL ES render states needed for this training course
	// Enable backface culling and depth test
	gl::CullFace(GL_BACK);
	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);

	// Use a nice bright blue as clear color
	gl::ClearColor(0.00, 0.70, 0.67, 1.0f);
	// Calculate the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
        projection = pvr::math::perspectiveFov(pvr::Api::OpenGLES2,scene->getCamera(0).getFOV(), (float)this->getHeight(),
		                                       (float)this->getWidth(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(),
		                                       glm::pi<pvr::float32>() * .5f);// rotate by 90 degree
	}
	else
	{
		projection = glm::perspectiveFov(scene->getCamera(0).getFOV(), (float)this->getWidth(),
		                                 (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRAssets::releaseView()
{
	// Deletes the textures
	gl::DeleteTextures(texDiffuse.size(), &texDiffuse[0]);

	// Delete program and shader objects
	gl::DeleteProgram(ShaderProgram.handle);

	gl::DeleteShader(vertShader);
	gl::DeleteShader(fragShader);

	// Delete buffer objects
	scene->destroy();
	gl::DeleteBuffers(vbo.size(), &vbo[0]);
	gl::DeleteBuffers(indexVbo.size(), &indexVbo[0]);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRAssets::renderFrame()
{
	// Clear the color and depth buffer
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Use shader program
	gl::UseProgram(ShaderProgram.handle);

	// Calculates the frame number to animate in a time-based manner.
	// get the time in milliseconds.
	frame += (float)getFrameTime() / 30.f; /*design-time target fps for animation*/

	if (frame > scene->getNumFrames() - 1) { frame = 0; }

	// Sets the scene animation to this frame
	scene->setCurrentFrame(frame);

	// Get the direction of the first light from the scene.
	glm::vec3 lightDirVec3;
	scene->getLightDirection(0, lightDirVec3);
	// For direction vectors, w should be 0
	glm::vec4 lightDirVec4(glm::normalize(lightDirVec3), 1.f);

	// Set up the view and projection matrices from the camera
	glm::mat4 mView;
	glm::vec3	vFrom, vTo, vUp;
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	scene->getCameraProperties(0, fFOV, vFrom, vTo, vUp);

	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we use glm::lookAt()
	mView = glm::lookAt(vFrom, vTo, vUp);

	//	A scene is composed of nodes. There are 3 types of nodes:
	//	- MeshNodes :
	//		references a mesh in the pMesh[].
	//		These nodes are at the beginning of the pNode[] array.
	//		And there are getNumMeshNodes() number of them.
	//		This way the .pod format can instantiate several times the same mesh
	//		with different attributes.
	//	- lights
	//	- cameras
	//	To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	for (unsigned int i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		// Get the node model matrix
		glm::mat4 mWorld = scene->getWorldMatrix(i);

		// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices
		glm::mat4 mModelView, mMVP;
		mModelView = mView * mWorld;
		mMVP = projection * mModelView;
		glm::mat4 worldIT = glm::inverseTranspose(mModelView);
		gl::UniformMatrix4fv(ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(mMVP));
		gl::UniformMatrix4fv(ShaderProgram.uiWorldViewITLoc, 1, GL_FALSE, glm::value_ptr(worldIT));

		// Pass the light direction in view space to the shader
		glm::vec4 vLightDir = mView * lightDirVec4;
		glm::vec3 dirModelVec3 = glm::normalize(*(glm::vec3*)&vLightDir);

		gl::Uniform3f(ShaderProgram.uiLightDirLoc, dirModelVec3.x, dirModelVec3.y, dirModelVec3.z);
		//	Now that the model-view matrix is set and the materials are ready,
		//	call another function to actually draw the mesh.
		drawMesh(i);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\param	nodeIndex		Node index of the mesh to draw
\brief	Draws a mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void OGLESIntroducingPVRAssets::drawMesh(int nodeIndex)
{
	int meshIndex = scene->getMeshNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = scene->getMesh(meshIndex);
	const pvr::int32 matId = scene->getMeshNode(nodeIndex).getMaterialIndex();
	gl::BindTexture(GL_TEXTURE_2D, texDiffuse[matId]);
	// bind the VBO for the mesh
	gl::BindBuffer(GL_ARRAY_BUFFER, vbo[meshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo[meshIndex]);

	// Enable the vertex attribute arrays
	gl::EnableVertexAttribArray(VertexArray);
	gl::EnableVertexAttribArray(NormalArray);
	gl::EnableVertexAttribArray(TexCoordArray);

	// Set the vertex attribute offsets
	const pvr::assets::VertexAttributeData* posAttrib = mesh.getVertexAttributeByName(AttribNames[0]);
	const pvr::assets::VertexAttributeData* normalAttrib = mesh.getVertexAttributeByName(AttribNames[1]);
	const pvr::assets::VertexAttributeData* texCoordAttrib = mesh.getVertexAttributeByName(AttribNames[2]);

	gl::VertexAttribPointer(VertexArray, posAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)posAttrib->getOffset());
	gl::VertexAttribPointer(NormalArray, normalAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)normalAttrib->getOffset());
	gl::VertexAttribPointer(TexCoordArray, texCoordAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)texCoordAttrib->getOffset());

	//	The geometry can be exported in 4 ways:
	//	- Indexed Triangle list
	//	- Non-Indexed Triangle list
	//	- Indexed Triangle strips
	//	- Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (indexVbo[meshIndex])
		{
			// Indexed Triangle list
			// Are our face indices unsigned shorts? If they aren't, then they are unsigned ints
			GLenum type = (mesh.getFaces().getDataType() == IndexType::IndexType16Bit) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, type, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces() * 3);
		}
	}
	else
	{
		pvr::uint32 offset = 0;
		// Are our face indices unsigned shorts? If they aren't, then they are unsigned ints
		GLenum type = (mesh.getFaces().getDataType() == IndexType::IndexType16Bit) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			if (indexVbo[meshIndex])
			{
				// Indexed Triangle strips
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, type,
				                 (void*)(size_t)(offset * mesh.getFaces().getDataSize()));
			}
			else
			{
				// Non-Indexed Triangle strips
				gl::DrawArrays(GL_TRIANGLE_STRIP, offset, mesh.getStripLength(i) + 2);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}

	// Safely disable the vertex attribute arrays
	gl::DisableVertexAttribArray(VertexArray);
	gl::DisableVertexAttribArray(NormalArray);
	gl::DisableVertexAttribArray(TexCoordArray);

	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!*********************************************************************************************************************
\return	auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.
		The user should return its Shell object defining the behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESIntroducingPVRAssets()); }
