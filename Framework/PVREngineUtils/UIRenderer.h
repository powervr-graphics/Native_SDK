/*!
\brief Contains implementations of functions for the classes in UIRenderer.h
\file PVREngineUtils/UIRenderer.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVREngineUtils/Sprite.h"


namespace pvr {
namespace ui {
/// <summary>Manages and render the sprites.</summary>
class UIRenderer
{
public:
	/// <summary>Information used for uploading required info to the shaders (matrices, attributes etc).</summary>
	struct ProgramData
	{
		enum Uniform { UniformMVPmtx, UniformFontTexture, UniformColor, UniformAlphaMode, UniformUVmtx, NumUniform };
		enum Attribute { AttributeVertex, AttributeUV, NumAttribute };
		int32 uniforms[NumUniform];
		int32 attributes[NumAttribute];
	};

	const api::Buffer& getFontIbo()
	{
		if (!_fontIbo.isValid())
		{
			// create the FontIBO
			std::vector<uint16> fontFaces;
			fontFaces.resize(impl::Font_::FontElement);

			for (uint32 i = 0; i < impl::Font_::MaxRenderableLetters; ++i)
			{
				fontFaces[i * 6] = 0 + i * 4;
				fontFaces[i * 6 + 1] = 3 + i * 4;
				fontFaces[i * 6 + 2] = 1 + i * 4;

				fontFaces[i * 6 + 3] = 3 + i * 4;
				fontFaces[i * 6 + 4] = 0 + i * 4;
				fontFaces[i * 6 + 5] = 2 + i * 4;
			}

			_fontIbo = getContext()->createBuffer(sizeof(fontFaces[0]) * impl::Font_::FontElement, types::BufferBindingUse::IndexBuffer, true);
			_fontIbo->update(&fontFaces[0], 0, (uint32)(sizeof(fontFaces[0]) * fontFaces.size()));
		}
		return _fontIbo;
	}

	const api::Buffer& getImageVbo()
	{
		if (_imageVbo.isNull())
		{
			// create the image vbo
			const float32 verts[] =
			{
				/*    Position  */
				-1.f, 1.f, 0.f, 1.0f, 0.0f, 1.0f, // upper left
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, -1.f, 0.f, 1.0f, 1.f, 0.0f, // lower right
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
			};
			_imageVbo = getContext()->createBuffer(sizeof(verts), types::BufferBindingUse::VertexBuffer, true);
			_imageVbo->update((void*)verts, 0, sizeof(verts));
		}
		return _imageVbo;
	}
	/// <summary>Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	UIRenderer() : _screenRotation(.0f), _numSprites(0) {}

	~UIRenderer()
	{
		release();
	}

	/// <summary>Return the graphics context the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	GraphicsContext& getContext() { return _context; }
	/// <summary>Return the graphics context the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	const GraphicsContext& getContext()const { return _context; }

	/// <summary>Returns the ProgramData used by this UIRenderer.</summary>
	const ProgramData& getProgramData() { return _programData; }

	/// <summary>Returns the pvr::api::GraphicsPipeline object used by this UIRenderer.</summary>
	api::ParentableGraphicsPipeline getPipeline() { return _pipeline; }

	/// <summary>Check that we have called beginRendering() and not called endRendering. See the beginRendering()
	/// method.</summary>
	bool isRendering() { return _activeCommandBuffer.isValid(); }

	/// <summary>Initialize the UIRenderer with a graphics context. MUST BE called exactly once before use, after a valid
	/// graphics context is available (usually, during initView).</summary>
	/// <param name="renderpass">A renderpass to use for this UIRenderer</param>
	/// <param name="subpass">The subpass to use for this UIRenderer</param>
	/// <param name="maxNumInstances"> maximum number of sprite instances to be allocated from this uirenderer.
	/// it must be atleast maxNumSprites becasue each sprites is an instance on its own.</param>
	/// <param name="maxNumSprites"> maximum number of renderable sprites (Text and Images) to be allocated from this uirenderer</param>
	Result init(api::RenderPass renderpass, uint32 subpass, uint32 maxNumInstances = 64, uint32 maxNumSprites = 64)
	{
		release();
		_mustEndCommandBuffer = false;
		_context = renderpass->getContext();
		_screenDimensions = glm::vec2(_context->getDisplayAttributes().width, _context->getDisplayAttributes().height);
		_renderpass = renderpass;
		_subpass = subpass;
		// screen rotated?
		if (_screenDimensions.y > _screenDimensions.x && _context->getDisplayAttributes().fullscreen)
		{
			rotateScreen90degreeCCW();
		}

		Result res;
		if (((res = init_CreateDescriptorSetLayout()) == Result::Success) &&
		    ((res = init_CreatePipelineAndRenderPass()) == Result::Success))
		{
			if (!setUpUboPools(maxNumInstances, maxNumSprites)) { res = Result::UnknownError; }
			if (!init_CreateDefaultSampler()) { res = Result::UnknownError; }
			if (!init_CreateDefaultSdkLogo()) { res = Result::UnknownError; }
			if (!init_CreateDefaultFont()) { res = Result::UnknownError; }
			if (!init_CreateDefaultTitle()) { res = Result::UnknownError; }
		}
		return res;
	}

	/// <summary>Release the engine and its resources. Must be called once after we are done with the UIRenderer.
	/// (usually, during releaseView).</summary>
	void release()
	{
		_renderpass.reset();
		_defaultFont.reset();
		_defaultTitle.reset();
		_defaultDescription.reset();
		_defaultControls.reset();
		_sdkLogo.reset();
		_renderpass.reset();

		_uboMaterial.reset();
		_uboMvp.reset();

		_texDescLayout.reset();
		_uboMvpDescLayout.reset();
		_uboMaterialLayout.reset();
		_pipelineLayout.reset();
		_pipeline.reset();
		_samplerBilinear.reset();
		_samplerTrilinear.reset();
		_activeCommandBuffer.reset();
		_fontIbo.reset();
		_imageVbo.reset();

		_descPool.reset();

		_context.reset();
	}

	/// <summary>Create a Text sprite. Initialize with string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text = "")
	{
		return createTextElement(text, _defaultFont);
	}

	/// <summary>Create Text sprite from string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text, const Font& font);

	TextElement createTextElement(const Font& font)
	{
		return createTextElement(std::string(""), font);
	}

	/// <summary>Create Text sprite from wide string. Uses the Default Font.</summary>
	/// <param name="text">Wide string that this Text object will be initialized with. Will use the Default Font.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text) { return createTextElement(text, _defaultFont); }

	/// <summary>Create Text sprite from wide string.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text, const Font& font);

	Text createText(const TextElement& textElement);


	/// <summary>Create a Text sprite. Initialize with string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text = "")
	{
		return createText(createTextElement(text));
	}

	/// <summary>Create Text sprite from string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text, const Font& font)
	{
		return createText(createTextElement(text, font));
	}

	Text createText(const Font& font)
	{
		return createText(createTextElement(font));
	}

	/// <summary>Create Text sprite from wide string. Uses the Default Font.</summary>
	/// <param name="text">Wide string that this Text object will be initialized with. Will use the Default Font.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::wstring& text)
	{
		return createText(createTextElement(text));
	}

	/// <summary>Create Text sprite from wide string.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::wstring& text, const Font& font)
	{
		return createText(createTextElement(text, font));
	}

	/// <summary>Get the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	float32 getRenderingDimX()const { return _screenDimensions.x; }

	glm::vec2 getRenderingDim()const { return _screenDimensions; }

	/// <summary>Get the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	float32 getRenderingDimY()const { return _screenDimensions.y; }

	Rectanglei getViewport()const { return Rectanglei(0, 0, (int32)getRenderingDimX(), (int32)getRenderingDimY()); }

	/// <summary>Set the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	void setRenderingDimX(float32 value) { _screenDimensions.x = value; }

	/// <summary>Set the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	void setRenderingDimY(float32 value) { _screenDimensions.y = value; }

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).
	/// </summary>
	/// <param name="apiTex">A pvr::api::TextureView object of the font texture file, which will be used directly.
	/// </param>
	/// <param name="tex">A pvr::TextureHeader of the same object. Necessary for the texture metadata.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload requires that you
	/// have already created a Texture2D object from the file and is suitable for sharing a texture between different
	/// UIRenderer objects.</remarks>
	Font createFont(api::TextureView& apiTex, const TextureHeader& tex, const api::Sampler& sampler = api::Sampler());

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).
	/// </summary>
	/// <param name="tex">A pvr::Texture object. A new pvr::api::Texture2D object will be created and used
	/// internally.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload directly uses a
	/// pvr::Texture for both data and metadata, but will create a new Texture2D so if using multiple
	/// UIRenderes using the same font, might not be the most efficient version.</remarks>
	Font createFont(const Texture& tex, const api::Sampler& sampler = api::Sampler());

	/// <summary>Create a pvr::ui::Image from an API texture.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="apiTex">A pvr::api::TextureView object of the texture file. Must be 2D. It will be used
	/// directly.</param>
	/// <param name="height">The height of the texture.</param>
	/// <param name="width">The width of the texture.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(api::TextureView& apiTex, int32 width, int32 height, const api::Sampler& sampler = api::Sampler());

	/// <summary>Create a pvr::ui::Image from a Texture asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="texture">A pvr::Texture object. Will be internally used to create an api::Texture2D to
	/// use.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(const Texture& texture, const api::Sampler& sampler = api::Sampler());

	/// <summary>Create a pvr::ui::Image from a Texture Atlas asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="tex">A pvr::Texture object. Will be internally used to create an api::Texture2D to
	/// use.</param>
	/// <param name="uv">Texture UV coordinate</param>
	/// <param name="width"> Texture Atlas width.</param>
	/// <param name="height"> Texture Atlas height</param>
	/// <param name="sampler">A sampler used for this Image (Optional)</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImageFromAtlas(api::TextureView& tex, const Rectanglef& uv, uint32 width, uint32 height,
	                           const api::Sampler& sampler = api::Sampler());

	/// <summary>Create a pvr::ui::MatrixGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	MatrixGroup createMatrixGroup();

	/// <summary>Create a pvr::ui::PixelGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	PixelGroup createPixelGroup();

	/// <summary>Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites
	/// after calling this function before calling endRendering.</summary>
	/// <param name="cb">The pvr::api::SecondaryCommandBuffer object where all the rendering commands will be put
	/// into.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(api::SecondaryCommandBuffer& cb)
	{
		beginRendering(cb, pvr::api::Fbo(), true);
	}

	void beginRendering(api::SecondaryCommandBuffer& cb, const pvr::api::Fbo& fbo, bool useRenderpass = false)
	{
		if (!cb->isRecording())
		{
			if (useRenderpass)
			{
				cb->beginRecording(_renderpass, _subpass);
			}
			else
			{
				cb->beginRecording(fbo, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else { _mustEndCommandBuffer = false; }
		cb->pushPipeline();// store the currently bound pipeline
		cb->bindPipeline(getPipeline());// bind the uirenderer pipeline
		_activeCommandBuffer = cb;
	}

	void beginRendering(api::CommandBuffer& cb)
	{
		debug_assertion(cb->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
		                " it must be in the Recording state");
		_mustEndCommandBuffer = false;
		cb->pushPipeline();// store the currently bound pipeline
		cb->bindPipeline(getPipeline());// bind the uirenderer pipeline
		_activeCommandBuffer = cb;
	}


	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided pvr::api::GraphicsPipeline.
	/// </summary>
	/// <param name="cb">The pvr::api::CommandBuffer object where all the rendering commands will be put into.
	/// </param>
	/// <param name="pipe">The pvr::api::GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(api::SecondaryCommandBuffer cb, pvr::api::GraphicsPipeline& pipe)
	{
		beginRendering(cb, pipe, pvr::api::Fbo(), true);
	}

	void beginRendering(api::SecondaryCommandBuffer cb, pvr::api::GraphicsPipeline& pipe,
	                    const pvr::api::Fbo& fbo, bool useFboRenderpass = false)
	{
		if (!cb->isRecording())
		{
			if (useFboRenderpass)
			{
				cb->beginRecording(_renderpass, _subpass);
			}
			else
			{
				cb->beginRecording(fbo, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else { _mustEndCommandBuffer = false; }
		cb->pushPipeline();
		cb->bindPipeline(pipe);
		_activeCommandBuffer = cb;
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided pvr::api::GraphicsPipeline.
	/// </summary>
	/// <param name="cb">The pvr::api::CommandBuffer object where all the rendering commands will be put into.
	/// </param>
	/// <param name="pipe">The pvr::api::GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(api::CommandBuffer cb, pvr::api::GraphicsPipeline& pipe)
	{
		debug_assertion(cb->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
		                " it must be in the Recording state");
		_mustEndCommandBuffer = false;
		cb->pushPipeline();
		cb->bindPipeline(pipe);
		_activeCommandBuffer = cb;
	}

	/// <summary>End rendering. Always call this method before submitting the commandBuffer passed to the UIRenderer.
	/// </summary>
	/// <remarks>This method must be called after you finish rendering sprites (after a call to beginRendering). The
	/// sequence must always be beginRendering, render ..., endRendering. Τry to group as many of the rendering
	/// commands (preferably all) between beginRendering and endRendering.</remarks>
	void endRendering()
	{
		if (_activeCommandBuffer.isValid())
		{
			_activeCommandBuffer->popPipeline();
			if (_mustEndCommandBuffer)
			{
				_mustEndCommandBuffer = false;
				_activeCommandBuffer->endRecording();
			}
			_activeCommandBuffer.reset();
		}
	}

	/// <summary>Get the pvr::api::CommandBuffer that is being used to currently render.</summary>
	/// <returns>If between a beginRendering and endRendering, the CommandBuffer used at beginRendering. Otherwise,
	/// null.</returns>
	api::CommandBufferBase& getActiveCommandBuffer() { return _activeCommandBuffer; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The default font. Constant overload.</returns>
	const Font& getDefaultFont() const { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>A pvr::ui::Font object of the default font.</returns>
	Font& getDefaultFont() { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image. Constant overload.</returns>
	const Image& getSdkLogo() const { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image.</returns>
	Image& getSdkLogo() { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultTitle() const { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultTitle() { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a description (subtitle)
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it). Constant overload.
	/// </returns>
	const Text& getDefaultDescription() const { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a description (subtitle
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultDescription() { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it). Constant overload.
	/// </returns>
	const Text& getDefaultControls() const { return _defaultControls; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultControls() { return _defaultControls; }

	/// <summary>Return the pvr::api::PipelineLayout object of the internal pvr::api::Pipeline object used by this
	/// UIRenderer.</summary>
	/// <returns>The pvr::api::PipelineLayout.</returns>
	pvr::api::PipelineLayout getPipelineLayout() { return _pipelineLayout; }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	glm::mat4 getProjection()const
	{
		return pvr::math::ortho(_context->getApiType(), 0.0, getRenderingDimX(), 0.0f, getRenderingDimY());
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCCW()
	{
		_screenRotation += glm::pi<pvr::float32>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCW()
	{
		_screenRotation -= glm::pi<pvr::float32>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	glm::mat4 getScreenRotation()const
	{
		return glm::rotate(_screenRotation, glm::vec3(0.0f, 0.0f, 1.f));
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	const api::DescriptorSetLayout& getTexDescriptorSetLayout()const
	{
		return _texDescLayout;
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	const api::DescriptorSetLayout& getUboDescSetLayout()const
	{
		return _uboMvpDescLayout;
	}

	/// <summary>Returns maximum renderable sprites (Text and Images)</summary>
	uint32 getMaxRenderableSprites()const { return _uboMaterial._numArrayId; }

	/// <summary>Return maximum number of instances supported (including sprites and groups)</summary>
	uint32 getMaxInstances()const { return _uboMvp._numArrayId; }

	/// <summary>return the number of available renderable sprites (Image and Text)</summary>
	uint32 getNumAvailableSprites()const
	{
		return _uboMaterial.getNumAvailableBufferArrays();
	}

	/// <summary>return the number of availble instance</summary>
	uint32 getNumAvailableInstances()const
	{
		return _uboMvp.getNumAvailableBufferArrays();
	}

private:


	friend class ::pvr::ui::impl::Image_;
	friend class ::pvr::ui::impl::Text_;
	friend class ::pvr::ui::impl::Group_;
	friend class ::pvr::ui::impl::Sprite_;
	friend class ::pvr::ui::impl::Font_;

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	pvr::uint64 generateGroupId() { return _groupId++; }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const api::DescriptorSetLayout&</returns>
	api::DescriptorPool& getDescriptorPool() { return _descPool; }
	api::Sampler getSamplerBilinear()const { return _samplerBilinear; }

	api::Sampler getSamplerTrilinear()const { return _samplerTrilinear; }

	struct UboMvp
	{
		friend class ::pvr::ui::UIRenderer;
		UboMvp(): _freeArrayId(0)
		{
		}
		bool init(GraphicsContext& context, api::DescriptorSetLayout& descLayout, api::DescriptorPool& pool, uint32 numElements);

		void reset()
		{
			_buffer = utils::StructuredMemoryView();
			_uboDescSetSet.reset();
		}

		void updateMvp(uint32 bufferArrayId, const glm::mat4x4& mvp)
		{
			_buffer.mapArrayIndex(0, bufferArrayId);
			_buffer.setValue(0, mvp);
			_buffer.unmap(0);
		}

		int32 getNewBufferSlice()
		{
			if (_freeArrayIds.size())
			{
				const uint32 id = _freeArrayIds.back();
				_freeArrayIds.pop_back();
				return id;
			}
			return (_freeArrayId < _numArrayId ? _freeArrayId++ : -1);
		}

		void releaseBufferSlice(uint32 id)
		{
			debug_assertion(id < _numArrayId, "Invalid id");
			_freeArrayIds.push_back(id);
		}

		void bindUboDynamic(api::CommandBufferBase& cb, const api::PipelineLayout& pipelayout, uint32 mvpBufferSlice)
		{
			uint32 dynamicOffsets[] =
			{
				_buffer.getAlignedElementArrayOffset(mvpBufferSlice)
			};
			cb->bindDescriptorSet(pipelayout, 1, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32 getNumAvailableBufferArrays()const
		{
			return (uint32)((_numArrayId - _freeArrayId) + _freeArrayIds.size());
		}


	private:

		uint32 _freeArrayId;
		pvr::utils::StructuredBufferView _buffer;
		pvr::api::DescriptorSet _uboDescSetSet;
		uint32 _numArrayId;
		std::vector<uint32> _freeArrayIds;
	};

	struct UboMaterial
	{
	public:
		friend class ::pvr::ui::UIRenderer;
		UboMaterial() :  _freeArrayId(0)
		{}

		void reset()
		{
			_buffer = utils::StructuredMemoryView();
			_uboDescSetSet.reset();
		}

		bool init(GraphicsContext& context, api::DescriptorSetLayout& descLayout, api::DescriptorPool& pool, uint32 numArrayId);
		void updateMaterial(uint32 arrayIndex, const glm::vec4& color, int32 alphaMode, const glm::mat4& uv);

		int32 getNewBufferArray()
		{
			return (_freeArrayId < _numArrayId ? _freeArrayId++ : -1);
		}

		void bindUboDynamic(api::CommandBufferBase& cb, const api::PipelineLayout& pipelayout, uint32 bufferSlice)
		{
			uint32 dynamicOffsets[] =
			{
				_buffer.getAlignedElementArrayOffset(bufferSlice)
			};
			cb->bindDescriptorSet(pipelayout, 2, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32 getNumAvailableBufferArrays()const
		{
			return (_numArrayId - _freeArrayId);
		}

	private:
		pvr::api::DescriptorSet _uboDescSetSet;
		uint32 _freeArrayId;
		uint32 _numArrayId;
		pvr::utils::StructuredBufferView _buffer;

	};

	UboMvp& getUbo() { return _uboMvp; }

	UboMaterial& getMaterial() { return _uboMaterial; }

	bool setUpUboPools(uint32 numInstances, uint32 numSprites);

	bool init_CreateDefaultFont();
	bool init_CreateDefaultSampler();
	bool init_CreateDefaultSdkLogo();
	bool init_CreateDefaultTitle();
	Result init_CreatePipelineAndRenderPass();
	Result init_CreateDescriptorSetLayout();

	api::RenderPass _renderpass;
	uint32 _subpass;
	ProgramData _programData;
	Font _defaultFont;
	Image _sdkLogo;
	Text _defaultTitle;
	Text _defaultDescription;
	Text _defaultControls;
	GraphicsContext _context;

	pvr::api::PipelineLayout _pipelineLayout;
	api::ParentableGraphicsPipeline _pipeline;
	api::DescriptorSetLayout _texDescLayout;
	api::DescriptorSetLayout _uboMvpDescLayout, _uboMaterialLayout;
	api::Sampler _samplerBilinear, _samplerTrilinear;
	api::DescriptorPool _descPool;
	api::CommandBufferBase _activeCommandBuffer;
	bool _mustEndCommandBuffer;
	api::Buffer _fontIbo;
	api::Buffer _imageVbo;
	glm::vec2 _screenDimensions;
	pvr::float32 _screenRotation;
	pvr::uint64 _groupId = 1;
	UboMvp _uboMvp;
	UboMaterial _uboMaterial;
	uint32 _numSprites;

};

}
}
