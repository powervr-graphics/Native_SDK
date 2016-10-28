/*!*********************************************************************************************************************
\file         PVRUIRenderer\Sprite.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains the Sprite classes and framework objects used by the UIRenderer (Sprite, Text, Image, Font, Group).
***********************************************************************************************************************/
#pragma once
#include "PVRApi/PVRApi.h"
#include "PVRCore/AxisAlignedBox.h"
#include "PVRCore/UnicodeConverter.h"
#include "PVRAssets/Texture/Texture.h"
#define NUM_BITS_GROUP_ID 8

/*!****************************************************************************************************************
\brief Main PowerVR Namespace
*******************************************************************************************************************/
namespace pvr {
//!\cond NO_DOXYGEN
class IGraphicsContext;
namespace assets { class Texture; }
typedef Rectangle<int32> Rectanglei;
//!\endcond
/*!****************************************************************************************************************
\brief Main namespace for the PVRUIRenderer Library. Contains the UIRenderer and the several Sprite classes.
*******************************************************************************************************************/
namespace ui {
//!\cond NO_DOXYGEN
class UIRenderer;
namespace impl {
class Font_;
class Text_;
class Image_;
class Group_;
class MatrixGroup_;
class PixelGroup_;
class Sprite_;
}
//!\endcond

/*!****************************************************************************************************************
\brief A Reference Counted Framework Object wrapping the Group_ class. Groups several sprites to apply some
       transformation to them and render them all together.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Group_> Group;

/*!****************************************************************************************************************
\class ::pvr::ui::MatrixGroup
\brief A Reference Counted Framework Object wrapping the Group_ class. Groups several sprites to apply some
matrix transformation to them and render them all together.
*******************************************************************************************************************/
typedef RefCountedResource<impl::MatrixGroup_> MatrixGroup;

/*!****************************************************************************************************************
\class ::pvr::ui::PixelGroup
\brief A Reference Counted Framework Object wrapping the PixelGroup class. Groups several sprites to apply intuitive
2D operations and layouts to them.
*******************************************************************************************************************/
typedef RefCountedResource<impl::PixelGroup_> PixelGroup;

/*!****************************************************************************************************************
\class ::pvr::ui::Sprite
\brief A Reference Counted Framework Object wrapping the Sprite_ interface. Represents anything you can use with
       the UIRenderer (Font, Text, Image, Group).
*******************************************************************************************************************/
typedef RefCountedResource<impl::Sprite_> Sprite;

/*!****************************************************************************************************************
\class ::pvr::ui::Font
\brief A Reference Counted Framework Object wrapping the Font_ class. Is an Image object augmented by font
       metadata. Is used by the Text class.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Font_> Font;

/*!****************************************************************************************************************
\class ::pvr::ui::Text
\brief A Reference Counted Framework Object wrapping the Text_ class. The Text is a Sprite and contains a
       string of characters to be displayed with the Font that it uses.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Text_> Text;

/*!****************************************************************************************************************
\class ::pvr::ui::Image
\brief A Reference Counted Framework Object wrapping the Image_ class. The Image is a Sprite and contains a 2D
       texture that can be displayed.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Image_> Image;


/*!****************************************************************************************************************
\brief An Enumeration of all the Anchor points that can be used to position a Sprite. An anchor point is the point
       to which all positioning will be relative to. Use this to facilitate the laying out of UIs.
*******************************************************************************************************************/
enum class Anchor {
	TopLeft, TopCenter, TopRight,
	CenterLeft, Center, CenterRight,
	BottomLeft, BottomCenter, BottomRight
};


/*!****************************************************************************************************************
\brief Contains the implementation of the raw UIRenderer classes. In order to use the library, use the PowerVR
	   Framework objects found in the pvr::ui namespace
*******************************************************************************************************************/
namespace impl {
/*!****************************************************************************************************************
\brief Base sprite class. Use through the Sprite framework object. Represents something that can be rendered with
       the UIRenderer. Texts, Images, Groups are all sprites (Fonts too although it would only be used as a sprite
	   to  display the entire font's glyphs).
*******************************************************************************************************************/
class Sprite_
{
public:
	/********************************************************************************************************
	\brief		Call this function after changing the sprite in any way, in order to update its internal
	            information. This function should be called before any rendering commands are submitted and
				before calling functions such as getDimensions, in order to actually process all the changes
				to the sprite.
	********************************************************************************************************/
	virtual void commitUpdates() const;

	virtual ~Sprite_() {}

