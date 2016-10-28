/*!*********************************************************************************************************************
\file         PVRUIRenderer\Sprite.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains implementations of functions for the Sprite_ class and subclasses (Sprite_, Text_, Image_, Font_,
              MatrixGroup_)
***********************************************************************************************************************/
#include "PVRUIRenderer/Sprite.h"
#include "PVRUIRenderer/UIRenderer.h"
using namespace glm;

namespace pvr {
namespace ui {
namespace impl {
struct UboData
{
	glm::mat4 mvp;
	glm::mat4 uv;
	glm::vec4 color;
	bool	  alphaMode;

	static const std::pair<StringHash, types::GpuDatatypes::Enum> EntryNames[4];
	enum Entry { MVP, UV, Color, AlphaMode, Count };
};

const std::pair<StringHash, types::GpuDatatypes::Enum> UboData::EntryNames[] =
{
	std::pair<StringHash, types::GpuDatatypes::Enum>("mvp", types::GpuDatatypes::mat4x4),
	std::pair<StringHash, types::GpuDatatypes::Enum>("uv", types::GpuDatatypes::mat4x4),
	std::pair<StringHash, types::GpuDatatypes::Enum>("color", types::GpuDatatypes::vec4),
	std::pair<StringHash, types::GpuDatatypes::Enum>("alphaMode", types::GpuDatatypes::integer),
};

Sprite_::Sprite_(UIRenderer& uiRenderer) :
	m_color(1.f, 1.f, 1.f, 1.f),
	m_alphaMode(false),
	m_uiRenderer(uiRenderer)
{
	m_boundingRect.clear();
}

void Sprite_::commitUpdates()const
{
	calculateMvp(0, glm::mat4(1.f), m_uiRenderer.getScreenRotation() * m_uiRenderer.getProjection(), m_uiRenderer.getViewport());
}

void Sprite_::render() const
{
	if (!m_uiRenderer.isRendering())
	{
		Log(Log.Error, "Sprite: Render called without first calling uiRenderer::begin to set up the commandbuffer.");
		return;
	}
	onRender(m_uiRenderer.getActiveCommandBuffer(), 0);
}

void Image_::writeUboDescriptorSet(pvr::uint64 /*parentId*/)const
{
	// update the ubo descriptor set
}

void Image_::updateUbo(pvr::uint64 parentIds)const
{
	if ((m_uiRenderer.getContext()->getApiType() > pvr::Api::OpenGLESMaxVersion))
	{
		if (m_mvpPools[parentIds].bufferView.getConnectedBuffer(0).isNull())
		{
			m_mvpPools[parentIds].bufferView.setupArray(m_uiRenderer.getContext(), 1, types::BufferViewTypes::UniformBuffer);
			m_mvpPools[parentIds].bufferView.addEntriesPacked(UboData::EntryNames, UboData::Entry::Count);

			m_mvpPools[parentIds].bufferView
			.connectWithBuffer(0, m_uiRenderer.getContext()->createBufferAndView(m_mvpPools[parentIds].bufferView.getAlignedElementSize(),
			                   pvr::types::BufferBindingUse::UniformBuffer, true),
			                   pvr::types::BufferViewTypes::UniformBuffer);
		}
		// update the ubo

		m_mvpPools[parentIds].bufferView.map(0, types::MapBufferFlags::Write, 0);
		m_mvpPools[parentIds].bufferView
		.setValue(UboData::MVP, m_mvpPools[parentIds].mvp)
		.setValue(UboData::Color, m_color)
		.setValue(UboData::AlphaMode, m_alphaMode);
		glm::vec3 scale(m_uv.getDimension().x, m_uv.getDimension().y, 1.0f);
		glm::mat4 uvTrans = glm::translate(glm::vec3(m_uv.x, m_uv.y, 0.0f)) *  glm::scale(scale);
		m_mvpPools[parentIds].bufferView.setValue(UboData::UV, uvTrans);
		m_mvpPools[parentIds].bufferView.unmap(0);

		if (m_mvpPools[parentIds].uboDescSet.isNull())
		{
			m_mvpPools[parentIds].uboDescSet =
			  m_uiRenderer.getDescriptorPool()->allocateDescriptorSet(m_uiRenderer.getUboDescSetLayout());
			m_mvpPools[parentIds].uboDescSet->update(api::DescriptorSetUpdate().setUbo(0, m_mvpPools[parentIds].bufferView.getConnectedBuffer(0)));
		}
	}
}

pvr::Result Image_::updateTextureDescriptorSet()const
{
	if (!m_texDescSet.isValid())
	{
		pvr::Log("Failed to create descriptor set for Image sprite");
		return pvr::Result::UnknownError;
	}
	// update the texture descriptor set
	if (m_isTextureDirty)
	{
		pvr::api::DescriptorSetUpdate descSetCreateParam;
		descSetCreateParam.setCombinedImageSampler(0, getTexture(), getSampler());
		m_texDescSet->update(descSetCreateParam);
		m_isTextureDirty = false;
	}
	return pvr::Result::Success;
}

void Image_::calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
                          pvr::Rectanglei const& viewport)const
{
	if (m_isPositioningDirty)
	{
		vec2 offset(0.0f);// offset the default center anchor point.

		switch (m_anchor)
		{
		case Anchor::Center:									break;
		case Anchor::TopLeft:		offset = vec2(-1.f, 1.f);	break;
		case Anchor::TopCenter:		offset = vec2(0.0f, 1.f);	break;
		case Anchor::TopRight:		offset = vec2(1.f, 1.f);	break;
		case Anchor::BottomLeft:	offset = vec2(-1.f, -1.f);	break;
		case Anchor::BottomCenter:	offset = vec2(0.0f, -1.f);	break;
		case Anchor::BottomRight:	offset = vec2(1.f, -1.f);	break;
		case Anchor::CenterLeft:	offset = vec2(-1.f, 0.0f);	break;
		case Anchor::CenterRight:	offset = vec2(1.f, 0.0f);	break;
		}

		//	offset = glm::vec2(0.0f);
		memset(glm::value_ptr(m_cachedMatrix), 0, sizeof(m_cachedMatrix));
		//m_matrix[0][0] = Will be set in Scale
		//m_matrix[1][1] = Will be set in Scale
		m_cachedMatrix[2][2] = 1.f; //Does not really matter - we don't have width...
		m_cachedMatrix[3][3] = 1.f;
		//READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK

		//4: Transform to SCREEN coordinates...
		//BECAUSE m_cachedMatrix IS A PURE ROTATION, WE CAN OPTIMISE THE 1st SCALING OP.
		//THIS IS : m_matrix = scale(m_matrix, toScreenCoordinates);
		m_cachedMatrix[0][0] = 1.f;
		m_cachedMatrix[1][1] = 1.f;

		//3: Rotate...
		m_cachedMatrix = rotate(m_cachedMatrix, m_rotation, vec3(0.f, 0.f, 1.f));

		//2: Scale...
		m_cachedMatrix = scale(m_cachedMatrix, vec3(m_scale.x * getWidth() * .5f, m_scale.y * getHeight() * .5f, 1.f));

		//1: Apply the offsetting (i.e. place the center at its correct spot. THIS IS NOT THE SCREEN POSITIONING, only the anchor.)
		m_cachedMatrix = translate(m_cachedMatrix, vec3(-offset, 0.0f));
	}

	glm::vec2 tmpPos;
	//5: Translate (screen coords)
	tmpPos.x = m_position.x * viewport.getDimension().x * .5f + viewport.getDimension().x * .5f + viewport.x + m_pixelOffset.x;
	tmpPos.y = m_position.y * viewport.getDimension().y * .5f + viewport.getDimension().y * .5f + viewport.y + m_pixelOffset.y;
	m_mvpPools[parentIds].mvp = viewProj  *  srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * m_cachedMatrix;

	updateUbo(parentIds);
}

void Image_::onRender(api::CommandBufferBase& commandBuffer, pvr::uint64 parentId) const
{
	commandBuffer->bindDescriptorSet(m_uiRenderer.getPipelineLayout(), 0, getTexDescriptorSet(), NULL, 0);
	if (m_uiRenderer.getContext()->getApiType() <= pvr::Api::OpenGLESMaxVersion)
	{
		commandBuffer->setUniformPtr<mat4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformMVPmtx], 1, &m_mvpPools[parentId].mvp);
		commandBuffer->setUniformPtr<vec4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformColor], 1, &m_color);
		commandBuffer->setUniformPtr<int32>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformAlphaMode], 1, &m_alphaMode);
		glm::vec3 scale(m_uv.getDimension().x, m_uv.getDimension().y, 1.0f);
		glm::mat4 uvTrans = glm::translate(glm::vec3(m_uv.x, m_uv.y, 0.0f)) *  glm::scale(scale);
		commandBuffer->setUniform<glm::mat4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformUVmtx], uvTrans);
	}
	else
	{
		commandBuffer->bindDescriptorSet(m_uiRenderer.getPipelineLayout(), 1, m_mvpPools[parentId].uboDescSet, NULL, 0);
	}
	commandBuffer->bindVertexBuffer(m_uiRenderer.getImageVbo(), 0, 0);
	commandBuffer->drawArrays(0, 6);
}

