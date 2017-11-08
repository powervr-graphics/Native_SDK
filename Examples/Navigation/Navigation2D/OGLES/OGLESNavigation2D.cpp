/*!*********************************************************************************************************************
\File         OGLESNavigation2D.cpp
\Title        Navigation2D
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a 2D navigation renderer.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "../NavDataProcess.h"

const pvr::utils::VertexBindings_Name vertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "UV0", "inTexCoords" },
};

namespace AttributeIndices {
enum Enum
{
	VertexArray = 0,
	TexCoordArray = 2
};
}

const pvr::StringHash SpriteFileNames[BuildingType::None] =
{
	pvr::StringHash("shop.pvr"), pvr::StringHash("bar.pvr"), pvr::StringHash("cafe.pvr"), pvr::StringHash("fastfood.pvr"),
	pvr::StringHash("pub.pvr"), pvr::StringHash("college.pvr"), pvr::StringHash("library.pvr"), pvr::StringHash("university.pvr"),
	pvr::StringHash("ATM.pvr"), pvr::StringHash("bank.pvr"), pvr::StringHash("restaurant.pvr"), pvr::StringHash("doctors.pvr"),
	pvr::StringHash("dentist.pvr"), pvr::StringHash("hospital.pvr"), pvr::StringHash("pharmacy.pvr"), pvr::StringHash("cinema.pvr"),
	pvr::StringHash("casino.pvr"), pvr::StringHash("theatre.pvr"), pvr::StringHash("fire.pvr"), pvr::StringHash("courthouse.pvr"),
	pvr::StringHash("police.pvr"), pvr::StringHash("postoffice.pvr"), pvr::StringHash("toilets.pvr"), pvr::StringHash("worship.pvr"),
	pvr::StringHash("petrol.pvr"), pvr::StringHash("parking.pvr"), pvr::StringHash("other.pvr"), pvr::StringHash("postbox.pvr"),
	pvr::StringHash("vets.pvr"), pvr::StringHash("embassy.pvr"), pvr::StringHash("hairdresser.pvr"), pvr::StringHash("butcher.pvr"),
	pvr::StringHash("optician.pvr"), pvr::StringHash("florist.pvr"),
};

struct Icon
{
	pvr::ui::Image image;
};

struct Label
{
	pvr::ui::Text text;
};

struct AmenityIconGroup
{
	pvr::ui::PixelGroup group;
	Icon icon;
	IconData iconData;
};

struct AmenityLabelGroup
{
	pvr::ui::PixelGroup group;
	Label label;
	IconData iconData;
};

enum class CameraMode
{
	Auto,
	Manual
};

struct TileRenderProperties
{
	uint32_t parkingNum;
	uint32_t buildNum;
	uint32_t innerNum;
	uint32_t areaNum;
	uint32_t serviceRoadNum;
	uint32_t otherRoadNum;
	uint32_t secondaryRoadNum;
	uint32_t primaryRoadNum;
	uint32_t trunkRoadNum;
	uint32_t motorwayNum;
};

struct TileRenderingResources
{
	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	pvr::RefCountedResource<pvr::ui::UIRenderer> renderer;

	pvr::ui::Font font;
	pvr::ui::PixelGroup tileGroup[LOD::Count];
	pvr::ui::PixelGroup cameraRotateGroup[LOD::Count];
	std::vector<Label> labels[LOD::Count];
	std::vector<AmenityIconGroup> amenityIcons[LOD::Count];
	std::vector<AmenityLabelGroup> amenityLabels[LOD::Count];

	uint32_t col;
	uint32_t row;
	TileRenderProperties properties;

	void reset()
	{
		gl::DeleteBuffers(1, &vbo);
		gl::DeleteBuffers(1, &ibo);
		gl::DeleteBuffers(1, &vao);
		for (int i = 0; i < LOD::Count; ++i)
		{
			cameraRotateGroup[i].reset();
			labels[i].clear();
			amenityIcons[i].clear();
			amenityLabels[i].clear();
			tileGroup[i].reset();
		}
		font.reset();
		renderer.reset();
	}

	//Sprites for icons
	pvr::ui::Image spriteImages[BuildingType::None];

	TileRenderingResources() {}
};

struct DeviceResources
{
	// Graphics context
	pvr::EglContext context;

	// Programs
	GLuint roadProgram;
	GLuint fillProgram;

	GLint roadColorUniformLocation;
	GLint roadTransformUniformLocation;

	GLenum roadRequiredSrcRGB;
	GLenum roadRequiredDstRGB;
	GLenum roadRequiredSrcAlpha;
	GLenum roadRequiredDstAlpha;

	GLint fillColorUniformLocation;
	GLint fillTransformUniformLocation;

	pvr::utils::VertexConfiguration vertexConfiguration;

	// Frame and primary command buffers
	GLuint fbo;

	//Texture atlas meta data.
	pvr::TextureHeader texAtlasHeader;
	//Array of UV offsets into the texture atlas.
	pvr::Rectanglef atlasOffsets[BuildingType::None];
	//Raw texture atlas containing all sprites.
	GLuint texAtlas;

	//Font texture data
	GLuint fontTexture;
	pvr::Texture fontHeader;
	GLuint fontSampler;

	std::vector<TileRenderingResources*> renderqueue;

	GLint defaultFbo;

	// UIRenderer used to display text
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

//Alpha, luminance texture.
const char* MapFile = "map.osm";
const char* FontFile = "font.pvr";
const float scales[LOD::Count] = { 10.0f, 7.0f, 5.0f, 3.0f, 2.0f };
const float routescales[LOD::Count] = { 11.0f, 10.0f, 7.0f, 5.0f, 2.0f };

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESNavigation2D : public pvr::Shell
{
	bool _visualizeTiles;
	std::unique_ptr<NavDataProcess> _OSMdata;

	// Graphics resources
	std::auto_ptr<DeviceResources> _deviceResources;
	std::vector<std::vector<TileRenderingResources>> _tileRenderingResources;

	uint16_t _currentScaleLevel;

	// Uniforms
	glm::mat4 _mapMVPMtx;
	glm::vec4 _clearColorUniform;
	/***Road types - colour uniforms***/
	glm::vec4 _roadAreaColorUniform;
	glm::vec4 _motorwayColor;
	glm::vec4 _trunkRoadColor;
	glm::vec4 _primaryRoadColor;
	glm::vec4 _secondaryRoadColor;
	glm::vec4 _serviceRoadColor;
	glm::vec4 _otherRoadColor;
	/**************************/
	glm::vec4 _parkingColorUniform;
	glm::vec4 _buildColorUniform;
	glm::vec4 _outlineColorUniform;

	// Transformation variables
	glm::vec2 _translation;
	float _scale;
	glm::mat4 _mapProjMtx;
	glm::mat4 _projMtx;
	float _rotation;

	std::vector<Plane> _clipPlanes;

	//Map tile dimensions
	uint32_t _numRows;
	uint32_t _numCols;

	float _totalRouteDistance;
	float _weight;
	float _keyFrameTime;

	CameraMode _cameraMode;

	pvr::ui::GLStateTracker _stateTracker;

	glm::dvec2 _mapWorldDim;

	float _timePassed;
	bool _increaseScale;
	bool _scaleChange;
	bool _updateRotation;
	bool _turning;
	uint16_t _previousScaleLevel;
	uint32_t _routeIndex;
	float _animTime;
	float _rotateTime;
	float _rotateAnimTime;
    float _screenWidth, _screenHeight;