	/*!****************************************************************************************************************
	\brief      Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after calling
				the commitUpdates function
	\return		The Sprite's bounding box.
	*******************************************************************************************************************/
	glm::vec2 getDimensions()const
	{
		return glm::vec2(m_boundingRect.getSize());
	}

	/*!****************************************************************************************************************
	\brief  RenderImmediate functions forgo the normal cycle of uiRenderer::beginRendering - Sprite::render -
	        uiRenderer::endRendering for convenience. Do not use if rendering multiple sprites to avoid needless state
			changes.
	\param  commandBuffer The pvr::api::CommandBuffer to render into.
	*******************************************************************************************************************/
	void renderImmediate(api::CommandBuffer& commandBuffer) const;


	/*!****************************************************************************************************************
	\brief  Render is the normal function to call to render a sprite. Before calling this function, call beginRendering
	        on the uiRenderer this sprite belongs to to set up the commandBuffer to render to. In general try to group as
			many render commands as possible between the beginRendering and endRendering. This overload does not apply
			any transformations to the sprite.
	*******************************************************************************************************************/
	void render() const;

	/*!****************************************************************************************************************
	\brief	Use this to use this sprite as Alpha channel only, setting its color to 1,1,1,a. Otherwise, an Alpha texture
	would render black. Always use this setting to render Fonts that have been generated
	with PVRTexTool as Alpha textures.
	\param	isAlphaOnly Pass "true" to flush all color channels to 1.0 and keep the alpha channel. Pass false (default
	state) to render with texture colors unchanged.
	*******************************************************************************************************************/
	void setAlphaRenderingMode(bool isAlphaOnly) const
	{
		m_alphaMode = isAlphaOnly;
	}

	/*!****************************************************************************************************************
	\brief	Set a modulation (multiplicative) color to the sprite, as a vector of normalised 32 bit float values.
	Range of values must be 0..1
	\param	color A glm::vec4 that contains color and alpha values in the range of 0..1. Initial value (1.0,1.0,1.0,1.0)
	*******************************************************************************************************************/
	void setColor(glm::vec4 color) const
	{
		m_color = color;
	}

	/*!****************************************************************************************************************
	\brief	Set a modulation (multiplicative) color to the sprite, as bytes (0..255).
	\param	r Red channel. Initial value 255.
	\param	g Green channel. Initial value 255.
	\param	b Blue channel. Initial value 255.
	\param	a Alpha channel. Initial value 255.
	*******************************************************************************************************************/
	void setColor(uint32 r, uint32 g, uint32 b, uint32 a) const
	{
		m_color[0] = static_cast<float>(r / 255.f);
		m_color[1] = static_cast<float>(g / 255.f);
		m_color[2] = static_cast<float>(b / 255.f);
		m_color[3] = static_cast<float>(a / 255.f);
	}

	/*!****************************************************************************************************************
	\brief	Set a modulation (multiplicative) color to the sprite, as normalised floating point values. Values must be
	        in the range of 0..1
	\param	r Red channel. Initial value 1.
	\param	g Green channel. Initial value 1.
	\param	b Blue channel. Initial value 1.
	\param	a Alpha channel. Initial value 1.
	*******************************************************************************************************************/
	void setColor(float r, float g, float b, float a) const
	{
		m_color.r = r;
		m_color.g = g;
		m_color.b = b;
		m_color.a = a;
	}

	/*!****************************************************************************************************************
	\brief	Set a modulation (multiplicative) color to the sprite, as bytes packed into an integer.
	\param	rgba 8 bit groups, least significant bits: Red, then Green, then Blue, then most significant bits is Alpha.
	*******************************************************************************************************************/
	void setColor(uint32 rgba) const
	{
		m_color[0] = static_cast<float>(rgba & 0xFF) / 255.f;
		m_color[1] = static_cast<float>(rgba >> 8 & 0xFF) / 255.f;
		m_color[2] = static_cast<float>(rgba >> 16 & 0xFF) / 255.f;
		m_color[3] = static_cast<float>(rgba >> 24 & 0xFF) / 255.f;
	}

	/*!****************************************************************************************************************
	\brief	Get the modulation (multiplicative) color of the sprite, as a glm::vec4.
	\return The sprites's modulation color. Values are normalised in the range of 0..1
	*******************************************************************************************************************/
	const glm::vec4& getColor()const { return m_color; }

	/*!****************************************************************************************************************
	\brief	This setting queries if this is set to render as Alpha channel only, (setting its color to 1,1,1,a).
	        Otherwise, an Alpha texture would render black. This setting is typically used to render Text with Fonts
			that have been generated with PVRTexTool as Alpha textures.
	\return	"true" if set to render as Alpha, false otherwise.
	*******************************************************************************************************************/
	bool getAlphaRenderingMode()const { return m_alphaMode == 1; }

