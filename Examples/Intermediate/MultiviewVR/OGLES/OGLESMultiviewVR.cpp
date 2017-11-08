/*!*********************************************************************************************************************
\File         MultiviewVR.cpp
\Title        Introducing the POD 3D file format
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to load POD files and play the animation with basic lighting
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

// Index to bind the attributes to vertex shaders
const uint32_t VertexArray = 0;
const uint32_t NormalArray = 1;
const uint32_t TexCoordArray = 2;
const uint32_t NumArraysPerView = 4;
//Shader files
const char FragShaderSrcFile[] = "FragShader_ES3.fsh";
const char VertShaderSrcFile[] = "VertShader_ES3.vsh";
const char TexQuadFragShaderSrcFile[] = "TexQuadFragShader_ES3.fsh";
const char TexQuadVertShaderSrcFile[] = "TexQuadVertShader_ES3.vsh";
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
	pvr::EglContext _context;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// OpenGL handles for shaders, textures and VBOs
	uint32_t _vertShader;
	uint32_t _fragShader;
	std::vector<GLuint> _vbo;
	std::vector<GLuint> _indexVbo;
	std::vector<GLuint> _texDiffuse;

	uint32_t _widthHigh;
	uint32_t _heightHigh;

	GLuint _vboQuad;
	GLuint _iboQuad;

	// UIRenderer class used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint handle;
		uint32_t uiMVPMatrixLoc;
		uint32_t uiLightDirLoc;
		uint32_t uiWorldViewITLoc;
	} _multiViewProgram;

	struct
	{
		GLuint handle;
		uint32_t layerIndexLoc;

	} _texQuadProgram;

	struct
	{
		GLuint fbo;
		GLuint colorTexture;
		GLuint depthTexture;
	} _multiViewFbo;

	// Variables to handle the animation in a time-based manner
	float _frame;
	glm::mat4 _projection[NumArraysPerView];
	glm::mat4 _mvp[NumArraysPerView];
	glm::mat4 _worldViewIT[NumArraysPerView];
	glm::vec3 _lightDir[NumArraysPerView];

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
	bool renderToMultiViewFbo();
	void drawHighLowResQuad();
	void drawMesh(int i32NodeIndex);
};

bool MultiviewVR::createMultiViewFbo()
{
	_widthHigh = getWidth() / 4;
	_heightHigh = getHeight() / 2;

	// generate the color texture
	{
		gl::GenTextures(1, &_multiViewFbo.colorTexture);
		gl::BindTexture(GL_TEXTURE_2D_ARRAY, _multiViewFbo.colorTexture);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, _widthHigh, _heightHigh, 4);
	}
	// generate the depth texture
	{
		gl::GenTextures(1, &_multiViewFbo.depthTexture);
		gl::BindTexture(GL_TEXTURE_2D_ARRAY, _multiViewFbo.depthTexture);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, _widthHigh, _heightHigh, 4);
	}

	// generate the fbo
	{
		gl::GenFramebuffers(1, &_multiViewFbo.fbo);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _multiViewFbo.fbo);
		// Attach texture to the framebuffer.
		gl::ext::FramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _multiViewFbo.colorTexture, 0, 0, 4);
		gl::ext::FramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _multiViewFbo.depthTexture, 0, 0, 4);

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

			("Failed to create Multi view fbo %s", errorStr);
			// Unbind the  framebuffer.
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			return false;
		}
	}

	return true;
}

/*!*********************************************************************************************************************
\brief  Load the material's textures
\return Return true if success
***********************************************************************************************************************/
bool MultiviewVR::loadTextures()
{
	uint32_t numMaterials = _scene->getNumMaterials();
	_texDiffuse.resize(numMaterials);
	for (uint32_t i = 0; i < numMaterials; ++i)
	{
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);
		if (material.defaultSemantics().getDiffuseTextureIndex() != -1)
		{
			// Load the diffuse texture map
			if (!pvr::utils::textureUpload(*this, _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str(), _texDiffuse[i]))
			{
				this->setExitMessage("Error: Couldn't load the %s file\n", _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
				return true;
			}

			gl::BindTexture(GL_TEXTURE_2D, _texDiffuse[i]);
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
		const uint16_t attribIndices[] = { 0, 1, 2 };
		if (!(_multiViewProgram.handle = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attributes, attribIndices, 3)))
		{
			setExitMessage("Unable to create default program (%s, %s)", VertShaderSrcFile, FragShaderSrcFile);
			return false;
		}

		// Set the sampler2D variable to the first texture unit
		gl::UseProgram(_multiViewProgram.handle);
		gl::Uniform1i(gl::GetUniformLocation(_multiViewProgram.handle, "sTexture"), 0);

		// Store the location of uniforms for later use
		_multiViewProgram.uiMVPMatrixLoc = gl::GetUniformLocation(_multiViewProgram.handle, "MVPMatrix");
		_multiViewProgram.uiLightDirLoc = gl::GetUniformLocation(_multiViewProgram.handle, "LightDirection");
		_multiViewProgram.uiWorldViewITLoc = gl::GetUniformLocation(_multiViewProgram.handle, "WorldViewIT");
	}

	// texture Quad program
	{
		const char* attributes[] = { "inVertex", "HighResTexCoord", "LowResTexCoord" };
		const uint16_t attribIndices[] = { 0, 1, 2 };

		if (!(_texQuadProgram.handle = pvr::utils::createShaderProgram(*this, TexQuadVertShaderSrcFile, TexQuadFragShaderSrcFile, attributes, attribIndices, 3)))
		{
			setExitMessage("Unable to create default program (%s, %s)", TexQuadVertShaderSrcFile, TexQuadFragShaderSrcFile);
			return false;
		}

		// Set the sampler2D variable to the first texture unit
		gl::UseProgram(_texQuadProgram.handle);
		gl::Uniform1i(gl::GetUniformLocation(_texQuadProgram.handle, "sTexture"), 0);
		_texQuadProgram.layerIndexLoc =  gl::GetUniformLocation(_texQuadProgram.handle, "layerIndex");
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool MultiviewVR::LoadVbos()
{
	_vbo.resize(_scene->getNumMeshes());
	_indexVbo.resize(_scene->getNumMeshes());
	gl::GenBuffers(_scene->getNumMeshes(), &_vbo[0]);

	// Load vertex data of all meshes in the _scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.

	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		// Load vertex data into buffer object
		const pvr::assets::Mesh& mesh = _scene->getMesh(i);
		size_t size = mesh.getDataSize(0);
		gl::BindBuffer(GL_ARRAY_BUFFER, _vbo[i]);
		gl::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), mesh.getData(0), GL_STATIC_DRAW);

		// Load index data into buffer object if available
		_indexVbo[i] = 0;
		if (mesh.getFaces().getData())
		{
			gl::GenBuffers(1, &_indexVbo[i]);
			size = mesh.getFaces().getDataSize();
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbo[i]);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), mesh.getFaces().getData(), GL_STATIC_DRAW);
		}
	}

	{
		// generate the quad _vbo and ibo
		const float halfDim = 1.f;
		// create quad vertices..
		const float vertexData[] =
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

		uint16_t indices[] = { 1, 2, 0, 0, 2, 3 };

		gl::GenBuffers(1, &_vboQuad);
		gl::GenBuffers(1, &_iboQuad);

		gl::BindBuffer(GL_ARRAY_BUFFER, _vboQuad);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboQuad);
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
	// Load the _scene
	if (!pvr::utils::loadModel(*this, SceneFile, _scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return pvr::Result::UnknownError;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The _scene does not contain a camera. Please add one and re-export.\n");
		return pvr::Result::InitializationError;
	}

	// We also check that the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		this->setExitMessage("ERROR: The _scene does not contain a light. Please add one and re-export.\n");
		return pvr::Result::InitializationError;
	}

	// Initialize variables used for the animation
	_frame = 0;
	return pvr::Result::Success;
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
	if (this->getMinApi() < pvr::Api::OpenGLES3)
	{
		Log(LogLevel::Information, "This demo requires a minimum api of OpenGLES3.");
	}

	// create an OGLES context
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3, this->getMaxApi());

	std::string ErrorStr;
	//Initialize the PowerVR OpenGL bindings. Must be called before using any of the gl:: commands.
	if (!gl::isGlExtensionSupported("GL_OVR_multiview"))
	{
		this->setExitMessage("ERROR: Required extension GL_OVR_multiview extension not supported.");
		return pvr::Result::UnsupportedRequest;
	}

	if (!createMultiViewFbo())
	{
		this->setExitMessage("Failed to create multiview fbo");
		return pvr::Result::UnknownError;
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
	gl::DepthFunc(GL_LEQUAL);

	// Calculate the _projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();

	// Set up the _projection matrices for each view. For each eye the _scene is rendered twice with different fov.
	// The narrower field of view gives half the size near plane of the wider fov in order to
	// render the center of the _scene at a higher resolution. The high and low resolution
	// images will then be interpolated in the fragment shader to create an image with higher resolutions for
	// pixel that are center of the screen and lower resolutions for pixels outside the center of the screen
	// 90 degrees.
	// 53.1301024 degrees.  half the size for the near plane. tan(90/2) == (tan(53.13 / 2) * 2)
	float fovWide = glm::radians(90.f);
	float fovNarrow = glm::radians(53.1301024f);

	if (isRotated)
	{
		_projection[0] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovWide, (float)_heightHigh,
		                 (float)_widthHigh, _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(),
		                 glm::pi<float>() * .5f);// rotate by 90 degree

		_projection[1] = _projection[0];

		_projection[2] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovNarrow, (float)_heightHigh,
		                 (float)_widthHigh, _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(),
		                 glm::pi<float>() * .5f);// rotate by 90 degree

		_projection[3] = _projection[2];

	}
	else
	{
		_projection[0] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovWide, (float)_widthHigh,
		                 (float)_heightHigh, _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());// rotate by 90 degree

		_projection[1] = _projection[0];

		_projection[2] = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fovNarrow, (float)_widthHigh,
		                 (float)_heightHigh, _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());// rotate by 90 degree

		_projection[3] = _projection[2];
	}

	if (!_uiRenderer.init(getWidth(), getHeight(), isFullScreen()))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_uiRenderer.getDefaultTitle()->setText("MultiviewVR");
	_uiRenderer.getDefaultTitle()->commitUpdates();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result MultiviewVR::releaseView()
{
	// Deletes the textures
	gl::DeleteTextures(static_cast<GLuint>(_texDiffuse.size()), _texDiffuse.data());

	// Delete program and shader objects
	gl::DeleteProgram(_multiViewProgram.handle);
	gl::DeleteProgram(_texQuadProgram.handle);

	// Delete buffer objects
	_scene->destroy();
	gl::DeleteBuffers(static_cast<GLuint>(_vbo.size()), _vbo.data());
	gl::DeleteBuffers(static_cast<GLuint>(_indexVbo.size()), _indexVbo.data());
	gl::DeleteBuffers(static_cast<GLuint>(_vbo.size()), &_vboQuad);
	gl::DeleteBuffers(static_cast<GLuint>(_indexVbo.size()), &_iboQuad);

	_uiRenderer.release();

	_context->release();

	return pvr::Result::Success;
}

