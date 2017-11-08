/*!
\brief Contains implementations of functions for the Sprite_ class and subclasses (Sprite_, Text_, Image_, Font_,
MatrixGroup_)
\file PVRUtils/Vulkan/SpriteVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRUtils/Vulkan/SpriteVk.h"
#include "PVRUtils/Vulkan/UIRendererVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/ImageVk.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::mat3;
using glm::mat3x4;
using glm::mat4;

namespace pvr {
namespace ui {
using namespace ::pvrvk;
namespace impl {
struct UboData
{
	glm::mat4 mvp;
	glm::mat4 uv;
	glm::vec4 color;
	bool    alphaMode;

	static const std::pair<StringHash, GpuDatatypes> EntryNames[4];
	enum Entry { MVP, UV, Color, AlphaMode, Count };
};

const std::pair<StringHash, GpuDatatypes> UboData::EntryNames[] =
{
	std::pair<StringHash, GpuDatatypes>("mvp", GpuDatatypes::mat4x4),
	std::pair<StringHash, GpuDatatypes>("uv", GpuDatatypes::mat4x4),
	std::pair<StringHash, GpuDatatypes>("color", GpuDatatypes::vec4),
	std::pair<StringHash, GpuDatatypes>("alphaMode", GpuDatatypes::Integer),
};

Sprite_::Sprite_(UIRenderer& uiRenderer) :
	_color(1.f, 1.f, 1.f, 1.f),
	_alphaMode(false),
	_uiRenderer(&uiRenderer)
{
	_boundingRect.clear();
}

void Sprite_::commitUpdates()const
{
	calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _uiRenderer->getProjection(),
	             _uiRenderer->getViewport());
}

void Sprite_::render() const
{
	if (!_uiRenderer->isRendering())
	{
		Log(LogLevel::Error, "Sprite: Render called without first calling uiRenderer::begin to set up the commandbuffer.");
		return;
	}
	onRender(_uiRenderer->getActiveCommandBuffer(), 0);
}

void Image_::updateUbo(uint64_t parentIds)const
{
	glm::vec3 scale(_uv.extent.width, _uv.extent.height, 1.0f);
	glm::mat4 uvTrans = glm::translate(glm::vec3(_uv.offset.x, _uv.offset.y, 0.0f)) *  glm::scale(scale);

	debug_assertion(_mvpData[parentIds].bufferArrayId != -1, "Invalid MVP Buffer ID");
	debug_assertion(_materialData.bufferArrayId != -1, "Invalid Material Buffer ID");
	// update the ubo
	_uiRenderer->getUbo().updateMvp(_mvpData[parentIds].bufferArrayId, _mvpData[parentIds].mvp);
	_uiRenderer->getMaterial().updateMaterial(_materialData.bufferArrayId, _color, _alphaMode, uvTrans);
}

pvr::Result Image_::updateTextureDescriptorSet() const
{
	if (!_texDescSet.isValid())
	{
		Log("Failed to create descriptor set for Image sprite");
		return pvr::Result::UnknownError;
	}
	// update the texture descriptor set
	if (_isTextureDirty)
	{
		WriteDescriptorSet writeDescSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _texDescSet, 0);
		writeDescSet.setImageInfo(0, DescriptorImageInfo(getTexture(), getSampler(),
		                          VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		_uiRenderer->getDevice()->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
		_isTextureDirty = false;
	}
	return pvr::Result::Success;
}

void Image_::calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj,
                          const Rect2Di& viewport)const
{
	if (_isPositioningDirty)
	{
		vec2 offset(0.0f);// offset the default center anchor point.

		switch (_anchor)
		{
		case Anchor::Center:                  break;
		case Anchor::TopLeft:   offset = vec2(-1.f, 1.f); break;
		case Anchor::TopCenter:   offset = vec2(0.0f, 1.f); break;
		case Anchor::TopRight:    offset = vec2(1.f, 1.f);  break;
		case Anchor::BottomLeft:  offset = vec2(-1.f, -1.f);  break;
		case Anchor::BottomCenter:  offset = vec2(0.0f, -1.f);  break;
		case Anchor::BottomRight: offset = vec2(1.f, -1.f); break;
		case Anchor::CenterLeft:  offset = vec2(-1.f, 0.0f);  break;
		case Anchor::CenterRight: offset = vec2(1.f, 0.0f); break;
		}

		memset(glm::value_ptr(_cachedMatrix), 0, sizeof(_cachedMatrix));
		//_matrix[0][0] = Will be set in Scale
		//_matrix[1][1] = Will be set in Scale
		_cachedMatrix[2][2] = 1.f; //Does not really matter - we don't have width...
		_cachedMatrix[3][3] = 1.f;
		//READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK

		//4: Transform to SCREEN coordinates...
		//BECAUSE _cachedMatrix IS A PURE ROTATION, WE CAN OPTIMISE THE 1st SCALING OP.
		//THIS IS : _matrix = scale(_matrix, toScreenCoordinates);
		_cachedMatrix[0][0] = 1.f;
		_cachedMatrix[1][1] = 1.f;

		//3: Rotate...
		_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, vec3(0.f, 0.f, 1.f));

		//2: Scale...
		_cachedMatrix = glm::scale(_cachedMatrix, vec3(_scale.x * getWidth() * .5f, _scale.y * getHeight() * .5f, 1.f));

		//1: Apply the offsetting (i.e. place the center at its correct spot. THIS IS NOT THE SCREEN POSITIONING, only the anchor.)
		_cachedMatrix = glm::translate(_cachedMatrix, vec3(-offset, 0.0f));
		_isPositioningDirty = false;
	}

	glm::vec2 tmpPos;
	//5: Translate (screen coords)
	tmpPos.x = _position.x * viewport.extent.width * .5f + viewport.extent.width * .5f + viewport.offset.x + _pixelOffset.x;
	tmpPos.y = _position.y * viewport.extent.height * .5f + viewport.extent.height * .5f + viewport.offset.y + _pixelOffset.y;
	_mvpData[parentIds].mvp = viewProj  *  srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
	updateUbo(parentIds);
}

void Image_::onRender(CommandBufferBase& commandBuffer, uint64_t parentId) const
{
	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 _uiRenderer->getPipelineLayout(), 0, getTexDescriptorSet(), nullptr, 0);
	_uiRenderer->getUbo().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _mvpData[parentId].bufferArrayId);
	_uiRenderer->getMaterial().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _materialData.bufferArrayId);
	commandBuffer->bindVertexBuffer(_uiRenderer->getImageVbo(), 0, 0);
	commandBuffer->draw(0, 6);
}

bool Image_::onAddInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId == -1)
	{
		int32_t id = _uiRenderer->getUbo().getNewBufferSlice();
		if (id == -1)
		{
			Log(LogLevel::Debug, "Failed to create instance of an Image. Reached maximum limit");
			return false;
		}
		_mvpData[parentId].bufferArrayId = id;
	}
	return true;
}

void Image_::onRemoveInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId != -1)
	{
		_uiRenderer->getUbo().releaseBufferSlice(_mvpData[parentId].bufferArrayId);
		_uiRenderer->getMaterial().releaseBufferArray(_materialData.bufferArrayId);
		_mvpData[parentId].bufferArrayId = -1;
		_materialData.bufferArrayId = -1;
	}
}

bool Image_::init()
{
	if (_materialData.bufferArrayId == -1 && (_materialData.bufferArrayId = _uiRenderer->getMaterial().getNewBufferArray()) == -1)
	{
		Log("Failed to create Image. Reached maximum material supported by this UIRenderer");
		return false;
	}
	return onAddInstance(0);
}

Image_::Image_(UIRenderer& uiRenderer, const ImageView& tex,
               uint32_t width, uint32_t height, const Sampler& sampler) :
	Sprite_(uiRenderer), _texW(width), _texH(height),
	_texture(tex), _isTextureDirty(true), _sampler(sampler)
{
	if (!_sampler.isValid())
	{
		_sampler = tex->getImage()->getNumMipMapLevels() > 1 ?
		           uiRenderer.getSamplerTrilinear() : uiRenderer.getSamplerBilinear();
	}
	_boundingRect.setMinMax(glm::vec3(width * -.5f, height * -.5f, 0.0f),
	                        glm::vec3(width * .5f, height * .5f, 0.0f));
	_texDescSet = uiRenderer.getDescriptorPool()->allocateDescriptorSet(uiRenderer.getTexDescriptorSetLayout());
}

bool Font_::loadFontData(const Texture& texture)
{
	const TextureHeader& texHeader = texture;
	_dim.x = texHeader.getWidth();
	_dim.y = texHeader.getHeight();

	const Header* header = reinterpret_cast<const Header*>(texture.getMetaDataMap()->at(TextureHeader::Header::PVRv3)
	                       .at(static_cast<uint32_t>(FontHeader)).getData());
	assertion(header != NULL);

	_header = *header;
	_header.numCharacters = _header.numCharacters & 0xFFFF;
	_header.numKerningPairs = _header.numKerningPairs & 0xFFFF;

	const std::map<uint32_t, TextureMetaData>& metaDataMap = texture.getMetaDataMap()->at(TextureHeader::Header::PVRv3);
	std::map<uint32_t, TextureMetaData>::const_iterator found;

	if (_header.numCharacters)
	{
		_characters.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontCharList));

		if (found != metaDataMap.end())
		{
			memcpy(&_characters[0], found->second.getData(), found->second.getDataSize());
		}

		_yOffsets.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontYoffset));

		if (found != metaDataMap.end())
		{
			memcpy(&_yOffsets[0], found->second.getData(), found->second.getDataSize());
		}

		_charMetrics.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontMetrics));

		if (found != metaDataMap.end())
		{
			memcpy(&_charMetrics[0], found->second.getData(), found->second.getDataSize());
		}

		_rects.resize(_header.numCharacters);
		found = metaDataMap.find(static_cast<uint32_t>(FontRects));

		if (found != metaDataMap.end())
		{
			memcpy(&_rects[0], found->second.getData(), found->second.getDataSize());
		}

		// Build UVs
		_characterUVs.resize(_header.numCharacters);
		for (int16_t uiChar = 0; uiChar < _header.numCharacters; uiChar++)
		{
			_characterUVs[uiChar].ul = _rects[uiChar].offset.x / static_cast<float>(_dim.x);
			_characterUVs[uiChar].ur = _characterUVs[uiChar].ul + _rects[uiChar].extent.width / static_cast<float>(_dim.x);
			_characterUVs[uiChar].vt = _rects[uiChar].offset.y / static_cast<float>(_dim.y);
			_characterUVs[uiChar].vb = _characterUVs[uiChar].vt + _rects[uiChar].extent.height / static_cast<float>(_dim.y);
		}
	}

	if (_header.numKerningPairs)
	{
		found = metaDataMap.find(static_cast<uint32_t>(FontKerning));
		_kerningPairs.resize(_header.numKerningPairs);

		if (found != metaDataMap.end())
		{
			memcpy(&_kerningPairs[0], found->second.getData(), found->second.getDataSize());
		}
	}
	return true;
}

uint32_t Font_::findCharacter(uint32_t character) const
{
	uint32_t* item = reinterpret_cast<uint32_t*>(bsearch(&character, &_characters[0],
	                 _characters.size(), sizeof(uint32_t), characterCompFunc));

	if (!item) { return static_cast<uint32_t>(InvalidChar); }

	uint32_t index = static_cast<uint32_t>(item - &_characters[0]);
	return index;
}

void Font_::applyKerning(uint32_t charA, uint32_t charB, float& offset)
{
	if (_kerningPairs.size())
	{
		uint64_t uiPairToSearch = (static_cast<uint64_t>(charA) << 32) | static_cast<uint64_t>(charB);
		KerningPair* pItem = (KerningPair*)bsearch(&uiPairToSearch, &_kerningPairs[0], _kerningPairs.size(), sizeof(KerningPair),
		                     kerningCompFunc);

		if (pItem) { offset += static_cast<float>(pItem->offset); }
	}
}

int32_t Font_::characterCompFunc(const void* a, const void* b)
{
	return (*static_cast<const int32_t*>(a) - * static_cast<const int32_t*>(b));
}

int32_t Font_::kerningCompFunc(const void* a, const void* b)
{
	KerningPair* pPairA = (KerningPair*)a;
	KerningPair* pPairB = (KerningPair*)b;

	if (pPairA->pair > pPairB->pair) { return 1; }

	if (pPairA->pair < pPairB->pair) { return -1; }

	return 0;
}

bool Font_::init(UIRenderer& uiRenderer, const ImageView& tex2D, const TextureHeader& tex,
                 const Sampler& sampler)
{
	setUIRenderer(&uiRenderer);
	_tex = tex2D;
	loadFontData(tex);
	if (tex.getPixelFormat().getNumChannels() == 1 &&
	    tex.getPixelFormat().getChannelContent(0) == 'a')
	{
		_alphaRenderingMode = true;
	}
	_texDescSet = uiRenderer.getDescriptorPool()->allocateDescriptorSet(uiRenderer.getTexDescriptorSetLayout());

	if (!_texDescSet.isValid())
	{
		Log("Failed to create descriptor set for Image sprite");
		return false;
	}
	// update the texture descriptor set
	WriteDescriptorSet writeDescSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _texDescSet, 0, 0);
	writeDescSet.setImageInfo(0, DescriptorImageInfo(_tex, (sampler.isValid() ? sampler : uiRenderer.getSamplerBilinear())));
	uiRenderer.getDevice()->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
	return true;
}

uint32_t TextElement_::updateVertices(float fZPos, float xPos, float yPos,
                                      const std::vector<uint32_t>& text, Vertex* const pVertices) const
{
	if (pVertices == NULL || text.empty()) { return 0; }
	_boundingRect.clear();
	/* Nothing to update */

	Font tmp = _font; Font_& font = *tmp;

	yPos -= font.getAscent();

	yPos = glm::round<float>(yPos);

	float preXPos = xPos;   // The original offset (after screen scale modification) of the X coordinate.

	float   kernOffset;
	float   fAOff;
	float   fYOffset;
	uint32_t    vertexCount = 0;
	int32_t   nextChar;

	size_t numCharsInString = text.size();

	for (size_t index = 0; index < numCharsInString; index++)
	{
		if (index > MaxLetters) { break; }

		// Newline
		if (text[index] == 0x0A)
		{
			xPos = preXPos;
			yPos -= glm::round<float>(static_cast<float>(font.getFontLineSpacing()));
			continue;
		}

		// Get the character
		uint32_t charIndex = font.findCharacter(text[index]);

		// No character found. Add a space.
		if (charIndex == Font_::InvalidChar)
		{
			xPos += glm::round<float>(static_cast<float>(font.getSpaceWidth()));
			continue;
		}

		kernOffset = 0;
		fYOffset = static_cast<float>(font.getYOffset(charIndex));
		// The A offset. Could include overhang or underhang.
		fAOff = glm::round<float>(static_cast<float>(font.getCharMetrics(charIndex).xOff));

		if (index < numCharsInString - 1)
		{
			nextChar = text[index + 1];
			font.applyKerning(text[index], nextChar, kernOffset);
		}

		const Font_::CharacterUV& charUV = font.getCharacterUV(charIndex);
		/* Filling vertex data */
		pVertices[vertexCount + 0].x = (xPos + fAOff);
		pVertices[vertexCount + 0].y = (yPos + fYOffset);
		pVertices[vertexCount + 0].z = (fZPos);
		pVertices[vertexCount + 0].rhw = (1.0f);
		pVertices[vertexCount + 0].tu = (charUV.ul);
		pVertices[vertexCount + 0].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 0].x, pVertices[vertexCount + 0].y, 0.0f);

		pVertices[vertexCount + 1].x = (xPos + fAOff + glm::round<float>(static_cast<float>(
		                                  font.getRectangle(charIndex).extent.width)));
		pVertices[vertexCount + 1].y = (yPos + fYOffset);
		pVertices[vertexCount + 1].z = (fZPos);
		pVertices[vertexCount + 1].rhw = (1.0f);
		pVertices[vertexCount + 1].tu = (charUV.ur);
		pVertices[vertexCount + 1].tv = (charUV.vt);
		_boundingRect.add(pVertices[vertexCount + 1].x, pVertices[vertexCount + 1].y, 0.0f);

		pVertices[vertexCount + 2].x = (xPos + fAOff);
		pVertices[vertexCount + 2].y = (yPos + fYOffset - glm::round<float>(static_cast<float>(font.getRectangle(charIndex).extent.height)));
		pVertices[vertexCount + 2].z = (fZPos);
		pVertices[vertexCount + 2].rhw = (1.0f);
		pVertices[vertexCount + 2].tu = (charUV.ul);
		pVertices[vertexCount + 2].tv = (charUV.vb);
		_boundingRect.add(pVertices[vertexCount + 2].x, pVertices[vertexCount + 2].y, 0.0f);

		pVertices[vertexCount + 3].x = (xPos + fAOff + glm::round<float>(static_cast<float>(font.getRectangle(charIndex).extent.width)));
		pVertices[vertexCount + 3].y = (yPos + fYOffset - round(static_cast<float>(font.getRectangle(charIndex).extent.height)));
		pVertices[vertexCount + 3].z = (fZPos);
		pVertices[vertexCount + 3].rhw = (1.0f);
		pVertices[vertexCount + 3].tu = (charUV.ur);
		pVertices[vertexCount + 3].tv = (charUV.vb);
		_boundingRect.add(pVertices[vertexCount + 3].x, pVertices[vertexCount + 3].y, 0.0f);

		// Add on this characters width
		xPos = xPos + glm::round<float>(static_cast<float>(font.getCharMetrics(charIndex).characterWidth + kernOffset) /** renderParam.scale.x*/);
		vertexCount += 4;
	}
	return vertexCount;
}