public:
	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

	void bindAndClearFramebuffer();
	void setDefaultStates();
	bool initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, const uint32_t col, const uint32_t row);
	void createBuffers();
	bool loadTexture();
	void setUniforms();
	void initRoute();

	void render();
	void updateLabels(uint32_t col, uint32_t row);
	void updateAmenities(uint32_t col, uint32_t row);
	void updateGroups(uint32_t col, uint32_t row);
	void updateAnimation();
	void calculateClipPlanes();
	bool inFrustum(glm::vec2 min, glm::vec2 max);
	void renderTile(const Tile& tile, TileRenderingResources& renderingResources);
	void createUIRendererItems();
	void eventMappedInput(pvr::SimplifiedInput e);
	void resetCameraVariables();
	void updateSubtitleText();

public:
	OGLESNavigation2D() : _totalRouteDistance(0.0f), _weight(0.0f), _projMtx(1.0), _rotation(0.0f), _cameraMode(CameraMode::Auto)
	{ }
};

void OGLESNavigation2D::resetCameraVariables()
{
	_weight = 0.0f;
	_currentScaleLevel = LOD::L4;
	_previousScaleLevel = _currentScaleLevel;
	_scale = scales[_currentScaleLevel];
	_rotation = 0.0f;
	_keyFrameTime = 0.0f;

	_timePassed = 0.0f;
	_routeIndex = 0;
	_animTime = 0.0f;
	_updateRotation = true;
	_rotateTime = 0.0f;
	_rotateAnimTime = 0.0f;
	_turning = false;
	_increaseScale = false;
	_scaleChange = false;
	_translation = _OSMdata->getRouteData()[_routeIndex].point;
}

/*!********************************************************************************************
\brief  Handles user input and updates live variables accordingly.
***********************************************************************************************/
void OGLESNavigation2D::eventMappedInput(pvr::SimplifiedInput e)
{
	const float transDelta = float(getFrameTime());
	switch (e)
	{
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
		if (_cameraMode == CameraMode::Auto)
		{
			_cameraMode = CameraMode::Manual;
		}
		else
		{
			_cameraMode = CameraMode::Auto;
		}
		resetCameraVariables();
		updateSubtitleText();
		break;
	// zoom in
	case pvr::SimplifiedInput::Action2:
		if (_cameraMode == CameraMode::Manual)
		{
			_scale *= 1.05f;
			_scale = glm::min(_scale, 10.0f);
		}
		break;
	// zoom out
	case pvr::SimplifiedInput::Action3:
		if (_cameraMode == CameraMode::Manual)
		{
			_scale *= .95f;
			_scale = glm::max(_scale, 1.75f);
		}
		break;
	case pvr::SimplifiedInput::Up:
		if (_cameraMode == CameraMode::Manual)
		{
			float fup = (-transDelta * 1.0f / _scale);
			_translation.y += fup;
		}
		break;
	case pvr::SimplifiedInput::Down:
		if (_cameraMode == CameraMode::Manual)
		{
			float fup = -(-transDelta * 1.0f / _scale);
			_translation.y += fup;
		}
		break;
	case pvr::SimplifiedInput::Left:
		if (_cameraMode == CameraMode::Manual)
		{
			float fright = (transDelta * 1.0f / _scale);
			_translation.x += fright;
		}
		break;
	case pvr::SimplifiedInput::Right:
		if (_cameraMode == CameraMode::Manual)
		{
			float fright = -(transDelta * 1.0f / _scale);
			_translation.x += fright;
		}
		break;
	}
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by the Shell once per run, before the rendering _deviceResources->context is created.
Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
If the rendering _deviceResources->context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESNavigation2D::initApplication()
{
	// As we are rendering in 2D we have no need for either of the depth of stencil buffers
	setDepthBitsPerPixel(0);
	setStencilBitsPerPixel(0);

	_clipPlanes.resize(4);

	_OSMdata.reset(new NavDataProcess(getAssetStream(MapFile)));
	pvr::Result result = _OSMdata->loadAndProcessData();

	resetCameraVariables();

	return result;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by PVRShell upon initialization or after a change in the rendering _deviceResources->context.
Used to initialize variables that are dependent on the rendering _deviceResources->context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESNavigation2D::initView()
{
	_deviceResources.reset(new DeviceResources());

	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());

	gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &_deviceResources->defaultFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);

	if (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2)
	{
		if (!gl::isGlExtensionSupported("GL_OES_vertex_array_object"))
		{
			setExitMessage("Unable to create vertex array objects as extension 'GL_OES_vertex_array_object' is unsupported.");
			return pvr::Result::InitializationError;
		}
	}

	if (!loadTexture())
	{
		return pvr::Result::UnknownError;
	}

	_numRows = _OSMdata->getNumRows();
	_numCols = _OSMdata->getNumCols();

	Log(LogLevel::Information, "Initialising Tile Data");

	_mapWorldDim = getMapWorldDimensions(*_OSMdata, _numCols, _numRows);

	_OSMdata->initTiles(glm::ivec2(getWidth(), getHeight()));

	_tileRenderingResources.resize(_numCols);
	for (uint32_t i = 0; i < _numCols; ++i)
	{
		_tileRenderingResources[i].resize(_numRows);
	}

	pvr::VertexAttributeInfo vertexInfo[] =
	{
		pvr::VertexAttributeInfo(0, pvr::DataType::Float32, 2, 0, "myVertex"),
		pvr::VertexAttributeInfo(1, pvr::DataType::Float32, 2, sizeof(float) * 2, "texCoord")
	};

	_deviceResources->vertexConfiguration.addVertexAttribute(0, vertexInfo[0]);
	_deviceResources->vertexConfiguration.addVertexAttribute(0, vertexInfo[1]);
	_deviceResources->vertexConfiguration.setInputBinding(0, sizeof(float) * 4);
	_deviceResources->vertexConfiguration.topology = pvr::PrimitiveTopology::TriangleList;

	const char* attributeNames[] = { vertexBindings[0].variableName.c_str(), vertexBindings[1].variableName.c_str() };
	const uint16_t attributeIndices[] = { static_cast<uint16_t>(AttributeIndices::VertexArray),
	                                      static_cast<uint16_t>(AttributeIndices::TexCoordArray)
	                                    };
	const uint32_t numAttributes = 2;

	{
		if (!(_deviceResources->roadProgram = pvr::utils::createShaderProgram(*this, "AA_VertShader.vsh", "AA_FragShader.fsh", attributeNames, attributeIndices, numAttributes)))
		{
			setExitMessage("Unable to create road program (%s, %s)", "AA_VertShader.vsh", "AA_FragShader.fsh");
			return pvr::Result::UnknownError;
		}

		_deviceResources->roadColorUniformLocation = gl::GetUniformLocation(_deviceResources->roadProgram, "myColor");
		_deviceResources->roadTransformUniformLocation = gl::GetUniformLocation(_deviceResources->roadProgram, "transform");

		_deviceResources->roadRequiredSrcRGB = GL_SRC_ALPHA;
		_deviceResources->roadRequiredDstRGB = GL_ONE_MINUS_SRC_ALPHA;
		_deviceResources->roadRequiredSrcAlpha = GL_ONE;
		_deviceResources->roadRequiredDstAlpha = GL_ZERO;
	}

	{
		if (!(_deviceResources->fillProgram = pvr::utils::createShaderProgram(*this, "VertShader.vsh", "FragShader.fsh", attributeNames, attributeIndices, numAttributes)))
		{
			setExitMessage("Unable to create fill program (%s, %s)", "VertShader.vsh", "FragShader.fsh");
			return pvr::Result::UnknownError;
		}

		_deviceResources->fillColorUniformLocation = gl::GetUniformLocation(_deviceResources->fillProgram, "myColor");
		_deviceResources->fillTransformUniformLocation = gl::GetUniformLocation(_deviceResources->fillProgram, "transform");
	}

	Log(LogLevel::Information, "Remapping item coordinate data");
	remapItemCoordinates(*_OSMdata, _numCols, _numRows, _mapWorldDim);

	Log(LogLevel::Information, "Creating UI renderer items");
	createUIRendererItems();

	setUniforms();
    
    bool isScreenRotate = this->isScreenRotated() && this->isFullScreen();
    _screenWidth = getWidth(), _screenHeight = getHeight();
    if(isScreenRotate)
    {
        std::swap(_screenWidth, _screenHeight);
    }

    _projMtx = pvr::math::ortho(_deviceResources->context->getApiVersion(), 0.0f, (float)_screenWidth, 0.0f, (float)_screenHeight);
    
	
	_mapProjMtx = _tileRenderingResources[0][0].renderer->getScreenRotation() * _projMtx;

	Log(LogLevel::Information, "Creating per Tile buffers");
	createBuffers();

	Log(LogLevel::Information, "Converting Route");
	initRoute();

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation2D");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	updateSubtitleText();

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);
	gl::ClearColor(_clearColorUniform.r, _clearColorUniform.g, _clearColorUniform.b, _clearColorUniform.a);
	gl::ClearDepthf(1.0f);
	gl::ClearStencil(0);

	setDefaultStates();

	return pvr::Result::Success;
}