Image_::Image_(UIRenderer& uiRenderer, api::TextureView& tex, uint32 width, uint32 height) :
	Sprite_(uiRenderer), m_texW(width), m_texH(height),  m_texture(tex),
	m_isTextureDirty(true)
{
	m_boundingRect.setMinMax(glm::vec3(width * -.5f, height * -.5f, 0.0f), glm::vec3(width * .5f, height * .5f, 0.0f));
	m_texDescSet = uiRenderer.getDescriptorPool()->allocateDescriptorSet(uiRenderer.getTexDescriptorSetLayout());
}

bool Font_::loadFontData(const assets::Texture& texture)
{
	const assets::TextureHeader& texHeader = texture.getHeader();
	m_texW = texHeader.getWidth();
	m_texH = texHeader.getHeight();

	const Header* header = reinterpret_cast<const Header*>(texture.getMetaDataMap()->at(assets::TextureHeader::Header::PVRv3)
	                       .at(FontHeader).getData());
	assertion(header != NULL);

	if (header->version != UIRenderer::getEngineVersion()) { return false; }

	m_header = *header;
	m_header.numCharacters = m_header.numCharacters & 0xFFFF;
	m_header.numKerningPairs = m_header.numKerningPairs & 0xFFFF;

	const std::map<uint32, assets::TextureMetaData>& metaDataMap = texture.getMetaDataMap()->at(assets::TextureHeader::Header::PVRv3);
	std::map<uint32, assets::TextureMetaData>::const_iterator found;

	if (m_header.numCharacters)
	{
		m_characters.resize(m_header.numCharacters);
		found = metaDataMap.find(FontCharList);

		if (found != metaDataMap.end())
		{
			memcpy(&m_characters[0], found->second.getData(), found->second.getDataSize());
		}

		m_yOffsets.resize(m_header.numCharacters);
		found = metaDataMap.find(FontYoffset);

		if (found != metaDataMap.end())
		{
			memcpy(&m_yOffsets[0], found->second.getData(), found->second.getDataSize());
		}

		m_charMetrics.resize(m_header.numCharacters);
		found = metaDataMap.find(FontMetrics);

		if (found != metaDataMap.end())
		{
			memcpy(&m_charMetrics[0], found->second.getData(), found->second.getDataSize());
		}

		m_rects.resize(m_header.numCharacters);
		found = metaDataMap.find(FontRects);

		if (found != metaDataMap.end())
		{
			memcpy(&m_rects[0], found->second.getData(), found->second.getDataSize());
		}

		// Build UVs
		m_characterUVs.resize(m_header.numCharacters);
		for (int16 uiChar = 0; uiChar < m_header.numCharacters; uiChar++)
		{
			m_characterUVs[uiChar].ul = m_rects[uiChar].x / (float32)m_texW;
			m_characterUVs[uiChar].ur = m_characterUVs[uiChar].ul + m_rects[uiChar].width / (float32)m_texW;
			m_characterUVs[uiChar].vt = m_rects[uiChar].y / (float32)m_texH;
			m_characterUVs[uiChar].vb = m_characterUVs[uiChar].vt + m_rects[uiChar].height / (float32)m_texH;
		}
	}

	if (m_header.numKerningPairs)
	{
		found = metaDataMap.find(FontKerning);
		m_kerningPairs.resize(m_header.numKerningPairs);

		if (found != metaDataMap.end())
		{
			memcpy(&m_kerningPairs[0], found->second.getData(), found->second.getDataSize());
		}
	}
	return true;
}