bool Text_::init()
{
	if (_materialData.bufferArrayId == -1)
	{
		_materialData.bufferArrayId = _uiRenderer->getMaterial().getNewBufferArray();
		if (_materialData.bufferArrayId == -1)
		{
			Log(LogLevel::Debug, "Failed to create Text. Reached maximum material supported by this UIRenderer");
			return false;
		}
	}
	return onAddInstance(0);
}

bool Text_::onAddInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId == -1)
	{
		if ((_mvpData[parentId].bufferArrayId = _uiRenderer->getUbo().getNewBufferSlice()) == -1)
		{
			Log(LogLevel::Debug, "Failed to create Text. Reached maximum instancing supported by this UIRenderer");
			return false;
		}
	}
	return true;
}

void TextElement_::regenerateText() const
{
	_utf32.clear();
	if (_isUtf8)
	{
		utils::UnicodeConverter::convertUTF8ToUTF32(reinterpret_cast<const utf8*>(_textStr.c_str()), _utf32);
	}
	else
	{
		if (sizeof(wchar_t) == 2 && _textWStr.length())
		{
			utils::UnicodeConverter::convertUTF16ToUTF32((const utf16*)_textWStr.c_str(), _utf32);
		}
		else if (_textWStr.length()) //if (sizeof(wchar_t) == 4)
		{
			_utf32.resize(_textWStr.size());
			memcpy(&_utf32[0], &_textWStr[0], _textWStr.size() * sizeof(_textWStr[0]));
		}
	}

	_vertices.clear();
	if (_vertices.size() < (_utf32.size() * 4)) { _vertices.resize(_utf32.size() * 4); }

	_numCachedVerts = updateVertices(0.0f, 0.f, 0.f, _utf32, _vertices.size() ? &_vertices[0] : 0);
	assertion((_numCachedVerts % 4) == 0);
	assertion((_numCachedVerts / 4) < MaxLetters);
	_isTextDirty = false;
}