	/*!****************************************************************************************************************
	\brief	Get the sprite's own transformation matrix. Does not contain hierarchical transformations from groups etc.
			This function is valid only after any changes to the sprite have been commited with commitUpdates as it is
			normally calculated in commitUpdates.
	\return	The sprite's final transformation matrix. If the sprite is rendered on its own, this is the matrix that will
	        be uploaded to the shader.
	*******************************************************************************************************************/
	const glm::mat4& getMatrix() const { return m_cachedMatrix; }


	/*!****************************************************************************************************************
	\internal
	\brief		Do not call directly. CommitUpdates will call this function.
				If writing new sprite classes, implement this function to calculate the mvp matrix of the sprite from
				any other possible representation that the sprite's data contains into its own m_cachedMatrix member.
	*******************************************************************************************************************/
	virtual void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                          pvr::Rectanglei const& viewport)const = 0;

	/********************************************************************************************************
	\internal
	\brief		Do not call directly. Render will call this function.
				If writing new sprites, implement this function to render a sprite with the provided
				transformation matrix. Necessary to support instanced rendering of the same sprite from
				different groups.
	********************************************************************************************************/
	virtual void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const = 0;

	math::AxisAlignedBox const& getBoundingBox()const { return m_boundingRect; }

	virtual glm::vec2 getScaledDimension()const = 0;

protected:
	friend class ::pvr::ui::UIRenderer;

	Sprite_(UIRenderer& uiRenderer);
	mutable math::AxisAlignedBox m_boundingRect; //< Bounding rectangle of the sprite
	mutable glm::vec4 m_color; //< Modulation color (multiplicative)
	mutable int32 m_alphaMode; //< Set the shader to render alpha-only
	UIRenderer& m_uiRenderer; //< UIRenderer this sprite belongs to
	mutable glm::mat4 m_cachedMatrix; //< Bounding rectangle of the sprite
	glm::mat4 m_viewProj;
};

/*!****************************************************************************************************************
\brief A component that can be positioned in 2D using 2d position, scale, rotation and anchored using its center or
       corners.
*******************************************************************************************************************/
class I2dComponent
{
protected:
	mutable Anchor m_anchor; //!< The position in the sprite relative to which all positioning calculations are done
	mutable glm::vec2 m_position;  //!< Position of the sprite relative to its UIRenderer area.
	mutable glm::vec2 m_scale;     //!< Scale of the sprite. A scale of 1 means natural size (1:1 mapping of sprite to screen pixels)
	mutable float32 m_rotation;    //!< Rotation of the sprite, in radians
	mutable bool m_isPositioningDirty;  //!< Used to avoid unnecessary expensive calculations if commitUpdate is called unnecessarily.
	mutable glm::ivec2 m_pixelOffset;
	mutable Rectanglef m_uv;
	mutable bool m_isUVDirty;
public:
	virtual ~I2dComponent() {}
	I2dComponent() : m_anchor(Anchor::Center), m_position(0.f, 0.f), m_scale(1.f, 1.f),
		m_rotation(0.f), m_isPositioningDirty(true), m_pixelOffset(0, 0),
		m_uv(0.0f, 0.0f, 1.0, 1.0f), m_isUVDirty(false) {}


	/*!**********************************************************************************************

	************************************************************************************************/
	I2dComponent const* setAnchor(Anchor anchor, const glm::vec2& ndcPos = glm::vec2(-1.f, -1.f))
	{
		setAnchor(anchor, ndcPos.x, ndcPos.y);
		return this;
	}

	I2dComponent const* setAnchor(Anchor anchor, pvr::float32 ndcPosX = -1.f, pvr::float32 ndcPosY = -1.f)const
	{
		m_anchor = anchor;
		m_position.x = ndcPosX;
		m_position.y = ndcPosY;
		m_isPositioningDirty = true;
		return this;
	}

	I2dComponent const* setPixelOffset(pvr::int32 offsetX, pvr::int32 offsetY)const
	{
		m_pixelOffset.x = offsetX, m_pixelOffset.y = offsetY;
		m_isPositioningDirty = true;
		return this;
	}

	I2dComponent const* setScale(glm::vec2 const& scale)const
	{
		m_scale = scale;
		m_isPositioningDirty = true;
		return this;
	}

	I2dComponent const* setScale(pvr::float32 scaleX, pvr::float32 scaleY)const
	{
		m_scale = glm::vec2(scaleX, scaleY);
		m_isPositioningDirty = true;
		return this;
	}