uint32 Font_::findCharacter(uint32 character) const
{
	uint32* item = reinterpret_cast<uint32*>(bsearch(&character, &m_characters[0],
	               m_characters.size(),
	               sizeof(uint32),
	               characterCompFunc));

	if (!item) { return InvalidChar; }

	uint32 index = static_cast<uint32>(item - &m_characters[0]);
	return index;
}

void Font_::applyKerning(uint32 charA, uint32 charB, float32& offset)
{
	if (m_kerningPairs.size())
	{
		uint64 uiPairToSearch = ((uint64)charA << 32) | (uint64)charB;
		KerningPair* pItem = (KerningPair*)bsearch(&uiPairToSearch, &m_kerningPairs[0], m_kerningPairs.size(), sizeof(KerningPair),
		                     kerningCompFunc);

		if (pItem) { offset += (float32)pItem->offset; }
	}
}

int32 Font_::characterCompFunc(const void* a, const void* b)
{
	return (*(int32*)a - * (int32*)b);
}

int32 Font_::kerningCompFunc(const void* a, const void* b)
{
	KerningPair* pPairA = (KerningPair*)a;
	KerningPair* pPairB = (KerningPair*)b;

	if (pPairA->pair > pPairB->pair) { return 1; }

	if (pPairA->pair < pPairB->pair) { return -1; }

	return 0;
}