void OGLESNavigation2D::updateSubtitleText()
{
	if (_cameraMode == CameraMode::Auto)
	{
		_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Automatic Camera Mode"));
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultDescription()->setText("Manual Camera Model use up/down/left/right to control the camera");
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESNavigation2D::renderFrame()
{
	debugLogApiError("Frame begin");

	updateAnimation();
	float r = glm::radians(_rotation);

	_mapMVPMtx = _mapProjMtx
	             * glm::translate(glm::vec3(_translation.x + _screenWidth * .5 /*center the map*/, _translation.y + _screenHeight * .5/*center the map*/, 0.0f))// final transform
	             * glm::translate(glm::vec3(-_translation.x, -_translation.y, 0.0f))// undo the translation
	             * glm::rotate(r, glm::vec3(0.0f, 0.0f, 1.0f))// rotate
	             * glm::scale(glm::vec3(_scale, _scale, 1.0f))// scale the focus area
	             * glm::translate(glm::vec3(_translation.x, _translation.y, 0.0f)); // translate the camera to the center of the current focus area

	calculateClipPlanes();

	render();

	//UIRENDERER
	{
		// render UI
		_deviceResources->uiRenderer.beginRendering();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}

	debugLogApiError("Frame end");

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering _deviceResources->context.
***********************************************************************************************************************/
pvr::Result OGLESNavigation2D::releaseView()
{
	//Clean up tile rendering resource data.
	_tileRenderingResources.clear();

	_OSMdata->releaseTileData();
	_OSMdata.reset(0);
	_OSMdata = nullptr;

	//Reset context and associated resources.
	_deviceResources.reset(0);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering _deviceResources->context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESNavigation2D::quitApplication()
{
	return pvr::Result::Success;
}

void OGLESNavigation2D::setDefaultStates()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->context->getOnScreenFbo());
	gl::UseProgram(0);

	// disable most states
	gl::Disable(GL_BLEND);
	gl::Disable(GL_DEPTH_TEST);
	gl::Disable(GL_STENCIL_TEST);
	gl::DepthMask(false);
	gl::StencilMask(0);

	// Disable back face culling
	gl::Disable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);

	gl::FrontFace(GL_CCW);

	gl::Viewport(0, 0, getWidth(), getHeight());
}

void OGLESNavigation2D::bindAndClearFramebuffer()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->defaultFbo);
	gl::Clear(GL_COLOR_BUFFER_BIT);
}

bool OGLESNavigation2D::initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, const uint32_t col, const uint32_t row)
{
	begin->renderer.construct();
	auto& renderer = *begin->renderer;
	if (!renderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		Log(LogLevel::Critical, "Cannot initialise UI Renderer\n");
		return false;
	}

	if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
	{
		begin->font = begin->renderer->createFont(_deviceResources->fontTexture, _deviceResources->fontHeader, _deviceResources->fontSampler);
	}
	else
	{
		begin->font = begin->renderer->createFont(_deviceResources->fontTexture, _deviceResources->fontHeader);
	}

	begin->col = col;
	begin->row = row;

	auto& tile = _OSMdata->getTiles()[col][row];

	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
		{
			for (uint32_t i = 0; i < BuildingType::None; ++i)
			{
				if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i)
				{
					begin->spriteImages[i] = begin->renderer->createImageFromAtlas(_deviceResources->texAtlas,
					                         _deviceResources->atlasOffsets[i],
					                         _deviceResources->texAtlasHeader.getWidth(),
					                         _deviceResources->texAtlasHeader.getHeight());
					begin->spriteImages[i]->commitUpdates();

					_stateTracker = begin->renderer->getStateTracker();
				}
			}
		}
	}

	for (auto it = begin + 1; it < end; ++it)
	{
		it->font = begin->font;
		it->renderer = begin->renderer;
		for (uint32_t lod = 0; lod < LOD::Count; ++lod)
		{
			for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
			{
				for (uint32_t i = 0; i < BuildingType::None; ++i)
				{
					if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i)
					{
						it->spriteImages[i] = begin->spriteImages[i];
					}
				}
			}
		}
		it->col = begin->col;
		it->row = begin->row;
	}
	return true;
}

