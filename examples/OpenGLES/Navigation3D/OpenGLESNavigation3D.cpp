#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#define NAV_3D
#include "../../common/NavDataProcess.h"
//#include "../Common.h"
#include "PVRCore/TPSCamera.h"
const float CameraMoveSpeed = 1.f;
static const float CamHeight = .2f;
static uint32_t routeIndex = 0;
// Camera Settings
const float CameraRotationSpeed = .5f;
const float CamRotationTime = 10000.f;

// Alpha, luminance texture.
const char* RoadTexFile = "Road.pvr";
const char* MapFile = "map.osm";
const char* FontFile = "font.pvr";

enum class PipelineState
{
	RoadPipe,
	FillPipe,
	OutlinePipe,
	PlanarShaderPipe,
	BuildingPipe,
};

struct ShaderProgramFill
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformColour,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];

	ShaderProgramFill()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramRoad
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformColour,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramRoad()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramPlanerShadow
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformShadowMatrix,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramPlanerShadow()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct ShaderProgramBuilding
{
	GLuint program;
	enum Uniform
	{
		UniformTransform,
		UniformViewMatrix,
		UniformLightDir,
		UniformColour,
		UniformCount
	};
	int32_t uniformLocation[UniformCount];
	ShaderProgramBuilding()
	{
		uniformLocation[0] = -1;
		uniformLocation[1] = -1;
	}
};

struct DeviceResources
{
	// Pipelines
	ShaderProgramRoad roadPipe;
	ShaderProgramFill fillPipe;
	ShaderProgramFill outlinePipe;
	ShaderProgramPlanerShadow planarShadowPipe;
	ShaderProgramBuilding buildingPipe;

	// Descriptor set for texture
	GLuint roadTex, fontTex;

	pvr::ui::Font font;
	pvr::ui::Text text;
	pvr::ui::MatrixGroup textMtxGroup;
	pvr::ui::UIRenderer uiRenderer;
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	Plane(glm::vec4 n)
	{
		float invLen = 1.0f / glm::length(glm::vec3(n));
		normal = glm::vec3(n) * invLen;
		distance = n.w * invLen;
	}

	Plane() : normal(glm::vec3()), distance(0.0f) {}
};

struct TileRenderingResources
{
	GLuint vbo;
	GLuint ibo;

	// Add car parking to indices
	uint32_t parkingNum;

	// Add road area ways to indices
	uint32_t areaNum;

	// Add road area outlines to indices
	uint32_t roadAreaOutlineNum;

	// Add roads to indices
	uint32_t motorwayNum;
	uint32_t trunkRoadNum;
	uint32_t primaryRoadNum;
	uint32_t secondaryRoadNum;
	uint32_t serviceRoadNum;
	uint32_t otherRoadNum;

	// Add buildings to indices
	uint32_t buildNum;

	// Add inner ways to indices
	uint32_t innerNum;

