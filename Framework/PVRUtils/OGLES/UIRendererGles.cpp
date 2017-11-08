/*!
\brief Contains implementations of functions for the UIRendererGles class.
\file PVRUtils/OGLES/UIRendererGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "UIRendererGles.h"
#include "ErrorsGles.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVRUtils/OGLES/HelperGles.h"
#include "PVRUtils/ArialBoldFont.h"
#include "PVRUtils/PowerVRLogo.h"
#include "PVRUtils/OGLES/UIRendererShaders_ES.h"

using std::map;
using std::vector;
namespace pvr {
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);

// store the current GL state
void GLState::storeCurrentGlState(bool isEs2)
{
	debugLogApiError("glState::storeCurrentGlState Enter");

	gl::GetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
	gl::GetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);
	gl::GetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
	gl::GetIntegerv(GL_BLEND, &blendEnabled);
	gl::GetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRgb);
	gl::GetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
	gl::GetIntegerv(GL_BLEND_DST_RGB, &blendDstRgb);
	gl::GetIntegerv(GL_BLEND_DST_RGB, &blendDstAlpha);
	gl::GetIntegerv(GL_BLEND_EQUATION_RGB, &blendEqationRgb);
	gl::GetIntegerv(GL_BLEND_EQUATION_RGB, &blendEqationAlpha);
	gl::GetBooleanv(GL_COLOR_WRITEMASK, colorMask);
	gl::GetIntegerv(GL_DEPTH_TEST, &depthTest);
	gl::GetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
	gl::GetIntegerv(GL_STENCIL_TEST, &stencilTest);
	gl::GetIntegerv(GL_CULL_FACE, &cullingEnabled);
	gl::GetIntegerv(GL_CULL_FACE_MODE, &culling);
	gl::GetIntegerv(GL_FRONT_FACE, &windingOrder);
	gl::GetIntegerv(GL_SAMPLER_BINDING, &sampler7);
	gl::GetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
	gl::GetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibo);
	if (!isEs2)
	{
		gl::GetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	}
	else
	{
		gl::GetIntegerv(GL_VERTEX_ARRAY_BINDING_OES, &vao);
	}

	if (vao != 0)
	{
		if (!isEs2)
		{
			gl::BindVertexArray(0);
		}
		else
		{
			gl::ext::BindVertexArrayOES(0);
		}
	}

	for (uint32_t i = 0; i < 8; i++)
	{
		GLint enabled;
		gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
		vertexAttribArray[i] = enabled == 1 ? GL_TRUE : GL_FALSE;

		if (vertexAttribArray[i])
		{
#ifdef GL_VERTEX_ATTRIB_BINDING
			GLint attribBinding;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_BINDING, &attribBinding);
			vertexAttribBindings[i] = attribBinding;
#else
			vertexAttribBindings[i] = static_cast<uint32_t>(-1);
#endif

			GLint attribSize;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &attribSize);
			vertexAttribSizes[i] = attribSize;

			GLint attribType;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &attribType);
			vertexAttribTypes[i] = attribType;

			GLint attribNormalized;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &attribNormalized);
			vertexAttribNormalized[i] = attribNormalized;

			GLint attribStride;
			gl::GetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &attribStride);
			vertexAttribStride[i] = attribStride;

			GLvoid* attribOffset;
			gl::GetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribOffset);
			vertexAttribOffset[i] = attribOffset;
		}
	}

	debugLogApiError("glState::storeCurrentGlState Exit");
}

void GLStateTracker::checkStateChanged(const GLState& currentGlState)
{
	activeProgramChanged = activeProgram != currentGlState.activeProgram;
	activeTextureUnitChanged = activeTextureUnit != currentGlState.activeTextureUnit;
	boundTextureChanged = boundTexture != currentGlState.boundTexture;

	// blending states
	blendEnabledChanged = blendEnabled != currentGlState.blendEnabled;
	blendSrcRgbChanged = blendDstRgb != currentGlState.blendSrcRgb;
	blendDstRgbChanged = blendDstRgb != currentGlState.blendDstRgb;
	blendSrcAlphaChanged = blendSrcAlpha != currentGlState.blendSrcAlpha;
	blendDstAlphaChanged = blendDstAlpha != currentGlState.blendDstAlpha;
	blendEqationRgbChanged = blendEqationRgb != currentGlState.blendEqationRgb;
	blendEqationAlphaChanged = blendEqationAlpha != currentGlState.blendEqationAlpha;

	// depth states
	depthTestChanged = depthTest != currentGlState.depthTest;
	depthMaskChanged = depthMask != currentGlState.depthMask;

	stencilTestChanged = stencilTest != currentGlState.stencilTest;

	cullingEnabledChanged = cullingEnabled != currentGlState.cullingEnabled;
	cullingChanged = culling != currentGlState.culling;
	windingOrderChanged = windingOrder != currentGlState.windingOrder;

	sampler7Changed = sampler7 != currentGlState.sampler7;
	if (vbo != -1)
	{
		vboChanged = vbo != currentGlState.vbo;
	}
	if (ibo != -1)
	{
		iboChanged = ibo != currentGlState.ibo;
	}
	if (vao != -1)
	{
		vaoChanged = vao != currentGlState.vao;
	}

	if (currentGlState.vao != 0)
	{
		vaoChanged = true;
	}

	colorMaskChanged = ((colorMask[0] != currentGlState.colorMask[0]) ||
	                    (colorMask[1] != currentGlState.colorMask[1]) ||
	                    (colorMask[2] != currentGlState.colorMask[2]) ||
	                    (colorMask[3] != currentGlState.colorMask[3]));

	for (uint32_t i = 0; i < 8; i++)
	{
		vertexAttribArrayChanged[i] = vertexAttribArray[i] != currentGlState.vertexAttribArray[i];

		vertexAttribPointerChanged[i] = vertexAttribBindings[i] != currentGlState.vertexAttribBindings[i] ||
		                                vertexAttribSizes[i] != currentGlState.vertexAttribSizes[i] ||
		                                vertexAttribTypes[i] != currentGlState.vertexAttribTypes[i] ||
		                                vertexAttribNormalized[i] != currentGlState.vertexAttribNormalized[i] ||
		                                vertexAttribStride[i] != currentGlState.vertexAttribStride[i] ||
		                                vertexAttribOffset[i] != currentGlState.vertexAttribOffset[i];
	}
}

void GLStateTracker::checkStateChanged(const GLStateTracker& stateTracker)
{
	activeProgramChanged = stateTracker.activeProgramChanged;
	activeTextureUnitChanged = stateTracker.activeTextureUnitChanged;
	boundTextureChanged = stateTracker.boundTextureChanged;

	// blending states
	blendEnabledChanged = stateTracker.blendEnabledChanged;
	blendSrcRgbChanged = stateTracker.blendSrcRgbChanged;
	blendDstRgbChanged = stateTracker.blendDstRgbChanged;
	blendSrcAlphaChanged = stateTracker.blendSrcAlphaChanged;
	blendDstAlphaChanged = stateTracker.blendDstAlphaChanged;
	blendEqationRgbChanged = stateTracker.blendEqationRgbChanged;
	blendEqationAlphaChanged = stateTracker.blendEqationAlphaChanged;

	// depth states
	depthTestChanged = stateTracker.depthTestChanged;
	depthMaskChanged = stateTracker.depthMaskChanged;

	stencilTestChanged = stateTracker.stencilTestChanged;

	cullingEnabledChanged = stateTracker.cullingEnabledChanged;
	cullingChanged = stateTracker.cullingChanged;
	windingOrderChanged = stateTracker.windingOrderChanged;

	sampler7Changed = stateTracker.sampler7Changed;
	if (vbo != -1)
	{
		vboChanged = stateTracker.vboChanged;
	}
	if (ibo != -1)
	{
		iboChanged = stateTracker.iboChanged;
	}
	if (vao != -1)
	{
		vaoChanged = stateTracker.vaoChanged;
	}

	if (stateTracker.vao != 0)
	{
		vaoChanged = true;
	}

	colorMaskChanged = stateTracker.colorMaskChanged;

	for (uint32_t i = 0; i < 8; i++)
	{
		vertexAttribArrayChanged[i] = stateTracker.vertexAttribArrayChanged[i];
		vertexAttribPointerChanged[i] = stateTracker.vertexAttribPointerChanged[i];
	}
}

void GLStateTracker::setUiState(bool isEs2)
{
	debugLogApiError("GLStateTracker::setState Enter");
	if (activeProgramChanged) { gl::UseProgram(activeProgram); }
	if (activeTextureUnitChanged) { gl::ActiveTexture(activeTextureUnit); }
	if (boundTextureChanged) { gl::BindTexture(GL_TEXTURE_2D, boundTexture); }
	if (blendEnabledChanged) { blendEnabled ? gl::Enable(GL_BLEND) : gl::Disable(GL_BLEND); }
	if (blendSrcRgbChanged || blendSrcAlphaChanged || blendDstRgbChanged || blendDstAlphaChanged)
	{
		gl::BlendFuncSeparate(blendSrcRgb, blendDstRgb, blendSrcAlpha, blendDstAlpha);
	}
	if (blendEqationRgbChanged || blendEqationAlphaChanged)
	{
		gl::BlendEquationSeparate(blendEqationRgb, blendEqationAlpha);
	}

	if (colorMaskChanged) { gl::ColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]); }
	if (depthTestChanged) { depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST); }
	if (depthMaskChanged) { gl::DepthMask(depthMask); }
	if (stencilTestChanged) { stencilTest ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST); }
	if (cullingEnabledChanged) { cullingEnabled ? gl::Enable(GL_CULL_FACE) : gl::Disable(GL_CULL_FACE); }
	if (cullingChanged) { gl::CullFace(culling); }
	if (windingOrderChanged) { gl::DepthMask(windingOrder); }
	if (sampler7Changed) { gl::BindSampler(7, sampler7); }
	if (vaoChanged)
	{
		if (!isEs2)
		{
			gl::BindVertexArray(0);
		}
		else
		{
			gl::ext::BindVertexArrayOES(0);
		}
	}
	if (vboChanged)	{ gl::BindBuffer(GL_ARRAY_BUFFER, vbo); }
	if (iboChanged)	{ gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); }

	for (uint32_t i = 0; i < 8; i++)
	{
		if (vertexAttribArrayChanged[i])
		{
			if (vertexAttribArray[i])
			{
				gl::EnableVertexAttribArray(i);

				if (vertexAttribPointerChanged[i])
				{
					gl::VertexAttribPointer(vertexAttribBindings[i], vertexAttribSizes[i], vertexAttribTypes[i],
					                        vertexAttribNormalized[i], vertexAttribStride[i],
					                        vertexAttribOffset[i]);
				}
			}
			else
			{
				gl::DisableVertexAttribArray(i);
			}
		}
	}

	debugLogApiError("GLStateTracker::setState Exit");
}

void GLStateTracker::restoreState(const GLState& currentGlState, bool isEs2)
{
	debugLogApiError("glState::restoreState Enter");

	if (activeProgramChanged) { gl::UseProgram(currentGlState.activeProgram); }
	if (activeTextureUnitChanged) { gl::ActiveTexture(currentGlState.activeTextureUnit); }
	if (boundTextureChanged) { gl::BindTexture(GL_TEXTURE_2D, currentGlState.boundTexture); }
	if (blendEnabledChanged) { currentGlState.blendEnabled ? gl::Enable(GL_BLEND) : gl::Disable(GL_BLEND); }
	if (blendSrcRgbChanged || blendSrcAlphaChanged || blendDstRgbChanged || blendDstAlphaChanged)
	{
		gl::BlendFuncSeparate(currentGlState.blendSrcRgb, currentGlState.blendDstRgb, currentGlState.blendSrcAlpha, currentGlState.blendDstAlpha);
	}
	if (blendEqationRgbChanged || blendEqationAlphaChanged)
	{
		gl::BlendEquationSeparate(currentGlState.blendEqationRgb, currentGlState.blendEqationAlpha);
	}

	if (colorMaskChanged) { gl::ColorMask(currentGlState.colorMask[0], currentGlState.colorMask[1], currentGlState.colorMask[2], currentGlState.colorMask[3]); }
	if (depthTestChanged) { currentGlState.depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST); }
	if (depthMaskChanged) { gl::DepthMask(currentGlState.depthMask); }
	if (stencilTestChanged) { currentGlState.stencilTest ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST); }
	if (cullingEnabledChanged) { currentGlState.cullingEnabled ? gl::Enable(GL_CULL_FACE) : gl::Disable(GL_CULL_FACE); }
	if (cullingChanged) { gl::CullFace(currentGlState.culling); }

	if (windingOrder) { gl::FrontFace(currentGlState.windingOrder); }
	if (sampler7Changed)
	{
		gl::BindSampler(7, currentGlState.sampler7);
	}
	if (vboChanged)
	{
		gl::BindBuffer(GL_ARRAY_BUFFER, currentGlState.vbo);
	}
	if (iboChanged)
	{
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentGlState.ibo);
	}

	for (uint32_t i = 0; i < 8; i++)
	{
		if (vertexAttribArrayChanged[i])
		{
			if (currentGlState.vertexAttribArray[i])
			{
				gl::EnableVertexAttribArray(i);

				if (vertexAttribPointerChanged[i])
				{
					gl::VertexAttribPointer(currentGlState.vertexAttribBindings[i], currentGlState.vertexAttribSizes[i], currentGlState.vertexAttribTypes[i],
					                        currentGlState.vertexAttribNormalized[i], currentGlState.vertexAttribStride[i],
					                        vertexAttribOffset[i]);
				}
			}
			else
			{
				gl::DisableVertexAttribArray(i);
			}
		}
	}

	if (vaoChanged)
	{
		if (!isEs2)
		{
			gl::BindVertexArray(currentGlState.vao);
		}
		else
		{
			gl::ext::BindVertexArrayOES(currentGlState.vao);
		}
	}

	debugLogApiError("glState::restoreState Exit");
}

void UIRenderer::checkStateChanged(const GLStateTracker& stateTracker)
{
	_uiStateTracker.checkStateChanged(stateTracker);
}

void UIRenderer::checkStateChanged()
{
	_uiStateTracker.checkStateChanged(_currentState);
}

void UIRenderer::restoreState(bool isEs2)
{
	_uiStateTracker.restoreState(_currentState, isEs2);
}

void UIRenderer::storeCurrentGlState(bool isEs2)
{
	_currentState.storeCurrentGlState(isEs2);
}

void UIRenderer::setUiState(bool isEs2)
{
	_uiStateTracker.setUiState(isEs2);
}

bool UIRenderer::init_CreateShaders()
{
	// Text_ pipe
	GLuint shaders[] = { 0, 0 };

	if (
	  !pvr::utils::loadShader(BufferStream("", _print3DShader_glsles200_vsh, _print3DShader_glsles200_vsh_size),
	                          ShaderType::VertexShader, NULL, 0, shaders[0]) ||
	  !pvr::utils::loadShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size),
	                          ShaderType::FragmentShader, NULL, 0, shaders[1]) ||
	  !shaders[0] || !shaders[1])
	{
		Log(LogLevel::Critical, "UIRenderer shaders could not be created.");
		return false;
	}

	const char* attributes[] = { "myVertex", "myUV" };
	const uint16_t attribIndices[] = { 0, 1 };

	if (!pvr::utils::createShaderProgram(shaders, 2, attributes, attribIndices, 2, _program))
	{
		Log(LogLevel::Critical, "UIRenderer shader program could not be created.");
		return false;
	}

	_uiStateTracker.activeProgram = _program;

	GLint prev_program;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prev_program);

	gl::UseProgram(_program);
	_programData.uniforms[UIRenderer::ProgramData::UniformMVPmtx] = gl::GetUniformLocation(_program, "myMVPMatrix");
	_programData.uniforms[UIRenderer::ProgramData::UniformFontTexture] = gl::GetUniformLocation(_program, "fontTexture");
	_programData.uniforms[UIRenderer::ProgramData::UniformColor] = gl::GetUniformLocation(_program, "varColor");
	_programData.uniforms[UIRenderer::ProgramData::UniformAlphaMode] = gl::GetUniformLocation(_program, "alphaMode");
	_programData.uniforms[UIRenderer::ProgramData::UniformUVmtx] = gl::GetUniformLocation(_program, "myUVMatrix");

	gl::Uniform1i(_programData.uniforms[UIRenderer::ProgramData::UniformFontTexture], 7);
	return true;
}

Font UIRenderer::createFont(const Texture& tex, GLuint sampler)
{
	auto results = utils::textureUpload(tex, isEs2(), true);
	if (!results.successful)
	{
		Log("UIRenderer::CreateFont::Failed to create font");
		return Font();
	}

	if (isEs2())
	{
		gl::BindTexture(GL_TEXTURE_2D, results.image);
		if (tex.getLayersSize().numMipLevels > 1)
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return createFont(results.image, tex, sampler);
}

Font UIRenderer::createFont(GLuint texture, const TextureHeader& texHeader, GLuint sampler)
{
	Font font;
	font.construct(*this, texture, texHeader, sampler);
	_fonts.push_back(font);
	return font;
}

Image UIRenderer::createImage(const Texture& texture, GLuint sampler)
{
	utils::TextureUploadResults res = utils::textureUpload(texture, isEs2(), true);

	if (isEs2())
	{
		gl::BindTexture(GL_TEXTURE_2D, res.image);
		if (texture.getLayersSize().numMipLevels > 1)
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return createImage(res.image, texture.getWidth(), texture.getHeight(), texture.getLayersSize().numMipLevels > 1, sampler);
}

MatrixGroup UIRenderer::createMatrixGroup()
{
	MatrixGroup group;
	group.construct(*this, generateGroupId());
	_sprites.push_back(group);
	group->commitUpdates();
	return group;
}

PixelGroup UIRenderer::createPixelGroup()
{
	PixelGroup group;
	group.construct(*this, generateGroupId());
	_sprites.push_back(group);
	group->commitUpdates();
	return group;
}

Image UIRenderer::createImage(GLuint tex, int32_t width, int32_t height, bool useMipmaps, GLuint sampler)
{
	return createImageFromAtlas(tex, Rectanglef(0.0f, 0.0f, 1.0f, 1.0f), width, height, useMipmaps, sampler);
}

pvr::ui::Image UIRenderer::createImageFromAtlas(GLuint texture, const Rectanglef& uv,
    uint32_t atlasWidth, uint32_t atlasHeight, bool useMipmaps, GLuint sampler)
{
	Image image;
	image.construct(*this, texture, atlasWidth, atlasHeight, useMipmaps, sampler);
	_sprites.push_back(image);
	// construct the scaling matrix
	// calculate the scale factor
	// convert from texel to normalize coord
	image->setUV(uv);
	image->commitUpdates();
	return image;
}

TextElement UIRenderer::createTextElement(const std::wstring& text, const Font& font)
{
	TextElement spriteText;
	spriteText.construct(*this, text, font);
	_textElements.push_back(spriteText);
	return spriteText;
}

Text UIRenderer::createText(const TextElement& textElement)
{
	Text text;
	text.construct(*this, textElement);
	_sprites.push_back(text);
	text->commitUpdates();
	return text;
}

TextElement UIRenderer::createTextElement(const std::string& text, const Font& font)
{
	TextElement spriteText;
	spriteText.construct(*this, text, font);
	_textElements.push_back(spriteText);
	return spriteText;
}

bool UIRenderer::init(uint32_t width, uint32_t height, bool fullscreen, bool isEs2)
{
	debugLogApiError("");
	_isEs2 = isEs2;
	release();
	_screenDimensions = glm::vec2(width, height);
	// screen rotated?
	if (_screenDimensions.y > _screenDimensions.x && fullscreen)
	{
		rotateScreen90degreeCCW();
	}

	bool result = true;
	debugLogApiError("UIRenderer::init 1");
	storeCurrentGlState(isEs2);
	debugLogApiError("UIRenderer::init 2");
	if (init_CreateShaders())
	{
		debugLogApiError("UIRenderer::init CreateShaders");
		if (!_isEs2)
		{
			if (!init_CreateDefaultSampler()) { result = false; }
			debugLogApiError("UIRenderer::init CreateDefaultSampler");
		}
		if (!init_CreateDefaultSdkLogo()) { result = false; }
		debugLogApiError("UIRenderer::init CreateDefaultSdkLogo");
		if (!init_CreateDefaultFont()) { result = false; }
		debugLogApiError("UIRenderer::init CreateDefaultFont");
		if (!init_CreateDefaultTitle()) { result = false; }
		debugLogApiError("UIRenderer::init CreateDefaultTitle");
	}

	// set some initial ui state tracking state
	_uiStateTracker.vertexAttribArray[0] = GL_TRUE;
	_uiStateTracker.vertexAttribArray[1] = GL_TRUE;

	_uiStateTracker.vertexAttribBindings[0] = 0;
	_uiStateTracker.vertexAttribSizes[0] = 4;
	_uiStateTracker.vertexAttribTypes[0] = GL_FLOAT;
	_uiStateTracker.vertexAttribNormalized[0] = GL_FALSE;
	_uiStateTracker.vertexAttribStride[0] = sizeof(float) * 6;
	_uiStateTracker.vertexAttribOffset[0] = NULL;

	_uiStateTracker.vertexAttribBindings[1] = 1;
	_uiStateTracker.vertexAttribSizes[1] = 2;
	_uiStateTracker.vertexAttribTypes[1] = GL_FLOAT;
	_uiStateTracker.vertexAttribNormalized[1] = GL_FALSE;
	_uiStateTracker.vertexAttribStride[1] = sizeof(float) * 6;
	_uiStateTracker.vertexAttribOffset[1] = reinterpret_cast<GLvoid*>(sizeof(float) * 4);

	checkStateChanged();
	restoreState(isEs2);
	debugLogApiError("UIRenderer::init RestoreState");

	return result;
}

bool UIRenderer::init_CreateDefaultSampler()
{
	if (!isEs2())
	{
		debugLogApiError("UIRenderer::init_CreateDefaultSampler Enter");
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1");
		gl::GenSamplers(1, &_samplerBilinear);
		gl::GenSamplers(1, &_samplerTrilinear);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1.1");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1.2");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1.3");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1.4");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 1.5");
		gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		debugLogApiError("UIRenderer::init_CreateDefaultSampler 2");

		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 2.1");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 2.2");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 2.3");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler 2.4");
		gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		debugLogApiError("UIRenderer::init_CreateDefaultSampler Exit");

		_uiStateTracker.sampler7 = _samplerBilinear;

		_samplerBilinearCreated = true;
		_samplerTrilinearCreated = true;
	}

	return true;
}

bool UIRenderer::init_CreateDefaultSdkLogo()
{
	Stream::ptr_type sdkLogo = Stream::ptr_type(new BufferStream("", _PowerVR_512x256_RG_pvr, _PowerVR_512x256_RG_pvr_size));
	Texture sdkTex;
	if (!assets::textureLoad(sdkLogo, TextureFileFormat::PVR, sdkTex))
	{
		Log(LogLevel::Warning, "UIRenderer: Could not create the PowerVR SDK Logo.");
		return false;
	}
	sdkTex.setPixelFormat(GeneratePixelType2<'l', 'a', 8, 8>::ID);

	_sdkLogo = createImage(sdkTex);
	if (_sdkLogo.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the PowerVR SDK Logo. Errors will be gotten if trying to render getSdkLogo().");
		return false;
	}
	_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));
	float scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1) { scalefactor = 1; }
	else if (scalefactor > .5) { scalefactor = .5; }
	else if (scalefactor > .25) { scalefactor = .25; }
	else if (scalefactor > .125) { scalefactor = .125; }
	else { scalefactor = .0625; }

	_sdkLogo->setScale(glm::vec2(scalefactor));
	_sdkLogo->commitUpdates();
	return true;
}

bool UIRenderer::init_CreateDefaultTitle()
{
	_defaultTitle = createText(createTextElement("DefaultTitle", _defaultFont));
	debugLogApiError("UIRenderer::init_CreateDefaultTitle createText0");
	_defaultDescription = createText(createTextElement("", _defaultFont));
	debugLogApiError("UIRenderer::init_CreateDefaultTitle createText1");
	_defaultControls = createText(createTextElement("", _defaultFont));
	debugLogApiError("UIRenderer::init_CreateDefaultTitle createText2");
	if (_defaultTitle.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the PowerVR Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
		_defaultTitle->commitUpdates();
	}

	if (_defaultDescription.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the Demo Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f -
		                               _defaultTitle->getFont()->getFontLineSpacing() / static_cast<float>(getRenderingDimY()) * 1.5f))
		->setScale(glm::vec2(.60, .60));
		_defaultDescription->commitUpdates();
	}

	if (_defaultControls.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the Demo Controls text. Errors will be gotten if trying to render getDefaultControls().");
		return false;
	}
	else
	{
		_defaultControls->setAnchor(Anchor::BottomLeft, glm::vec2(-.98f, -.98f))->setScale(glm::vec2(.5, .5));
		_defaultControls->commitUpdates();
	}
	debugLogApiError("UIRenderer::init_CreateDefaultTitle Exit");

	return true;
}

bool UIRenderer::init_CreateDefaultFont()
{
	Texture fontTex;
	Stream::ptr_type arialFontTex;
	float maxRenderDim = glm::max<float>(getRenderingDimX(), getRenderingDimY());
	// pick the right font size of this resolution.
	if (maxRenderDim <= 800)
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_36_pvr, _arialbd_36_pvr_size));
	}
	else if (maxRenderDim <= 1000)
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_46_pvr, _arialbd_46_pvr_size));
	}
	else
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_56_pvr, _arialbd_56_pvr_size));
	}

	if (!assets::textureLoad(arialFontTex, TextureFileFormat::PVR, fontTex))
	{
		Log(LogLevel::Warning, "UIRenderer initialisation: Could not create the default font. Errors will be"
		    " gotten if trying to render with getDefaultFont().");
		return false;
	}
	fontTex.setPixelFormat(GeneratePixelType1<'a', 8>::ID);

	_defaultFont = createFont(fontTex);
	if (_defaultFont.isNull())
	{
		Log(LogLevel::Warning, "UIRenderer initialisation: Could not create the default font. Errors will be"
		    " gotten if trying to render with getDefaultFont().");
		return false;
	}
	return true;
}
}// namespace ui
}// namespace pvr
//!\endcond