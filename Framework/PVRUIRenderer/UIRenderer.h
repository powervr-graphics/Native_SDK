/*!*********************************************************************************************************************
\file         PVRUIRenderer\UIRenderer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains implementations of functions for the classes in UIRenderer.h
***********************************************************************************************************************/
#pragma once
#include "PVRUIRenderer/Sprite.h"


namespace pvr {
namespace ui {
/*!****************************************************************************************************************
\brief Manages and render the sprites.
*******************************************************************************************************************/
class UIRenderer
{
public:
	/*!****************************************************************************************************************
	\brief Information used for uploading required info to the shaders (matrices, attributes etc).
	*******************************************************************************************************************/
	struct ProgramData
	{
		enum Uniform { UniformMVPmtx, UniformFontTexture, UniformColor, UniformAlphaMode, UniformUVmtx, NumUniform };
		enum Attribute { AttributeVertex, AttributeUV, NumAttribute };
		int32 uniforms[NumUniform];
		int32 attributes[NumAttribute];
	};

	const api::Buffer& getFontIbo()
	{
		if (!m_fontIbo.isValid())
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

			m_fontIbo = getContext()->createBuffer(sizeof(fontFaces[0]) * impl::Font_::FontElement, types::BufferBindingUse::IndexBuffer, true);
			m_fontIbo->update(&fontFaces[0], 0, (uint32)(sizeof(fontFaces[0]) * fontFaces.size()));
		}
		return m_fontIbo;
	}

	const api::Buffer& getImageVbo()
	{
		if (m_imageVbo.isNull())
		{
			// create the image vbo
			const float32 verts[] =
			{
				/*		Position	*/
				-1.f, 1.f, 0.f, 1.0f, 0.0f, 1.0f, // upper left
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, -1.f, 0.f, 1.0f, 1.f, 0.0f, // lower right
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
			};
			m_imageVbo = getContext()->createBuffer(sizeof(verts), types::BufferBindingUse::VertexBuffer, true);
			m_imageVbo->update((void*)verts, 0, sizeof(verts));
		}
		return m_imageVbo;
	}
	/*!************************************************************************************************************
	\brief Constructor. Does not produce a ready-to-use object, use the init function before use.
	***************************************************************************************************************/
	UIRenderer() : screenRotation(.0f) {}

	~UIRenderer() { release(); }

	/*!************************************************************************************************************
	\brief Return the graphics context the UIRenderer was initialized with. If the UIrenderer was not initialized,
	       behaviour is undefined.
	***************************************************************************************************************/
	GraphicsContext& getContext() { return m_context; }
	/*!************************************************************************************************************
	\brief Return the graphics context the UIRenderer was initialized with. If the UIrenderer was not initialized,
	       behaviour is undefined.
	***************************************************************************************************************/
	const GraphicsContext& getContext()const { return m_context; }

	/*!************************************************************************************************************
	\brief Returns the ProgramData used by this UIRenderer.
	***************************************************************************************************************/
	const ProgramData& getProgramData() { return m_programData; }

	/*!************************************************************************************************************
	\brief Returns the pvr::api::GraphicsPipeline object used by this UIRenderer.
	***************************************************************************************************************/
	api::ParentableGraphicsPipeline getPipeline() { return m_pipeline; }

	/*!************************************************************************************************************
	\brief Check that we have called beginRendering() and not called endRendering. See the beginRendering() method.
	***************************************************************************************************************/
	bool isRendering() { return m_activeCommandBuffer.isValid(); }

	/*!************************************************************************************************************
	\brief Initialize the UIRenderer with a graphics context. MUST BE called exactly once before use, after a valid
	       graphics context is available (usually, during initView).
	***************************************************************************************************************/
	Result init(const api::RenderPass& renderpass, uint32 subpass)
	{
		release();
		m_mustEndCommandBuffer = false;
		m_context = renderpass->getContext();
		m_screenDimensions = glm::vec2(m_context->getDisplayAttributes().width, m_context->getDisplayAttributes().height);
		m_renderpass = renderpass;
		m_subpass = subpass;
		// screen rotated?
		if (m_screenDimensions.y > m_screenDimensions.x && m_context->getDisplayAttributes().fullscreen)
		{
			rotateScreen90degreeCCW();
		}

		Result res;
		if (((res = init_CreateDescriptorSetLayout()) == Result::Success) &&
		    ((res = init_CreatePipelineAndRenderPass()) == Result::Success))
		{
			if (!init_CreateDefaultSampler()) { res = Result::UnknownError; }
			if (!init_CreateDefaultSdkLogo()) { res = Result::UnknownError; }
			if (!init_CreateDefaultFont()) { res = Result::UnknownError; }
			if (!init_CreateDefaultTitle()) { res = Result::UnknownError; }
		}
		return res;
	}