	I2dComponent const* setRotation(pvr::float32 rotate)const
	{
		m_rotation = rotate;
		m_isPositioningDirty = true;
		return this;
	}

protected:
	friend class UIRenderer;
	/*!*******************************************************************************************************************************
	\brief	set the UV corrdinate
	\return	Return
	\param	uv
	**********************************************************************************************************************************/
	I2dComponent const* setUV(const pvr::Rectanglef& uv)const
	{
		m_uv = uv;
		m_isUVDirty = true;
		return this;
	}
};

/*!****************************************************************************************************************
\brief Use this class through the Refcounted Framework Object pvr::ui::Image. Represents a 2D Image (aka Texture).
       Can be used like all Sprites and additionally contains methods required for working with Images.
*******************************************************************************************************************/
class Image_ : public Sprite_, public I2dComponent
{
	friend class ::pvr::ui::UIRenderer;
	friend class ::pvr::ui::impl::Text_;
public:
	Image_(UIRenderer& uiRenderer, api::TextureView& tex, uint32 width, uint32 height);

	/*!****************************************************************************************************************
	\return Texture width.
	*******************************************************************************************************************/
	uint32 getWidth()const { return m_texW; }

	/*!****************************************************************************************************************
	\return Texture height.
	*******************************************************************************************************************/
	uint32 getHeight()const { return m_texH; }

	/*!****************************************************************************************************************
	\brief	Function that will be automatically called by the uiRenderer. Do not call.
	*******************************************************************************************************************/
//	void calculateMvp(const glm::mat4& matrix, pvr::uint64 parentIds) const;

	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const;

	/*!****************************************************************************************************************
	\brief	Function that will be automatically called by the uiRenderer. Do not call.
	*******************************************************************************************************************/
	void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const;

	/*!****************************************************************************************************************
	\brief	Function that will be automatically called by the uiRenderer. Do not call.
	*******************************************************************************************************************/
	void bindTexture(IGraphicsContext& context);

	/*!****************************************************************************************************************
	\return The pvr::api::Texture2D object that this Image wraps. Const overload.
	*******************************************************************************************************************/
	const api::TextureView& getTexture()const { return m_texture; }

	/*!****************************************************************************************************************
	\return The pvr::api::Texture2D object that this Image wraps.
	*******************************************************************************************************************/
	api::TextureView& getTexture() { return m_texture; }

	/*!****************************************************************************************************************
	\return The pvr::api::Sampler that this Image will use for sampling the texture. Const overload.
	*******************************************************************************************************************/
	const api::Sampler& getSampler()const { return m_sampler; }

	/*!****************************************************************************************************************
	\return The pvr::api::Sampler that this Image will use for sampling the texture. Const overload.
	*******************************************************************************************************************/
	api::Sampler& getSampler() { return m_sampler; }

	/*!****************************************************************************************************************
	\param sampler The pvr::api::Sampler that this Image will use for sampling the texture.
	*******************************************************************************************************************/
	void setSampler(const api::Sampler& sampler) {	m_isTextureDirty = true; m_sampler = sampler; }

	/*!****************************************************************************************************************
	\return The descriptorSet containing this Image's texture.
	*******************************************************************************************************************/
	const api::DescriptorSet& getTexDescriptorSet() const
	{
		updateTextureDescriptorSet();
		return m_texDescSet;
	}

	glm::vec2 getScaledDimension()const { return getDimensions() * m_scale;}


protected:
	struct InstanceData
	{
		glm::mat4						mvp;		// model-view-projection
		utils::StructuredMemoryView		bufferView; // ubo buffer view
		api::DescriptorSet				uboDescSet;
	};
	typedef std::map<pvr::uint64, InstanceData> UboPool;
	pvr::Result updateTextureDescriptorSet()const;
	void writeUboDescriptorSet(pvr::uint64 parentId)const;
	void updateUbo(pvr::uint64 parentIds)const;
	mutable api::DescriptorSet m_texDescSet; //!< The descriptor set containing the texture of this object
	uint32 m_texW;						//!< Width of the image
	uint32 m_texH;						//!< Height of the image
	api::TextureView m_texture;		//!< The texture object of this image
	api::Sampler m_sampler;				//!< The sampler used by this image
	mutable UboPool m_mvpPools;
	mutable bool m_isTextureDirty;
};

/*!****************************************************************************************************************
\brief Use this class through the Refcounted Framework Object pvr::ui::Font. Is an Image_ containing font characters
       along with the metadata necessary for rendering text with them. Although it can be used like an Image_, this
       does not make some sense since it would just display the characters as a texture atlas. Text objects will
	   contain a reference to a Font to render with.
*******************************************************************************************************************/
class Font_ : public Image_
{
public:
	/*!****************************************************************************************************************
	\brief struct containing the UV's corresponding to the UV coordinates of a character of a Font.
	*******************************************************************************************************************/
	struct CharacterUV
	{
		float32 ul;
		float32 vt;
		float32 ur;
		float32 vb;
	};