void OGLESNavigation2D::renderTile(const Tile& tile, TileRenderingResources& renderingResources)
{
	uint32_t offset = 0;

	// Bind the vertex and index buffers for the tile
	if (_stateTracker.vao != renderingResources.vao)
	{
		if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
		{
			gl::BindVertexArray(renderingResources.vao);
		}
		else
		{
			gl::ext::BindVertexArrayOES(renderingResources.vao);
		}
		_stateTracker.vao = renderingResources.vao;
		_stateTracker.vaoChanged = true;
	}

	if (_stateTracker.activeTextureUnit != 0 || _stateTracker.activeTextureUnitChanged)
	{
		_stateTracker.activeTextureUnit = GL_TEXTURE0;
		gl::ActiveTexture(GL_TEXTURE0);
		_stateTracker.activeTextureUnitChanged = true;
	}
	else
	{
		_stateTracker.activeTextureUnitChanged = false;
	}

	if (_stateTracker.boundTexture != _deviceResources->texAtlas || _stateTracker.boundTextureChanged)
	{
		_stateTracker.boundTexture = _deviceResources->texAtlas;
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texAtlas);

		_stateTracker.boundTextureChanged = true;
	}
	else
	{
		_stateTracker.boundTextureChanged = false;
	}

	if (renderingResources.properties.parkingNum > 0 || renderingResources.properties.buildNum > 0 ||
	    renderingResources.properties.innerNum > 0 || renderingResources.properties.areaNum > 0)
	{
		if (_stateTracker.activeProgram != _deviceResources->fillProgram)
		{
			gl::UseProgram(_deviceResources->fillProgram);
			_stateTracker.activeProgram = _deviceResources->fillProgram;
			_stateTracker.activeProgramChanged = true;
		}

		if (_stateTracker.blendEnabled)
		{
			gl::Disable(GL_BLEND);
			_stateTracker.blendEnabled = false;
			_stateTracker.blendEnabledChanged = true;
		}

		// Draw the car parking
		if (renderingResources.properties.parkingNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_parkingColorUniform));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.parkingNum, GL_UNSIGNED_INT, 0);
			offset += renderingResources.properties.parkingNum;
		}

		// Draw the buildings
		if (renderingResources.properties.buildNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_buildColorUniform));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.buildNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.buildNum;
		}

		// Draw the insides of car parking and buildings for polygons with holes
		if (renderingResources.properties.innerNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_clearColorUniform));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.innerNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.innerNum;
		}

		// Draw the road areas
		if (renderingResources.properties.areaNum > 0)
		{
			gl::UniformMatrix4fv(_deviceResources->fillTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));
			gl::Uniform4fv(_deviceResources->fillColorUniformLocation, 1, glm::value_ptr(_roadAreaColorUniform));

			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.areaNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.areaNum;
		}
	}

	if (renderingResources.properties.serviceRoadNum > 0 || renderingResources.properties.otherRoadNum > 0 ||
	    renderingResources.properties.secondaryRoadNum > 0 || renderingResources.properties.primaryRoadNum > 0 ||
	    renderingResources.properties.trunkRoadNum > 0 || renderingResources.properties.motorwayNum > 0)
	{
		if (_stateTracker.activeProgram != _deviceResources->roadProgram)
		{
			gl::UseProgram(_deviceResources->roadProgram);
			_stateTracker.activeProgram = _deviceResources->roadProgram;
			_stateTracker.activeProgramChanged = true;
		}

		if (!_stateTracker.blendEnabled)
		{
			gl::Enable(GL_BLEND);
			_stateTracker.blendEnabled = true;
			_stateTracker.blendEnabledChanged = true;
		}

		if (_stateTracker.blendSrcRgb != _deviceResources->roadRequiredSrcRGB ||
		    _stateTracker.blendDstRgb != _deviceResources->roadRequiredDstRGB ||
		    _stateTracker.blendSrcAlpha != _deviceResources->roadRequiredSrcAlpha ||
		    _stateTracker.blendDstAlpha != _deviceResources->roadRequiredDstAlpha)
		{
			gl::BlendFuncSeparate(_deviceResources->roadRequiredSrcRGB, _deviceResources->roadRequiredDstRGB,
			                      _deviceResources->roadRequiredSrcAlpha, _deviceResources->roadRequiredDstAlpha);

			_stateTracker.blendSrcRgb = _deviceResources->roadRequiredSrcRGB;
			_stateTracker.blendDstRgb = _deviceResources->roadRequiredDstRGB;
			_stateTracker.blendSrcAlpha = _deviceResources->roadRequiredSrcAlpha;
			_stateTracker.blendDstAlpha = _deviceResources->roadRequiredDstAlpha;

			_stateTracker.blendSrcRgbChanged = true;
			_stateTracker.blendDstRgbChanged = true;
			_stateTracker.blendSrcAlphaChanged = true;
			_stateTracker.blendDstAlphaChanged = true;
		}

		gl::UniformMatrix4fv(_deviceResources->roadTransformUniformLocation, 1, GL_FALSE, glm::value_ptr(_mapMVPMtx));

		/**** Draw the roads ****/
		// REVERSE order of importance.
		//Service Roads
		if (renderingResources.properties.serviceRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_serviceRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.serviceRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.serviceRoadNum;
		}

		//Other (any other roads)
		if (renderingResources.properties.otherRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_otherRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.otherRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.otherRoadNum;
		}

		//Secondary Roads
		if (renderingResources.properties.secondaryRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_secondaryRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.secondaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.secondaryRoadNum;
		}

		//Primary Roads
		if (renderingResources.properties.primaryRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_primaryRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.primaryRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));

			offset += renderingResources.properties.primaryRoadNum;
		}

		//Trunk Roads
		if (renderingResources.properties.trunkRoadNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_trunkRoadColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.trunkRoadNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.trunkRoadNum;
		}

		if (renderingResources.properties.motorwayNum > 0)
		{
			gl::Uniform4fv(_deviceResources->roadColorUniformLocation, 1, glm::value_ptr(_motorwayColor));
			gl::DrawElements(GL_TRIANGLES, renderingResources.properties.motorwayNum, GL_UNSIGNED_INT, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * 4)));
			offset += renderingResources.properties.motorwayNum;
		}
	}
}