	/*!************************************************************************************************************
	\brief Release the engine and its resources. Must be called once after we are done with the UIRenderer.
	       (usually, during releaseView).
	***************************************************************************************************************/
	void release()
	{
		this->m_renderpass.reset();
		this->m_defaultFont.reset();
		this->m_defaultTitle.reset();
		this->m_defaultDescription.reset();
		this->m_defaultControls.reset();
		this->m_sdkLogo.reset();
		this->m_renderpass.reset();

		this->m_pipelineLayout.reset();
		this->m_pipeline.reset();
		this->m_texDescLayout.reset();
		this->m_uboDescLayout.reset();
		this->m_samplerBilinear.reset();
		this->m_samplerTrilinear.reset();
		this->m_activeCommandBuffer.reset();
		this->m_fontIbo.reset();
		this->m_imageVbo.reset();
		this->m_descPool.reset();

		this->m_context.reset();
	}

	/*!****************************************************************************************************************
	\brief Create a Text sprite. Initialize with string. Uses default font.
	\param text std::string object that this Text object will be initialized with
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText(const std::string& text)
	{
		return createText(text, m_defaultFont);
	}

	/*!****************************************************************************************************************
	\brief Create a Text sprite. Initially empty (use setText). Uses default font.
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText()
	{
		return createText(m_defaultFont);
	}

	/*!****************************************************************************************************************
	\brief Create a Text sprite. Initially empty (use setText). Uses default font.
	\param font The font that the text will be using. The font must belong to the same UIrenderer object.
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText(const Font& font);

	/*!****************************************************************************************************************
	\brief Create Text sprite from string.
	\param text String object that this Text object will be initialized with
	\param font The font that the text will be using. The font must belong to the same UIrenderer object.
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText(const std::string& text, const Font& font);

	/*!****************************************************************************************************************
	\brief Create Text sprite from wide string. Uses the Default Font.
	\param text Wide string that this Text object will be initialized with. Will use the Default Font.
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText(const std::wstring& text)
	{
		return createText(text, m_defaultFont);
	}
	/*!****************************************************************************************************************
	\brief Create Text sprite from wide string.
	\param text text to be rendered.
	\param font The font that the text will be using. The font must belong to the same UIrenderer object.
	\return Text framework object, Null framework object if failed.
	*******************************************************************************************************************/
	Text createText(const std::wstring& text, const Font& font);

	/*!****************************************************************************************************************
	\brief Get the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements. Initial
	value is the Screen Width.
	*******************************************************************************************************************/
	float32 getRenderingDimX()const { return m_screenDimensions.x; }

	glm::vec2 getRenderingDim()const { return m_screenDimensions; }


	/*!****************************************************************************************************************
	\brief Set the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements. Initial
	value is the Screen Width.
	*******************************************************************************************************************/
	void setRenderingDimX(float32 value) { m_screenDimensions.x = value; }

	/*!****************************************************************************************************************
	\brief Get the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements. Initial
	value is the Screen Height.
	*******************************************************************************************************************/
	float32 getRenderingDimY()const { return m_screenDimensions.y; }

	Rectanglei getViewport()const { return Rectanglei(0, 0, (int32)getRenderingDimX(), (int32)getRenderingDimY()); }

	/*!****************************************************************************************************************
	\brief Set the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements. Initial
	value is the Screen Height.
	*******************************************************************************************************************/
	void setRenderingDimY(uint32 value) { m_screenDimensions.y = (pvr::float32)value; }