bool MultiviewVR::renderToMultiViewFbo()
{
	debugLogApiError("renderFrame begin");
	gl::Viewport(0, 0, _widthHigh, _heightHigh);
	// Clear the color and depth buffer
	gl::BindFramebuffer(GL_FRAMEBUFFER, _multiViewFbo.fbo);
	gl::ClearColor(0.00, 0.70, 0.67, 1.0f); // Use a nice bright blue as clear color
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader program
	gl::UseProgram(_multiViewProgram.handle);

	// Calculates the _frame number to animate in a time-based manner.
	// get the time in milliseconds.
	_frame += (float)getFrameTime() / 30.f; /*design-time target fps for animation*/

	if (_frame > _scene->getNumFrames() - 1) { _frame = 0; }

	// Sets the _scene animation to this _frame
	_scene->setCurrentFrame(_frame);

	// Get the direction of the first light from the _scene.
	glm::vec3 lightDirVec3;
	_scene->getLightDirection(0, lightDirVec3);
	// For direction vectors, w should be 0
	glm::vec4 lightDirVec4(glm::normalize(lightDirVec3), 1.f);

	// Set up the view and _projection matrices from the camera
	glm::mat4 viewLeft, viewRight;
	glm::vec3 vFrom, vTo, vUp;
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	_scene->getCameraProperties(0, fFOV, vFrom, vTo, vUp);

	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we use glm::lookAt()
	viewLeft = glm::lookAt(vFrom - viewOffset, vTo, vUp);
	viewRight = glm::lookAt(vFrom + viewOffset, vTo, vUp);

	// left
	_lightDir[0] = glm::vec3(viewLeft * lightDirVec4);
	_lightDir[0] = _lightDir[2] = glm::normalize(_lightDir[0]);

	//right
	_lightDir[1] = glm::vec3(viewRight * lightDirVec4);
	_lightDir[1] = _lightDir[3] = glm::normalize(_lightDir[1]);

	// Pass the light direction in view space to the shader
	gl::Uniform3fv(_multiViewProgram.uiLightDirLoc, NumArraysPerView, glm::value_ptr(_lightDir[0]));

	//  A _scene is composed of nodes. There are 3 types of nodes:
	//  - MeshNodes :
	//    references a mesh in the pMesh[].
	//    These nodes are at the beginning of the pNode[] array.
	//    And there are getNumMeshNodes() number of them.
	//    This way the .pod format can instantiate several times the same mesh
	//    with different attributes.
	//  - lights
	//  - cameras
	//  To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.
	for (uint32_t i = 0; i < _scene->getNumMeshNodes(); ++i)
	{
		// Get the node model matrix
		glm::mat4 mWorld = _scene->getWorldMatrix(i);
		glm::mat4 worldViewLeft = viewLeft *  mWorld;
		glm::mat4 worldViewRight = viewRight * mWorld;

		_worldViewIT[0] = _worldViewIT[2] = glm::inverseTranspose(worldViewLeft);
		_worldViewIT[1] = _worldViewIT[3] = glm::inverseTranspose(worldViewRight);


		// Pass the model-view-_projection matrix (MVP) to the shader to transform the vertice
		_mvp[0] = _projection[0] * worldViewLeft;
		_mvp[1] = _projection[1] * worldViewRight;
		_mvp[2] = _projection[2] * worldViewLeft;
		_mvp[3] = _projection[3] * worldViewRight;

		debugLogApiError("renderFrame before _mvp");
		gl::UniformMatrix4fv(_multiViewProgram.uiMVPMatrixLoc, 4, GL_FALSE, glm::value_ptr(_mvp[0]));
		gl::UniformMatrix4fv(_multiViewProgram.uiWorldViewITLoc, 4, GL_FALSE, glm::value_ptr(_worldViewIT[0]));
		debugLogApiError("renderFrame after _mvp");

		//  Now that the model-view matrix is set and the materials are ready,
		//  call another function to actually draw the mesh.
		debugLogApiError("renderFrame before draw");
		drawMesh(i);
		debugLogApiError("renderFrame after draw");
	}

	debugLogApiError("renderFrame end");
	return true;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result MultiviewVR::renderFrame()
{
	renderToMultiViewFbo();

	gl::BindFramebuffer(GL_FRAMEBUFFER, _context->getOnScreenFbo());
	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::Viewport(0, 0, getWidth(), getHeight());
	// Clear the color and depth buffer
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Use shader program
	gl::UseProgram(_texQuadProgram.handle);
	debugLogApiError("TexQuad DrawArrays begin");
	gl::BindTexture(GL_TEXTURE_2D_ARRAY, _multiViewFbo.colorTexture);
	debugLogApiError("TexQuad DrawArrays begin");
	uint32_t offset = sizeof(float) * 8;
	for (uint32_t i = 0; i < 2; ++i)
	{
		gl::Viewport(getWidth() / 2 * i, 0, getWidth() / 2, getHeight());

		debugLogApiError("TexQuad DrawArrays begin");
		// Draw the quad
		gl::Uniform1i(_texQuadProgram.layerIndexLoc, i);
		drawHighLowResQuad();
		debugLogApiError("TexQuad DrawArrays after");
	}
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);

	// reset the viewport to render the UI in the correct position in the screen
	gl::Viewport(0, 0, getWidth(), getHeight());

	//UIRENDERER
	{
		// render UI
		_uiRenderer.beginRendering();
		_uiRenderer.getSdkLogo()->render();
		_uiRenderer.getDefaultTitle()->render();
		_uiRenderer.endRendering();
	}

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	GLenum attach = GL_DEPTH;
	gl::InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attach);

	_context->swapBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\param  nodeIndex   Node index of the mesh to draw