	/*!****************************************************************************************************************
	\brief struct representing the metrics of a character of a Font.
	*******************************************************************************************************************/
	struct CharMetrics
	{
		int16	xOff; //!< Prefix offset
		uint16	characterWidth;
	};

	/*!****************************************************************************************************************
	\brief Enumeration values useful for text rendering. PVRTexTool uses these values when creating fonts.
	*******************************************************************************************************************/
	enum
	{
		InvalidChar = 0xFDFDFDFD,
		FontHeader = 0xFCFC0050,
		FontCharList = 0xFCFC0051,
		FontRects = 0xFCFC0052,
		FontMetrics = 0xFCFC0053,
		FontYoffset = 0xFCFC0054,
		FontKerning = 0xFCFC0055,
		MaxRenderableLetters = 0xFFFF >> 2,
		FontElement = MaxRenderableLetters * 6,
	};

	/*!****************************************************************************************************************
	\brief Constructor. Do not use - use the UIRenderer::createFont.
	*******************************************************************************************************************/
	Font_(UIRenderer& uiRenderer, api::TextureView& tex2D, const assets::Texture& tex) :
		Image_(uiRenderer, tex2D, tex.getWidth(), tex.getHeight())
	{
		loadFontData(tex);
		if (tex.getPixelFormat().getNumberOfChannels() == 1 && tex.getPixelFormat().getChannelContent(0) == 'a')
		{
			setAlphaRenderingMode(true);
		}
	}

	/*!****************************************************************************************************************
	\brief Load the font data from the font texture.
	*******************************************************************************************************************/
	bool loadFontData(const assets::Texture& texture);

	/*!****************************************************************************************************************
	\brief Find the index of a character inside the internal font character list. Only useful for custom font use.
	\param character The value of a character. Accepts ASCII through to UTF32 characters.
	\return The index of the character inside the internal font list.
	*******************************************************************************************************************/
	uint32 findCharacter(uint32 character)const;

	/*!****************************************************************************************************************
	\brief Apply kerning to two characters (give the offset required by the specific pair).
	\param charA The first (left) character of the pair.
	\param charB The second (right) character of the pair.
	\param offset Output parameter, the offset that must be applied to the second character due to kerning.
	*******************************************************************************************************************/
	void applyKerning(uint32 charA, uint32 charB, float32& offset);

	/*!****************************************************************************************************************
	\param index The internal index of the character. Use findCharacter to get the index of a specific known character.
	\return A CharMetrics object representing the character metrics of the character with that index.
	*******************************************************************************************************************/
	const CharMetrics& getCharMetrics(uint32 index)const { return m_charMetrics[index]; }

	/*!****************************************************************************************************************
	\param index The internal index of the character. Use findCharacter to get the index of a specific known character.
	\return A CharMetrics object representing the character metrics of the character with that index.
	*******************************************************************************************************************/
	const CharacterUV& getCharacterUV(uint32 index)const { return m_characterUVs[index]; }

	/*!****************************************************************************************************************
	\param index The internal index of the character. Use findCharacter to get the index of a specific known character.
	\return The rectangle where this character exists in the font texture.
	*******************************************************************************************************************/
	const Rectanglei& getRectangle(uint32 index)const { return m_rects[index]; }

	/*!****************************************************************************************************************
	\return The spacing between baseline to baseline of this font, in pixels.
	*******************************************************************************************************************/
	int16 getFontLineSpacing() const { return m_header.lineSpace; }

	/*!****************************************************************************************************************
	\return The distance from Baseline to Ascent of this font, in pixels.
	*******************************************************************************************************************/
	int16 getAscent() const { return m_header.ascent; }

	/*!****************************************************************************************************************
	\return The width, in pixels, of the Space character.
	*******************************************************************************************************************/
	uint8 getSpaceWidth() const { return m_header.spaceWidth; }

	/*!****************************************************************************************************************
	\return The Y offset of the font.
	*******************************************************************************************************************/
	int32 getYOffset(uint32 index) const { return m_yOffsets[index]; }