void TextElement_::updateVbo()const
{
	if (_vertices.size())
	{
		if (_vbo.isNull() || _vbo->getSize() < sizeof(Vertex) * _vertices.size())
		{
			_vbo = utils::createBuffer(_uiRenderer->getDevice(), static_cast<uint32_t>(sizeof(Vertex) * _vertices.size()),
			                           VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
		}
		Device deviceTemp = _uiRenderer->getDevice()->getReference();
		pvr::utils::updateBuffer(deviceTemp, _vbo, _vertices.data(), 0,
		                         static_cast<uint32_t>(sizeof(Vertex) * _vertices.size()), true);
	}
}

void TextElement_::onRender(CommandBufferBase& commands) const
{
	if (_vbo.isValid())
	{
		commands->bindVertexBuffer(_vbo, 0, 0);
		commands->bindIndexBuffer(_uiRenderer->getFontIbo(), 0, VkIndexType::e_UINT16);
		commands->drawIndexed(0, (glm::min<int32_t>(_numCachedVerts, 0xFFFC) >> 1) * 3, 0, 0, 1);
	}
}

void Text_::calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
                         Rect2Di const& viewport)const
{
	_text->updateText();
	math::AxisAlignedBox lastBox = _boundingRect;
	_boundingRect = _text->getBoundingBox();
	if (_isPositioningDirty || _boundingRect != lastBox)
	{
		vec2 offset;

		switch (_anchor)
		{
		case Anchor::Center:    offset = vec2(_boundingRect.center()); break;
		case Anchor::TopLeft:   offset = vec2(_boundingRect.topLeftNear()); break;
		case Anchor::TopCenter:   offset = vec2(_boundingRect.topCenterNear()); break;
		case Anchor::TopRight:    offset = vec2(_boundingRect.topRightNear()); break;
		case Anchor::BottomLeft:  offset = vec2(_boundingRect.bottomLeftNear()); break;
		case Anchor::BottomCenter:  offset = vec2(_boundingRect.bottomCenterNear()); break;
		case Anchor::BottomRight: offset = vec2(_boundingRect.bottomRightNear()); break;
		case Anchor::CenterLeft:  offset = vec2(_boundingRect.centerLeftNear()); break;
		case Anchor::CenterRight: offset = vec2(_boundingRect.centerRightNear()); break;
		}

		_cachedMatrix = glm::mat4(1.f);

		//_matrix = translate(vec3(pos, 0.f));  //5: Finally, move it to its position
		//ASSUMING IDENTITY MATRIX! - OPTIMIZE OUT THE OPS

		//4: Bring to Pixel (screen) coordinates.
		//BECAUSE _matrix IS A PURE ROTATION, WE CAN OPTIMISE THE 1st SCALING OP.
		//THIS IS : _matrix = scale(_matrix, toScreenCoordinates);

		_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, vec3(0.f, 0.f, 1.f)); //3: rotate it
		_cachedMatrix = glm::scale(_cachedMatrix, vec3(_scale, 1.f)); //2: Scale
		_cachedMatrix = glm::translate(_cachedMatrix, vec3(-offset, 0.f)); //1: Anchor the text properly
		_isPositioningDirty = false;
	}

	glm::vec2 tmpPos;
	tmpPos.x = _position.x * viewport.extent.width * .5f + viewport.extent.width * .5f;
	tmpPos.y = _position.y * viewport.extent.height * .5f + viewport.extent.height * .5f;

	tmpPos.x += viewport.offset.x + _pixelOffset.x;
	tmpPos.y += viewport.offset.y + _pixelOffset.y;

	_mvpData[parentIds].mvp =  viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * _cachedMatrix;
	updateUbo(parentIds);
}