	/*!****************************************************************************************************************
	\brief Create a font from a given texture (use PVRTexTool to create the font texture from a font file).
	\param apiTex A pvr::api::TextureView object of the font texture file, which will be used directly.
	\param tex A pvr::assets::TextureHeader of the same object. Necessary for the texture metadata.
	\return A new Font object. Null object if failed.
	\description Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	             function overloads to create a pvr::ui::Font object to render your sprites. This overload requires that
				 you have already created a Texture2D object from the file and is suitable for sharing a texture between
				 different UIRenderer objects.
	*******************************************************************************************************************/
	Font createFont(api::TextureView& apiTex, const assets::TextureHeader& tex);

	/*!****************************************************************************************************************
	\brief Create a font from a given texture (use PVRTexTool to create the font texture from a font file).
	\param tex A pvr::assets::Texture object. A new pvr::api::Texture2D object will be created and used internally.
	\return A new Font object. Null object if failed.
	\description Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	             function overloads to create a pvr::ui::Font object to render your sprites. This overload directly uses
				 a pvr::assets::Texture for both data and metadata, but will create a new Texture2D so if using multiple
				 UIRenderes using the same font, might not be the most efficient version.
	*******************************************************************************************************************/
	Font createFont(const assets::Texture& tex);

	/*!****************************************************************************************************************
	\brief Create a pvr::ui::Image from an API texture.
	\param apiTex A pvr::api::TextureView object of the texture file. Must be 2D. It will be used directly.
	\param height The height of the texture.
	\param width  The width of the texture.
	\return A new Image object of the specified texture. Null object if failed.
	*******************************************************************************************************************/
	Image createImage(api::TextureView& apiTex, int32 width, int32 height);

	/*!****************************************************************************************************************
	\brief Create a pvr::ui::Image from a Texture asset.
	\param texture A pvr::assets::Texture object. Will be internally used to create an api::Texture2D to use.
	\return A new Image object of the specified texture. Null object if failed.
	*******************************************************************************************************************/
	Image createImage(const assets::Texture& texture);

	Image createImageFromAtlas(api::TextureView& tex, const Rectanglef& uv, uint32 width, uint32 height);

	/*!****************************************************************************************************************
	\brief Create a pvr::ui::MatrixGroup.
	\return A new Group to display different sprites together. Null object if failed.
	*******************************************************************************************************************/
	MatrixGroup createMatrixGroup();

	/*!****************************************************************************************************************
	\brief Create a pvr::ui::PixelGroup.
	\return A new Group to display different sprites together. Null object if failed.
	*******************************************************************************************************************/
	PixelGroup createPixelGroup();

