/*!*********************************************************************************************************************
\File         MultiviewVR.cpp
\Title        Introducing the POD 3D file format
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to load POD files and play the animation with basic  lighting
***********************************************************************************************************************/
//Main include file for the PVRShell library. Use this file when you will not be linking the PVRApi library.
#include "PVRShell/PVRShellNoPVRApi.h"
//Main include file for the PVRAssets library.
#include "PVRAssets/PVRAssets.h"
//The OpenGL ES bindings used throughout this SDK. Use by calling gl::initGL and then using all the OpenGL ES functions from the gl::namespace.
// (So, glTextImage2D becomes gl::TexImage2D)
#include "PVRNativeApi/NativeGles.h"

using namespace pvr;
using namespace pvr::types;
// Index to bind the attributes to vertex shaders
const pvr::uint32 VertexArray = 0;
const pvr::uint32 NormalArray = 1;
const pvr::uint32 TexCoordArray = 2;
const pvr::uint32 NumArraysPerView = 4;
//Shader files
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";
const char TexQuadFragShaderSrcFile[] = "TexQuadFragShader.fsh";
const char TexQuadVertShaderSrcFile[] = "TexQuadVertShader.vsh";
//POD scene file
const char SceneFile[] = "GnomeToy.pod";

const pvr::StringHash AttribNames[] =
{
	"POSITION",
	"NORMAL",
	"UV0",
};

glm::vec3 viewOffset(1.5f, 0.0f, 0.0f);

/*!*********************************************************************************************************************
\brief Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class MultiviewVR : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle  scene;

	// OpenGL handles for shaders, textures and VBOs
	pvr::uint32 vertShader;
	pvr::uint32 fragShader;
	std::vector<GLuint> vbo;
	std::vector<GLuint> indexVbo;
	std::vector<GLuint> texDiffuse;

	uint32 width_high;
	uint32 height_high;

	GLuint vboQuad;
	GLuint iboQuad;
	// Group shader programs and their uniform locations together
	struct
	{
		GLuint handle;
		pvr::uint32 uiMVPMatrixLoc;
		pvr::uint32 uiLightDirLoc;
		pvr::uint32 uiWorldViewITLoc;
	} multiViewProgram;
	struct
	{
		GLuint handle;
		pvr::uint32 layerIndexLoc;

	} texQuadProgram;

	struct MultiViewFbo
	{
		native::HFbo_ fbo;
		native::HTexture_ color;
		native::HTexture_ depth;
	};
	MultiViewFbo multiViewFbo;
	// Variables to handle the animation in a time-based manner
	pvr::float32 frame;
	glm::mat4 projection[NumArraysPerView];
	glm::mat4 mvp[NumArraysPerView];
	glm::mat4 worldViewIT[NumArraysPerView];
	glm::vec3 lightDir[NumArraysPerView];
public:
	//pvr::Shell implementation.
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool loadTextures();
	bool loadShaders();
	bool LoadVbos();
	bool createMultiViewFbo();
	pvr::Result renderToMultiViewFbo();
	Result loadTexturePVR(const StringHash& filename, GLuint& outTexHandle,
	                      pvr::Texture* outTexture, TextureHeader* outDescriptor);

	void drawHighLowResQuad();

	void drawMesh(int i32NodeIndex);
};


bool MultiviewVR::createMultiViewFbo()
{
	width_high = getWidth() / 4;
	height_high = getHeight() / 2;


	// generate the color texture
	{
		gl::GenTextures(1, &multiViewFbo.color.handle);
		gl::BindTexture(GL_TEXTURE_2D_ARRAY, multiViewFbo.color.handle);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width_high, height_high, 4);
	}
	// generate the depth texture
	{
		gl::GenTextures(1, &multiViewFbo.depth.handle);
		gl::BindTexture(GL_TEXTURE_2D_ARRAY, multiViewFbo.depth.handle);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width_high, height_high, 4);
	}

	// generate the fbo
	{
		gl::GenFramebuffers(1, &multiViewFbo.fbo.handle);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, multiViewFbo.fbo.handle);
		// Attach texture to the framebuffer.
		glext::FramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		                                      multiViewFbo.color.handle, 0, 0, 4);
		glext::FramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		                                      multiViewFbo.depth.handle, 0, 0, 4);

		GLenum result = gl::CheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE)
		{
			const char* errorStr = NULL;
			switch (result)
			{
			case GL_FRAMEBUFFER_UNDEFINED: errorStr = "GL_FRAMEBUFFER_UNDEFINED"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED: errorStr = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errorStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
			}

			pvr::Log("Failed to create Multi view fbo %s", errorStr);
			// Unbind the  framebuffer.
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			return false;
		}
	}

	return true;
}


/*!*********************************************************************************************************************
\brief  Create Shader program from vertex and fragment shader. Logs error if one occurs. (Code Extracted from PVRApi.)
\return Return pvr::Result::Success on success
\param  shaders[] An array of OpenGL ES compiled shaders that will be combined into a shader program
\param  count The number of shaders into shaders
\param  attribs An array of attribute names, which will each be assigned successive attribute locations.
\param  attribCount The number of attributes in attribs
\param[out] shaderProg The shader program, if successful.
***********************************************************************************************************************/
Result createShaderProgram(native::HShader_ shaders[], uint32 count, const char** attribs, uint32 attribCount, GLuint& shaderProg)
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


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Example specific methods
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*!*********************************************************************************************************************
\brief  load pvr texture
\return Result::Success on success
\param  const StringHash & filename
\param  GLuint & outTexHandle
\param  pvr::Texture * outTexture
\param  assets::TextureHeader * outDescriptor
***********************************************************************************************************************/
Result MultiviewVR::loadTexturePVR(const StringHash& filename, GLuint& outTexHandle, pvr::Texture* outTexture,
                                   TextureHeader* outDescriptor)
{
	Texture tempTexture;
	pvr::nativeGles::TextureUploadResults results;
	Stream::ptr_type assetStream = this->getAssetStream(filename);

	if (!assetStream.get())
	{
		Log(Log.Error, "AssetStore.loadTexture error for filename %s : File not found", filename.c_str());
		return Result::NotFound;
	}
	results.result = assets::textureLoad(assetStream, TextureFileFormat::PVR, tempTexture);
	if (results.result == Result::Success)
	{
		bool isDecompressed;
		types::ImageAreaSize areaSize; PixelFormat pixelFmt;
		results = pvr::nativeGles::textureUpload(getPlatformContext(), tempTexture);
	}
	if (results.result != Result::Success)
	{
		Log(Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
		    filename.c_str(), Log.getResultCodeString(results.result));
		return results.result;
	}
	if (outTexture) { *outTexture = tempTexture; }
	outTexHandle = results.image;
	return results.result;
}

