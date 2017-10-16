/*!
\brief Contains the Sprite classes and framework objects used by the UIRenderer (Sprite, Text, Image, Font, Group).
\file PVREngineUtils/Sprite.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/StructuredMemory.h"
#include "PVRCore/Math/AxisAlignedBox.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/Texture.h"
#define NUM_BITS_GROUP_ID 8

/// <summary>Main PowerVR Namespace</summary>
namespace pvr {
//!\cond NO_DOXYGEN
class IGraphicsContext;
class Texture;
typedef Rectangle<int32> Rectanglei;
//!\endcond
/// <summary>Main namespace for the PVREngineUtils Library. Contains the UIRenderer and the several Sprite classes.
/// </summary>
namespace ui {
//!\cond NO_DOXYGEN
class UIRenderer;
namespace impl {
class Font_;
class Text_;
class TextElement_;
class Image_;
class Group_;
class MatrixGroup_;
class PixelGroup_;
class Sprite_;
}
//!\endcond

/// <summary>A Reference Counted Framework Object wrapping the Group_ class. Groups several sprites to apply
/// some transformation to them and render them all together.</summary>
typedef RefCountedResource<impl::Group_> Group;

/*!  \class ::pvr::ui::MatrixGroup */
/// <summary>A Reference Counted Framework Object wrapping the Group_ class. Groups several sprites to apply
/// some matrix transformation to them and render them all together.</summary>
typedef RefCountedResource<impl::MatrixGroup_> MatrixGroup;

/*!  \class ::pvr::ui::PixelGroup */
/// <summary>A Reference Counted Framework Object wrapping the PixelGroup class. Groups several sprites to apply
/// intuitive 2D operations and layouts to them.</summary>
typedef RefCountedResource<impl::PixelGroup_> PixelGroup;

/*!  \class ::pvr::ui::Sprite */
/// <summary>A Reference Counted Framework Object wrapping the Sprite_ interface. Represents anything you can
/// use with the UIRenderer (Font, Text, Image, Group).</summary>
typedef RefCountedResource<impl::Sprite_> Sprite;

typedef RefCountedResource<impl::Text_> Text;

/*!  \class ::pvr::ui::Font */
/// <summary>A Reference Counted Framework Object wrapping the Font_ class. Is an Image object augmented by font
/// metadata. Is used by the Text class.</summary>
typedef RefCountedResource<impl::Font_> Font;

/*!  \class ::pvr::ui::Text */
/// <summary>A Reference Counted Framework Object wrapping the Text_ class. The Text is a Sprite and contains a
/// string of characters to be displayed with the Font that it uses.</summary>
typedef RefCountedResource<impl::TextElement_> TextElement;

/*!  \class ::pvr::ui::Image */
/// <summary>A Reference Counted Framework Object wrapping the Image_ class. The Image is a Sprite and contains
/// a 2D texture that can be displayed.</summary>
typedef RefCountedResource<impl::Image_> Image;


/// <summary>An Enumeration of all the Anchor points that can be used to position a Sprite. An anchor point is
/// the point to which all positioning will be relative to. Use this to facilitate the laying out of UIs.
/// </summary>
enum class Anchor
{
	TopLeft, TopCenter, TopRight,
	CenterLeft, Center, CenterRight,
	BottomLeft, BottomCenter, BottomRight
};

/// <summary>Contains the implementation of the raw UIRenderer classes. In order to use the library, use the PowerVR
/// Framework objects found in the pvr::ui namespace</summary>
namespace impl {
/// <summary>Base sprite class. Use through the Sprite framework object. Represents something that can be rendered
/// with the UIRenderer. Texts, Images, Groups are all sprites (Fonts too although it would only be used as a
/// sprite to display the entire font's glyphs).</summary>
class Sprite_
{
public:

	/// <summary>Call this function after changing the sprite in any way, in order to update its internal
	/// information. This function should be called before any rendering commands are submitted and
	/// before calling functions such as getDimensions, in order to actually process all the changes
	/// to the sprite.</summary>
	virtual void commitUpdates() const;

	virtual ~Sprite_() {}

	/// <summary>Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box.</returns>
	glm::vec2 getDimensions()const
	{
		return glm::vec2(_boundingRect.getSize());
	}