/*!*********************************************************************************************************************
\return Return true if no error occurred, false if the sampler descriptor set is not valid.
\brief  Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.
***********************************************************************************************************************/
bool OGLESNavigation2D::loadTexture()
{
	// load the diffuse texture
	if (!pvr::utils::textureUpload(*this, FontFile, _deviceResources->fontHeader, _deviceResources->fontTexture,
	                               _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		setExitMessage("FAILED to load texture %s.", FontFile);
		return false;
	}

	if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
	{
		// create font sampler
		gl::GenSamplers(1, &_deviceResources->fontSampler);

		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::SamplerParameteri(_deviceResources->fontSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugLogApiError("Unable to create the font sampler");
	}
	else
	{
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fontTexture);
		_stateTracker.boundTexture = _deviceResources->fontTexture;

		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugLogApiError("Unable to setup the texture parameters for the font texture");
	}

	//Load & generate texture atlas for icons.
	if (!pvr::utils::generateTextureAtlas(*this, SpriteFileNames, _deviceResources->atlasOffsets,
	                                      BuildingType::None, &_deviceResources->texAtlas, &_deviceResources->texAtlasHeader,
	                                      _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		Log(LogLevel::Critical, "Failed to generate texture atlas.");
		return false;
	}

	if (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2)
	{
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texAtlas);
		_stateTracker.boundTexture = _deviceResources->texAtlas;

		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return true;
}

/*!*********************************************************************************************************************
\brief  Setup uniforms used for drawing the map.
***********************************************************************************************************************/
void OGLESNavigation2D::setUniforms()
{
	// Set colour uniforms
	_clearColorUniform = glm::vec4(0.6863f, 0.9333f, 0.9333f, 1.0f);

	//Roads
	_roadAreaColorUniform = glm::vec4(0.9960f, 0.9960f, 0.9960f, 1.0f);

	_motorwayColor = glm::vec4(0.9098f, 0.5725f, 0.6352f, 1.0f);
	_trunkRoadColor = glm::vec4(0.9725f, 0.6980f, 0.6117f, 1.0f);
	_primaryRoadColor = glm::vec4(0.9882f, 0.8392f, 0.6431f, 1.0f);
	_secondaryRoadColor = glm::vec4(1.0f, 1.0f, 0.5019f, 1.0f);
	_serviceRoadColor = glm::vec4(0.996f, 0.996f, 0.996f, 1.0f);
	_otherRoadColor = glm::vec4(0.996f, 0.996f, 0.996f, 1.0f);

	_buildColorUniform = glm::vec4(1.0f, 0.7411f, 0.3568f, 1.0f);
	_parkingColorUniform = glm::vec4(0.9412f, 0.902f, 0.549f, 1.0f);
	_outlineColorUniform = glm::vec4(0.4392f, 0.5412f, 0.5647f, 1.0f);
}

/*!*********************************************************************************************************************
\brief  Converts pre-computed route into the appropriate co-ordinate space and calculates the routes total true distance
and partial distances between each node which is used later to animate the route.
***********************************************************************************************************************/
void OGLESNavigation2D::initRoute()
{
	convertRoute(_mapWorldDim, _numCols, _numRows, *_OSMdata, _weight, _rotation, _totalRouteDistance);
	if (_cameraMode == CameraMode::Auto)
	{
		//Initial weighting for first iteration of the animation
		_weight = _OSMdata->getRouteData()[0].distanceToNext / _totalRouteDistance;
		_keyFrameTime = 0.0f;
		_rotation = _OSMdata->getRouteData()[0].rotation;
	}
}

/*!*********************************************************************************************************************
\brief  Creates vertex and index buffers and records the secondary command buffers for each tile.
***********************************************************************************************************************/
void OGLESNavigation2D::createBuffers()
{
	uint32_t col = 0;
	uint32_t row = 0;
	// get the map dimension
	// calculate the aspect ratio
	// rescale the map with the aspect ratio

	for (uint32_t col = 0; col < _OSMdata->getTiles().size(); ++col)
	{
		auto& tileCol = _OSMdata->getTiles()[col];
		for (uint32_t row = 0; row < tileCol.size(); ++row)
		{
			Tile& tile = tileCol[row];

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				Tile::VertexData vertData(glm::vec2(remap(nodeIterator->second.coords, _OSMdata->getTiles()[0][0].min,
				                                    _OSMdata->getTiles()[_numCols - 1][_numRows - 1].max, -_mapWorldDim * .5, _mapWorldDim * .5)),
				                          nodeIterator->second.texCoords);

				tile.vertices.push_back(vertData);
			}

			auto& renderingResources = _tileRenderingResources[col][row];

			// Add car parking to indices
			renderingResources.properties.parkingNum = generateIndices(tile, tile.parkingWays);
			// Add buildings to indices
			renderingResources.properties.buildNum = generateIndices(tile, tile.buildWays);
			// Add inner ways to indices
			renderingResources.properties.innerNum = generateIndices(tile, tile.innerWays);
			// Add road area ways to indices
			renderingResources.properties.areaNum = generateIndices(tile, tile.areaWays);
			// Add roads to indices
			renderingResources.properties.serviceRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Service);
			renderingResources.properties.otherRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Other);
			renderingResources.properties.secondaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			renderingResources.properties.primaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			renderingResources.properties.trunkRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			renderingResources.properties.motorwayNum = generateIndices(tile, tile.roadWays, RoadTypes::Motorway);

			// Create vertex and index buffers
			//Interleaved vertex buffer (vertex position + texCoord)
			if (tile.vertices.size())
			{
				auto& tileRes = _tileRenderingResources[col][row];

				{
					// vertices buffer
					gl::GenBuffers(1, &tileRes.vbo);
					gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);

					uint32_t vboSize = static_cast<uint32_t>(tile.vertices.size() * sizeof(tile.vertices[0]));
					gl::BufferData(GL_ARRAY_BUFFER, vboSize, tile.vertices.data(), GL_STATIC_DRAW);
				}

				{
					// indices buffer
					gl::GenBuffers(1, &tileRes.ibo);
					gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.ibo);
					uint32_t iboSize = static_cast<uint32_t>(tile.indices.size() * sizeof(tile.indices[0]));
					gl::BufferData(GL_ARRAY_BUFFER, iboSize, tile.indices.data(), GL_STATIC_DRAW);
				}

				if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
				{
					gl::GenVertexArrays(1, &tileRes.vao);
					gl::BindVertexArray(tileRes.vao);
				}
				else
				{
					gl::ext::GenVertexArraysOES(1, &tileRes.vao);
					gl::ext::BindVertexArrayOES(tileRes.vao);
				}

				GLsizei stride = sizeof(float) * 4;
				gl::BindBuffer(GL_ARRAY_BUFFER, tileRes.vbo);
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileRes.ibo);

				// enable vertex attrib pointers
				for (auto it = _deviceResources->vertexConfiguration.attributes.begin(), end = _deviceResources->vertexConfiguration.attributes.end(); it != end; ++it)
				{
					gl::EnableVertexAttribArray(it->index);
					GLenum type = pvr::utils::convertToGles(it->format);
					bool isNormalised = pvr::dataTypeIsNormalised(it->format);
					auto offset = it->offsetInBytes;

					gl::VertexAttribPointer(it->index, it->width, type, isNormalised, stride, reinterpret_cast<const void*>(offset));


					_stateTracker.vertexAttribArray[it->index] = GL_TRUE;
					_stateTracker.vertexAttribArrayChanged[it->index] = true;

					_stateTracker.vertexAttribBindings[it->index] = it->index;
					_stateTracker.vertexAttribSizes[it->index] = it->width;
					_stateTracker.vertexAttribTypes[it->index] = type;
					_stateTracker.vertexAttribNormalized[it->index] = isNormalised;
					_stateTracker.vertexAttribStride[it->index] = 0;
					_stateTracker.vertexAttribOffset[it->index] = reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(offset));

					_stateTracker.vertexAttribPointerChanged[it->index] = true;
				}

				if (_deviceResources->context->getApiVersion() != pvr::Api::OpenGLES2)
				{
					gl::BindVertexArray(0);
				}
				else
				{
					gl::ext::BindVertexArrayOES(0);
				}

				for (auto it = _deviceResources->vertexConfiguration.attributes.begin(), end = _deviceResources->vertexConfiguration.attributes.end(); it != end; ++it)
				{
					gl::DisableVertexAttribArray(it->index);
				}
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  Update animation using pre-computed path for the camera to follow.
***********************************************************************************************************************/
void OGLESNavigation2D::updateAnimation()
{
	static const float scaleAnimTime = 350.0f;
	static const float rotationScaler = 50.0f;
	static const float scaleGracePeriod = 8000.0f;
	static const float baseSpeed = 28.f;
	static float r1 = 0.0f;
	static float r2 = 0.0f;

	float dt = float(getFrameTime());
	_timePassed += dt;
	if (_cameraMode == CameraMode::Auto)
	{
		if (!_turning)
		{
			if (_keyFrameTime > 0.0f)
			{
				//Interpolate between two positions.
				_translation = glm::mix(_OSMdata->getRouteData()[_routeIndex].point,
				                        _OSMdata->getRouteData()[_routeIndex + 1].point, _animTime / _keyFrameTime);
			}
			else
			{
				_translation = _OSMdata->getRouteData()[_routeIndex].point;
			}
			_animTime += dt / _scale;
		}
		if (_OSMdata->getRouteData().size() > 2)
		{
			if (_animTime >= _keyFrameTime)
			{
				_turning = true;
				if (_updateRotation)
				{
					r1 = _OSMdata->getRouteData()[_routeIndex].rotation;
					r2 = _OSMdata->getRouteData()[_routeIndex + 1].rotation;

					float angleDiff = glm::abs(r1 - r2);

					if (angleDiff > 180.0f)
					{
						if (r1 > r2)
						{
							r2 += 360.0f;
						}
						else
						{
							r2 -= 360.0f;
						}
					}

					float diff = (r2 > r1) ? r2 - r1 : r1 - r2;
					// Calculate the time to animate the _rotation based on angle.
					_rotateTime = glm::abs(rotationScaler * (diff / (2.0f * glm::pi<float>())));
					_updateRotation = false;
				}

				if (_rotateTime > dt)
				{
					_rotation = glm::mix(r1, r2, _rotateAnimTime / _rotateTime);
				}
				_rotateAnimTime += dt;

				if (_rotateAnimTime >= _rotateTime)
				{
					_rotation = r2;
					_updateRotation = true;
					_turning = false;
					_rotateAnimTime = 0.0f;
				}
			}

			if (_animTime >= _keyFrameTime && !_turning)
			{
				_animTime = 0.0f;

				//Iterate through the route
				if (++_routeIndex == _OSMdata->getRouteData().size() - 1)
				{
					_rotation = _OSMdata->getRouteData()[0].rotation;
					_routeIndex = 0;
				}

				//get new weighting for this part of the route.
				_weight = _OSMdata->getRouteData()[_routeIndex].distanceToNext / _totalRouteDistance;
				_keyFrameTime = (float(_OSMdata->getRouteData().size()) * baseSpeed * glm::sqrt(_totalRouteDistance)) * _weight;
			}
		}
		else
		{
			Log(LogLevel::Debug, "Could not find multiple routes in the map data");
		}
	}

	// Check for _scale changes
	if (_cameraMode == CameraMode::Manual)
	{
		_currentScaleLevel = LOD::L4;
		for (int32_t i = LOD::L4; i >= 0; --i)
		{
			if (_scale > scales[_currentScaleLevel])
			{
				_currentScaleLevel = i;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if (_timePassed >= scaleGracePeriod)
		{
			_previousScaleLevel = _currentScaleLevel;
			if (_increaseScale)
			{
				if (++_currentScaleLevel == LOD::L4)
				{
					_increaseScale = false;
				}
			}
			else
			{
				if (--_currentScaleLevel == LOD::L1)
				{
					_increaseScale = true;
				}
			}

			_timePassed = 0.0f;
			_scaleChange = _previousScaleLevel != _currentScaleLevel;
		}

		if (_scaleChange)
		{
			if (_timePassed >= scaleAnimTime)
			{
				_scaleChange = false;
			}
			// interpolate
			_scale = glm::mix(routescales[_previousScaleLevel] * 1.5f, routescales[_currentScaleLevel] * 1.5f, _timePassed / scaleAnimTime);
		}
	}
}

bool skipAmenityLabel(AmenityLabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = (float)glm::distance(labelData.coords, glm::dvec2(extent));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

bool skipLabel(LabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this text crosses the tile boundary or the text overruns the end of the road segment.
	if (labelData.distToBoundary < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if the text overruns the end of the road segment.
	if (labelData.distToEndOfSegment < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = (float)glm::distance(labelData.coords, glm::dvec2(extent));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

/*!*********************************************************************************************************************
\brief  Record the primary command buffer.
***********************************************************************************************************************/
void OGLESNavigation2D::createUIRendererItems()
{
	for (uint32_t col = 0; col < _numCols; ++col)
	{
		for (uint32_t row = 0; row < _numRows; row++)
		{
			initializeRenderers(&_tileRenderingResources[col][row], &_tileRenderingResources[col][std::min(row + 1, _numRows - 1)], col, row);
		}
	}

	for (uint32_t col = 0; col < _numCols; ++col)
	{
		for (uint32_t row = 0; row < _numRows; ++row)
		{
			auto& tile = _OSMdata->getTiles()[col][row];
			auto& tileRes = _tileRenderingResources[col][row];
			for (uint32_t lod = 0; lod < LOD::Count; ++lod)
			{
				glm::dvec3 extent(0, 0, 0);
				if (!tile.icons[lod].empty() || !tile.labels[lod].empty() || !tile.amenityLabels[lod].empty())
				{
					tileRes.tileGroup[lod] = tileRes.renderer->createPixelGroup();
					auto& group = tileRes.tileGroup[lod];
					auto& camGroup = tileRes.cameraRotateGroup[lod] = tileRes.renderer->createPixelGroup();
					group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

					for (auto && icon : tile.icons[lod])
					{
						tileRes.amenityIcons[lod].push_back(AmenityIconGroup());
						auto& tileResIcon = tileRes.amenityIcons[lod].back();

						tileResIcon.iconData = icon;
						tileResIcon.group = tileRes.renderer->createPixelGroup();

						tileResIcon.group->add(tileRes.spriteImages[icon.buildingType]);

						// create the image - or at least take a copy that we'll work with from now on
						tileResIcon.icon.image = tileRes.spriteImages[icon.buildingType];
						tileResIcon.icon.image->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

						// flip the icon
						tileResIcon.icon.image->setRotation(glm::pi<float>());
						tileResIcon.icon.image->commitUpdates();

						// add the amenity icon to the group
						tileResIcon.group->add(tileResIcon.icon.image);
						tileResIcon.group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);
						tileResIcon.group->commitUpdates();

						group->add(tileResIcon.group);
					}

					for (auto && amenityLabel : tile.amenityLabels[lod])
					{
						tileRes.amenityLabels[lod].push_back(AmenityLabelGroup());
						auto& tileResAmenityLabel = tileRes.amenityLabels[lod].back();

						tileResAmenityLabel.iconData = amenityLabel.iconData;

						tileResAmenityLabel.group = tileRes.renderer->createPixelGroup();

						tileResAmenityLabel.label.text = tileRes.renderer->createText(amenityLabel.name, tileRes.font);
						debug_assertion(tileResAmenityLabel.label.text.isValid(), "Amenity label must be a valid UIRenderer Text Element");
						tileResAmenityLabel.label.text->setColor(0.f, 0.f, 0.f, 1.f);
						tileResAmenityLabel.label.text->setAlphaRenderingMode(true);

						float txtScale = 1.0f / (scales[lod + 1] * 12.0f);

						tileResAmenityLabel.label.text->setScale(txtScale, txtScale);
						tileResAmenityLabel.label.text->setPixelOffset(-glm::abs(tileResAmenityLabel.iconData.coords - amenityLabel.coords));
						tileResAmenityLabel.label.text->commitUpdates();

						if (skipAmenityLabel(amenityLabel, tileResAmenityLabel.label, extent))
						{
							continue;
						}

						// add the label to its corresponding amenity group
						tileResAmenityLabel.group->add(tileResAmenityLabel.label.text);
						tileResAmenityLabel.group->commitUpdates();

						group->add(tileResAmenityLabel.group);
					}

					for (auto && label : tile.labels[lod])
					{
						tileRes.labels[lod].push_back(Label());
						auto& tileResLabel = tileRes.labels[lod].back();

						tileResLabel.text = tileRes.renderer->createText(label.name, tileRes.font);
						debug_assertion(tileResLabel.text.isValid(), "Label must be a valid UIRenderer Text Element");

						tileResLabel.text->setColor(0.f, 0.f, 0.f, 1.f);
						tileResLabel.text->setAlphaRenderingMode(true);

						float txtScale = label.scale * 2.0f;

						tileResLabel.text->setScale(txtScale, txtScale);
						tileResLabel.text->setPixelOffset(label.coords);
						tileResLabel.text->commitUpdates();

						if (skipLabel(label, tileResLabel, extent))
						{
							continue;
						}

						group->add(tileResLabel.text);
					}

					group->commitUpdates();
					camGroup->add(group);
					camGroup->commitUpdates();
				}
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  Find the tiles that need to be rendered.
***********************************************************************************************************************/
void OGLESNavigation2D::render()
{
	_deviceResources->renderqueue.clear();

	for (uint32_t i = 0; i < _numCols; ++i)
	{
		for (uint32_t j = 0; j < _numRows; ++j)
		{
			auto& tile = _tileRenderingResources[i][j];
			if (inFrustum(_OSMdata->getTiles()[i][j].screenMin, _OSMdata->getTiles()[i][j].screenMax))
			{
				_deviceResources->renderqueue.push_back(&tile);

				// Update text elements
				updateLabels(i, j);

				// Update icons (points of interest)
				updateAmenities(i, j);

				// Update icons (points of interest)
				updateGroups(i, j);
			}
		}
	}

	bindAndClearFramebuffer();

	for (auto && tile : _deviceResources->renderqueue)
	{
		if (tile->renderer.isValid())
		{
			renderTile(_OSMdata->getTiles()[tile->col][tile->row], *tile);
		}
		for (int lod = _currentScaleLevel; lod < LOD::Count; ++lod)
		{
			if (tile->cameraRotateGroup[lod].isValid())
			{
				tile->renderer->beginRendering(_stateTracker);
				tile->cameraRotateGroup[lod]->render();
				tile->renderer->endRendering(_stateTracker);
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  Capture frustum planes from the current View Projection matrix
***********************************************************************************************************************/
void OGLESNavigation2D::calculateClipPlanes()
{
	glm::vec4 rowX = glm::vec4(_mapMVPMtx[0][0], _mapMVPMtx[1][0], _mapMVPMtx[2][0], _mapMVPMtx[3][0]);
	glm::vec4 rowY = glm::vec4(_mapMVPMtx[0][1], _mapMVPMtx[1][1], _mapMVPMtx[2][1], _mapMVPMtx[3][1]);
	glm::vec4 rowW = glm::vec4(_mapMVPMtx[0][3], _mapMVPMtx[1][3], _mapMVPMtx[2][3], _mapMVPMtx[3][3]);

	_clipPlanes[0] = Plane(rowW - rowX); //Right
	_clipPlanes[1] = Plane(rowW + rowX); //Left
	_clipPlanes[2] = Plane(rowW - rowY); //Top
	_clipPlanes[3] = Plane(rowW + rowY); //Bottom
}

/*!*********************************************************************************************************************
\param min The minimum co-ordinates of the bounding box.
\param max The maximum co-ordinates of the bounding box.
\return boolean True if inside the view frustum, false if outside.
\brief  Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
Only the top, bottom, left and right planes of the view frustum are taken into consideration to optimize the intersection test.
***********************************************************************************************************************/
bool OGLESNavigation2D::inFrustum(glm::vec2 min, glm::vec2 max)
{
	//Test the axis-aligned bounding box against each frustum plane,
	//cull if all points are outside of one the view frustum planes.
	for (uint32_t i = 0; i < _clipPlanes.size(); ++i)
	{
		uint32_t pointsOut = 0;

		// Test the points against the plane
		if ((_clipPlanes[i].normal.x * min.x + _clipPlanes[i].normal.y * min.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * max.x + _clipPlanes[i].normal.y * min.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * max.x + _clipPlanes[i].normal.y * max.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * min.x + _clipPlanes[i].normal.y * max.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}

		//If all four corners are outside of the plane then it is not visible.
		if (pointsOut == 4)
		{
			return false;
		}
	}
	return true;
}

void OGLESNavigation2D::updateGroups(uint32_t col, uint32_t row)
{
	const glm::vec2 pixelOffset = _translation * _scale;
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		if (tileRes.tileGroup[lod].isValid())
		{
			tileRes.tileGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.tileGroup[lod]->setPixelOffset(pixelOffset.x, pixelOffset.y);
			tileRes.tileGroup[lod]->setScale(_scale, _scale);
			tileRes.tileGroup[lod]->commitUpdates();
		}
		if (tileRes.cameraRotateGroup[lod].isValid())
		{
			tileRes.cameraRotateGroup[lod]->setRotation(glm::radians(_rotation));
			tileRes.cameraRotateGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.cameraRotateGroup[lod]->commitUpdates();
		}
	}
}

/*!*********************************************************************************************************************
\param col  Column index for tile.
\param row  Row index for tile.
\brief Update the renderable text (dependant on LOD level) using the pre-processed data (position, scale, _rotation, std::string) and UIRenderer.
***********************************************************************************************************************/
void OGLESNavigation2D::updateLabels(uint32_t col, uint32_t row)
{
	Tile& tile = _OSMdata->getTiles()[col][row];
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t labelIdx = 0; labelIdx < tile.labels[lod].size(); ++labelIdx)
		{
			auto& tileResLabelLod = tileRes.labels[lod];

			if (tileResLabelLod.empty())
			{
				continue;
			}

			auto& tileLabel = tile.labels[lod][labelIdx];
			auto& tileResLabel = tileRes.labels[lod][labelIdx];
			if (tileResLabel.text.isNull())
			{
				continue;
			}

			glm::dvec2 offset(0, 0);

			float txtScale = tileLabel.scale * 2.0f;

			// Make sure road text is displayed upright (between 0 deg and 180 deg), otherwise flip it.
			float total_angle = tileLabel.rotation + _rotation; // Use that to calculate if the text is upright
			float angle = tileLabel.rotation;

			// check whether the label needs flipping
			// we add a small buffer onto the total angles to reduce the chance of parts of roads being flipped whilst other parts are not
			if ((total_angle - 0.2f) < 0.0f)
			{
				angle += glm::degrees(glm::pi<float>());
			}
			else if ((total_angle + 0.2f) > glm::degrees(glm::pi<float>()) && (total_angle + 0.2f) < (2.0f * glm::degrees(glm::pi<float>())))
			{
				angle -= glm::degrees(glm::pi<float>());
			}

			float aabbHeight = tileResLabel.text->getBoundingBox().getSize().y;

			offset.y += tileLabel.scale * aabbHeight * 0.6f; // CENTRE THE TEXT ON THE ROAD

			// rotate the label to align with the road rotation
			tileResLabel.text->setRotation(glm::radians(angle));
			tileResLabel.text->setScale(txtScale, txtScale);
			tileResLabel.text->commitUpdates();
		}
	}
}

/*!*********************************************************************************************************************
\param col  Column index for tile.
\param row  Row index for tile.
\brief Update renderable icon, dependant on LOD level (for buildings such as; cafe, pub, library etc.) using the pre-processed data (position, type) and UIRenderer.
***********************************************************************************************************************/
void OGLESNavigation2D::updateAmenities(uint32_t col, uint32_t row)
{
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t amenityIconIndex = 0; amenityIconIndex < tileRes.amenityIcons[lod].size(); ++amenityIconIndex)
		{
			AmenityIconGroup& amenityIcon = tileRes.amenityIcons[lod][amenityIconIndex];
			debug_assertion(amenityIcon.icon.image.isValid(), "Amenity Icon must be a valid UIRenderer Icon");

			float iconScale = (1.0f / (_scale * 20.0f));
			iconScale = glm::clamp(iconScale, amenityIcon.iconData.scale, amenityIcon.iconData.scale * 2.0f);

			amenityIcon.icon.image->setScale(glm::vec2(iconScale, iconScale));
			amenityIcon.icon.image->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityIcon.group->setRotation(glm::radians(-_rotation));
			amenityIcon.group->setPixelOffset(static_cast<float>(amenityIcon.iconData.coords.x), static_cast<float>(amenityIcon.iconData.coords.y));
			amenityIcon.group->commitUpdates();
		}

		for (uint32_t amenityLabelIndex = 0; amenityLabelIndex < tileRes.amenityLabels[lod].size(); ++amenityLabelIndex)
		{
			AmenityLabelGroup& amenityLabel = tileRes.amenityLabels[lod][amenityLabelIndex];
			if (amenityLabel.label.text.isNull())
			{
				continue;
			}

			float txtScale = 1.0f / (_scale * 15.0f);

			amenityLabel.label.text->setScale(txtScale, txtScale);
			// move the label below the icon based on the size of the label
			amenityLabel.label.text->setPixelOffset(0.0f, -2.2f * amenityLabel.label.text->getBoundingBox().getHalfExtent().y * txtScale);
			amenityLabel.label.text->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityLabel.group->setRotation(glm::radians(-_rotation));
			amenityLabel.group->setPixelOffset(static_cast<float>(amenityLabel.iconData.coords.x), static_cast<float>(amenityLabel.iconData.coords.y));
			amenityLabel.group->commitUpdates();
		}
	}
}

/*!*********************************************************************************************************************
\return Return an unique_ptr to a new Demo class, supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OGLESNavigation2D()); }