	/*!****************************************************************************************************************
	\return The number of faces in the font.
	*******************************************************************************************************************/
	static uint16* getFontFaces();
private:
#ifdef _WIN32
	static int32 __cdecl characterCompFunc(const void* a, const void* b);
	static int32 __cdecl kerningCompFunc(const void* a, const void* b);
#else
	static int32 characterCompFunc(const void* a, const void* b);
	static int32 kerningCompFunc(const void* a, const void* b);
#endif

	struct Header	//12 bytes
	{
		uint8	version;/*!< Version of Font_. */
		uint8	spaceWidth;/*!< The width of the 'Space' character. */
		int16	numCharacters;/*!< Total number of characters contained in this file. */
		int16	numKerningPairs;/*!< Number of characters which kern against each other. */
		int16	ascent;/*!< The height of the character, in pixels, from the base line. */
		int16	lineSpace;/*!< The base line to base line dimension, in pixels. */
		int16	borderWidth;/*!< px Border around each character. */
	} m_header;
#pragma pack(push,4)// force 4byte alignment
	struct KerningPair
	{
		uint64 pair;		/*!< OR'd pair for 32bit characters */
		int32  offset;		/*!< Kerning offset (in pixels) */
	};
#pragma pack(pop)
	std::vector<uint32>			m_characters;
	std::vector<KerningPair>	m_kerningPairs;
	std::vector<CharMetrics>	m_charMetrics;
	std::vector<CharacterUV>	m_characterUVs;
	std::vector<Rectanglei>		m_rects;
	std::vector<int32>			m_yOffsets;
};

/*!****************************************************************************************************************
\brief UIRenderer vertex format.
*******************************************************************************************************************/
struct Vertex
{
	float32	x;//< x position
	float32	y;//< y position
	float32	z;//< z position
	float32	rhw;

	//	uint32		color;
	float32	tu;	//< texture u coordinate
	float32	tv; //< texture v coordinate

	void setData(float32 x, float32 y, float32 z, float32 rhw, float32 u, float32 v)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rhw = rhw;
		this->tu = u;
		this->tv = v;
	}
};

/*!****************************************************************************************************************
\brief Use this class through the Refcounted Framework Object pvr::ui::Text. Represents some text that can be
       rendered as a normal Sprite_ and additionally contains the necessary text manipulation functions.
*******************************************************************************************************************/
class Text_ : public Sprite_, public I2dComponent
{
public:
	enum { MaxLetters = 5120 };
	/*!****************************************************************************************************************
	\internal
	\brief Constructor. Do not use - use UIRenderer::createText
	*******************************************************************************************************************/
	Text_(UIRenderer& uiRenderer, const Font& font);

	/*!****************************************************************************************************************
	\internal
	\brief Constructor. Do not use - use UIRenderer::createText
	*******************************************************************************************************************/
	Text_(UIRenderer& uiRenderer, const std::string& text, const Font& font);

	/*!****************************************************************************************************************
	\internal
	\brief Constructor. Do not use - use UIRenderer::createText
	*******************************************************************************************************************/
	Text_(UIRenderer& uiRenderer, const std::wstring& text, const Font& font);

	/*!****************************************************************************************************************
	\brief Set the text of this sprite
	\param str An std::string containing the text to set this object to.
	*******************************************************************************************************************/
	Text_& setText(const std::string& str);

	/*!****************************************************************************************************************
	\brief Set the text of this sprite (wide string version)
	\param str An std::wstring containing the text to set this object to.
	*******************************************************************************************************************/
	Text_& setText(const std::wstring& str);
#ifdef PVR_SUPPORT_MOVE_SEMANTICS
	/*!****************************************************************************************************************
	\internal
	\brief Constructor. Do not use - use UIRenderer::createText.
	*******************************************************************************************************************/
	Text_(UIRenderer& uiRenderer, std::wstring&& text, const Font& font);

	/*!****************************************************************************************************************
	\brief Set the text of this sprite (wide string version)
	\param str An std::wstring containing the text to set this object to.
	*******************************************************************************************************************/
	Text_& setText(std::wstring&& str);
	/*!****************************************************************************************************************
	\brief Constructor. Do not use - use UIRenderer::createText.
	*******************************************************************************************************************/
	Text_(UIRenderer& uiRenderer, std::string&& text, const Font& font);

	/*!****************************************************************************************************************
	\brief Set the text of this sprite
	\param str An std::string containing the text to set this object to.
	*******************************************************************************************************************/
	Text_& setText(std::string&& str);
#endif

	/*!****************************************************************************************************************
	\brief Get the internal UTF32 representation of the text as an std::vector<int32>.
	\return The internal UTF32 representation of the text as an std::vector<int32>.
	*******************************************************************************************************************/
	std::vector<uint32>& getUTF32()const;