uint32 Text_::updateVertices(float32 fZPos, float32 xPos, float32 yPos, const std::vector<uint32>& text,
                             Vertex* const pVertices) const
{
	if (pVertices == NULL || text.empty()) { return 0; }
	m_boundingRect.clear();
	/* Nothing to update */

	Font tmp = m_font; Font_& font = *tmp;

	yPos -= font.getAscent();

	yPos = round<float32>(yPos);

	float32 preXPos = xPos;		// The original offset (after screen scale modification) of the X coordinate.

	float32		kernOffset;
	float32		fAOff;
	float32		fYOffset;
	uint32		vertexCount = 0;
	int32		nextChar;

	size_t numCharsInString = text.size();

	for (size_t index = 0; index < numCharsInString; index++)
	{
		if (index > MaxLetters) { break; }

		// Newline
		if (text[index] == 0x0A)
		{
			xPos = preXPos;
			yPos -= round<float32>((float32)font.getFontLineSpacing());
			continue;
		}

		// Get the character
		uint32 charIndex = font.findCharacter(text[index]);

		// No character found. Add a space.
		if (charIndex == Font_::InvalidChar)
		{
			xPos += round<float32>((float32)font.getSpaceWidth());
			continue;
		}

		kernOffset = 0;
		fYOffset = float32(font.getYOffset(charIndex));
		// The A offset. Could include overhang or underhang.
		fAOff = round<float32>((float32)font.getCharMetrics(charIndex).xOff);

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
		m_boundingRect.add(pVertices[vertexCount + 0].x, pVertices[vertexCount + 0].y, 0.0f);

		pVertices[vertexCount + 1].x = (xPos + fAOff + round<float32>((float32)font.getRectangle(charIndex).width));
		pVertices[vertexCount + 1].y = (yPos + fYOffset);
		pVertices[vertexCount + 1].z = (fZPos);
		pVertices[vertexCount + 1].rhw = (1.0f);
		pVertices[vertexCount + 1].tu = (charUV.ur);
		pVertices[vertexCount + 1].tv = (charUV.vt);
		m_boundingRect.add(pVertices[vertexCount + 1].x, pVertices[vertexCount + 1].y, 0.0f);

		pVertices[vertexCount + 2].x = (xPos + fAOff);
		pVertices[vertexCount + 2].y = (yPos + fYOffset - round<float32>((float32)font.getRectangle(charIndex).height));
		pVertices[vertexCount + 2].z = (fZPos);
		pVertices[vertexCount + 2].rhw = (1.0f);
		pVertices[vertexCount + 2].tu = (charUV.ul);
		pVertices[vertexCount + 2].tv = (charUV.vb);
		m_boundingRect.add(pVertices[vertexCount + 2].x, pVertices[vertexCount + 2].y, 0.0f);

		pVertices[vertexCount + 3].x = (xPos + fAOff + round<float32>((float32)font.getRectangle(charIndex).width));
		pVertices[vertexCount + 3].y = (yPos + fYOffset - round((float32)font.getRectangle(charIndex).height));
		pVertices[vertexCount + 3].z = (fZPos);
		pVertices[vertexCount + 3].rhw = (1.0f);
		pVertices[vertexCount + 3].tu = (charUV.ur);
		pVertices[vertexCount + 3].tv = (charUV.vb);
		m_boundingRect.add(pVertices[vertexCount + 3].x, pVertices[vertexCount + 3].y, 0.0f);

		// Add on this characters width
		xPos = xPos + round<float32>((float32)(font.getCharMetrics(charIndex).characterWidth + kernOffset) /** renderParam.scale.x*/);
		vertexCount += 4;
	}

	if (m_vbo.isNull() || m_vbo->getSize() < sizeof(Vertex)* vertexCount)
	{
		m_vbo = m_uiRenderer.getContext()->createBuffer(sizeof(Vertex) * vertexCount, types::BufferBindingUse::VertexBuffer, true);
	}
	m_vbo->update(pVertices, 0, m_vbo->getSize());
	return vertexCount;
}