/*!*********************************************************************************************************************
\brief  Load model
\return Return Result::Success on success
\param  assetProvider Assets stream provider
\param  filename Name of the model file
\param  outModel Returned loaded model
***********************************************************************************************************************/
Result loadModel(pvr::IAssetProvider* assetProvider, const char* filename, assets::ModelHandle& outModel)
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
\brief  Load the material's textures
\return Return true if success
***********************************************************************************************************************/
bool MultiviewVR::loadTextures()
{
	pvr::uint32 numMaterials = scene->getNumMaterials();
	texDiffuse.resize(numMaterials);
	for (pvr::uint32 i = 0; i < numMaterials; ++i)
	{
		const pvr::assets::Model::Material& material = scene->getMaterial(i);
		if (material.defaultSemantics().getDiffuseTextureIndex() != -1)
		{
			// Load the diffuse texture map
			if (loadTexturePVR(scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName(),
			                   texDiffuse[i], NULL, 0) != pvr::Result::Success)
			{
				Log("Failed to load texture %s", scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
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
\brief  Loads and compiles the shaders and links the shader programs required for this training course
\return Return true if no error occurred
***********************************************************************************************************************/
bool MultiviewVR::loadShaders()
{
	// Load and compile the shaders from files.
	{
		const char* attributes[] = { "inVertex", "inNormal", "inTexCoord" };
		pvr::assets::ShaderFile fileVersioning;
		fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
		native::HShader_ shaders[2];
		if (!pvr::nativeGles::loadShader(*fileVersioning.getStreamForSpecificApi(pvr::Api::OpenGLES3), ShaderType::VertexShader, 0, 0, shaders[0]))
		{
			return false;
		}

		fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
		if (!pvr::nativeGles::loadShader(*fileVersioning.getStreamForSpecificApi(pvr::Api::OpenGLES3), ShaderType::FragmentShader, 0, 0, shaders[1]))
		{
			return false;
		}
		
		if (createShaderProgram(shaders, 2, attributes, sizeof(attributes) / sizeof(attributes[0]), multiViewProgram.handle) != pvr::Result::Success)
		{
			return false;
		}

		// Set the sampler2D variable to the first texture unit
		gl::UseProgram(multiViewProgram.handle);
		gl::Uniform1i(gl::GetUniformLocation(multiViewProgram.handle, "sTexture"), 0);

		// Store the location of uniforms for later use
		multiViewProgram.uiMVPMatrixLoc = gl::GetUniformLocation(multiViewProgram.handle, "MVPMatrix");
		multiViewProgram.uiLightDirLoc = gl::GetUniformLocation(multiViewProgram.handle, "LightDirection");
		multiViewProgram.uiWorldViewITLoc = gl::GetUniformLocation(multiViewProgram.handle, "WorldViewIT");
		gl::DeleteShader(shaders[0]);
		gl::DeleteShader(shaders[1]);
	}

	// texture Quad program
	{
		const char* attributes[] = { "inVertex", "HighResTexCoord", "LowResTexCoord" };
		pvr::assets::ShaderFile fileVersioning;
		fileVersioning.populateValidVersions(TexQuadVertShaderSrcFile, *this);
		native::HShader_ shaders[2];
		if (!pvr::nativeGles::loadShader(*fileVersioning.getStreamForSpecificApi(pvr::Api::OpenGLES3),
		                                 ShaderType::VertexShader, 0, 0, shaders[0]))
		{
			return false;
		}

		fileVersioning.populateValidVersions(TexQuadFragShaderSrcFile, *this);
		if (!pvr::nativeGles::loadShader(*fileVersioning.getStreamForSpecificApi(pvr::Api::OpenGLES3),
		                                 ShaderType::FragmentShader, 0, 0, shaders[1]))
		{
			return false;
		}
		if (createShaderProgram(shaders, 2, attributes, sizeof(attributes) / sizeof(attributes[0]),
		                        texQuadProgram.handle) != pvr::Result::Success)
		{
			return false;
		}

		// Set the sampler2D variable to the first texture unit
		gl::UseProgram(texQuadProgram.handle);
		gl::Uniform1i(gl::GetUniformLocation(texQuadProgram.handle, "sTexture"), 0);
		texQuadProgram.layerIndexLoc =  gl::GetUniformLocation(texQuadProgram.handle, "layerIndex");
		gl::DeleteShader(shaders[0]);
		gl::DeleteShader(shaders[1]);
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool MultiviewVR::LoadVbos()
{
	vbo.resize(scene->getNumMeshes());
	indexVbo.resize(scene->getNumMeshes());
	gl::GenBuffers(scene->getNumMeshes(), &vbo[0]);

	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.

	for (uint32 i = 0; i < scene->getNumMeshes(); ++i)
	{
		// Load vertex data into buffer object
		const pvr::assets::Mesh& mesh = scene->getMesh(i);
		size_t size = mesh.getDataSize(0);
		gl::BindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		gl::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), mesh.getData(0), GL_STATIC_DRAW);

		// Load index data into buffer object if available
		indexVbo[i] = 0;
		if (mesh.getFaces().getData())
		{
			gl::GenBuffers(1, &indexVbo[i]);
			size = mesh.getFaces().getDataSize();
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo[i]);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), mesh.getFaces().getData(), GL_STATIC_DRAW);
		}
	}

	{
		// generate the quad vbo and ibo
		const pvr::float32 halfDim = 1.f;
		// create quad vertices..
		const pvr::float32 vertexData[] =
		{
			-halfDim, halfDim, // top left
			-halfDim, -halfDim,// bottom left
			halfDim, -halfDim,//  bottom right
			halfDim, halfDim,// top right

			// texCoords
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
		};

		pvr::uint16 indices[] = { 1, 2, 0, 0, 2, 3 };

		gl::GenBuffers(1, &vboQuad);
		gl::GenBuffers(1, &iboQuad);

		gl::BindBuffer(GL_ARRAY_BUFFER, vboQuad);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboQuad);
		gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		// unbind the buffers
		gl::BindBuffer(GL_ARRAY_BUFFER, 0);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	return true;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.).
    If the rendering context is lost, InitApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result MultiviewVR::initApplication()
{
	// Load the scene
	pvr::Result rslt = pvr::Result::Success;
	setMinApiType(pvr::Api::OpenGLES3);
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
\return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
        If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result MultiviewVR::quitApplication() {  return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result MultiviewVR::initView()
{
	pvr::string ErrorStr;
	//Initialize the PowerVR OpenGL bindings. Must be called before using any of the gl:: commands.
	gl::initGl();
	glext::initGlext();
	if (glext::FramebufferTextureMultiviewOVR == NULL)
	{
		this->setExitMessage("ERROR: Required extension GL_OVR_multiview extension not supported.");
		return pvr::Result::UnsupportedRequest;
	}

	if (!createMultiViewFbo())
	{
		this->setExitMessage("Failed to create multiview fbo");
		return pvr::Result::UnableToOpen;
	}

	//  Initialize VBO data
	if (!LoadVbos())
	{
		this->setExitMessage("Failed to load vbos");
		return pvr::Result::UnknownError;
	}

	//  Load textures
	if (!loadTextures())
	{
		this->setExitMessage("Failed to load textures");
		return pvr::Result::UnknownError;
	}

	//  Load and compile the shaders & link programs
	if (!loadShaders())
	{
		this->setExitMessage("Failed to load shaders");
		return pvr::Result::UnknownError;
	}

	//  Set OpenGL ES render states needed for this training course
	// Enable backface culling and depth test
	gl::CullFace(GL_BACK);
	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);
	//gl::Disable(GL_CULL_FACE);
	gl::DepthFunc(GL_LEQUAL);

	// Calculate the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();


	// Set up the projection matrices for each view. For each eye the scene is rendered twice with different fov.
	// The narrower field of view gives half the size near plane of the wider fov in order to
	// render the center of the scene at a higher resolution. The high and low resolution
	// images will then be interpolated in the fragment shader to create an image with higher resolutions for
	// pixel that are center of the screen and lower resolutions for pixels outside the center of the screen
	// 90 degrees.
	// 53.1301024 degrees.  half the size for the near plane. tan(90/2) == (tan(53.13 / 2) * 2)
	pvr::float32 fovWide = glm::radians(90.f);
	pvr::float32 fovNarrow = glm::radians(53.1301024f);

	if (isRotated)
	{
		projection[0] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovWide, (float)height_high,
		                (float)width_high, scene->getCamera(0).getNear(), scene->getCamera(0).getFar(),
		                glm::pi<pvr::float32>() * .5f);// rotate by 90 degree

		projection[1] = projection[0];

		projection[2] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovNarrow, (float)height_high,
		                (float)width_high, scene->getCamera(0).getNear(), scene->getCamera(0).getFar(),
		                glm::pi<pvr::float32>() * .5f);// rotate by 90 degree

		projection[3] = projection[2];

	}
	else
	{
		projection[0] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovWide, (float)width_high,
		                (float)height_high, scene->getCamera(0).getNear(), scene->getCamera(0).getFar());// rotate by 90 degree

		projection[1] = projection[0];

		projection[2] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovNarrow, (float)width_high,
		                (float)height_high, scene->getCamera(0).getNear(), scene->getCamera(0).getFar());// rotate by 90 degree

		projection[3] = projection[2];

	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result MultiviewVR::releaseView()
{
	// Deletes the textures
	gl::DeleteTextures(static_cast<GLuint>(texDiffuse.size()), texDiffuse.data());

	// Delete program and shader objects
	gl::DeleteProgram(multiViewProgram.handle);
	gl::DeleteProgram(texQuadProgram.handle);

	// Delete buffer objects
	scene->destroy();
	gl::DeleteBuffers(static_cast<GLuint>(vbo.size()), vbo.data());
	gl::DeleteBuffers(static_cast<GLuint>(indexVbo.size()), indexVbo.data());
	gl::DeleteBuffers(static_cast<GLuint>(vbo.size()), &vboQuad);
	gl::DeleteBuffers(static_cast<GLuint>(indexVbo.size()), &iboQuad);

	return pvr::Result::Success;
}

pvr::Result MultiviewVR::renderToMultiViewFbo()
{
	pvr::nativeGles::logApiError("renderFrame begin");
	gl::Viewport(0, 0, width_high, height_high);
	// Clear the color and depth buffer
	gl::BindFramebuffer(GL_FRAMEBUFFER, multiViewFbo.fbo.handle);
	gl::ClearColor(0.00, 0.70, 0.67, 1.0f); // Use a nice bright blue as clear color
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader program
	gl::UseProgram(multiViewProgram.handle);

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
	glm::mat4 viewLeft, viewRight;
	glm::vec3 vFrom, vTo, vUp;
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	scene->getCameraProperties(0, fFOV, vFrom, vTo, vUp);

	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we use glm::lookAt()
	viewLeft = glm::lookAt(vFrom - viewOffset , vTo, vUp);
	viewRight = glm::lookAt(vFrom + viewOffset, vTo, vUp);

	// left
	lightDir[0] = glm::vec3(viewLeft * lightDirVec4);
	lightDir[0] = lightDir[2] = glm::normalize(lightDir[0]);

	//right
	lightDir[1] = glm::vec3(viewRight * lightDirVec4);
	lightDir[1] = lightDir[3] = glm::normalize(lightDir[1]);

	// Pass the light direction in view space to the shader
	gl::Uniform3fv(multiViewProgram.uiLightDirLoc, NumArraysPerView, glm::value_ptr(lightDir[0]));

	//  A scene is composed of nodes. There are 3 types of nodes:
	//  - MeshNodes :
	//    references a mesh in the pMesh[].
	//    These nodes are at the beginning of the pNode[] array.
	//    And there are getNumMeshNodes() number of them.
	//    This way the .pod format can instantiate several times the same mesh
	//    with different attributes.
	//  - lights
	//  - cameras
	//  To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		// Get the node model matrix
		glm::mat4 mWorld = scene->getWorldMatrix(i);
		glm::mat4 worldViewLeft = viewLeft *  mWorld;
		glm::mat4 worldViewRight = viewRight * mWorld;

		worldViewIT[0] = worldViewIT[2] = glm::inverseTranspose(worldViewLeft);
		worldViewIT[1] = worldViewIT[3] = glm::inverseTranspose(worldViewRight);


		// Pass the model-view-projection matrix (MVP) to the shader to transform the vertice
		mvp[0] = projection[0] * worldViewLeft;
		mvp[1] = projection[1] * worldViewRight;
		mvp[2] = projection[2] * worldViewLeft;
		mvp[3] = projection[3] * worldViewRight;

		pvr::nativeGles::logApiError("renderFrame before mvp");
		gl::UniformMatrix4fv(multiViewProgram.uiMVPMatrixLoc, 4, GL_FALSE, glm::value_ptr(mvp[0]));
		gl::UniformMatrix4fv(multiViewProgram.uiWorldViewITLoc, 4, GL_FALSE, glm::value_ptr(worldViewIT[0]));
		pvr::nativeGles::logApiError("renderFrame after mvp");

		//  Now that the model-view matrix is set and the materials are ready,
		//  call another function to actually draw the mesh.
		pvr::nativeGles::logApiError("renderFrame before draw");
		drawMesh(i);
		pvr::nativeGles::logApiError("renderFrame after draw");
	}
	pvr::nativeGles::logApiError("renderFrame end");
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result MultiviewVR::renderFrame()
{
	renderToMultiViewFbo();

	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::Viewport(0, 0, getWidth(), getHeight());
	// Clear the color and depth buffer
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Use shader program
	gl::UseProgram(texQuadProgram.handle);
	pvr::nativeGles::logApiError("TexQuad DrawArrays begin");
	gl::BindTexture(GL_TEXTURE_2D_ARRAY, multiViewFbo.color.handle);
	pvr::nativeGles::logApiError("TexQuad DrawArrays begin");
	uint32 offset = sizeof(pvr::float32) * 8;
	for (pvr::uint32 i = 0; i < 2; ++i)
	{
		gl::Viewport(getWidth() / 2 * i, 0, getWidth() / 2, getHeight());

		pvr::nativeGles::logApiError("TexQuad DrawArrays begin");
		// Draw the quad
		gl::Uniform1i(texQuadProgram.layerIndexLoc, i);
		drawHighLowResQuad();
		pvr::nativeGles::logApiError("TexQuad DrawArrays after");
	}
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);

	GLenum attach = GL_DEPTH;
	gl::InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attach);
	return Result::Success;
}

