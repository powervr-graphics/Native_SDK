/*!
\brief Contains implementations of functions for the classes in UIRenderer.h
\file PVRUtils/Vulkan/UIRendererVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRUtils/Vulkan/SpriteVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"

namespace pvr {
namespace ui {
using namespace ::pvrvk;
/// <summary>Manages and render the sprites.</summary>
class UIRenderer
{
public:
	/// <summary>Information used for uploading required info to the shaders (matrices, attributes etc).</summary>
	struct ProgramData
	{
		/// <summary>Uniform index information.</summary>
		enum Uniform { UniformMVPmtx, UniformFontTexture, UniformColor, UniformAlphaMode, UniformUVmtx, NumUniform };
		/// <summary>Attribute index information.</summary>
		enum Attribute { AttributeVertex, AttributeUV, NumAttribute };
		/// <summary>An array of uniforms used by the UIRenderer.</summary>
		int32_t uniforms[NumUniform];
		/// <summary>An array of attributes used by the UIRenderer.</summary>
		int32_t attributes[NumAttribute];
	};

	/// <summary>Retrieves the Font index buffer.</summary>
	/// <returns>The pvrvk::Buffer corresponding to the Font index buffer.</returns>
	const Buffer& getFontIbo()
	{
		if (!_fontIbo.isValid())
		{
			// create the FontIBO
			std::vector<uint16_t> fontFaces;
			fontFaces.resize(impl::Font_::FontElement);

			for (uint32_t i = 0; i < impl::Font_::MaxRenderableLetters; ++i)
			{
				fontFaces[i * 6] = static_cast<uint16_t>(0 + i * 4);
				fontFaces[i * 6 + 1] = static_cast<uint16_t>(3 + i * 4);
				fontFaces[i * 6 + 2] = static_cast<uint16_t>(1 + i * 4);

				fontFaces[i * 6 + 3] = static_cast<uint16_t>(3 + i * 4);
				fontFaces[i * 6 + 4] = static_cast<uint16_t>(0 + i * 4);
				fontFaces[i * 6 + 5] = static_cast<uint16_t>(2 + i * 4);
			}

			_fontIbo = utils::createBuffer(getDevice(), sizeof(fontFaces[0]) * impl::Font_::FontElement,
			                               VkBufferUsageFlags::e_INDEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
			Device deviceTemp = getDevice()->getReference();
			pvr::utils::updateBuffer(deviceTemp, _fontIbo, &fontFaces[0], 0, static_cast<uint32_t>(sizeof(fontFaces[0]) * fontFaces.size()), true);
		}
		return _fontIbo;
	}

	/// <summary>Retrieves the Image vertex buffer.</summary>
	/// <returns>The pvrvk::Buffer corresponding to the Image vertex buffer.</returns>
	const Buffer& getImageVbo()
	{
		if (_imageVbo.isNull())
		{
			// create the image vbo
			const float verts[] =
			{
				/*    Position  */
				-1.f, 1.f, 0.f, 1.0f, 0.0f, 1.0f, // upper left
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, -1.f, 0.f, 1.0f, 1.f, 0.0f, // lower right
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
			};
			_imageVbo = utils::createBuffer(getDevice(), sizeof(verts), VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
			Device deviceTemp = getDevice()->getReference();
			pvr::utils::updateBuffer(deviceTemp, _imageVbo, static_cast<const void*>(verts), 0, sizeof(verts), true);
		}
		return _imageVbo;
	}
	/// <summary>Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	UIRenderer() : _screenRotation(.0f), _numSprites(0) {}

	/// <summary>Move Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	UIRenderer(UIRenderer&& rhs):
		_renderpass(std::move(rhs._renderpass)),
		_subpass(std::move(rhs._subpass)),
		_programData(std::move(rhs._programData)),
		_defaultFont(std::move(rhs._defaultFont)),
		_sdkLogo(std::move(rhs._sdkLogo)),
		_defaultTitle(std::move(rhs._defaultTitle)),
		_defaultDescription(std::move(rhs._defaultDescription)),
		_defaultControls(std::move(rhs._defaultControls)),
		_device(std::move(rhs._device)),
		_pipelineLayout(std::move(rhs._pipelineLayout)),
		_pipeline(std::move(rhs._pipeline)),
		_texDescLayout(std::move(rhs._texDescLayout)),
		_uboMvpDescLayout(std::move(rhs._uboMvpDescLayout)),
		_uboMaterialLayout(std::move(rhs._uboMaterialLayout)),
		_samplerBilinear(std::move(rhs._samplerBilinear)),
		_samplerTrilinear(std::move(rhs._samplerTrilinear)),
		_descPool(std::move(rhs._descPool)),
		_activeCommandBuffer(std::move(rhs._activeCommandBuffer)),
		_mustEndCommandBuffer(std::move(rhs._mustEndCommandBuffer)),
		_fontIbo(std::move(rhs._fontIbo)),
		_imageVbo(std::move(rhs._imageVbo)),
		_screenDimensions(std::move(rhs._screenDimensions)),
		_screenRotation(std::move(rhs._screenRotation)),
		_groupId(std::move(rhs._groupId)),
		_uboMvp(std::move(rhs._uboMvp)),
		_uboMaterial(std::move(rhs._uboMaterial)),
		_numSprites(std::move(rhs._numSprites)),
		_sprites(std::move(rhs._sprites)),
		_textElements(std::move(rhs._textElements)),
		_fonts(std::move(rhs._fonts))

	{
		upDateResourceOwnsership();
	}

	/// <summary>Assignment Operator overload. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	UIRenderer& operator=(UIRenderer&& rhs)
	{
		if (this == &rhs) { return *this; }
		_renderpass = std::move(rhs._renderpass);
		_subpass = std::move(rhs._subpass);
		_programData = std::move(rhs._programData);
		_defaultFont = std::move(rhs._defaultFont);
		_sdkLogo = std::move(rhs._sdkLogo);
		_defaultTitle = std::move(rhs._defaultTitle);
		_defaultDescription = std::move(rhs._defaultDescription);
		_defaultControls = std::move(rhs._defaultControls);
		_device = std::move(rhs._device);
		_pipelineLayout = std::move(rhs._pipelineLayout);
		_pipeline = std::move(rhs._pipeline);
		_texDescLayout = std::move(rhs._texDescLayout);
		_uboMvpDescLayout = std::move(rhs._uboMvpDescLayout);
		_uboMaterialLayout = std::move(rhs._uboMaterialLayout);
		_samplerBilinear = std::move(rhs._samplerBilinear);
		_samplerTrilinear = std::move(rhs._samplerTrilinear);
		_descPool = std::move(rhs._descPool);
		_activeCommandBuffer = std::move(rhs._activeCommandBuffer);
		_mustEndCommandBuffer = std::move(rhs._mustEndCommandBuffer);
		_fontIbo = std::move(rhs._fontIbo);
		_imageVbo = std::move(rhs._imageVbo);
		_screenDimensions = std::move(rhs._screenDimensions);
		_screenRotation = std::move(rhs._screenRotation);
		_groupId = std::move(rhs._groupId);
		_uboMvp = std::move(rhs._uboMvp);
		_uboMaterial = std::move(rhs._uboMaterial);
		_numSprites = std::move(rhs._numSprites);
		upDateResourceOwnsership();
		return *this;
	}

	//!\cond NO_DOXYGEN
	UIRenderer& operator=(const UIRenderer& rhs) = delete;
	UIRenderer(UIRenderer& rhs) = delete;
	//!\endcond

	/// <summary>Destructor for the UIRenderer which will release all resources currently in use.</summary>
	~UIRenderer()
	{
		release();
	}

	/// <summary>Return thedevice the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	/// <returns>The pvrvk::Device which was used to initialise the UIRenderer.</returns>
	DeviceWeakPtr& getDevice() { return _device; }

	/// <summary>Return the device the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	/// <returns>The pvrvk::Device which was used to initialise the UIRenderer.</returns>
	const DeviceWeakPtr& getDevice()const { return _device; }

	/// <summary>Returns the ProgramData used by this UIRenderer.</summary>
	/// <returns>The ProgramData structure used by the UIRenderer.</returns>
	const ProgramData& getProgramData() { return _programData; }

	/// <summary>Returns the GraphicsPipeline object used by this UIRenderer.</summary>
	/// <returns>The graphics pipeline being used by the UIRenderer.</returns>
	GraphicsPipeline getPipeline() { return _pipeline; }

	/// <summary>Check that we have called beginRendering() and not called endRendering. See the beginRendering() method.</summary>
	/// <returns>True if the command buffer is currently recording.</returns>
	bool isRendering() { return _activeCommandBuffer->isRecording(); }

	/// <summary>Initialize the UIRenderer with a graphics context.
	/// MUST BE called exactly once before use, after a valid
	/// graphics context is available (usually, during initView).
	/// Initialising creates its Default Text Font and PowerVR SDK logo. Therefore
	/// the calle must handle the texture uploads via assetLoader.
	/// </summary>
	/// <param name="width">The width of the screen used for rendering.</param>
	/// <param name="height">The height of the screen used for rendering</param>
	/// <param name="fullscreen">Indicates whether the rendering is occuring in full screen mode.</param>
	/// <param name="renderpass">A renderpass to use for this UIRenderer</param>
	/// <param name="subpass">The subpass to use for this UIRenderer</param>
	/// <param name="commandPool">The pvrvk::CommandPool object to use for allocating command buffers</param>
	/// <param name="queue">The pvrvk::Queue object to use for submitting command buffers</param>
	/// <param name="createDefaultLogo">Specifies whether a default logo should be initialised</param>
	/// <param name="maxNumInstances"> maximum number of sprite instances to be allocated from this uirenderer.
	/// it must be atleast maxNumSprites becasue each sprites is an instance on its own.</param>
	/// <param name="maxNumSprites"> maximum number of renderable sprites (Text and Images)
	/// to be allocated from this uirenderer</param>
	/// <returns>True indicating the result of initialising the UIRenderer was successful otherwise False.</returns>
	bool init(uint32_t width, uint32_t height, bool fullscreen, const RenderPass& renderpass, uint32_t subpass, CommandPool& commandPool,
	          Queue& queue, bool createDefaultLogo = true, bool createDefaultTitle = true, bool createDefaultFont = true, uint32_t maxNumInstances = 64, uint32_t maxNumSprites = 64);

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

		_sprites.clear();
		_fonts.clear();
		_textElements.clear();

		_device.reset();
	}

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text = "")
	{
		return createTextElement(text, _defaultFont);
	}

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.
	/// </param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::string& text, const Font& font);

	/// <summary>Create a Text Element sprite from a pvr::ui::Font. A default string will be used</summary>
	/// <param name="font">The font that the text element will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const Font& font)
	{
		return createTextElement(std::string(""), font);
	}

	/// <summary>Create Text sprite from wide std::wstring. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text) { return createTextElement(text, _defaultFont); }

	/// <summary>Create Text sprite from wide std::string.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	TextElement createTextElement(const std::wstring& text, const Font& font);

	/// <summary>Create a Text sprite from a TextElement.</summary>
	/// <param name="textElement">text element to initialise a Text framework object from.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const TextElement& textElement);

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text = "")
	{
		return createText(createTextElement(text));
	}

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::string& text, const Font& font)
	{
		return createText(createTextElement(text, font));
	}

	/// <summary>Create a Text sprite from a pvr::ui::Font. Uses a default text string</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const Font& font)
	{
		return createText(createTextElement(font));
	}

	/// <summary>Create Text sprite from wide std::string. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <returns>Text framework object, Null framework object if failed.</returns>
	Text createText(const std::wstring& text)
	{
		return createText(createTextElement(text));
	}

	/// <summary>Create Text sprite from wide std::string.</summary>
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
	/// <returns>Render width of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimX()const { return _screenDimensions.x; }

	/// <summary>Get the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <returns>Render height of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimY()const { return _screenDimensions.y; }

	/// <summary>Get the rendering dimensions of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Render width and height of the rectangle the UIRenderer is using for rendering.</returns>
	glm::vec2 getRenderingDim()const { return _screenDimensions; }

	/// <summary>Get the viewport of the rectangle the UIRenderer is rendering to. Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Viewport width and height of the rectangle the UIRenderer is using for rendering.</returns>
	Rect2Di getViewport()const { return Rect2Di(0, 0, static_cast<int32_t>(getRenderingDimX()), static_cast<uint32_t>(getRenderingDimY())); }

	/// <summary>Set the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	/// <param name="value">The new rendering width.</param>
	void setRenderingDimX(float value) { _screenDimensions.x = value; }

	/// <summary>Set the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <param name="value">The new rendering height.</param>
	void setRenderingDimY(float value) { _screenDimensions.y = value; }

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).
	/// </summary>
	/// <param name="image">An ImageView object of the font texture file, which will be used directly.</param>
	/// <param name="textureHeader">A pvr::TextureHeader of the same object. Necessary for the texture metadata.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload requires that you
	/// have already created a Texture2D object from the file and is suitable for sharing a texture between different
	/// UIRenderer objects.</remarks>
	Font createFont(const ImageView& image, const TextureHeader& textureHeader, const Sampler& sampler = Sampler());

	/// <summary>Create a pvr::ui::Image from a pvrvk::ImageView image.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="image">A TextureView object of the texture file. Must be 2D. It will be used
	/// directly.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(const ImageView& image, const Sampler& sampler = Sampler());

	/// <summary>Create a pvr::ui::Image from a Texture Atlas asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="image">A pvrvk::ImageView object. Will be internally used to create an vk::Texture2D to
	/// use.</param>
	/// <param name="uv">Texture UV coordinate</param>
	/// <param name="sampler">A sampler used for this Image (Optional)</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImageFromAtlas(const ImageView& image, const Rect2Df& uv, const Sampler& sampler = Sampler());

	/// <summary>Create a pvr::ui::MatrixGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	MatrixGroup createMatrixGroup();

	/// <summary>Create a pvr::ui::PixelGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	PixelGroup createPixelGroup();

	/// <summary>Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites
	/// after calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put
	/// into.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(SecondaryCommandBuffer& commandBuffer)
	{
		beginRendering(commandBuffer, Framebuffer(), true);
	}

	/// <summary>Begin rendering to a specific CommandBuffer, using a specific Framebuffer and optionally a renderpass.
	/// Must be called to render sprites. DO NOT update sprites after calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="framebuffer">A framebuffer object which will be used to begin the command buffer.</param>
	/// <param name="useRenderpass">Specifies whether a renderpass should be used to begin the command buffer.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(SecondaryCommandBuffer& commandBuffer, const Framebuffer& framebuffer, bool useRenderpass = false)
	{
		if (!commandBuffer->isRecording())
		{
			if (useRenderpass)
			{
				commandBuffer->begin(_renderpass, _subpass);
			}
			else
			{
				commandBuffer->begin(framebuffer, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else { _mustEndCommandBuffer = false; }
		commandBuffer->bindPipeline(getPipeline());// bind the uirenderer pipeline
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites after
	/// calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The CommandBuffer object where all the rendering commands will be put into.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(CommandBuffer& commandBuffer)
	{
		debug_assertion(commandBuffer->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
		                " it must be in the Recording state");
		_mustEndCommandBuffer = false;
		commandBuffer->bindPipeline(getPipeline());// bind the uirenderer pipeline
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.
	/// </summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(SecondaryCommandBuffer commandBuffer, GraphicsPipeline& pipe)
	{
		beginRendering(commandBuffer, pipe, Framebuffer(), true);
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.
	/// </summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <param name="framebuffer">A framebuffer object which will be used to begin the command buffer.</param>
	/// <param name="useRenderpass">Specifies whether a renderpass should be used to begin the command buffer.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(SecondaryCommandBuffer commandBuffer, GraphicsPipeline& pipe,
	                    const Framebuffer& framebuffer, bool useRenderpass = false)
	{
		if (!commandBuffer->isRecording())
		{
			if (useRenderpass)
			{
				commandBuffer->begin(_renderpass, _subpass);
			}
			else
			{
				commandBuffer->begin(framebuffer, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else { _mustEndCommandBuffer = false; }
		commandBuffer->bindPipeline(pipe);
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.
	/// </summary>
	/// <param name="commandBuffer">The CommandBuffer object where all the rendering commands will be put into.
	/// </param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(CommandBuffer commandBuffer, GraphicsPipeline& pipe)
	{
		debug_assertion(commandBuffer->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
		                " it must be in the Recording state");
		_mustEndCommandBuffer = false;
		commandBuffer->bindPipeline(pipe);
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>End rendering. Always call this method before submitting the commandBuffer passed to the UIRenderer.</summary>
	/// <remarks>This method must be called after you finish rendering sprites (after a call to beginRendering). The
	/// sequence must always be beginRendering, render ..., endRendering. Τry to group as many of the rendering
	/// commands (preferably all) between beginRendering and endRendering.</remarks>
	void endRendering()
	{
		if (_activeCommandBuffer.isValid())
		{
			if (_mustEndCommandBuffer)
			{
				_mustEndCommandBuffer = false;
				_activeCommandBuffer->end();
			}
			_activeCommandBuffer.reset();
		}
	}

	/// <summary>Get the CommandBuffer that is being used to currently render.</summary>
	/// <returns>If between a beginRendering and endRendering, the CommandBuffer used at beginRendering. Otherwise,
	/// null.</returns>
	CommandBufferBase& getActiveCommandBuffer() { return _activeCommandBuffer; }

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

	/// <summary>Return the PipelineLayout object of the internal Pipeline object used by this
	/// UIRenderer.</summary>
	/// <returns>The PipelineLayout.</returns>
	PipelineLayout getPipelineLayout() { return _pipelineLayout; }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	glm::mat4 getProjection()const
	{
		return pvr::math::ortho(Api::Vulkan, 0.0, getRenderingDimX(), 0.0f, getRenderingDimY());
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCCW()
	{
		_screenRotation += glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCW()
	{
		_screenRotation -= glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	glm::mat4 getScreenRotation()const
	{
		return glm::rotate(_screenRotation, glm::vec3(0.0f, 0.0f, 1.f));
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	const DescriptorSetLayout& getTexDescriptorSetLayout()const
	{
		return _texDescLayout;
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	const DescriptorSetLayout& getUboDescSetLayout()const
	{
		return _uboMvpDescLayout;
	}

	/// <summary>Returns maximum renderable sprites (Text and Images)</summary>
	/// <returns>The maximum number of renderable sprites</returns>
	uint32_t getMaxRenderableSprites()const { return _uboMaterial._numArrayId; }

	/// <summary>Return maximum number of instances supported (including sprites and groups)</summary>
	/// <returns>The maximum number of instances</returns>
	uint32_t getMaxInstances()const { return _uboMvp._numArrayId; }

	/// <summary>return the number of available renderable sprites (Image and Text)</summary>
	/// <returns>The number of remaining sprite slots</returns>
	uint32_t getNumAvailableSprites()const
	{
		return _uboMaterial.getNumAvailableBufferArrays();
	}

	/// <summary>return the number of availble instance</summary>
	/// <returns>The number of remaining instance slots</returns>
	uint32_t getNumAvailableInstances()const
	{
		return _uboMvp.getNumAvailableBufferArrays();
	}

	Buffer suballocateBuffer(VkDeviceSize size, VkBufferUsageFlags flags)
	{
		Buffer _buffer = utils::createBuffer(_device, size, flags, VkMemoryPropertyFlags(0));

		if (!_buffer.isValid())
		{
			Log("Failed to create UIRenderer Material buffer");
			return _buffer;
		}
		pvr::utils::SuballocatedMemory mem = _bufferAllocator->suballocate(size);
		if (!mem.isValid())
		{
			Log("Failed to suballocate memory for UIRenderer buffer");
		}
		_buffer->bindMemory(mem, mem->offset());

		return _buffer;
	}

private:

	void upDateResourceOwnsership()
	{
		std::for_each(_sprites.begin(), _sprites.end(),
		[this](SpriteWeakRef & sprite) { sprite->setUIRenderer(this); });

		std::for_each(_fonts.begin(), _fonts.end(),
		[this](FontWeakRef & font) { font->setUIRenderer(this); });

		std::for_each(_textElements.begin(), _textElements.end(),
		[this](TextElementWeakRef & textElement) { textElement->setUIRenderer(this); });
	}

	friend class pvr::ui::impl::Image_;
	friend class pvr::ui::impl::Text_;
	friend class pvr::ui::impl::Group_;
	friend class pvr::ui::impl::Sprite_;
	friend class pvr::ui::impl::Font_;

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	uint64_t generateGroupId() { return _groupId++; }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const vk::DescriptorSetLayout&</returns>
	DescriptorPool& getDescriptorPool() { return _descPool; }
	Sampler getSamplerBilinear()const { return _samplerBilinear; }

	Sampler getSamplerTrilinear()const { return _samplerTrilinear; }

	struct UboMvp
	{
		friend class ::pvr::ui::UIRenderer;
		UboMvp(): _freeArrayId(0)
		{
		}
		bool init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer);
		void initlayout(Device& device, uint32_t numElements);

		void reset()
		{
			_buffer.reset();
			_uboDescSetSet.reset();
		}

		void updateMvp(uint32_t bufferArrayId, const glm::mat4x4& mvp);

		int32_t getNewBufferSlice()
		{
			if (_freeArrayIds.size())
			{
				const uint32_t id = _freeArrayIds.back();
				_freeArrayIds.pop_back();
				return id;
			}
			return (_freeArrayId < _numArrayId ? _freeArrayId++ : -1);
		}

		void releaseBufferSlice(uint32_t id)
		{
			debug_assertion(id < _numArrayId, "Invalid id");
			_freeArrayIds.push_back(id);
		}

		void bindUboDynamic(CommandBufferBase& cb, const PipelineLayout& pipelayout,
		                    uint32_t mvpBufferSlice)
		{
			uint32_t dynamicOffsets[] =
			{
				static_cast<uint32_t>(_structuredBufferView.getDynamicSliceOffset(mvpBufferSlice))
			};
			cb->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, pipelayout, 1, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32_t getNumAvailableBufferArrays()const
		{
			return static_cast<uint32_t>((_numArrayId - _freeArrayId) + _freeArrayIds.size());
		}

	private:

		uint32_t _freeArrayId;
		utils::StructuredBufferView _structuredBufferView;
		pvrvk::Buffer _buffer;
		size_t _memoffset_cached;
		DeviceMemory _memory_cached;

		DescriptorSet _uboDescSetSet;
		uint32_t _numArrayId;
		std::vector<uint32_t> _freeArrayIds;
	};

	struct UboMaterial
	{
	public:
		friend class ::pvr::ui::UIRenderer;
		UboMaterial() :  _freeArrayId(0)
		{}

		void reset()
		{
			_buffer.reset();
			_uboDescSetSet.reset();
		}

		bool init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer);
		void initlayout(Device& device, uint32_t numArrayId);

		void updateMaterial(uint32_t arrayIndex, const glm::vec4& color, int32_t alphaMode, const glm::mat4& uv);

		int32_t getNewBufferArray()
		{
			if (_freeArrayIds.size())
			{
				const uint32_t id = _freeArrayIds.back();
				_freeArrayIds.pop_back();
				return id;
			}
			return (_freeArrayId < _numArrayId ? _freeArrayId++ : -1);
		}

		void releaseBufferArray(uint32_t id)
		{
			debug_assertion(id < _numArrayId, "Invalid id");
			_freeArrayIds.push_back(id);
		}

		void bindUboDynamic(CommandBufferBase& cb, const PipelineLayout& pipelayout, uint32_t bufferSlice)
		{
			uint32_t dynamicOffsets[] =
			{
				static_cast<uint32_t>(_structuredBufferView.getDynamicSliceOffset(bufferSlice))
			};
			cb->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, pipelayout, 2, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32_t getNumAvailableBufferArrays()const
		{
			return static_cast<uint32_t>(((_numArrayId - _freeArrayId) + _freeArrayIds.size()));
		}

	private:
		DescriptorSet _uboDescSetSet;
		uint32_t _freeArrayId;
		uint32_t _numArrayId;
		utils::StructuredBufferView _structuredBufferView;
		pvrvk::Buffer _buffer;
		size_t _memoffset_cached;
		DeviceMemory _memory_cached;
		std::vector<uint32_t> _freeArrayIds;

	};

	UboMvp& getUbo() { return _uboMvp; }

	UboMaterial& getMaterial() { return _uboMaterial; }

	void setUpUboPoolLayouts(uint32_t numInstances, uint32_t numSprites);
	bool setUpUboPools(uint32_t numInstances, uint32_t numSprites);

	utils::ImageUploadResults init_CreateDefaultFont(CommandBuffer& cmdBuffer);
	utils::ImageUploadResults init_CreateDefaultSdkLogo(CommandBuffer& cmdBuffer);
	bool init_CreateDefaultSampler();
	bool init_CreateDefaultTitle();
	Result init_CreatePipelineAndRenderPass();
	Result init_CreateDescriptorSetLayout();


	pvr::utils::MemorySuballocator _bufferAllocator;

	std::vector<SpriteWeakRef> _sprites;
	std::vector<TextElementWeakRef> _textElements;
	std::vector<FontWeakRef> _fonts;

	RenderPass _renderpass;
	uint32_t _subpass;
	ProgramData _programData;
	Font _defaultFont;
	Image _sdkLogo;
	Text _defaultTitle;
	Text _defaultDescription;
	Text _defaultControls;
	DeviceWeakPtr _device;

	PipelineLayout _pipelineLayout;
	GraphicsPipeline _pipeline;
	DescriptorSetLayout _texDescLayout;
	DescriptorSetLayout _uboMvpDescLayout;
	DescriptorSetLayout _uboMaterialLayout;
	Sampler _samplerBilinear;
	Sampler _samplerTrilinear;
	DescriptorPool _descPool;
	CommandBufferBase _activeCommandBuffer;
	bool _mustEndCommandBuffer;
	Buffer _fontIbo;
	Buffer _imageVbo;
	glm::vec2 _screenDimensions;
	float _screenRotation;
	uint64_t _groupId = 1;
	UboMvp _uboMvp;
	UboMaterial _uboMaterial;
	uint32_t _numSprites;
};
}
}