	/// <summary>RenderImmediate functions forgo the normal cycle of uiRenderer::beginRendering - Sprite::render -
	/// uiRenderer::endRendering for convenience. Do not use if rendering multiple sprites to avoid needless state
	/// changes.</summary>
	/// <param name="commandBuffer">The pvr::api::CommandBuffer to render into.</param>
	void renderImmediate(api::CommandBuffer& commandBuffer) const;


	/// <summary>Render is the normal function to call to render a sprite. Before calling this function, call
	/// beginRendering on the uiRenderer this sprite belongs to to set up the commandBuffer to render to. In general
	/// try to group as many render commands as possible between the beginRendering and endRendering. This overload
	/// does not apply any transformations to the sprite.</summary>
	void render() const;

	/// <summary>Use this to use this sprite as Alpha channel only, setting its color to 1,1,1,a. Otherwise, an Alpha
	/// texture would render black. Always use this setting to render Fonts that have been generated with PVRTexTool
	/// as Alpha textures.</summary>
	/// <param name="isAlphaOnly">Pass "true" to flush all color channels to 1.0 and keep the alpha channel. Pass false
	/// (default state) to render with texture colors unchanged.</param>
	void setAlphaRenderingMode(bool isAlphaOnly) const
	{
		_alphaMode = isAlphaOnly;
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as a vector of normalised 32 bit float values.
	/// Range of values must be 0..1</summary>
	/// <param name="color">A glm::vec4 that contains color and alpha values in the range of 0..1. Initial value
	/// (1.0,1.0,1.0,1.0)</param>
	void setColor(glm::vec4 color) const
	{
		_color = color;
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as bytes (0..255).</summary>
	/// <param name="r">Red channel. Initial value 255.</param>
	/// <param name="g">Green channel. Initial value 255.</param>
	/// <param name="b">Blue channel. Initial value 255.</param>
	/// <param name="a">Alpha channel. Initial value 255.</param>
	void setColor(uint32 r, uint32 g, uint32 b, uint32 a) const
	{
		_color[0] = static_cast<float>(r / 255.f);
		_color[1] = static_cast<float>(g / 255.f);
		_color[2] = static_cast<float>(b / 255.f);
		_color[3] = static_cast<float>(a / 255.f);
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as normalised floating point values. Values
	/// must be in the range of 0..1</summary>
	/// <param name="r">Red channel. Initial value 1.</param>
	/// <param name="g">Green channel. Initial value 1.</param>
	/// <param name="b">Blue channel. Initial value 1.</param>
	/// <param name="a">Alpha channel. Initial value 1.</param>
	void setColor(float r, float g, float b, float a) const
	{
		_color.r = r;
		_color.g = g;
		_color.b = b;
		_color.a = a;
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as bytes packed into an integer.</summary>
	/// <param name="rgba">8 bit groups, least significant bits: Red, then Green, then Blue, then most significant
	/// bits is Alpha.</param>
	void setColor(uint32 rgba) const
	{
		_color[0] = static_cast<float>(rgba & 0xFF) / 255.f;
		_color[1] = static_cast<float>(rgba >> 8 & 0xFF) / 255.f;
		_color[2] = static_cast<float>(rgba >> 16 & 0xFF) / 255.f;
		_color[3] = static_cast<float>(rgba >> 24 & 0xFF) / 255.f;
	}

	/// <summary>Get the modulation (multiplicative) color of the sprite, as a glm::vec4.</summary>
	/// <returns>The sprites's modulation color. Values are normalised in the range of 0..1</returns>
	const glm::vec4& getColor()const { return _color; }

	/// <summary>This setting queries if this is set to render as Alpha channel only, (setting its color to 1,1,1,a).
	/// Otherwise, an Alpha texture would render black. This setting is typically used to render Text with Fonts that
	/// have been generated with PVRTexTool as Alpha textures.</summary>
	/// <returns>"true" if set to render as Alpha, false otherwise.</returns>
	bool getAlphaRenderingMode()const { return _alphaMode == 1; }

	/// <summary>Get the sprite's own transformation matrix. Does not contain hierarchical transformations from groups
	/// etc. This function is valid only after any changes to the sprite have been commited with commitUpdates as it
	/// is normally calculated in commitUpdates.</summary>
	/// <returns>The sprite's final transformation matrix. If the sprite is rendered on its own, this is the matrix
	/// that will be uploaded to the shader.</returns>
	const glm::mat4& getMatrix() const { return _cachedMatrix; }


	/// <summary>Do not call directly. CommitUpdates will call this function. If writing new sprite classes,
	/// implement this function to calculate the mvp matrix of the sprite from any other possible representation that
	/// the sprite's data contains into its own _cachedMatrix member.</summary>
	virtual void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                          pvr::Rectanglei const& viewport)const = 0;

	/********************************************************************************************************
	\brief    Do not call directly. Render will call this function.
	      If writing new sprites, implement this function to render a sprite with the provided
	      transformation matrix. Necessary to support instanced rendering of the same sprite from
	      different groups.
	********************************************************************************************************/
	virtual void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const
	{}

	math::AxisAlignedBox const& getBoundingBox()const { return _boundingRect; }

	virtual glm::vec2 getScaledDimension()const = 0;

protected:
	virtual bool onAddInstance(uint64 parentId) = 0;
	virtual void onRemoveInstance(uint64){}
	friend class ::pvr::ui::impl::Group_;
	friend class ::pvr::ui::UIRenderer;
	Sprite_(UIRenderer& uiRenderer);
	mutable math::AxisAlignedBox _boundingRect; //< Bounding rectangle of the sprite
	mutable glm::vec4 _color; //< Modulation color (multiplicative)
	mutable int32 _alphaMode; //< Set the shader to render alpha-only
	UIRenderer& _uiRenderer; //< UIRenderer this sprite belongs to
	mutable glm::mat4 _cachedMatrix; //< Bounding rectangle of the sprite
	glm::mat4 _viewProj;
};

/// <summary>A component that can be positioned in 2D using 2d position, scale, rotation and anchored using its
/// center or corners.</summary>
class I2dComponent
{
protected:
	mutable Anchor _anchor; //!< The position in the sprite relative to which all positioning calculations are done
	mutable glm::vec2 _position;  //!< Position of the sprite relative to its UIRenderer area.
	mutable glm::vec2 _scale;     //!< Scale of the sprite. A scale of 1 means natural size (1:1 mapping of sprite to screen pixels)
	mutable float32 _rotation;    //!< Rotation of the sprite, in radians
	mutable bool _isPositioningDirty;  //!< Used to avoid unnecessary expensive calculations if commitUpdate is called unnecessarily.
	mutable glm::ivec2 _pixelOffset;
	mutable Rectanglef _uv;
	mutable bool _isUVDirty;
public:
	virtual ~I2dComponent() {}
	I2dComponent() : _anchor(Anchor::Center), _position(0.f, 0.f), _scale(1.f, 1.f),
		_rotation(0.f), _isPositioningDirty(true), _pixelOffset(0, 0),
		_uv(0.0f, 0.0f, 1.0, 1.0f), _isUVDirty(true) {}


	/// <summary>Set the anchor and position ("centerpoint") of this component. The anchor is the point around which
	/// all operations (e.g. scales, rotations) will happen.</summary>
	/// <param name="anchor">The anchor point</param>
	/// <param name="ndcPos">The normalized device coordinates (-1..1) where the anchor should be in its group
	/// </param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setAnchor(Anchor anchor, const glm::vec2& ndcPos)
	{
		setAnchor(anchor, ndcPos.x, ndcPos.y);
		return this;
	}

	/// <summary>Set the anchor and position ("centerpoint") of this component. The anchor is the point around which
	/// all operations (e.g. scales, rotations) will happen.</summary>
	/// <param name="anchor">The anchor point</param>
	/// <param name="ndcPosX">The normalized (-1..1) horizontal coordinate where the anchor should be in its group
	/// </param>
	/// <param name="ndcPosY">The normalized (-1..1) vertical coordinate where the anchor should be in its group
	/// </param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setAnchor(Anchor anchor, pvr::float32 ndcPosX = -1.f, pvr::float32 ndcPosY = -1.f)const
	{
		_anchor = anchor;
		_position.x = ndcPosX;
		_position.y = ndcPosY;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the pixel offset of this object. Pixel offset is applied after every other calculation, so it
	/// always moves the final (transformed) sprite by the specified number of pixels in each direction.</summary>
	/// <param name="offsetX">Number of pixels to move the sprite right (negative for left)</param>
	/// <param name="offsetY">Number of pixels to move the sprite up (negative for down)</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setPixelOffset(pvr::int32 offsetX, pvr::int32 offsetY)const
	{
		_pixelOffset.x = offsetX, _pixelOffset.y = offsetY;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the scale of this object.</summary>
	/// <param name="scale">The scale of this object. (1,1) is natural scale.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setScale(glm::vec2 const& scale)const
	{
		_scale = scale;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the scale of this object.</summary>
	/// <param name="scaleX">The scale of this object in the X direction. 1 is natural scale.</param>
	/// <param name="scaleY">The scale of this object in the Y direction. 1 is natural scale.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setScale(pvr::float32 scaleX, pvr::float32 scaleY)const
	{
		_scale = glm::vec2(scaleX, scaleY);
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the rotation of this object on the screen (in fact, its parent group's) plane.</summary>
	/// <param name="radians">The Counter Clockwise rotation of this object, in radians, around its Z axis.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setRotation(pvr::float32 radians)const
	{
		_rotation = radians;
		_isPositioningDirty = true;
		return this;
	}
	I2dComponent const* setUV(const pvr::Rectanglef& uv)const
	{
		_uv = uv;
		_isUVDirty = true;
		return this;
	}

protected:
	friend class UIRenderer;
};

/// <summary>Use this class through the Refcounted Framework Object pvr::ui::Image. Represents a 2D Image (aka
/// Texture). Can be used like all Sprites and additionally contains methods required for working with Images.
/// </summary>
class Image_ : public Sprite_, public I2dComponent
{
	friend class ::pvr::ui::UIRenderer;
	friend class ::pvr::ui::impl::Text_;
public:
	/// <summary>Internal</summary>
	Image_(UIRenderer& uiRenderer, const api::TextureView& tex, uint32 width, uint32 height, const api::Sampler& sampler);

	/// <summary>Get the width of this image width in pixels.</summary>
	/// <returns>Image width in pixels.</returns>
	uint32 getWidth()const { return _texW; }

	/// <summary>Get the height of this image width in pixels.</summary>
	/// <returns>Image width in pixels.</returns>
	uint32 getHeight()const { return _texH; }

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const;

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const;

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void bindTexture(IGraphicsContext& context);

	/// <summary>Retrieve the pvr::api::Texture2D object that this Image wraps.</summary>
	/// <returns>The pvr::api::Texture2D object that this Image wraps.</returns>
	const api::TextureView& getTexture()const { return _texture; }

	/// <summary>Retrieve the pvr::api::Texture2D object that this Image wraps.</summary>
	/// <returns>The pvr::api::Texture2D object that this Image wraps.</returns>
	api::TextureView& getTexture() { return _texture; }

	/// <summary>Retrieve the pvr::api::Sampler that this Image will use for sampling the texture. Const overload.
	/// </summary>
	/// <returns>The pvr::api::Sampler that this Image will use for sampling the texture. Const overload.</returns>
	const api::Sampler& getSampler()const { return _sampler; }

	/// <summary>Retrieve the pvr::api::Sampler that this Image will use for sampling the texture.</summary>
	/// <returns>The pvr::api::Sampler that this Image will use for sampling the texture.</returns>
	api::Sampler& getSampler() { return _sampler; }

	/// <summary>Retrieve the descriptorSet containing this Image's texture.</summary>
	/// <returns>The descriptorSet containing this Image's texture.</returns>
	const api::DescriptorSet& getTexDescriptorSet() const
	{
		updateTextureDescriptorSet();
		return _texDescSet;
	}

	/// <summary>Get the size of this texture after applying scale</summary>
	/// <returns>The size of this texture after applying scale</returns>
	glm::vec2 getScaledDimension()const { return getDimensions() * _scale;}

	bool onAddInstance(uint64 parentId);

protected:
	void onRemoveInstance(uint64 parentud);


	struct MvpUboData
	{
		glm::mat4 mvp;    // model-view-projection
		mutable int32 bufferArrayId;
		MvpUboData() : bufferArrayId(-1) {}
	};

	struct MaterialUboData
	{
		glm::vec4 color;
		uint32    isAlphaMode;
		mutable int32   bufferArrayId;
		MaterialUboData() : bufferArrayId(-1) {}
	} _materialData;

	typedef std::map<pvr::uint64, MvpUboData> InstanceUboData;

	bool init();

	pvr::Result updateTextureDescriptorSet()const;
	void writeUboDescriptorSet(pvr::uint64 parentId)const;
	void updateUbo(pvr::uint64 parentIds)const;
	mutable api::DescriptorSet _texDescSet; //!< The descriptor set containing the texture of this object
	uint32 _texW;           //!< Width of the image
	uint32 _texH;           //!< Height of the image
	api::TextureView _texture;    //!< The texture object of this image
	api::Sampler _sampler;        //!< The sampler used by this image
	mutable InstanceUboData _mvpData;
	mutable bool _isTextureDirty;
};

/// <summary>Use this class through the Refcounted Framework Object pvr::ui::Font. Is an Image_ containing font
/// characters along with the metadata necessary for rendering text with them. Although it can be used like an
/// Image_, this does not make some sense since it would just display the characters as a texture atlas. Text
/// objects will contain a reference to a Font to render with.</summary>
class Font_
{
public:
	/// <summary>struct containing the UV's corresponding to the UV coordinates of a character of a Font.</summary>
	struct CharacterUV
	{
		float32 ul;
		float32 vt;
		float32 ur;
		float32 vb;
	};

	/// <summary>struct representing the metrics of a character of a Font.</summary>
	struct CharMetrics
	{
		int16 xOff; //!< Prefix offset
		uint16  characterWidth;
	};

	/// <summary>Enumeration values useful for text rendering. PVRTexTool uses these values when creating fonts.</summary>
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

	/// <summary>Constructor. Do not use - use the UIRenderer::createFont.</summary>
	Font_(UIRenderer& uiRenderer, api::TextureView& tex2D, const Texture& tex, const api::Sampler& sampler)
	{
		init(uiRenderer, tex2D, tex, sampler);
	}

	/// <summary>Load the font data from the font texture.</summary>
	bool loadFontData(const Texture& texture);

	/// <summary>Find the index of a character inside the internal font character list. Only useful for custom font
	/// use.</summary>
	/// <param name="character">The value of a character. Accepts ASCII through to UTF32 characters.</param>
	/// <returns>The index of the character inside the internal font list.</returns>
	uint32 findCharacter(uint32 character)const;

	/// <summary>Apply kerning to two characters (give the offset required by the specific pair).</summary>
	/// <param name="charA">The first (left) character of the pair.</param>
	/// <param name="charB">The second (right) character of the pair.</param>
	/// <param name="offset">Output parameter, the offset that must be applied to the second character due to kerning.
	/// </param>
	void applyKerning(uint32 charA, uint32 charB, float32& offset);

	/// <summary>Get the character metrix of this font</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific
	/// known character.</param>
	/// <returns>A CharMetrics object representing the character metrics of the character with that index.
	/// </returns>
	const CharMetrics& getCharMetrics(uint32 index)const { return _charMetrics[index]; }

	/// <summary>Get the UVs of the characters of this font</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific
	/// known character.</param>
	/// <returns>A CharMetrics object representing the character metrics of the character with that index.
	/// </returns>
	const CharacterUV& getCharacterUV(uint32 index)const { return _characterUVs[index]; }

	/// <summary>Get the rectangle for a specific character</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific
	/// known character.</param>
	/// <returns>The rectangle where this character exists in the font texture.</returns>
	const Rectanglei& getRectangle(uint32 index)const { return _rects[index]; }

	/// <summary>Get the spacing between baseline to baseline of this font, in pixels.</summary>
	/// <returns>The spacing between baseline to baseline of this font, in pixels.</returns>
	int16 getFontLineSpacing() const { return _header.lineSpace; }

	/// <summary>Get the distance between baseline to Ascent of this font, in pixels.</summary>
	/// <returns>The distance from Baseline to Ascent of this font, in pixels.</returns>
	int16 getAscent() const { return _header.ascent; }

	/// <summary>Get the width, in pixels, of the Space character.</summary>
	/// <returns>The width, in pixels, of the Space character.</returns>
	uint8 getSpaceWidth() const { return _header.spaceWidth; }

	/// <summary>Get the Y offset of the font.</summary>
	/// <returns>The Y offset of the font.</returns>
	int32 getYOffset(uint32 index) const { return _yOffsets[index]; }

	bool isAlphaRendering()const { return _alphaRenderingMode != 0; }

	const api::DescriptorSet& getTexDescriptorSet()const
	{
		return _texDescSet;
	}

	/// <summary>Get the number of faces the font.</summary>
	/// <returns>The number of faces in the font.</returns>
	static uint16* getFontFaces();
private:
	static int32 PVR_API_FUNC characterCompFunc(const void* a, const void* b);
	static int32 PVR_API_FUNC kerningCompFunc(const void* a, const void* b);

	bool init(UIRenderer& uiRenderer, api::TextureView& tex2D, const Texture& tex, const api::Sampler& sampler);

	struct Header //12 bytes
	{
		uint8 version;/*!< Version of Font_. */
		uint8 spaceWidth;/*!< The width of the 'Space' character. */
		int16 numCharacters;/*!< Total number of characters contained in this file. */
		int16 numKerningPairs;/*!< Number of characters which kern against each other. */
		int16 ascent;/*!< The height of the character, in pixels, from the base line. */
		int16 lineSpace;/*!< The base line to base line dimension, in pixels. */
		int16 borderWidth;/*!< px Border around each character. */
	} _header;
#pragma pack(push,4)// force 4byte alignment
	struct KerningPair
	{
		uint64 pair;    /*!< Shifted and OR'd pair for 32bit characters */
		int32  offset;    /*!< Kerning offset (in pixels) */
	};
#pragma pack(pop)
	std::vector<uint32>     _characters;
	std::vector<KerningPair>  _kerningPairs;
	std::vector<CharMetrics>  _charMetrics;
	std::vector<CharacterUV>  _characterUVs;
	std::vector<Rectanglei>   _rects;
	std::vector<int32>      _yOffsets;

	api::TextureView _tex;
	glm::uvec2 _dim;
	uint32 _alphaRenderingMode;
	api::DescriptorSet _texDescSet;
};

/// <summary>UIRenderer vertex format.</summary>
struct Vertex
{
	float32 x;//< x position
	float32 y;//< y position
	float32 z;//< z position
	float32 rhw;

	//  uint32    color;
	float32 tu; //< texture u coordinate
	float32 tv; //< texture v coordinate

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


class TextElement_
{
public:
	enum { MaxLetters = 5120 };

	TextElement_(UIRenderer& uiRenderer,  const Font& font) : _uiRenderer(uiRenderer), _font(font), _isTextDirty(false) {}

	TextElement_(UIRenderer& uiRenderer, const std::string& str,  const Font& font) : _uiRenderer(uiRenderer),
		_font(font)
	{
		setText(str);
		updateText();
	}

	TextElement_(UIRenderer& uiRenderer, const std::wstring& str,  const Font& font) : _uiRenderer(uiRenderer),
		_font(font)
	{
		setText(str);
		updateText();
	}

	/// <summary>Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box.</returns>
	glm::vec2 getDimensions()const { return glm::vec2(_boundingRect.getSize()); }

	math::AxisAlignedBox const& getBoundingBox()const { return _boundingRect; }

	TextElement_& setText(const std::string& str);

	TextElement_& setText(const std::wstring& str);
#ifdef PVR_SUPPORT_MOVE_SEMANTICS
	TextElement_& setText(std::wstring&& str);

	TextElement_& setText(std::string&& str);
#endif

	const string& getString()const { return _textStr; }

	const std::wstring& getWString()const { return _textWStr; }

	glm::vec2 measureText()const
	{
		return glm::vec2(_boundingRect.getSize());
	}

	const Font& getFont()const { return _font; }

private:
	friend class ::pvr::ui::impl::Text_;

	bool updateText()const
	{
		if (_isTextDirty)
		{
			regenerateText();
			updateVbo();
			_isTextDirty = false;
			return true;
		}
		return false;
	}

	void regenerateText() const;
	void initVbo();
	void updateVbo()const;
	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void onRender(api::CommandBufferBase& commands) const;

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	uint32 updateVertices(float32 fZPos, float32 xPos, float32 yPos, const std::vector<uint32>& text, Vertex* const pVertices) const;

	uint32 _spaceWidth;
	bool _isUtf8;
	mutable bool _isTextDirty;
	mutable api::Buffer _vbo;
	mutable Font _font;
	mutable std::string _textStr;
	mutable std::wstring _textWStr;
	mutable std::vector<uint32> _utf32;
	mutable std::vector<Vertex> _vertices;
	mutable int32 _numCachedVerts;
	UIRenderer& _uiRenderer;
	mutable math::AxisAlignedBox _boundingRect; //< Bounding rectangle of the sprite
};

/// <summary>Use this class through the Refcounted Framework Object pvr::ui::Text. Represents some text that can
/// be rendered as a normal Sprite_ and additionally contains the necessary text manipulation functions.</summary>
class Text_ : public Sprite_, public I2dComponent
{
public:
	/// <summary>Constructor. Do not use - use UIRenderer::createText</summary>
	Text_(UIRenderer& uiRenderer, const TextElement& text);

	/// <summary>Get the pvr::ui::Font object that this Text object uses for rendering.</summary>
	const Font getFont()const { return getTextElement()->getFont(); }

	TextElement getTextElement() { return _text; }

	const TextElement getTextElement()const { return _text; }

	glm::vec2 getScaledDimension()const
	{
		return getDimensions() * _scale;
	}

	Text_& setText(const std::string& str) { getTextElement()->setText(str); return *this; }

	Text_& setText(const std::wstring& str) { getTextElement()->setText(str); return *this;}
#ifdef PVR_SUPPORT_MOVE_SEMANTICS
	Text_& setText(std::wstring&& str) { getTextElement()->setText(std::forward<std::wstring>(str)); return *this;}

	Text_& setText(std::string&& str) { getTextElement()->setText(std::forward<std::string>(str)); return *this;}
#endif

	void onRender(api::CommandBufferBase& commands, pvr::uint64 parentId) const;

private:	
	friend class ::pvr::ui::UIRenderer;

	void onRemoveInstance(uint64 parentId);


	struct MvpUboData
	{
		glm::mat4 mvp;    // model-view-projection
		mutable int32 bufferArrayId;
		MvpUboData() : bufferArrayId(-1) {}
	};

	struct MaterialUboData
	{
		glm::vec4 color;
		uint32  isAlphaMode;
		mutable int32   bufferArrayId;
		MaterialUboData() : bufferArrayId(-1) {}
	} _materialData;

	const api::DescriptorSet& getTexDescriptorSet() const { return getFont()->getTexDescriptorSet(); }
	bool init();
	bool onAddInstance(uint64 parentId);

	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const;

	void updateUbo(pvr::uint64 parentId)const;
	mutable TextElement _text;
	mutable std::map<pvr::uint64, MvpUboData> _mvpData;
};

/// <summary>Abstract container for sprites. See MatrixGroup or PixelGroup. A group contains references to a number of
/// sprites, allowing hierarchical transformations to be applied to them</summary>
/// <remarks>A very complex transformation using the "id" member is used to optimize the group transformations,
/// as each child needs to hold the transformations</remarks>
class Group_ : public Sprite_
{
protected:
//!\cond NO_DOXYGEN
	struct SpriteEntryEquals
	{
		Sprite sprite;
		SpriteEntryEquals(const Sprite& sprite) : sprite(sprite) {}
		bool operator()(const Sprite& rhs)  { return sprite == rhs; }
	};
	typedef std::vector<Sprite> ChildContainer;
	mutable ChildContainer _children;
	pvr::uint64 _id;
//!\endcond
public:
	/// <summary>Constructor. Internal use. The parameter groupid is an implementation detail used to implement and optimize
	/// group behaviour, and cannot be trivially determined.</summary>
	Group_(UIRenderer& uiRenderer, pvr::uint64 groupid) : Sprite_(uiRenderer), _id(groupid) { }

	/// <summary>Add a Sprite (Text, Image etc.) to this Group. All sprites in the group will be transformed together
	/// when calling Render on the group.
	/// NOTE: Adding  Sprites in to group requires re-recording the commandbuffer </summary>
	/// <param name="sprite">The Sprite to add.</param>
	/// <returns>Pointer to this object, in order to easily chan add commands.</returns>
	Group_* add(const Sprite& sprite);

	void add(const Sprite* sprites, uint32 numSprites)
	{
		std::for_each(sprites, sprites + numSprites,
					  [&](const Sprite& sprite)
		{
			add(sprite);
		});
	}

	/// <summary>Remove a Sprite from this Group. Linear search (Complexity O(n) )</summary>
	/// <param name="sprite">The Sprite to remove.</param>
	void remove(const Sprite& sprite)
	{
		ChildContainer::iterator it = std::find_if(_children.begin(), _children.end(), SpriteEntryEquals(sprite));
		if (it != _children.end())
		{
			_boundingRect.remove((*it)->getBoundingBox());
			(*it)->onRemoveInstance(_id);
			_children.erase(it);
		}
	}

	/// <summary>Remove all sprites in this group. Requires commandbuffer re-recording inorder to take affect</param>
	void removeAll()
	{
		_children.erase(_children.begin(), _children.end());
		_boundingRect.clear();
	}

	/// <summary>Internal function. Do not call directly. Triggered when all item transformations have been applied and the
	/// final matrices can be calculated.</summary>
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const
	{
		glm::mat4 tmpMatrix = srt * _cachedMatrix;
		//My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
		{
			(*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj, viewport);
		}
	}

	/// <summary>Internal function that UIRenderer calls to render. Do not call directly.</summary>
	virtual void onRender(api::CommandBufferBase& commandBuffer, pvr::uint64 parentId) const
	{
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
		{
			(*it)->onRender(commandBuffer, packId(parentId, _id));
		}
	}

	glm::vec2 getScaledDimension()const
	{
		glm::vec2 dim(0);
		for (uint32 i = 0; i < _children.size(); ++i)
		{
			dim += _children[i]->getScaledDimension();
		}
		return dim;
	}

protected:
	void onRemoveInstance(uint32 parentId)
	{
		for (uint32 i = 0; i < _children.size(); ++i)
		{
			_children[i]->onRemoveInstance(packId(parentId, _id));
		}
	}

	bool onAddInstance(uint64 parentId)
	{
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
		{
			if (!(*it)->onAddInstance(packId(parentId, _id))) { return false; }
		}
		return true;
	}
	pvr::uint64 packId(pvr::uint64 parentIds, pvr::uint64 id)const
	{
		pvr::uint64 packed = parentIds << NUM_BITS_GROUP_ID;
		return packed | id;
	}
};

/// <summary>This class is wrapped into the pvr::ui::Group Refcounted Framework Object. Use to apply a
/// transformation to several Sprites and render them together (for example, layout some sprites to form a UI and
/// then apply translation or rotation effects to all of them to change the page).</summary>
class MatrixGroup_ : public Group_
{
private:
	glm::mat4 _viewProj;
public:
	/// <summary>Constructor. Do not call - use UIRenderer::createGroup.</summary>
	MatrixGroup_(UIRenderer& uiRenderer, pvr::uint64 id);

	/// <summary>Set the scale/rotation/translation matrix of this group. If other transformations are added to this
	/// matrix, unexpected results may occur when rendering the sprites.</summary>
	/// <param name="srt">The scale/rotation/translation matrix of this group</param>
	void setScaleRotateTranslate(const glm::mat4& srt)
	{
		_cachedMatrix = srt;
	}
	/// <summary>Set the projection matrix of this group</summary>
	/// <param name="viewProj">A projection matrix which will be used to render all members of this group</param>
	void setViewProjection(const glm::mat4& viewProj)
	{
		_viewProj = viewProj;
	}

	/// <summary>Call this method when you are finished updating the sprites (text, matrices, positioning etc.), and
	/// BEFORE the beginRendering command, to commit any changes you have done to the sprites. This function must not
	/// be called during rendering.</summary>
	void commitUpdates()const;


	/// <summary>Constructor. Do not call - use UIRenderer::createGroup.</summary>
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const
	{
		glm::mat4 tmpMatrix = srt * _cachedMatrix;
		//My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
		{
			(*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj, viewport);
		}
	}
};

/// <summary>This class is wrapped into the pvr::ui::Group Refcounted Framework Object. Use to apply a
/// transformation to several Sprites and render them together (for example, layout some sprites to form a UI and
/// then apply translation or rotation effects to all of them to change the page).</summary>
class PixelGroup_ : public Group_, public I2dComponent
{

public:
	/// <summary>Internal function that UIRenderer calls to render. Do not call directly.</summary>
	void calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
	                  pvr::Rectanglei const& viewport)const;

	/// <summary>Constructor. Do not call - use UIRenderer::createGroup.</summary>
	PixelGroup_(UIRenderer& uiRenderer, pvr::uint64 id) : Group_(uiRenderer, id)
	{}

	/// <summary>Set the size (extent) of this pixel group</summary>
	/// <param name="size">The size of this pixel group, used to position the items it contains. It DOES NOT perform
	/// clipping - items can very well be placed outside the size of the group, and they will be rendered correctly as
	/// long as they are within the screen/viewport.</param>
	/// <returns>Pointer to this item</returns>
	PixelGroup_* setSize(glm::vec2 const& size)
	{
		_boundingRect.setMinMax(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(size.x, size.y, 0.0f));
		return this;
	}

private:
};
}
}
}