/*!*********************************************************************************************************************
\param  nodeIndex   Node index of the mesh to draw
\brief  Draws a mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void MultiviewVR::drawMesh(int nodeIndex)
{
	int meshIndex = scene->getMeshNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = scene->getMesh(meshIndex);
	const pvr::int32 matId = scene->getMeshNode(nodeIndex).getMaterialIndex();
	pvr::nativeGles::logApiError("before BindTexture");
	gl::BindTexture(GL_TEXTURE_2D, texDiffuse[matId]);
	pvr::nativeGles::logApiError("after  BindTexture");
	// bind the VBO for the mesh

	gl::BindBuffer(GL_ARRAY_BUFFER, vbo[meshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo[meshIndex]);

	// Enable the vertex attribute arrays
	pvr::nativeGles::logApiError("before EnableVertexAttribArray");
	gl::EnableVertexAttribArray(VertexArray);
	gl::EnableVertexAttribArray(NormalArray);
	gl::EnableVertexAttribArray(TexCoordArray);
	pvr::nativeGles::logApiError("after EnableVertexAttribArray");
	// Set the vertex attribute offsets
	const pvr::assets::VertexAttributeData* posAttrib = mesh.getVertexAttributeByName(AttribNames[0]);
	const pvr::assets::VertexAttributeData* normalAttrib = mesh.getVertexAttributeByName(AttribNames[1]);
	const pvr::assets::VertexAttributeData* texCoordAttrib = mesh.getVertexAttributeByName(AttribNames[2]);

	gl::VertexAttribPointer(VertexArray, posAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)posAttrib->getOffset());
	gl::VertexAttribPointer(NormalArray, normalAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)normalAttrib->getOffset());
	gl::VertexAttribPointer(TexCoordArray, texCoordAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)texCoordAttrib->getOffset());

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (indexVbo[meshIndex])
		{
			// Indexed Triangle list
			// Are our face indices unsigned shorts? If they aren't, then they are unsigned ints
			GLenum type = (mesh.getFaces().getDataType() == IndexType::IndexType16Bit) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			pvr::nativeGles::logApiError("before DrawElements");
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, type, 0);
			pvr::nativeGles::logApiError("after DrawElements");
		}
		else
		{
			// Non-Indexed Triangle list
			pvr::nativeGles::logApiError("before DrawArrays");
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces() * 3);
			pvr::nativeGles::logApiError("after DrawArrays");
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
				pvr::nativeGles::logApiError("before DrawElements");
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, type,
				                 (void*)(size_t)(offset * mesh.getFaces().getDataSize()));
				pvr::nativeGles::logApiError("after DrawElements");
			}
			else
			{
				// Non-Indexed Triangle strips
				pvr::nativeGles::logApiError("before DrawArrays");
				gl::DrawArrays(GL_TRIANGLE_STRIP, offset, mesh.getStripLength(i) + 2);
				pvr::nativeGles::logApiError("after DrawArrays");
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
\brief  Different texture coordinates are used for the high and low resolution images.
      High resolution image should be drawn at half the size of the low resolution
    image and centered in the middle of the screen.
***********************************************************************************************************************/
void MultiviewVR::drawHighLowResQuad()
{
	// high res texture coord
	static const float32 texHighRes[] =
	{
		-.5f, -.5f,// lower left
		1.5f, -.5f,// lower right
		-.5f, 1.5f,// upper left
		1.5f, 1.5f// upper right
	};
	// low res texture coord
	static const float32 texLowRes[] =
	{
		0.f, 0.f,// lower left
		1.f, 0.f,// lower right
		0.f, 1.f,// upper left
		1.f, 1.f//  upper right
	};

	const float vertexData[] =
	{
		-1.f, -1.f, // lower left
		1.f, -1.f,  // lower right
		-1.f, 1.f,  // upper left
		1.f, 1.f    // upper right
	};

	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);
	gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertexData);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texHighRes);
	gl::VertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, texLowRes);
	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);
	gl::DisableVertexAttribArray(2);
}

/*!*********************************************************************************************************************
\return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell.
    The user should return its Shell object defining the behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {  return std::auto_ptr<pvr::Shell>(new MultiviewVR()); }