	/*!****************************************************************************************************************
	\brief Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites after
	       calling this function before calling endRendering.
	\param cb The pvr::api::SecondaryCommandBuffer object where all the rendering commands will be put into.
	\description THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	             UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group
				 as many rendering commands as possible between begin and end, to avoid needless state changes.
	*******************************************************************************************************************/
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
				cb->beginRecording(m_renderpass, m_subpass);
			}
			else
			{
				cb->beginRecording(fbo, m_subpass);
			}
			m_mustEndCommandBuffer = true;
		}
		else { m_mustEndCommandBuffer = false; }
		cb->pushPipeline();// store the currently bound pipeline
		cb->bindPipeline(getPipeline());// bind the uirenderer pipeline
		m_activeCommandBuffer = cb;
	}

	void beginRendering(api::CommandBuffer& cb)
	{
		debug_assertion(cb->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer, it must be in the Recording state");
		m_mustEndCommandBuffer = false;
		cb->pushPipeline();// store the currently bound pipeline
		cb->bindPipeline(getPipeline());// bind the uirenderer pipeline
		m_activeCommandBuffer = cb;
	}


	/*!****************************************************************************************************************
	\brief Begin rendering to a specific CommandBuffer, with a custom user-provided pvr::api::GraphicsPipeline.
	\param cb The pvr::api::CommandBuffer object where all the rendering commands will be put into.
	\param pipe The pvr::api::GraphicsPipeline to use for rendering.
	\description THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group
	as many rendering commands as possible between beginRendering and endRendering, to avoid needless
	state changes. Use this overload to render with a custom GraphicsPipeline.
	*******************************************************************************************************************/
	void beginRendering(api::SecondaryCommandBuffer cb, pvr::api::GraphicsPipeline& pipe)
	{
		beginRendering(cb, pipe, pvr::api::Fbo(), true);
	}

	void beginRendering(api::SecondaryCommandBuffer cb, pvr::api::GraphicsPipeline& pipe, const pvr::api::Fbo& fbo, bool useRenderpass = false)
	{
		if (!cb->isRecording())
		{
			if (useRenderpass)
			{
				cb->beginRecording(m_renderpass, m_subpass);
			}
			else
			{
				cb->beginRecording(fbo, m_subpass);
			}
			m_mustEndCommandBuffer = true;
		}
		else { m_mustEndCommandBuffer = false; }
		cb->pushPipeline();
		cb->bindPipeline(pipe);
		m_activeCommandBuffer = cb;
	}

	/*!****************************************************************************************************************
	\brief Begin rendering to a specific CommandBuffer, with a custom user-provided pvr::api::GraphicsPipeline.
	\param cb The pvr::api::CommandBuffer object where all the rendering commands will be put into.
	\param pipe The pvr::api::GraphicsPipeline to use for rendering.
	\description THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group
	as many rendering commands as possible between beginRendering and endRendering, to avoid needless
	state changes. Use this overload to render with a custom GraphicsPipeline.
	*******************************************************************************************************************/
	void beginRendering(api::CommandBuffer cb, pvr::api::GraphicsPipeline& pipe)
	{
		debug_assertion(cb->isRecording(), "UIRenderer: If a Primary command buffer is passed to the UIRenderer, it must be in the Recording state");
		m_mustEndCommandBuffer = false;
		cb->pushPipeline();
		cb->bindPipeline(pipe);
		m_activeCommandBuffer = cb;
	}

	/*!****************************************************************************************************************
	\brief End rendering. Always call this method before submitting the commandBuffer passed to the UIRenderer.
	\description This method must be called after you finish rendering sprites (after a call to beginRendering). The
	             sequence must always be beginRendering, render ..., endRendering. Τry to group as many of the rendering
				 commands (preferably all) between beginRendering and endRendering.
	*******************************************************************************************************************/
	void endRendering()
	{
		if (m_activeCommandBuffer.isValid())
		{
			m_activeCommandBuffer->popPipeline();
			if (m_mustEndCommandBuffer)
			{
				m_mustEndCommandBuffer = false;
				m_activeCommandBuffer->endRecording();
			}
			m_activeCommandBuffer.reset();
		}
	}

	/*!****************************************************************************************************************
	\brief	Get the pvr::api::CommandBuffer that is being used to currently render.
	\return	If between a beginRendering and endRendering, the CommandBuffer used at beginRendering. Otherwise, null.
	*******************************************************************************************************************/
	api::CommandBufferBase& getActiveCommandBuffer() { return m_activeCommandBuffer; }

	/*!****************************************************************************************************************
	\return	The version of the UIRenderer.
	*******************************************************************************************************************/
	static uint32 getEngineVersion() { return 1; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	       initialized. Used throughout the PowerVR SDK Examples.
	\return The default font. Constant overload.
	*******************************************************************************************************************/
	const Font& getDefaultFont() const { return m_defaultFont; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	initialized. Used throughout the PowerVR SDK Examples.
	\return A pvr::ui::Font object of the default font.
	*******************************************************************************************************************/
	Font& getDefaultFont() { return m_defaultFont; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	       UIRenderer is initialized. Used throughout the PowerVR SDK Examples.
	\return The PowerVR SDK pvr::ui::Image. Constant overload.
	*******************************************************************************************************************/
	const Image& getSdkLogo() const { return m_sdkLogo; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	UIRenderer is initialized. Used throughout the PowerVR SDK Examples.
	\return The PowerVR SDK pvr::ui::Image.
	*******************************************************************************************************************/
	Image& getSdkLogo() { return m_sdkLogo; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left, large) for
	       convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at will.
	       Used throughout the PowerVR SDK Examples.
	\return The Default title pvr::ui::Text. Originally empty (use setText on it).  Constant overload.
	*******************************************************************************************************************/
	const Text& getDefaultTitle() const { return m_defaultTitle; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left, large) for
	       convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at will.
	       Used throughout the PowerVR SDK Examples.
	\return The Default title pvr::ui::Text. Originally empty (use setText on it).
	*******************************************************************************************************************/
	Text& getDefaultTitle() { return m_defaultTitle; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a description (subtitle)
	(top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	be resized and repositioned at will. Used throughout the PowerVR SDK Examples.
	\return The Default descritption pvr::ui::Text. Originally empty (use setText on it). Constant overload.
	*******************************************************************************************************************/
	const Text& getDefaultDescription() const { return m_defaultDescription; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a description (subtitle
	       (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
		   be resized and repositioned at will. Used throughout the PowerVR SDK Examples.
	\return The Default descritption pvr::ui::Text. Originally empty (use setText on it).
	*******************************************************************************************************************/
	Text& getDefaultDescription() { return m_defaultDescription; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	(bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can
	be resized and repositioned at will. Used throughout the PowerVR SDK Examples.
	\return The Default Controls pvr::ui::Text. Originally empty (use setText on it). Constant overload.
	*******************************************************************************************************************/
	const Text& getDefaultControls() const { return m_defaultControls; }

	/*!****************************************************************************************************************
	\brief The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	(bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can
	be resized and repositioned at will. Used throughout the PowerVR SDK Examples.
	\return The Default Controls pvr::ui::Text. Originally empty (use setText on it).
	*******************************************************************************************************************/
	Text& getDefaultControls() { return m_defaultControls; }

	/*!****************************************************************************************************************
	\brief Return the pvr::api::PipelineLayout object of the internal pvr::api::Pipeline object used by this UIRenderer.
	\return The pvr::api::PipelineLayout.
	*******************************************************************************************************************/
	pvr::api::PipelineLayout getPipelineLayout() { return m_pipelineLayout; }

	glm::mat4 getProjection()
	{
		return pvr::math::ortho(m_context->getApiType(), 0.0, (pvr::float32)getRenderingDimX(),
		                        0.0f, (pvr::float32)getRenderingDimY());
	}

	void rotateScreen90degreeCCW()
	{
		screenRotation += glm::pi<pvr::float32>() * .5f;
		std::swap(m_screenDimensions.x, m_screenDimensions.y);
	}

	void rotateScreen90degreeCW()
	{
		screenRotation -= glm::pi<pvr::float32>() * .5f;
		std::swap(m_screenDimensions.x, m_screenDimensions.y);
	}

	glm::mat4 getScreenRotation()
	{
		return glm::rotate(screenRotation, glm::vec3(0.0f, 0.0f, 1.f));
	}

	/*!********************************************************************************************
	\brief	return the default DescriptorSetLayout. ONLY to be used by the Sprites
	\return	const api::DescriptorSetLayout&
	***********************************************************************************************/
	const api::DescriptorSetLayout& getTexDescriptorSetLayout()const
	{
		return m_texDescLayout;
	}

	const api::DescriptorSetLayout& getUboDescSetLayout()const
	{
		return m_uboDescLayout;
	}

	api::DescriptorPool& getDescriptorPool() { return m_descPool; }

	api::RenderPass m_renderpass;
	uint32 m_subpass;
private:

	bool init_CreateDefaultFont();
	bool init_CreateDefaultSampler();
	bool init_CreateDefaultSdkLogo();
	bool init_CreateDefaultTitle();
	Result init_CreatePipelineAndRenderPass();
	Result init_CreateDescriptorSetLayout();
	pvr::uint64 generateGroupId();

	ProgramData m_programData;
	Font m_defaultFont;
	Image m_sdkLogo;
	Text m_defaultTitle;
	Text m_defaultDescription;
	Text m_defaultControls;

	GraphicsContext m_context;

	pvr::api::PipelineLayout m_pipelineLayout;
	api::ParentableGraphicsPipeline m_pipeline;
	api::DescriptorSetLayout m_texDescLayout;
	api::DescriptorSetLayout m_uboDescLayout;
	api::Sampler m_samplerBilinear, m_samplerTrilinear;
	api::DescriptorPool m_descPool;
	api::CommandBufferBase m_activeCommandBuffer;
	bool m_mustEndCommandBuffer;
	api::Buffer m_fontIbo;
	api::Buffer m_imageVbo;
	glm::vec2 m_screenDimensions;
	pvr::float32 screenRotation;
};

}
}