	TileRenderingResources()
	{
		vbo = 0;
		ibo = 0;
		parkingNum = 0;
		areaNum = 0;
		roadAreaOutlineNum = 0;
		motorwayNum = 0;
		trunkRoadNum = 0;
		primaryRoadNum = 0;
		secondaryRoadNum = 0;
		serviceRoadNum = 0;
		otherRoadNum = 0;
		buildNum = 0;
		innerNum = 0;
	}
};

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESNavigation3D : public pvr::Shell
{
public:
	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

	void createShadowMatrix()
	{
		const glm::vec4 ground = glm::vec4(0.0, 1.0, 0.0, 0.0);
		const glm::vec4 light = glm::vec4(glm::normalize(glm::vec3(0.25f, 2.4f, -1.15f)), 0.0f);
		const float d = glm::dot(ground, light);

		shadowMatrix[0][0] = (float)(d - light.x * ground.x);
		shadowMatrix[1][0] = (float)(0.0 - light.x * ground.y);
		shadowMatrix[2][0] = (float)(0.0 - light.x * ground.z);
		shadowMatrix[3][0] = (float)(0.0 - light.x * ground.w);

		shadowMatrix[0][1] = (float)(0.0 - light.y * ground.x);
		shadowMatrix[1][1] = (float)(d - light.y * ground.y);
		shadowMatrix[2][1] = (float)(0.0 - light.y * ground.z);
		shadowMatrix[3][1] = (float)(0.0 - light.y * ground.w);

		shadowMatrix[0][2] = (float)(0.0 - light.z * ground.x);
		shadowMatrix[1][2] = (float)(0.0 - light.z * ground.y);
		shadowMatrix[2][2] = (float)(d - light.z * ground.z);
		shadowMatrix[3][2] = (float)(0.0 - light.z * ground.w);

		shadowMatrix[0][3] = (float)(0.0 - light.w * ground.x);
		shadowMatrix[1][3] = (float)(0.0 - light.w * ground.y);
		shadowMatrix[2][3] = (float)(0.0 - light.w * ground.z);
		shadowMatrix[3][3] = (float)(d - light.w * ground.w);
	}

private:
	std::unique_ptr<NavDataProcess> OSMdata;

	// Graphics context.
	pvr::EglContext context;

	// Graphics resources - buffers, samplers, descriptors.
	std::unique_ptr<DeviceResources> deviceResources;

	struct GlesStateTracker
	{
		GLuint boundTextures[4];
		GLuint boundProgram;
		GlesStateTracker() : boundProgram(0)
		{
			memset(boundTextures, 0, sizeof(boundTextures));
		}
	} glesStates;

	// UI object for text.
	pvr::ui::UIRenderer _uiRenderer;

	std::vector<std::vector<std::unique_ptr<TileRenderingResources> > > tileRenderingResources;

	// Uniforms
	glm::mat4 viewProjMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 lightDir;

	// Transformation variables
	glm::mat4 perspectiveMatrix;
	glm::mat4 ui_orthoMtx;

	pvr::math::ViewingFrustum viewFrustum;

	// Window variables
	uint32_t windowWidth;
	uint32_t windowHeight;

	// Map tile dimensions
	uint32_t numRows;
	uint32_t numCols;

	float totalRouteDistance;
	float keyFrameTime;
	std::string currentRoad;

	glm::mat4 shadowMatrix;
	pvr::TPSCamera camera;
	void createBuffers();
	bool loadTexture();
	void setUniforms();

	void executeCommands();
	void updateAnimation();
	void calculateTransform();
	void calculateClipPlanes();
	bool InFrustum(glm::vec2 min, glm::vec2 max);
	void executeCommands(const TileRenderingResources& tileRes);
	void createPrograms();
	void setPipelineStates(PipelineState pipelineState);
	// Calculate the key frametime between one point to another.
	float calculateRouteKeyFrameTime(const glm::dvec2& start, const glm::dvec2& end)
	{
		return ::calculateRouteKeyFrameTime(start, end, totalRouteDistance, CameraMoveSpeed);
	}

	glm::vec3 cameraTranslation;
	template<class ShaderProgram>
	void bindProgram(const ShaderProgram& program);

	void bindTexture(uint32_t index, GLuint texture)
	{
		// if (glesStates.boundTextures[index] != texture)
		{
			gl::ActiveTexture(GL_TEXTURE0 + index);
			gl::BindTexture(GL_TEXTURE_2D, texture);
			debugThrowOnApiError("OGLESNavigation3D::bindTexture");
			glesStates.boundTextures[index] = texture;
		}
	}

public:
	OGLESNavigation3D() : ui_orthoMtx(1.0), totalRouteDistance(0.0f), keyFrameTime(0.0f), shadowMatrix(1.0), cameraTranslation(0.0f) {}
};

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESNavigation3D::initApplication()
{
	OSMdata.reset(new NavDataProcess(getAssetStream(MapFile), glm::ivec2(getWidth(), getHeight())));
	pvr::Result result = OSMdata->loadAndProcessData();

	if (result != pvr::Result::Success)
		return result;

	setDepthBitsPerPixel(24);
	setStencilBitsPerPixel(8);
	createShadowMatrix();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESNavigation3D::initView()
{
	deviceResources.reset(new DeviceResources());

	// Acquire graphics context.
	context = pvr::createEglContext();
	context->init(getWindow(), getDisplay(), getDisplayAttributes());
	// Initialise uiRenderer
	deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen());

	windowWidth = static_cast<uint32_t>(deviceResources->uiRenderer.getRenderingDimX());
	windowHeight = static_cast<uint32_t>(deviceResources->uiRenderer.getRenderingDimY());

	OSMdata->initTiles();
	numRows = OSMdata->getNumRows();
	numCols = OSMdata->getNumCols();
	tileRenderingResources.resize(numCols);

	for (uint32_t i = 0; i < numCols; ++i)
	{
		tileRenderingResources[i].resize(numRows);
	}
	deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation3D");
	deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	if (!loadTexture())
	{
		return pvr::Result::UnknownError;
	}

	deviceResources->text = deviceResources->uiRenderer.createText(deviceResources->font);
	deviceResources->text->setColor(0.0f, 0.0f, 0.0f, 1.0f);
	deviceResources->text->setScale(0.25f, 0.25f);
	deviceResources->text->setPixelOffset(0.0f, static_cast<float>(-int32_t(windowHeight / 3)));
	deviceResources->text->commitUpdates();

	deviceResources->textMtxGroup = deviceResources->uiRenderer.createMatrixGroup();
	deviceResources->textMtxGroup->add(deviceResources->text);
	deviceResources->textMtxGroup->commitUpdates();

	createPrograms();
	setUniforms();
	createBuffers();
	OSMdata->convertRoute(glm::dvec2(0), 0, 0, totalRouteDistance);

	const glm::dvec2& camStartPosition = OSMdata->getRouteData()[routeIndex].point;
	camera.setTargetPosition(glm::vec3(camStartPosition.x, 0.f, camStartPosition.y));
	camera.setHeight(.2f);
	camera.setDistanceFromTarget(.55f);
	currentRoad = OSMdata->getRouteData()[routeIndex].name;

	return pvr::Result::Success;
}