void Text_::regenerateText() const
{

	m_utf32.clear();

	if (m_isUtf8)
	{
		utils::UnicodeConverter::convertUTF8ToUTF32(reinterpret_cast<const utf8*>(m_textStr.c_str()), m_utf32);
	}
	else
	{
		if (sizeof(wchar_t) == 2)
		{
			utils::UnicodeConverter::convertUTF16ToUTF32((const utf16*)m_textWStr.c_str(), m_utf32);
		}
		else //if (sizeof(wchar_t) == 4)
		{
			m_utf32.resize(m_textWStr.size());
			memcpy(&m_utf32[0], &m_textWStr[0], m_textWStr.size() * sizeof(m_textWStr[0]));
		}
	}

	m_vertices.clear();
	if (m_vertices.size() < (m_utf32.size() * 4)) { m_vertices.resize(m_utf32.size() * 4); }

	m_numCachedVerts = updateVertices(0.0f, 0.f, 0.f, m_utf32, m_vertices.size() ? &m_vertices[0] : 0);
	assertion((m_numCachedVerts % 4) == 0);
	assertion((m_numCachedVerts / 4) < MaxLetters);
	m_isTextDirty = false;
}

void Text_::calculateMvp(pvr::uint64 parentIds, glm::mat4 const& srt, const glm::mat4& viewProj,
                         pvr::Rectanglei const& viewport)const
{
	if (m_isTextDirty)
	{
		regenerateText();
		m_isPositioningDirty = true;
	}

	if (m_isPositioningDirty)
	{
		vec2 offset;

		switch (m_anchor)
		{
		case Anchor::Center:		offset = vec2(m_boundingRect.center()); break;
		case Anchor::TopLeft:		offset = vec2(m_boundingRect.topLeftNear()); break;
		case Anchor::TopCenter:		offset = vec2(m_boundingRect.topCenterNear()); break;
		case Anchor::TopRight:		offset = vec2(m_boundingRect.topRightNear()); break;
		case Anchor::BottomLeft:	offset = vec2(m_boundingRect.bottomLeftNear()); break;
		case Anchor::BottomCenter:	offset = vec2(m_boundingRect.bottomCenterNear()); break;
		case Anchor::BottomRight:	offset = vec2(m_boundingRect.bottomRightNear()); break;
		case Anchor::CenterLeft:	offset = vec2(m_boundingRect.centerLeftNear()); break;
		case Anchor::CenterRight:	offset = vec2(m_boundingRect.centerRightNear()); break;
		}

		m_cachedMatrix = glm::mat4(1.f);

		//m_matrix = translate(vec3(pos, 0.f));  //5: Finally, move it to its position
		//ASSUMING IDENTITY MATRIX! - OPTIMIZE OUT THE OPS

		//4: Bring to Pixel (screen) coordinates.
		//BECAUSE m_matrix IS A PURE ROTATION, WE CAN OPTIMISE THE 1st SCALING OP.
		//THIS IS : m_matrix = scale(m_matrix, toScreenCoordinates);

		m_cachedMatrix = glm::rotate(m_cachedMatrix, m_rotation, vec3(0.f, 0.f, 1.f)); //3: rotate it
		m_cachedMatrix = glm::scale(m_cachedMatrix, vec3(m_scale, 1.f)); //2: Scale
		m_cachedMatrix = glm::translate(m_cachedMatrix, vec3(-offset, 0.f)); //1: Anchor the text properly
		m_isPositioningDirty = false;
	}

	glm::vec2 tmpPos;
	tmpPos.x = m_position.x * viewport.getDimension().x * .5f + viewport.getDimension().x * .5f;
	tmpPos.y = m_position.y * viewport.getDimension().y * .5f + viewport.getDimension().y * .5f;

	tmpPos.x += viewport.x + m_pixelOffset.x;
	tmpPos.y += viewport.y + m_pixelOffset.y;

	m_mvpPools[parentIds].mvp =  viewProj * srt * glm::translate(glm::vec3(tmpPos, 0.0f)) * m_cachedMatrix;
	updateUbo(parentIds);
}