void Text_::updateUbo(uint64_t parentIds)const
{
	// update the descriptor set once
	debug_assertion(_mvpData[parentIds].bufferArrayId != -1, "Invalid MVP Buffer ID");
	debug_assertion(_materialData.bufferArrayId != -1, "Invalid Material Buffer ID");
	_uiRenderer->getUbo().updateMvp(_mvpData[parentIds].bufferArrayId, _mvpData[parentIds].mvp);
	_uiRenderer->getMaterial().updateMaterial(_materialData.bufferArrayId, _color, 1, glm::mat4(1.f));
}

void Text_::onRender(CommandBufferBase& commandBuffer, uint64_t parentId) const
{
	updateUbo(parentId);
	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 _uiRenderer->getPipelineLayout(), 0, getTexDescriptorSet(), nullptr, 0);

	_uiRenderer->getUbo().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _mvpData[parentId].bufferArrayId);
	_uiRenderer->getMaterial().bindUboDynamic(commandBuffer, _uiRenderer->getPipelineLayout(), _materialData.bufferArrayId);

	_text->onRender(commandBuffer);
}

void Text_::onRemoveInstance(uint64_t parentId)
{
	if (_mvpData[parentId].bufferArrayId != -1)
	{
		_uiRenderer->getUbo().releaseBufferSlice(_mvpData[parentId].bufferArrayId);
		_uiRenderer->getMaterial().releaseBufferArray(_materialData.bufferArrayId);
		_mvpData[parentId].bufferArrayId = -1;
		_materialData.bufferArrayId = -1;
	}
}