	/*!****************************************************************************************************************
	\brief	Function that will be automatically called by the uiRenderer. Do not call.
	*******************************************************************************************************************/
	void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const;

	/*!****************************************************************************************************************
	\brief	Function that will be automatically called by the uiRenderer. Do not call.
	*******************************************************************************************************************/
	uint32 updateVertices(float32 fZPos, float32 xPos, float32 yPos, const std::vector<uint32>& text, Vertex* const pVertices) const;

	/*!****************************************************************************************************************
	\brief	Get the pvr::ui::Font object that this Text object uses for rendering.
	*******************************************************************************************************************/
	const Font& getFont() { return m_font; }
	glm::vec2 measureText()
	{
		glm::vec3 const& size = m_boundingRect.getSize();
		return glm::vec2(size.x, size.y);
	}


	glm::vec2 getScaledDimension()const
	{
		return getDimensions() * m_scale;
	}


private:
	struct InstanceData
	{
		glm::mat4 mvp;				// model-view-projection
		utils::StructuredMemoryView bufferView;	// ubo buffer view
		api::DescriptorSet uboDescSet;
	};
	const api::DescriptorSet& getTexDescriptorSet() const {	return m_font->getTexDescriptorSet(); }
	void regenerateText() const;
	void initializeText(const std::wstring& str);
	void calculateMvp(const glm::mat4& matrix, pvr::uint64 parentIds) const;
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const;
	void updateUbo(pvr::uint64 parentId)const;
	uint32 m_spaceWidth;
	bool m_isUtf8;
	mutable Font m_font;
	mutable bool m_isTextDirty;
	mutable api::Buffer m_vbo;
	mutable std::string m_textStr;
	mutable std::wstring m_textWStr;
	mutable std::vector<uint32> m_utf32;
	mutable std::vector<Vertex>	m_vertices;
	mutable int32 m_numCachedVerts;
	mutable std::map<pvr::uint64, InstanceData> m_mvpPools;
};

/*!****************************************************************************************************************
\brief  Abstract container for sprites. See MatrixGroup or PixelGroup. A group contains references to a number
		of sprites, allowing hierarchical transformations to be applied to them
\details	A very complex transformation using the "id" member is used to optimize the group transformations, as
		each child needs to hold the transformations
*******************************************************************************************************************/
class Group_ : public Sprite_
{
protected:
//!\cond NO_DOXYGEN
	struct SpriteEntryEquals
	{
		Sprite sprite;
		SpriteEntryEquals(const Sprite& sprite) : sprite(sprite) {}
		bool operator()(const Sprite& rhs)	{	return sprite == rhs;	}
	};
	typedef std::vector<Sprite> ChildContainer;
	mutable ChildContainer m_children;
	pvr::uint64 m_id;
//!\endcond
public:
	/*!****************************************************************************************************************
	\brief  Constructor. Internal use. The parameter groupid is an implementation detail used to implement and optimize
			group behaviour, and cannot be trivially determined.
	*******************************************************************************************************************/
	Group_(UIRenderer& uiRenderer, pvr::uint64 groupid) : Sprite_(uiRenderer), m_id(groupid) { }

	/*!****************************************************************************************************************
	\brief  Add a Sprite (Text, Image etc.) to this Group. All sprites in the group will be transformed together when
	calling Render on the group.
	\param  sprite The Sprite to add.
	\return Pointer to this object, in order to easily chan add commands.
	*******************************************************************************************************************/
	Group_* add(const Sprite& sprite)
	{
		m_children.push_back(sprite);
		m_boundingRect.add(sprite->getDimensions().x, sprite->getDimensions().y, 0.0f);
		return this;
	}

	/*!****************************************************************************************************************
	\brief  Remove a Sprite from this Group. Linear search (Complexity O(n) )
	\param  sprite The Sprite to remove.
	*******************************************************************************************************************/
	void remove(const Sprite& sprite)
	{
		ChildContainer::iterator it = std::find_if(m_children.begin(), m_children.end(), SpriteEntryEquals(sprite));
		if (it != m_children.end())
		{
			m_boundingRect.remove((*it)->getBoundingBox());
			m_children.erase(it);
		}
	}

	/*!****************************************************************************************************************
	\internal
	\brief  Internal function. Do not call directly. Triggered when all item transformations have been applied and the
	final matrices can be calculated.
	*******************************************************************************************************************/
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const
	{
		glm::mat4 tmpMatrix = srt * m_cachedMatrix;
		//My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = m_children.begin(); it != m_children.end(); ++it)
		{
			(*it)->calculateMvp(packId(parentIds, m_id), tmpMatrix, viewProj, viewport);
		}
	}