void Text_::updateUbo(pvr::uint64 parentIds)const
{
	// update the descriptor set once
	if (m_uiRenderer.getContext()->getApiType() > Api::OpenGLESMaxVersion)
	{
		utils::StructuredMemoryView& view = m_mvpPools[parentIds].bufferView;
		if (view.getConnectedBuffer(0).isNull())
		{

			view.setupArray(m_uiRenderer.getContext(), 1, types::BufferViewTypes::UniformBuffer);
			view.addEntriesPacked(UboData::EntryNames, UboData::Count);
			view.connectWithBuffer(0, m_uiRenderer.getContext()->createBufferAndView(view.getAlignedElementSize(),
			                       types::BufferBindingUse::UniformBuffer, true), types::BufferViewTypes::UniformBuffer);
		}
		view.map(0, types::MapBufferFlags::Write, 0);
		view.setValue(UboData::MVP, m_mvpPools[parentIds].mvp);
		view.setValue(UboData::Color, m_color);
		view.setValue(UboData::AlphaMode, 1);
		glm::mat4 uvTrans(1.f);
		view.setValue(UboData::UV, uvTrans);
		view.unmap(0);
		if (m_mvpPools[parentIds].uboDescSet.isNull())
		{
			m_mvpPools[parentIds].uboDescSet =
			  m_uiRenderer.getDescriptorPool()->allocateDescriptorSet(m_uiRenderer.getUboDescSetLayout());
			m_mvpPools[parentIds].uboDescSet->update(api::DescriptorSetUpdate().setUbo(0, m_mvpPools[parentIds].bufferView.getConnectedBuffer(0)));
		}
	}
}