Text_::Text_(UIRenderer& uiRenderer, const TextElement& text) : Sprite_(uiRenderer), _text(text)
{
	_alphaMode = text->getFont()->isAlphaRendering();
}

/// <summary>You must always submit your outstanding operations to a texture before calling setText. Because
/// setText will edit the content of VBOs and similar, these must be submitted before changing the text. To avoid
/// that, prefer using more Text objects.</summary>
TextElement_& TextElement_::setText(const std::string& str)
{
	_isTextDirty = true;
	_isUtf8 = true;
	_textStr.assign(str);
	// check if need reallocation
	return *this;
}

TextElement_& TextElement_::setText(const std::wstring& str)
{
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	_textWStr = str;
	return *this;
	// check if need reallocation
}

TextElement_& TextElement_::setText(std::string&& str)
{
	_isTextDirty = true;
	_isUtf8 = true;
	_textWStr.clear();
	_textStr = std::move(str);
	return *this;
}

TextElement_& TextElement_::setText(std::wstring&& str)
{
	_isTextDirty = true;
	_isUtf8 = false;
	_textStr.clear();
	_textWStr = std::move(str);
	return *this;
}

MatrixGroup_::MatrixGroup_(UIRenderer& uiRenderer, uint64_t id) : Group_(uiRenderer, id) {}

