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
//The OpenGL ES bindings used throughout this SDK. Use by calling gl::initGL and then using all the OpenGL ES functions from the gl::namespace.
// (So, glTextImage2D becomes gl::TexImage2D)
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/TextureUtils.h"
#include "PVRNativeApi/ShaderUtils.h"
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
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool loadTextures();
	bool loadShaders();
	bool LoadVbos();

	Result loadTexturePVR(const StringHash& filename, GLuint& outTexHandle,
	                            pvr::assets::Texture* outTexture, assets::TextureHeader* outDescriptor);

	void drawMesh(int i32NodeIndex);
};

/*!*********************************************************************************************************************
\brief	Create Shader program from vertex and fragment shader. Logs error if one occurs. (Code Extracted from PVRApi.)
\return	Return pvr::Result::Success on success
\param	shaders[] An array of OpenGL ES compiled shaders that will be combined into a shader program
\param	count The number of shaders into shaders
\param	attribs An array of attribute names, which will each be assigned successive attribute locations.
\param	attribCount The number of attributes in attribs
\param[out]	shaderProg The shader program, if successful.
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
\brief	load pvr texture
\return	Result::Success on success
\param	const StringHash & filename
\param	GLuint & outTexHandle
\param	pvr::assets::Texture * outTexture
\param	assets::TextureHeader * outDescriptor
***********************************************************************************************************************/
Result OGLESIntroducingPVRAssets::loadTexturePVR(const StringHash& filename, GLuint& outTexHandle, pvr::assets::Texture* outTexture,
    assets::TextureHeader* outDescriptor)
{
	assets::Texture tempTexture;
	Result result;
	native::HTexture_ textureHandle;
	Stream::ptr_type assetStream = this->getAssetStream(filename);

	if (!assetStream.get())
	{
		Log(Log.Error, "AssetStore.loadTexture error for filename %s : File not found", filename.c_str());
		return Result::NotFound;
	}
	result = assets::textureLoad(assetStream, assets::TextureFileFormat::PVR, tempTexture);
	if (result == Result::Success)
	{
		bool isDecompressed;
		types::ImageAreaSize areaSize; PixelFormat pixelFmt;
		pvr::utils::textureUpload(getPlatformContext(), tempTexture, textureHandle, areaSize, pixelFmt, isDecompressed);
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

	native::HShader_ shaders[2];
	if (!pvr::utils::loadShader(native::HContext_(), *fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES2), ShaderType::VertexShader, 0, 0,
	                            shaders[0]))
	{
		return false;
	}

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	if (!pvr::utils::loadShader(native::HContext_(), *fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES2), ShaderType::FragmentShader, 0,
	                            0, shaders[1]))
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
		pvr::uint32 size = (uint32)mesh.getDataSize(0);
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
pvr::Result OGLESIntroducingPVRAssets::initApplication()
{
	// Load the scene
	pvr::Result rslt = pvr::Result::Success;
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
pvr::Result OGLESIntroducingPVRAssets::quitApplication() {	return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRAssets::initView()
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
		projection = pvr::math::perspectiveFov(pvr::Api::OpenGLES2, scene->getCamera(0).getFOV(), (float)this->getHeight(),
		                                       (float)this->getWidth(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(),
		                                       glm::pi<pvr::float32>() * .5f);// rotate by 90 degree
	}
	else
	{
		projection = pvr::math::perspectiveFov(pvr::Api::OpenGLES2, scene->getCamera(0).getFOV(), (float)this->getWidth(),
		                                       (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRAssets::releaseView()
{
	// Deletes the textures
	gl::DeleteTextures((GLsizei)texDiffuse.size(), &texDiffuse[0]);

	// Delete program and shader objects
	gl::DeleteProgram(ShaderProgram.handle);

	gl::DeleteShader(vertShader);
	gl::DeleteShader(fragShader);

	// Delete buffer objects
	scene->destroy();
	gl::DeleteBuffers((GLsizei)vbo.size(), &vbo[0]);
	gl::DeleteBuffers((GLsizei)indexVbo.size(), &indexVbo[0]);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRAssets::renderFrame()
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

	gl::VertexAttribPointer(VertexArray, posAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)posAttrib->getOffset());
	gl::VertexAttribPointer(NormalArray, normalAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)normalAttrib->getOffset());
	gl::VertexAttribPointer(TexCoordArray, texCoordAttrib->getN(), GL_FLOAT, GL_FALSE, mesh.getStride(0), (void*)(size_t)texCoordAttrib->getOffset());

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