\brief  Draws a mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void MultiviewVR::drawMesh(int nodeIndex)
{
	int meshIndex = _scene->getMeshNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshIndex);
	const int32_t matId = _scene->getMeshNode(nodeIndex).getMaterialIndex();
	debugLogApiError("before BindTexture");
	gl::BindTexture(GL_TEXTURE_2D, _texDiffuse[matId]);
	debugLogApiError("after  BindTexture");
	// bind the VBO for the mesh

	gl::BindBuffer(GL_ARRAY_BUFFER, _vbo[meshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbo[meshIndex]);

	// Enable the vertex attribute arrays
	debugLogApiError("before EnableVertexAttribArray");
	gl::EnableVertexAttribArray(VertexArray);
	gl::EnableVertexAttribArray(NormalArray);
	gl::EnableVertexAttribArray(TexCoordArray);
	debugLogApiError("after EnableVertexAttribArray");
	// Set the vertex attribute offsets
	const pvr::assets::VertexAttributeData* posAttrib = mesh.getVertexAttributeByName(AttribNames[0]);
	const pvr::assets::VertexAttributeData* normalAttrib = mesh.getVertexAttributeByName(AttribNames[1]);
	const pvr::assets::VertexAttributeData* texCoordAttrib = mesh.getVertexAttributeByName(AttribNames[2]);

	gl::VertexAttribPointer(VertexArray, posAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), reinterpret_cast<const void*>(static_cast<uintptr_t>(posAttrib->getOffset())));
	gl::VertexAttribPointer(NormalArray, normalAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), reinterpret_cast<const void*>(static_cast<uintptr_t>(normalAttrib->getOffset())));
	gl::VertexAttribPointer(TexCoordArray, texCoordAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), reinterpret_cast<const void*>(static_cast<uintptr_t>(texCoordAttrib->getOffset())));

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_indexVbo[meshIndex])
		{
			// Indexed Triangle list
			// Are our face indices unsigned shorts? If they aren't, then they are unsigned ints
			GLenum type = (mesh.getFaces().getDataType() == pvr::IndexType::IndexType16Bit) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			debugLogApiError("before DrawElements");
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, type, 0);
			debugLogApiError("after DrawElements");
		}
		else
		{
			// Non-Indexed Triangle list
			debugLogApiError("before DrawArrays");
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces() * 3);
			debugLogApiError("after DrawArrays");
		}
	}
	else
	{
		uint32_t offset = 0;
		// Are our face indices unsigned shorts? If they aren't, then they are unsigned ints
		GLenum type = (mesh.getFaces().getDataType() == pvr::IndexType::IndexType16Bit) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			if (_indexVbo[meshIndex])
			{
				// Indexed Triangle strips
				debugLogApiError("before DrawElements");
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, type,
				                 reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * mesh.getFaces().getDataSize())));
				debugLogApiError("after DrawElements");
			}
			else
			{
				// Non-Indexed Triangle strips
				debugLogApiError("before DrawArrays");
				gl::DrawArrays(GL_TRIANGLE_STRIP, offset, mesh.getStripLength(i) + 2);
				debugLogApiError("after DrawArrays");
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
	static const float texHighRes[] =
	{
		-.5f, -.5f,// lower left
		1.5f, -.5f,// lower right
		-.5f, 1.5f,// upper left
		1.5f, 1.5f// upper right
	};
	// low res texture coord
	static const float texLowRes[] =
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
std::unique_ptr<pvr::Shell> pvr::newDemo() {  return std::unique_ptr<pvr::Shell>(new MultiviewVR()); }