void MatrixGroup_::commitUpdates() const
{
	calculateMvp(0, glm::mat4(1.f), _uiRenderer->getScreenRotation() * _viewProj, _uiRenderer->getViewport());
}

void PixelGroup_::calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj,
                               Rect2Di const& viewport) const
{
	vec2 offset(_boundingRect.center());// offset the default center anchor point.

	switch (_anchor)
	{
	case Anchor::Center:                                break;
	case Anchor::TopLeft:   offset = glm::vec2(_boundingRect.topLeftNear());    break;
	case Anchor::TopCenter:   offset = glm::vec2(_boundingRect.topCenterNear());    break;
	case Anchor::TopRight:    offset = glm::vec2(_boundingRect.topRightNear());   break;
	case Anchor::BottomLeft:  offset = glm::vec2(_boundingRect.bottomLeftNear()); break;
	case Anchor::BottomCenter:  offset = glm::vec2(_boundingRect.bottomCenterNear()); break;
	case Anchor::BottomRight: offset = glm::vec2(_boundingRect.bottomRightNear());  break;
	case Anchor::CenterLeft:  offset = glm::vec2(_boundingRect.centerLeftNear()); break;
	case Anchor::CenterRight: offset = glm::vec2(_boundingRect.centerRightNear());  break;
	}

	memset(glm::value_ptr(_cachedMatrix), 0, sizeof(_cachedMatrix));
	_cachedMatrix[0][0] = 1.f;
	_cachedMatrix[1][1] = 1.f;
	_cachedMatrix[2][2] = 1.f; //Does not really matter - we don't have width...
	_cachedMatrix[3][3] = 1.f;

	//*** READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK
	//- translate the anchor to the origin
	//- do the scale and then the rotation around the anchor
	//- do the final translation
	glm::vec2 tmpPos;
	// tranform from ndc to screen space
	tmpPos.x = static_cast<float>(math::ndcToPixel(_position.x, viewport.extent.width));
	tmpPos.y = static_cast<float>(math::ndcToPixel(_position.y, viewport.extent.height));
	// add the final pixel offset
	tmpPos.x += static_cast<float>(_pixelOffset.x) + static_cast<float>(viewport.offset.x);
	tmpPos.y += static_cast<float>(_pixelOffset.y) + static_cast<float>(viewport.offset.y);

	_cachedMatrix[3][0] = tmpPos.x;
	_cachedMatrix[3][1] = tmpPos.y;

	_cachedMatrix = glm::rotate(_cachedMatrix, _rotation, glm::vec3(0.f, 0.f, 1.f));
	_cachedMatrix = glm::scale(_cachedMatrix, glm::vec3(_scale, 1.f));
	_cachedMatrix = glm::translate(_cachedMatrix, glm::vec3(-offset, 0.0f));

	glm::mat4 tmpMatrix = srt * _cachedMatrix;
	//My cached matrix should always be up-to-date unless overridden. No effect.
	for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it)
	{
		(*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj,
		                    Rect2Di(0, 0, static_cast<int32_t>(_boundingRect.getSize().x), static_cast<int32_t>(_boundingRect.getSize().y)));
	}
}

Group_* Group_::add(const Sprite& sprite)
{
	_children.push_back(sprite);
	_boundingRect.add(sprite->getDimensions().x, sprite->getDimensions().y, 0.0f);
	if (!_children.back()->onAddInstance(_id))
	{
		_children.pop_back();
	}
	return this;
}

}// namespace impl
}// namespace ui
}// namespace pvr
//!\endcond