void OGLESNavigation3D::createPrograms()
{
	// Create pipeline objects
	pvr::Stream::ptr_type vertShaderStream = getAssetStream("VertShader_ES2.vsh");
	pvr::Stream::ptr_type fragShaderStream = getAssetStream("FragShader_ES2.fsh");

	pvr::Stream::ptr_type aa_vertStream = getAssetStream("AA_VertShader_ES2.vsh");
	pvr::Stream::ptr_type aa_fragStream = getAssetStream("AA_FragShader_ES2.fsh");

	pvr::Stream::ptr_type perVertexLight_vertStream = getAssetStream("PerVertexLight_VertShader_ES2.vsh");
	pvr::Stream::ptr_type planerShadow_vertStream = getAssetStream("PlanarShadow_VertShader_ES2.vsh");
	pvr::Stream::ptr_type planerShadow_fragStream = getAssetStream("PlanarShadow_FragShader_ES2.fsh");

	const char* attribNames[] = { "myVertex", "texCoord", "normal" };
	const uint16_t attribIndicies[] = { 0, 1, 2 };
	const GLuint shaders[] = {
		pvr::utils::loadShader(*vertShaderStream, pvr::ShaderType::VertexShader, nullptr, 0), // 0
		pvr::utils::loadShader(*fragShaderStream, pvr::ShaderType::FragmentShader, nullptr, 0), // 1
		pvr::utils::loadShader(*aa_vertStream, pvr::ShaderType::VertexShader, nullptr, 0), // 2
		pvr::utils::loadShader(*aa_fragStream, pvr::ShaderType::FragmentShader, nullptr, 0), // 3
		pvr::utils::loadShader(*perVertexLight_vertStream, pvr::ShaderType::VertexShader, nullptr, 0), // 4
		pvr::utils::loadShader(*planerShadow_vertStream, pvr::ShaderType::VertexShader, nullptr, 0), // 5
		pvr::utils::loadShader(*planerShadow_fragStream, pvr::ShaderType::FragmentShader, nullptr, 0), // 6
	};

	// Road program
	{
		deviceResources->roadPipe.program = pvr::utils::createShaderProgram(&shaders[2], 2, attribNames, attribIndicies, ARRAY_SIZE(attribNames), nullptr);
		deviceResources->roadPipe.uniformLocation[ShaderProgramRoad::UniformTransform] = gl::GetUniformLocation(deviceResources->roadPipe.program, "transform");
		deviceResources->roadPipe.uniformLocation[ShaderProgramRoad::UniformColour] = gl::GetUniformLocation(deviceResources->roadPipe.program, "myColour");

		gl::UseProgram(deviceResources->roadPipe.program);
		gl::Uniform1i(gl::GetUniformLocation(deviceResources->roadPipe.program, "sTexture"), 0);
	}

	// Fill program and Outline program
	{
		deviceResources->fillPipe.program = pvr::utils::createShaderProgram(shaders, 2, attribNames, attribIndicies, ARRAY_SIZE(attribNames), nullptr);
		deviceResources->fillPipe.uniformLocation[ShaderProgramFill::UniformTransform] = gl::GetUniformLocation(deviceResources->fillPipe.program, "transform");
		deviceResources->fillPipe.uniformLocation[ShaderProgramFill::UniformColour] = gl::GetUniformLocation(deviceResources->fillPipe.program, "myColour");
		deviceResources->outlinePipe = deviceResources->fillPipe;
	}

	// Building program
	{
		const GLuint myShaders[] = { shaders[4], shaders[1] };
		deviceResources->buildingPipe.program = pvr::utils::createShaderProgram(myShaders, 2, attribNames, attribIndicies, ARRAY_SIZE(attribNames), nullptr);
		deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformTransform] = gl::GetUniformLocation(deviceResources->buildingPipe.program, "transform");
		deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformViewMatrix] = gl::GetUniformLocation(deviceResources->buildingPipe.program, "viewMatrix");
		deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformLightDir] = gl::GetUniformLocation(deviceResources->buildingPipe.program, "lightDir");
		deviceResources->buildingPipe.uniformLocation[ShaderProgramBuilding::UniformColour] = gl::GetUniformLocation(deviceResources->buildingPipe.program, "myColour");
	}

	// Planar shadow program
	{
		deviceResources->planarShadowPipe.program = pvr::utils::createShaderProgram(&shaders[5], 2, attribNames, attribIndicies, ARRAY_SIZE(attribNames), nullptr);
		deviceResources->planarShadowPipe.uniformLocation[ShaderProgramPlanerShadow::UniformTransform] =
			gl::GetUniformLocation(deviceResources->planarShadowPipe.program, "transform");
		deviceResources->planarShadowPipe.uniformLocation[ShaderProgramPlanerShadow::UniformShadowMatrix] =
			gl::GetUniformLocation(deviceResources->planarShadowPipe.program, "shadowMatrix");
	}
}