void Text_::onRender(api::CommandBufferBase& commandBuffer, pvr::uint64 parentId) const
{
	if (!m_utf32.size()) { return; }
	updateUbo(parentId);
	commandBuffer->bindDescriptorSet(m_uiRenderer.getPipelineLayout(), 0, getTexDescriptorSet(), NULL, 0);
	if (m_uiRenderer.getContext()->getApiType() <= pvr::Api::OpenGLESMaxVersion)
	{
		commandBuffer->setUniformPtr<mat4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformMVPmtx], 1, &m_mvpPools[parentId].mvp);
		commandBuffer->setUniformPtr<vec4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformColor], 1, &m_color);
		commandBuffer->setUniformPtr<int32>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformAlphaMode], 1, (int32*)&m_alphaMode);
		commandBuffer->setUniform<mat4>(m_uiRenderer.getProgramData().uniforms[UIRenderer::ProgramData::UniformUVmtx], mat4(1.));
	}
	else
	{
		commandBuffer->bindDescriptorSet(m_uiRenderer.getPipelineLayout(), 1, m_mvpPools[parentId].uboDescSet);
	}
	commandBuffer->bindVertexBuffer(m_vbo, 0, 0);
	commandBuffer->bindIndexBuffer(m_uiRenderer.getFontIbo(), 0, types::IndexType::IndexType16Bit);
	commandBuffer->drawIndexed(0, (min<int32>(m_numCachedVerts, 0xFFFC) >> 1) * 3, 0, 0, 1);
}

Text_::Text_(UIRenderer& uiRenderer, const Font& font) : Sprite_(uiRenderer), m_isUtf8(true), m_font(font), m_isTextDirty(true)
{
	m_alphaMode = font->getAlphaRenderingMode();
}

Text_::Text_(UIRenderer& uiRenderer, const std::string& text, const Font& font) : Sprite_(uiRenderer), m_isUtf8(true), m_font(font), m_isTextDirty(true), m_textStr(text)
{
	m_alphaMode = font->getAlphaRenderingMode();
}

Text_::Text_(UIRenderer& uiRenderer, const std::wstring& text, const Font& font) : Sprite_(uiRenderer), m_isUtf8(false), m_font(font), m_isTextDirty(true),	m_textWStr(text)
{
	m_alphaMode = font->getAlphaRenderingMode();
}

#ifdef PVR_SUPPORT_MOVE_SEMANTICS
Text_::Text_(UIRenderer& uiRenderer, std::string&& text, const Font& font) : Sprite_(uiRenderer), m_isUtf8(true), m_font(font), m_isTextDirty(true), m_textStr(text)
{
	m_alphaMode = font->getAlphaRenderingMode();
}

Text_::Text_(UIRenderer& uiRenderer, std::wstring&& text, const Font& font) : Sprite_(uiRenderer), m_isUtf8(false), m_font(font), m_isTextDirty(true), m_textWStr(text)
{
	m_alphaMode = font->getAlphaRenderingMode();
}

#endif

/*!*****************************************************************************************************************
\brief You must always submit your outstanding operations to a texture before calling setText. Because setText will
edit the content of VBOs and similar, these must be submitted before changing the text.
To avoid that, prefer using more Text objects.
********************************************************************************************************************/
Text_& Text_::setText(const std::string& str)
{
	m_isTextDirty = true;
	m_isUtf8 = true;
	m_textStr.assign(str);
	// check if need reallocation
	return *this;
}

Text_& Text_::setText(const std::wstring& str)
{
	m_isTextDirty = true;
	m_isUtf8 = false;
	m_textStr.clear();
	m_textWStr = str;
	return *this;
	// check if need reallocation
}