	/*!****************************************************************************************************************
	\internal
	\brief  Internal function that UIRenderer calls to render. Do not call directly.
	*******************************************************************************************************************/
	virtual void onRender(api::CommandBufferBase& commandBuffer, pvr::uint64 parentId) const
	{
		for (ChildContainer::iterator it = m_children.begin(); it != m_children.end(); ++it)
		{
			(*it)->onRender(commandBuffer, packId(parentId, m_id));
		}
	}

	glm::vec2 getScaledDimension()const
	{
		glm::vec2 dim(0);
		for (uint32 i = 0; i < m_children.size(); ++i)
		{
			dim += m_children[i]->getScaledDimension();
		}
		return dim;
	}

protected:
	pvr::uint64 packId(pvr::uint64 parentIds, pvr::uint64 id)const
	{
		pvr::uint64 packed = parentIds << NUM_BITS_GROUP_ID;
		return packed | id;
	}
};

/*!****************************************************************************************************************
\brief	This class is wrapped into the pvr::ui::Group Refcounted Framework Object. Use to apply a transformation
to several Sprites and render them together (for example, layout some sprites to form a UI and then
apply translation or rotation effects to all of them to change the page).
*******************************************************************************************************************/
class MatrixGroup_ : public Group_
{
private:
	glm::mat4 m_viewProj;
public:
	/*!****************************************************************************************************************
	\brief  Set the scale/rotation/translation matrix of this group. If other transformations are added to this matrix,
	        unexpected results may occur when rendering the sprites.
	\param srt  The scale/rotation/translation matrix of this group
	*******************************************************************************************************************/
	void setScaleRotateTranslate(const glm::mat4& srt)
	{
		m_cachedMatrix = srt;
	}
	/*!****************************************************************************************************************
	\brief  Set the projection matrix of this group
	\param viewProj A projection matrix which will be used to render all members of this group
	*******************************************************************************************************************/
	void setViewProjection(const glm::mat4& viewProj)
	{
		m_viewProj = viewProj;
	}

	/*!****************************************************************************************************************
	\brief  Call this method when you are finished updating the sprites (text, matrices, positioning etc.), and BEFORE
	the beginRendering command, to commit any changes you have done to the sprites. This function must not be called
	during rendering.
	*******************************************************************************************************************/
	void commitUpdates()const;

	/*!****************************************************************************************************************
	\internal
	\brief  Constructor. Do not call - use UIRenderer::createGroup.
	*******************************************************************************************************************/
	MatrixGroup_(UIRenderer& uiRenderer, pvr::uint64 id);

	/*!****************************************************************************************************************
	\internal
	\brief  Constructor. Do not call - use UIRenderer::createGroup.
	*******************************************************************************************************************/
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const
	{
		glm::mat4 tmpMatrix = srt * m_cachedMatrix;
		//My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = m_children.begin(); it != m_children.end(); ++it)
		{
			(*it)->calculateMvp(packId(parentIds, m_id), tmpMatrix, viewProj, viewport);
		}
	}
};

/*!****************************************************************************************************************
\brief	This class is wrapped into the pvr::ui::Group Refcounted Framework Object. Use to apply a transformation
to several Sprites and render them together (for example, layout some sprites to form a UI and then
apply translation or rotation effects to all of them to change the page).
*******************************************************************************************************************/
class PixelGroup_ : public Group_, public I2dComponent
{

public:
	/*!****************************************************************************************************************
	\internal
	\brief  Internal function that UIRenderer calls to render. Do not call directly.
	*******************************************************************************************************************/
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvr::Rectanglei const& viewport)const;

	/*!****************************************************************************************************************
	\internal
	\brief  Constructor. Do not call - use UIRenderer::createGroup.
	*******************************************************************************************************************/
	PixelGroup_(UIRenderer& uiRenderer, pvr::uint64 id) : Group_(uiRenderer, id)
	{}

	/*!****************************************************************************************************************
	\brief	Set the size (extent) of this pixel group
	\return	Pointer to this item
	\param	size The size of this pixel group, used to position the items it contains. It DOES NOT perform clipping - items
	        can very well be placed outside the size of the group, and they will be rendered correctly as long as they
			are within the screen/viewport.
	*******************************************************************************************************************/
	PixelGroup_* setSize(glm::vec2 const& size)
	{
		m_boundingRect.setMinMax(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(size.x, size.y, 0.0f));
		return this;
	}

private:
};
}
}
}