void OGLESNavigation3D::setPipelineStates(PipelineState pipelineState)
{
	gl::EnableVertexAttribArray(0); // pos
	gl::EnableVertexAttribArray(1); // tex
	gl::EnableVertexAttribArray(2); // normal

	gl::VertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)(sizeof(float) * 3));
	gl::VertexAttribPointer(2, 3, GL_FLOAT, false, 0, (void*)(sizeof(float) * 5));

	gl::Disable(GL_BLEND);
	gl::CullFace(GL_NONE);
	gl::Enable(GL_DEPTH);
	gl::DepthFunc(GL_LEQUAL);
	// Classic Alpha blending, but preserving framebuffer alpha to avoid artifacts on compositors that actually use the alpha value.
	gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	gl::BlendEquation(GL_FUNC_ADD);

	switch (pipelineState)
	{
	case PipelineState::RoadPipe:
		gl::Enable(GL_BLEND);
		break;
	case PipelineState::PlanarShaderPipe:
		gl::Enable(GL_BLEND);
		gl::StencilFunc(GL_EQUAL, 0, 0xff);
		gl::StencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		gl::DepthFunc(GL_LESS);
		break;
	default:
		break;
	}
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred, false if the sampler descriptor set is not valid.
\brief	Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.
***********************************************************************************************************************/
bool OGLESNavigation3D::loadTexture()
{
	/// Road Texture
	pvr::Texture tex = pvr::assets::textureLoad(getAssetStream(RoadTexFile), pvr::TextureFileFormat::PVR);

	pvr::utils::TextureUploadResults uploadResultRoadTex = pvr::utils::textureUpload(tex, context->getApiVersion() == pvr::Api::OpenGLES2, true);

	deviceResources->roadTex = uploadResultRoadTex.image;
	gl::BindTexture(GL_TEXTURE_2D, deviceResources->roadTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT)

	/// Font Texture
	tex = pvr::assets::textureLoad(getAssetStream(FontFile), pvr::TextureFileFormat::PVR);
	pvr::utils::TextureUploadResults uploadResulFontTex = pvr::utils::textureUpload(tex, context->getApiVersion() == pvr::Api::OpenGLES2, true);
	deviceResources->fontTex = uploadResulFontTex.image;
	gl::BindTexture(GL_TEXTURE_2D, deviceResources->fontTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	deviceResources->font = deviceResources->uiRenderer.createFont(deviceResources->fontTex, tex);
	return deviceResources->font.isValid();
}

/*!*********************************************************************************************************************
\brief	Setup uniforms used for drawing the map.
***********************************************************************************************************************/
void OGLESNavigation3D::setUniforms()
{
	ui_orthoMtx = pvr::math::ortho(context->getApiVersion(), 0.0f, float(windowWidth), 0.0f, float(windowHeight));

	perspectiveMatrix = deviceResources->uiRenderer.getScreenRotation() *
		pvr::math::perspectiveFov(context->getApiVersion(), glm::radians(45.0f), float(windowWidth), float(windowHeight), 0.01f, 3.f);
}

/*!*********************************************************************************************************************
\brief	Creates vertex and index buffers and records the secondary command buffers for each tile.
***********************************************************************************************************************/
void OGLESNavigation3D::createBuffers()
{
	uint32_t col = 0;
	uint32_t row = 0;

	for (auto& tileCol : OSMdata->getTiles())
	{
		for (Tile& tile : tileCol)
		{
			tileRenderingResources[col][row].reset(new TileRenderingResources());
			TileRenderingResources& tileResource = *tileRenderingResources[col][row];

			// Set the min and max coordinates for the tile
			tile.screenMin = remap(tile.min, OSMdata->getTiles()[0][0].min, OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));
			tile.screenMax = remap(tile.max, OSMdata->getTiles()[0][0].min, OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				glm::vec2 remappedPos =
					glm::vec2(remap(nodeIterator->second.coords, OSMdata->getTiles()[0][0].min, OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5)));
				glm::vec3 vertexPos = glm::vec3(remappedPos.x, nodeIterator->second.height, remappedPos.y);

				Tile::VertexData vertData(vertexPos, nodeIterator->second.texCoords);
				//	Log("Vertex data pos(%f, %f, %f)   tex(%f, %f)   normal(%f, %f, %f)", vertData.pos.x, vertData.pos.y, vertData.pos.z, vertData.texCoord.x, vertData.texCoord.y,
				//	vertData.normal.x, vertData.normal.y, vertData.normal.z);
				tile.vertices.push_back(vertData);
			}

			// Add car parking to indices
			tileResource.parkingNum = generateIndices(tile, tile.parkingWays);

			// Add road area ways to indices
			tileResource.areaNum = generateIndices(tile, tile.areaWays);

			// Add road area outlines to indices
			tileResource.roadAreaOutlineNum = generateIndices(tile, tile.areaOutlineIds);

			// Add roads to indices
			tileResource.motorwayNum = generateIndices(tile, tile.roadWays, RoadTypes::Motorway);
			tileResource.trunkRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			tileResource.primaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			tileResource.secondaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			tileResource.serviceRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Service);
			tileResource.otherRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Other);

			// Add buildings to indices
			tileResource.buildNum = generateIndices(tile, tile.buildWays);

			// Add inner ways to indices
			tileResource.innerNum = generateIndices(tile, tile.innerWays);

			generateNormals(tile, static_cast<uint32_t>(tile.indices.size() - (tileResource.innerNum + tileResource.buildNum)), tileResource.buildNum);

			// Create vertex and index buffers
			// Interleaved vertex buffer (vertex position + texCoord)
			gl::GenBuffers(1, &tileRenderingResources[col][row]->vbo);
			const uint32_t vboSize = (uint32_t)(tile.vertices.size() * sizeof(tile.vertices[0]));
			gl::BindBuffer(GL_ARRAY_BUFFER, tileRenderingResources[col][row]->vbo);
			gl::BufferData(GL_ARRAY_BUFFER, vboSize, tile.vertices.data(), GL_STATIC_DRAW);

			const uint32_t iboSize = (uint32_t)(tile.indices.size() * sizeof(tile.indices[0]));
			gl::GenBuffers(1, &tileRenderingResources[col][row]->ibo);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileRenderingResources[col][row]->ibo);
			gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, tile.indices.data(), GL_STATIC_DRAW);
			row++;
		}
		row = 0;
		col++;
	}
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESNavigation3D::renderFrame()
{
	updateAnimation();
	calculateTransform();
	calculateClipPlanes();

	// Update commands
	executeCommands();

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	context->swapBuffers();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Handle user input.
***********************************************************************************************************************/
void OGLESNavigation3D::updateAnimation()
{
	if (OSMdata->getRouteData().size() == 0)
	{
		return;
	}
	static bool turning = false;
	static float animTime = 0.0f;
	static float rotateTime = 0.0f;
	static float currentRotationTime = 0.0f;
	static float currentRotation = static_cast<float>(OSMdata->getRouteData()[routeIndex].rotation);
	static glm::dvec2 camStartPosition = OSMdata->getRouteData()[routeIndex].point;
	static glm::dvec2 camEndPosition;
	static glm::dvec2 camLerpPos = glm::dvec2(0.0f);
	static bool destinationReached = false;
	static float routeRestartTime = 0.;
	float dt = float(getFrameTime());
	camEndPosition = OSMdata->getRouteData()[routeIndex + 1].point;
	keyFrameTime = calculateRouteKeyFrameTime(camStartPosition, camEndPosition);
	// Do the transaltion if the camera is not turning

	if (destinationReached && routeRestartTime >= 2000)
	{
		destinationReached = false;
		routeRestartTime = 0.0f;
	}
	if (destinationReached)
	{
		routeRestartTime += dt;
		return;
	}

	if (!turning)
	{
		// Interpolate between two positions.
		camLerpPos = glm::mix(camStartPosition, camEndPosition, animTime / keyFrameTime);

		cameraTranslation = glm::vec3(camLerpPos.x, CamHeight, camLerpPos.y);
		camera.setTargetPosition(glm::vec3(camLerpPos.x, 0.0f, camLerpPos.y));
		camera.setTargetLookAngle(currentRotation);
	}
	if (animTime >= keyFrameTime)
	{
		const float r1 = static_cast<float>(OSMdata->getRouteData()[routeIndex].rotation);
		const float r2 = static_cast<float>(OSMdata->getRouteData()[routeIndex + 1].rotation);
		if ((!turning && fabs(r2 - r1) > 3.f) || (turning))
		{
			float diff = r2 - r1;
			float diffAbs = fabs(diff);
			if (diffAbs > 180.f)
			{
				if (diff > 0.f) // if the difference is positive angle then do negative rotation
					diff = -(360.f - diffAbs);
				else // else do a positive rotation
					diff = (360.f - diffAbs);
			}
			diffAbs = fabs(diff);
			if (rotateTime == 0.0f)
				rotateTime = 18.f * diffAbs; // 18ms for an angle * angle diff
			currentRotationTime += dt;
			currentRotationTime = glm::clamp(currentRotationTime, 0.0f, rotateTime);
			if (currentRotationTime >= rotateTime)
			{
				turning = false;
			}
			else
			{
				turning = true;
				currentRotation = glm::mix(r1, r1 + diff, currentRotationTime / rotateTime);
				camera.setTargetLookAngle(currentRotation);
			}
		}
	}
	if (animTime >= keyFrameTime && !turning)
	{
		turning = false;
		currentRotationTime = 0.0f;
		rotateTime = 0.0f;
		// Iterate through the route
		if (++routeIndex == OSMdata->getRouteData().size() - 1)
		{
			currentRotation = static_cast<float>(OSMdata->getRouteData()[0].rotation);
			routeIndex = 0;
			destinationReached = true;
			routeRestartTime = 0.0f;
		}
		else
		{
			currentRotation = static_cast<float>(OSMdata->getRouteData()[routeIndex].rotation);
		}
		currentRoad = OSMdata->getRouteData()[routeIndex].name;
		animTime = 0.0f;
		// Reset the route.
		camStartPosition = OSMdata->getRouteData()[routeIndex].point;
	}
	viewMatrix = camera.getViewMatrix();
	animTime += dt;
}

/*!*********************************************************************************************************************
\brief	Calculate the View Projection Matrix.
***********************************************************************************************************************/
void OGLESNavigation3D::calculateTransform()
{
	lightDir = glm::normalize(glm::mat3(viewMatrix) * glm::vec3(0.25f, -2.4f, -1.15f));
	viewProjMatrix = perspectiveMatrix * viewMatrix;
}

/*!*********************************************************************************************************************
\brief	Record the primary command buffer.
***********************************************************************************************************************/
void OGLESNavigation3D::executeCommands()
{
	gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	for (uint32_t i = 0; i < numCols; ++i)
	{
		for (uint32_t j = 0; j < numRows; ++j)
		{
			// Only queue up commands if the tile is visible.
			if (InFrustum(OSMdata->getTiles()[i][j].screenMin, OSMdata->getTiles()[i][j].screenMax))
			{
				executeCommands(*tileRenderingResources[i][j]);
			}
		}
	}

	deviceResources->text->setText(currentRoad);
	deviceResources->text->commitUpdates();
	deviceResources->textMtxGroup->setViewProjection(ui_orthoMtx);
	deviceResources->textMtxGroup->commitUpdates();

	// Render UI elements.
	deviceResources->uiRenderer.beginRendering();
	deviceResources->textMtxGroup->render();
	deviceResources->uiRenderer.getDefaultTitle()->render();
	deviceResources->uiRenderer.getSdkLogo()->render();
	deviceResources->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief	Capture frustum planes from the current View Projection matrix
***********************************************************************************************************************/
void OGLESNavigation3D::calculateClipPlanes()
{
	pvr::math::getFrustumPlanes(context->getApiVersion(), viewProjMatrix, viewFrustum);
}

/*!*********************************************************************************************************************
\param min The minimum co-ordinates of the bounding box.
\param max The maximum co-ordinates of the bounding box.
\return	boolean True if inside the view frustum, false if outside.
\brief	Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
Only the near, far, left and right planes of the view frustum are taken into consideration to optimize the intersection test.
***********************************************************************************************************************/
bool OGLESNavigation3D::InFrustum(glm::vec2 min, glm::vec2 max)
{
	// Test the axis-aligned bounding box against each frustum plane,
	// cull if all points are outside of one the view frustum planes.
	pvr::math::AxisAlignedBox aabb;
	aabb.setMinMax(glm::vec3(min.x, 0.f, min.y), glm::vec3(max.x, 5.0f, max.y));
	return pvr::math::aabbInFrustum(aabb, viewFrustum);
}
template<class ShaderProgram>
void OGLESNavigation3D::bindProgram(const ShaderProgram& program)
{
	// if (program.program != glesStates.boundProgram)
	{
		gl::UseProgram(program.program);
		glesStates.boundProgram = program.program;
	}
}

void OGLESNavigation3D::executeCommands(const TileRenderingResources& tileRes)
{
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);

	gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileRes.ibo);
	gl::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)0);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)(sizeof(float) * 3));
	gl::VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Tile::VertexData), (const void*)(sizeof(float) * 5));
	gl::Disable(GL_CULL_FACE);
	gl::DepthMask(GL_TRUE);
	gl::Enable(GL_DEPTH_TEST);

	gl::DepthFunc(GL_LEQUAL);
	gl::FrontFace(GL_CCW);
	gl::Disable(GL_BLEND);

	uint32_t offset = 0;
	const uint32_t parkingNum = tileRes.parkingNum;
	const uint32_t areaNum = tileRes.areaNum;
	const uint32_t motorwayNum = tileRes.motorwayNum;
	const uint32_t roadAreaOutlineNum = tileRes.roadAreaOutlineNum;
	const uint32_t trunkRoadNum = tileRes.trunkRoadNum;
	const uint32_t primaryRoadNum = tileRes.primaryRoadNum;
	const uint32_t secondaryRoadNum = tileRes.secondaryRoadNum;
	const uint32_t serviceRoadNum = tileRes.serviceRoadNum;
	const uint32_t otherRoadNum = tileRes.otherRoadNum;
	const uint32_t buildNum = tileRes.buildNum;
	const uint32_t innerNum = tileRes.innerNum;
	if (parkingNum > 0)
	{
		bindProgram(deviceResources->fillPipe);
		gl::UniformMatrix4fv(deviceResources->fillPipe.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
		gl::Uniform4fv(deviceResources->fillPipe.uniformLocation[ShaderProgramFill::Uniform::UniformColour], 1, glm::value_ptr(ParkingColourUniform));
		gl::DrawElements(GL_TRIANGLES, parkingNum, GL_UNSIGNED_INT, nullptr);
		offset += parkingNum * sizeof(uint32_t);
	}
	if (areaNum > 0)
	{
		const ShaderProgramFill& program = deviceResources->fillPipe;
		bindProgram(program);
		gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
		gl::Uniform4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformColour], 1, glm::value_ptr(RoadAreaColourUniform));
		gl::DrawElements(GL_TRIANGLES, areaNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += areaNum * sizeof(uint32_t);
	}
	if (roadAreaOutlineNum > 0)
	{
		const ShaderProgramFill& program = deviceResources->outlinePipe;
		bindProgram(program);
		gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
		gl::Uniform4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformColour], 1, glm::value_ptr(OutlineColourUniform));
		gl::DrawElements(GL_LINES, roadAreaOutlineNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += roadAreaOutlineNum * sizeof(uint32_t);
	}

	// Draw the roads
	const ShaderProgramRoad& program = deviceResources->roadPipe;
	gl::Enable(GL_BLEND);
	// Classic Alpha blending, but preserving framebuffer alpha to avoid artifacts on compositors that actually use the alpha value.
	gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	gl::BlendEquation(GL_FUNC_ADD);

	// Motorways
	bindProgram(program);
	gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramRoad::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
	bindTexture(0, deviceResources->roadTex);
	if (motorwayNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(MotorwayColour));
		gl::DrawElements(GL_TRIANGLES, motorwayNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += motorwayNum * sizeof(uint32_t);
	}

	// Trunk roads
	if (trunkRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(TrunkRoadColour));
		gl::DrawElements(GL_TRIANGLES, trunkRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += trunkRoadNum * sizeof(uint32_t);
	}

	// Primary roads
	if (primaryRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(PrimaryRoadColour));
		gl::DrawElements(GL_TRIANGLES, primaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += primaryRoadNum * sizeof(uint32_t);
	}

	// Road roads
	if (secondaryRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(SecondaryRoadColour));
		gl::DrawElements(GL_TRIANGLES, secondaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += secondaryRoadNum * sizeof(uint32_t);
	}
	// Service Roads
	if (serviceRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(ServiceRoadColour));
		gl::DrawElements(GL_TRIANGLES, serviceRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += serviceRoadNum * sizeof(uint32_t);
	}

	// Other (any other roads)
	if (otherRoadNum > 0)
	{
		gl::Uniform4fv(program.uniformLocation[ShaderProgramRoad::UniformColour], 1, glm::value_ptr(OtherRoadColour));
		gl::DrawElements(GL_TRIANGLES, otherRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += otherRoadNum * sizeof(uint32_t);
	}
	// Draw the buildings & shadows
	if (buildNum > 0)
	{
		const ShaderProgramBuilding& buildingProgram = deviceResources->buildingPipe;
		bindProgram(buildingProgram);

		gl::UniformMatrix4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
		gl::UniformMatrix4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformViewMatrix], 1, false, glm::value_ptr(viewMatrix));
		gl::Uniform3fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformLightDir], 1, glm::value_ptr(lightDir));
		gl::Uniform4fv(buildingProgram.uniformLocation[ShaderProgramBuilding::Uniform::UniformColour], 1, glm::value_ptr(BuildColourUniform));

		gl::DrawElements(GL_TRIANGLES, buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));

		// Planar shadows for buildings only.
		// Classic Alpha blending, but preserving framebuffer alpha to avoid artifacts on compositors that actually use the alpha value.
		gl::Enable(GL_BLEND);
		gl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
		gl::StencilFunc(GL_EQUAL, 0x0, 0xff);
		gl::StencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		gl::Enable(GL_STENCIL_TEST);
		gl::BlendEquation(GL_FUNC_ADD);
		const ShaderProgramPlanerShadow& shadowProgram = deviceResources->planarShadowPipe;
		bindProgram(shadowProgram);
		gl::UniformMatrix4fv(shadowProgram.uniformLocation[ShaderProgramPlanerShadow::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));
		gl::UniformMatrix4fv(shadowProgram.uniformLocation[ShaderProgramPlanerShadow::Uniform::UniformShadowMatrix], 1, false, glm::value_ptr(shadowMatrix));

		gl::DrawElements(GL_TRIANGLES, buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += buildNum * sizeof(uint32_t);
		gl::Disable(GL_STENCIL_TEST);
		gl::Disable(GL_BLEND);
	}

	if (innerNum > 0)
	{
		const ShaderProgramFill& program = deviceResources->fillPipe;
		bindProgram(program);
		gl::UniformMatrix4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformTransform], 1, false, glm::value_ptr(viewProjMatrix));

		gl::Uniform4fv(program.uniformLocation[ShaderProgramFill::Uniform::UniformColour], 1, glm::value_ptr(ClearColor));
		gl::DrawElements(GL_TRIANGLES, innerNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
		offset += innerNum * sizeof(uint32_t);
	}
}

/*!*********************************************************************************************************************
\return	auto ptr of the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its PVRShell object defining
the behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new OGLESNavigation3D());
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESNavigation3D::releaseView()
{
	// Clean up tile rendering resource data.
	for (auto& resourceCol : tileRenderingResources)
	{
		for (auto& resource : resourceCol)
		{
			resource.reset(0);
		}
	}

	OSMdata.reset();

	// Reset context and associated resources.
	deviceResources.reset(0);
	if (context.get())
		context->release();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESNavigation3D::quitApplication()
{
	return pvr::Result::Success;
}