#ifdef PVR_SUPPORT_MOVE_SEMANTICS
Text_& Text_::setText(std::string&& str)
{
	m_isTextDirty = true;
	m_isUtf8 = true;
	m_textWStr.clear();
	m_textStr = std::move(str);
	return *this;
}

Text_& Text_::setText(std::wstring&& str)
{
	m_isTextDirty = true;
	m_isUtf8 = false;
	m_textStr.clear();
	m_textWStr = std::move(str);
	return *this;
}
#endif

MatrixGroup_::MatrixGroup_(UIRenderer& uiRenderer, pvr::uint64 id) :
	Group_(uiRenderer, id) {}

void MatrixGroup_::commitUpdates() const
{
	calculateMvp(0, glm::mat4(1.f), m_uiRenderer.getScreenRotation() * m_viewProj, m_uiRenderer.getViewport());
}

void PixelGroup_::calculateMvp(pvr::uint64 parentIds, const glm::mat4& srt, const glm::mat4& viewProj,
                               pvr::Rectanglei const& viewport) const
{
	vec2 offset(m_boundingRect.center());// offset the default center anchor point.

	switch (m_anchor)
	{
	case Anchor::Center:																break;
	case Anchor::TopLeft:		offset = glm::vec2(m_boundingRect.topLeftNear());		break;
	case Anchor::TopCenter:		offset = glm::vec2(m_boundingRect.topCenterNear());		break;
	case Anchor::TopRight:		offset = glm::vec2(m_boundingRect.topRightNear());		break;
	case Anchor::BottomLeft:	offset = glm::vec2(m_boundingRect.bottomLeftNear());	break;
	case Anchor::BottomCenter:	offset = glm::vec2(m_boundingRect.bottomCenterNear());	break;
	case Anchor::BottomRight:	offset = glm::vec2(m_boundingRect.bottomRightNear());	break;
	case Anchor::CenterLeft:	offset = glm::vec2(m_boundingRect.centerLeftNear());	break;
	case Anchor::CenterRight:	offset = glm::vec2(m_boundingRect.centerRightNear());	break;
	}

	memset(glm::value_ptr(m_cachedMatrix), 0, sizeof(m_cachedMatrix));
	m_cachedMatrix[0][0] = 1.f;
	m_cachedMatrix[1][1] = 1.f;
	m_cachedMatrix[2][2] = 1.f; //Does not really matter - we don't have width...
	m_cachedMatrix[3][3] = 1.f;

	//*** READ THIS BOTTOM TO TOP DUE TO THE WAY THE OPTIMISED GLM FUNCTIONS WORK
	//- translate the anchor to the origin
	//- do the scale and then the rotation around the anchor
	//- do the final translation
	glm::vec2 tmpPos;
	// tranform from ndc to screen space
	tmpPos.x = (float32)math::ndcToPixel(m_position.x, viewport.getDimension().x);
	tmpPos.y = (float32)math::ndcToPixel(m_position.y, viewport.getDimension().y);
	// add the final pixel offset
	tmpPos.x += (float32)m_pixelOffset.x + (float32)viewport.x;
	tmpPos.y += (float32)m_pixelOffset.y + (float32)viewport.y;

	m_cachedMatrix[3][0] = tmpPos.x;
	m_cachedMatrix[3][1] = tmpPos.y;

	m_cachedMatrix = rotate(m_cachedMatrix, m_rotation, glm::vec3(0.f, 0.f, 1.f));
	m_cachedMatrix = scale(m_cachedMatrix, glm::vec3(m_scale, 1.f));
	m_cachedMatrix = translate(m_cachedMatrix, glm::vec3(-offset, 0.0f));

	glm::mat4 tmpMatrix = srt * m_cachedMatrix;
	//My cached matrix should always be up-to-date unless overridden. No effect.
	for (ChildContainer::iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		(*it)->calculateMvp(packId(parentIds, m_id), tmpMatrix, viewProj, Rectanglei(0, 0, (int32)m_boundingRect.getSize().x, (int32)m_boundingRect.getSize().y));
	}
}

}
}
}
