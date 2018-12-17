/*!*********************************************************************************************************************
\File         OpenGLESIBL.cpp
\Title        Introducing Physcially Based Rendering
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  This example demonstrates how to use Physically based rendering using Metallic-Roughness work flow showcasing 2 scenes (helmet and sphere) with Image based lighting
			  (IBL). The Technique presented here is based on Epic Games publication http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
***********************************************************************************************************************/

/*!
	IBL Description
	Material: Metallic-Roughness
	============================
	- Albedo map: This is a raw color of the material. This map shouldn't contains any shading information like Ambient Occlusion which is
	very often baked in the diffuse map for phong model.
	It does not only influence the diffuse color, but also the specular color of the material as well.
	When the metallness is one(metallic material) the base color is the specular.

	- MetallicRoughness map: The metallic-roughness texture.
	The metalness values are sampled from the B channel and roughness values are sampled from the G channel, other channels are ignored.

	BRDF
	====
	*Diffuse BRDF: Lambertian diffuse
	f = Cdiff / PI
	Cdiff: Diffuse albedo of the material.

	*Specular BRDF: Cook-Torance
	f = D * F * G / (4 * (N.L) * (N.V));
	D: NDF (Normal Distribution function), It computes the distribution of the microfacets for the shaded surface
	F: Describes how light reflects and refracts at the intersection of two different media (most often in computer graphics : Air and the shaded surface)
	G: Defines the shadowing from the microfacets
	N.L:  is the dot product between the normal of the shaded surface and the light direction.
	N.V is the dot product between the normal of the shaded surface and the view direction.

	IBL workflow
	============
	IBL is one of the most common technique for implmenting global illumination. The idea is that using environmap as light source.

	IBL Diffuse:
	The application load/ generates a diffuse Irradiance map: This is normally done in offline but the code is left here for education
	purpose. Normally when lambert diffuse is used in games, it is the light color multiplied by the visibility factor( N dot L).
	But when using Indirectional lighting (IBL)  the visibility factor is not considered because the light is coming from every where.
	So the diffuse factor is the light color.
	All the pixels in the environment map is a light source, so when shading a point it has to be lit by many pixels from the environment map.
	Sampling multiple texels for shading a single point is not practical for realtime application. Therefore these samples are precomputed
	in the diffuse irradiance map. So at run time it would be a single texture fetch for the given reflection direction.

	IBL Specular & BRDF_LUT:
	Specular reflections looks shiny when the roughness values is low and it becames blurry when the roughness value is high.
	This is encoded in the specular irradiance texture.
	We use the same technique, Split-Sum-Approximation presented by Epics Games, each mip level of this image contains the environment map specular reflectance.
	Mip level 0 contains samples for roughness value 0, and the remaining miplevels get blurry for each mip level as the roughness value increases to 1.

	The samples encoded in this map is the result of the specular BRDF of the environment map. For each pixels in the environemt map,
	computes the Cook-Torrentz microfacet BRDF and stores those results.

	Using the mip map for storing blured images for each roughness value has one draw backs, Specular antialising.
	This happens for the level 0. Since we are using the mip map for different purpose, we can't use mipmapping technique
	to solve the aliasing artifact for high resoultion texture which is level0 of the specular irradiance map.
	Other mip map levels doesn'y have this issue as they are blured and low res.

	To solve this issue we use another texture for doing mipmaping for level 0 of the specular Irradiance map.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/texture/TextureUtils.h"

// Content file names
// Shaders
const char PBRVertShaderFileName[] = "PBRVertShader.vsh";
const char PBRFragShaderFileName[] = "PBRFragShader.fsh";
const char SkyboxVertShaderFileName[] = "SkyboxVertShader.vsh";
const char SkyboxFragShaderFileName[] = "SkyboxFragShader.fsh";
const char IrradianceVertShaderFileName[] = "IrradianceVertShader.vsh";
const char IrradianceFragShaderFileName[] = "IrradianceFragShader.fsh";
const char PreFilterFragShaderFileName[] = "PreFilterFragShader.fsh";

// Scenes
const char HelmetSceneFileName[] = "damagedHelmet.gltf";
const char SphereSceneFileName[] = "sphere.pod";

// Textures
const char SkyboxTexFile[] = "MonValley_baked_lightmap.pvr";
const char DiffuseIrradianceMapTexFile[] = "DiffuseIrradianceMap.pvr";
const char PrefilterEnvMapTexFile[] = "PrefilterEnvMap.pvr";
const char PrefilterL0MipMapTexFile[] = "PrefilterL0MipMap.pvr";
const char BrdfLUTTexFile[] = "brdfLUT.pvr";

const uint32_t IrradianceMapDim = 64;
const uint32_t PrefilterEnvMapDim = 256;

const uint32_t NumSphereRows = 4;
const uint32_t NumSphereColumns = 6;
const uint32_t NumInstances = NumSphereRows * NumSphereColumns;

const bool LoadIrradianceMap = true;
const bool LoadPrefilteredMap = true;
const bool LoadBRDFLUT = true;

const glm::vec3 lightDir[1] = {
	glm::normalize(glm::vec3(0.0f, -0.5f, 0.5f)),
};

enum class SceneMode
{
	Helmet,
	Sphere,
	NumScenes
};

class SkyboxPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, bool loadIrradianceMap, bool loadPrefilteredMap, bool srgbFramebuffer, bool isBufferStorageExtSupported)
	{
		this->isBufferStorageExtSupported = isBufferStorageExtSupported;

		// load the environment map.
		skyBoxMap = pvr::utils::textureUpload(assetProvider, SkyboxTexFile, true);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxMap);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (loadIrradianceMap)
		{
			irradianceMap = pvr::utils::textureUpload(assetProvider, DiffuseIrradianceMapTexFile, false);
		}
		else
		{
			irradianceMap = generateIrradianceMap(assetProvider, DiffuseIrradianceMapTexFile);
		}

		gl::BindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		// set the mip map interpolates to linear so that if the roughness value is not right on the mip level,
		// it will interpolate between the two closest mip levels.
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (loadPrefilteredMap)
		{
			pvr::Texture envTexture;
			prefilteredMap = pvr::utils::textureUpload(assetProvider, PrefilterEnvMapTexFile, envTexture, false);
			prefilteredL0MipMap = pvr::utils::textureUpload(assetProvider, PrefilterL0MipMapTexFile, envTexture, false);

			numPrefilteredMipLevels = envTexture.getNumMipMapLevels();
		}
		else
		{
			generatePrefilteredMap(assetProvider, PrefilterEnvMapTexFile, PrefilterL0MipMapTexFile, prefilteredMap, prefilteredL0MipMap);
		}

		gl::BindTexture(GL_TEXTURE_CUBE_MAP, prefilteredMap);
		// set the mip map interpolates to linear so that if the roughness value is not right on the mip level,
		// it will interpolate between the two closest mip levels.
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		gl::BindTexture(GL_TEXTURE_CUBE_MAP, prefilteredL0MipMap);
		// set the mip map interpolates to linear so that if the roughness value is not right on the mip level,
		// it will interpolate between the two closest mip levels.
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		std::vector<const char*> defines;
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		// create the program
		program = pvr::utils::createShaderProgram(
			assetProvider, SkyboxVertShaderFileName, SkyboxFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		// set up the uniform buffer
		pvr::utils::StructuredMemoryDescription viewDesc;
		viewDesc.addElement("InvVPMatrix", pvr::GpuDatatypes::mat4x4);
		viewDesc.addElement("EyePos", pvr::GpuDatatypes::vec3);
		uboView.init(viewDesc);

		gl::GenBuffers(1, &uboBuffer);
		gl::BindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
		gl::BufferData(GL_UNIFORM_BUFFER, uboView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never unmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, uboBuffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)uboView.getSize(), nullptr, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, uboView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			uboView.pointToMappedMemory(memory);
		}
	}

	uint32_t getNumPrefilteredMipLevels() const
	{
		return numPrefilteredMipLevels;
	}

	GLuint getDiffuseIrradianceMap()
	{
		return irradianceMap;
	}

	GLuint getPrefilteredMap()
	{
		return prefilteredMap;
	}

	GLuint getPrefilteredMipMap()
	{
		return prefilteredL0MipMap;
	}

	void render(const glm::mat4& viewProjMtx, const glm::vec3& eyePos)
	{
		gl::ActiveTexture(GL_TEXTURE8);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxMap);

		// Disable the depth testing, no need.
		gl::Disable(GL_DEPTH_TEST);
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, uboBuffer, 0, uboView.getSize());

		void* mappedMemory = nullptr;
		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)uboView.getSize(), GL_MAP_WRITE_BIT);
			uboView.pointToMappedMemory(mappedMemory);
		}

		uboView.getElement(0).setValue(glm::inverse(viewProjMtx));
		uboView.getElement(1).setValue(eyePos);

		if (!isBufferStorageExtSupported)
		{
			gl::UnmapBuffer(GL_UNIFORM_BUFFER);
		}

		gl::UseProgram(program);
		gl::DrawArrays(GL_TRIANGLES, 0, 6);
	}

	~SkyboxPass()
	{
		gl::DeleteProgram(program);
		gl::DeleteTextures(1, &skyBoxMap);
		gl::DeleteTextures(1, &irradianceMap);
		gl::DeleteTextures(1, &prefilteredMap);
		gl::DeleteTextures(1, &prefilteredL0MipMap);
		gl::DeleteBuffers(1, &uboBuffer);
	}

private:
	// Generates specular irradiance map.
	void generatePrefilteredMap(pvr::IAssetProvider& assetProvider, const char* specIrradianceMap, const char* specIrrL0Mipmap, GLuint& prefilteredMap, GLuint& prefilteredL0MipMap)
	{
		// Create the quad vertices.
		const float quadVertices[] = {
			-1, 1, 1.f, // upper left
			-1, -1, 1.f, // lower left
			1, 1, 1.f, // upper right
			1, 1, 1.f, // upper right
			-1, -1, 1.f, // lower left
			1, -1, 1.f // lower right
		};

		GLuint vbo;
		gl::GenBuffers(1, &vbo);
		gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		gl::EnableVertexAttribArray(0);
		gl::VertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
		const GLenum texFormat = GL_RGBA8;
		const uint32_t formatStride = sizeof(uint8_t) * 4;

		// Discard the last two mipmaps. From our experimentation keeping the last miplevel 4x4 avoids blocky texel artifacts for materials with roughness values of 1.0.
		const uint32_t DISCARD_SPECULAR_MIP_LEVELS = 2;

		// calculate number of mip map levels
		const uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(PrefilterEnvMapDim)) + 1.0f - DISCARD_SPECULAR_MIP_LEVELS); // prefilterMap

		std::vector<uint32_t> mipLevelDimensions(numMipLevels);

		// Compute the mip level size
		for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i)
		{
			mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels + DISCARD_SPECULAR_MIP_LEVELS - 1 - i));
		}

		// create the program
		GLuint program = pvr::utils::createShaderProgram(assetProvider, IrradianceVertShaderFileName, IrradianceFragShaderFileName, nullptr, nullptr, 0);
		debugThrowOnApiError("ERROR");

		// Create the framebuffer and render target
		GLuint rtRoughness;
		gl::GenTextures(1, &rtRoughness);
		debugThrowOnApiError("ERROR");
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, rtRoughness);
		gl::TexStorage2D(GL_TEXTURE_CUBE_MAP, numMipLevels, texFormat, static_cast<GLsizei>(PrefilterEnvMapDim), static_cast<GLsizei>(PrefilterEnvMapDim));
		debugThrowOnApiError("ERROR");

		GLuint rtFullMip;
		gl::GenTextures(1, &rtFullMip);
		debugThrowOnApiError("ERROR");
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, rtFullMip);
		gl::TexStorage2D(GL_TEXTURE_CUBE_MAP, numMipLevels, texFormat, static_cast<GLsizei>(PrefilterEnvMapDim), static_cast<GLsizei>(PrefilterEnvMapDim));
		debugThrowOnApiError("ERROR");

		std::vector<char> texDataIrrRoughness(formatStride * PrefilterEnvMapDim * PrefilterEnvMapDim * numMipLevels * 6);
		std::vector<char> texDataIrrMip(formatStride * PrefilterEnvMapDim * PrefilterEnvMapDim * numMipLevels * 6);
		uint32_t dataOffset = 0;

		const glm::mat4 cubeView[6] = {
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f)), // +X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f)), // -X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f)), // +Y
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f)), // -Y
			glm::scale(glm::vec3(1.0f, -1.0f, 1.f)), // +Z
			glm::scale(glm::vec3(-1.0f, -1.0f, -1.f)), // -Z
		};

		gl::UseProgram(program);
		debugThrowOnApiError("ERROR");

		gl::ActiveTexture(GL_TEXTURE0);
		debugThrowOnApiError("ERROR");

		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxMap);
		debugThrowOnApiError("ERROR");

		gl::ActiveTexture(GL_TEXTURE1);
		debugThrowOnApiError("ERROR");

		for (uint32_t i = 0; i < numMipLevels; ++i)
		{
			for (uint32_t j = 0; j < 6; ++j)
			{
				gl::UniformMatrix4fv(0, 1, false, glm::value_ptr(cubeView[j]));
				GLuint fbo;
				gl::GenFramebuffers(1, &fbo);
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
				gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, rtRoughness, i);
				gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, rtFullMip, i);
				debug_assertion(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid fbo");

				GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
				gl::DrawBuffers(ARRAY_SIZE(bufs), bufs);
				gl::Viewport(0, 0, mipLevelDimensions[i], mipLevelDimensions[i]);
				gl::Uniform1f(1, static_cast<float>(i) / static_cast<float>(numMipLevels - 1));
				debugThrowOnApiError("ERROR");
				gl::Uniform1f(2, static_cast<GLfloat>(IrradianceMapDim));
				debugThrowOnApiError("ERROR");
				gl::DrawArrays(GL_TRIANGLES, 0, 6);
				debugThrowOnApiError("ERROR");

				gl::BindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
				gl::ReadBuffer(GL_COLOR_ATTACHMENT0);
				gl::ReadPixels(0, 0, mipLevelDimensions[i], mipLevelDimensions[i], GL_RGBA, GL_UNSIGNED_BYTE, texDataIrrRoughness.data() + dataOffset);

				gl::ReadBuffer(GL_COLOR_ATTACHMENT1);
				gl::ReadPixels(0, 0, mipLevelDimensions[i], mipLevelDimensions[i], GL_RGBA, GL_UNSIGNED_BYTE, texDataIrrMip.data() + dataOffset);

				dataOffset += formatStride * mipLevelDimensions[i] * mipLevelDimensions[i];
				gl::DeleteFramebuffers(1, &fbo);
				debugThrowOnApiError("ERROR");
			}
		}

		gl::DeleteBuffers(1, &vbo);
		gl::DeleteProgram(program);

		{
			// store it in to the file.
			pvr::TextureHeader texHeader;
			texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
			texHeader.setColorSpace(pvr::ColorSpace::lRGB);
			texHeader.setDepth(1);
			texHeader.setWidth(IrradianceMapDim);
			texHeader.setHeight(IrradianceMapDim);
			texHeader.setNumMipMapLevels(numMipLevels);
			texHeader.setNumFaces(6);
			texHeader.setNumArrayMembers(1);
			texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

			pvr::Texture tex(texHeader, (const char*)texDataIrrRoughness.data());
			pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(specIrradianceMap, "w");
			pvr::assetWriters::TextureWriterPVR writerPVR;
			writerPVR.openAssetStream(std::move(fileStream));
			writerPVR.writeAsset(tex);
			prefilteredMap = rtRoughness;
		}

		{
			// store it in to the file.
			pvr::TextureHeader texHeader;
			texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
			texHeader.setColorSpace(pvr::ColorSpace::lRGB);
			texHeader.setDepth(1);
			texHeader.setWidth(IrradianceMapDim);
			texHeader.setHeight(IrradianceMapDim);
			texHeader.setNumMipMapLevels(numMipLevels);
			texHeader.setNumFaces(6);
			texHeader.setNumArrayMembers(1);
			texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

			pvr::Texture tex(texHeader, texDataIrrMip.data());
			pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(specIrrL0Mipmap, "w");
			pvr::assetWriters::TextureWriterPVR writerPVR;
			writerPVR.openAssetStream(std::move(fileStream));
			writerPVR.writeAsset(tex);
			prefilteredL0MipMap = rtFullMip;
		}

		numPrefilteredMipLevels = numMipLevels;
	}

	GLuint generateIrradianceMap(pvr::IAssetProvider& assetProvider, const char* fileName)
	{
		// Create the quad vertices.
		const float quadVertices[] = {
			-1, 1, 1.f, // upper left
			-1, -1, 1.f, // lower left
			1, 1, 1.f, // upper right
			1, 1, 1.f, // upper right
			-1, -1, 1.f, // lower left
			1, -1, 1.f // lower right
		};

		GLuint vbo;
		gl::GenBuffers(1, &vbo);
		gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		gl::EnableVertexAttribArray(0);
		gl::VertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

		const uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(IrradianceMapDim)) + 1.0);

		// calcuate the mip level dimensions.
		std::vector<uint32_t> mipLevelDimensions(numMipLevels);
		for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i)
		{
			mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels - i - 1));
		}

		GLuint program = pvr::utils::createShaderProgram(assetProvider, IrradianceVertShaderFileName, IrradianceFragShaderFileName, nullptr, nullptr, 0);
		GLuint renderTarget;
		gl::GenTextures(1, &renderTarget);
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, renderTarget);

		GLenum texFormat = GL_RGBA8;
		const uint32_t formatStride = sizeof(uint8_t) * 4;
		gl::TexStorage2D(GL_TEXTURE_CUBE_MAP, numMipLevels, texFormat, IrradianceMapDim, IrradianceMapDim);
		std::vector<char> texData(formatStride * IrradianceMapDim * IrradianceMapDim * numMipLevels * 6);
		uint32_t dataOffset = 0;

		const glm::mat4 cubeView[6] = {
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f)), // +X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f)), // -X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f)), // +Y
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f)), // -Y
			glm::scale(glm::vec3(1.0f, -1.0f, 1.f)), // +Z
			glm::scale(glm::vec3(-1.0f, -1.0f, -1.f)), // -Z
		};

		gl::UseProgram(program);
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxMap);
		gl::ActiveTexture(GL_TEXTURE1);
		for (uint32_t i = 0; i < numMipLevels; ++i)
		{
			for (uint32_t j = 0; j < 6; ++j)
			{
				gl::UniformMatrix4fv(0, 1, false, glm::value_ptr(cubeView[j]));
				GLuint fbo;
				gl::GenFramebuffers(1, &fbo);
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
				gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, renderTarget, i);
				debug_assertion(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid fbo");
				debugThrowOnApiError("ERROR");
				gl::Viewport(0, 0, mipLevelDimensions[i], mipLevelDimensions[i]);
				gl::DrawArrays(GL_TRIANGLES, 0, 6);
				debugThrowOnApiError("ERROR");

				gl::BindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
				gl::ReadBuffer(GL_COLOR_ATTACHMENT0);
				gl::ReadPixels(0, 0, mipLevelDimensions[i], mipLevelDimensions[i], GL_RGBA, GL_UNSIGNED_BYTE, texData.data() + dataOffset);
				debugThrowOnApiError("ERROR");

				dataOffset += formatStride * mipLevelDimensions[i] * mipLevelDimensions[i];
				gl::DeleteFramebuffers(1, &fbo);
				debugThrowOnApiError("ERROR");
			}
		}
		gl::DeleteBuffers(1, &vbo);
		gl::DeleteProgram(program);

		// copy it in to the file.
		pvr::TextureHeader texHeader;
		texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
		texHeader.setColorSpace(pvr::ColorSpace::lRGB);
		texHeader.setDepth(1);
		texHeader.setWidth(IrradianceMapDim);
		texHeader.setHeight(IrradianceMapDim);
		texHeader.setNumMipMapLevels(numMipLevels);
		texHeader.setNumFaces(6);
		texHeader.setNumArrayMembers(1);
		texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

		pvr::Texture tex(texHeader, (const char*)texData.data());
		pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(fileName, "w");
		pvr::assetWriters::TextureWriterPVR writerPVR;
		writerPVR.openAssetStream(std::move(fileStream));
		writerPVR.writeAsset(tex);

		return renderTarget;
	}

	GLuint program;
	GLuint skyBoxMap;
	GLuint irradianceMap; // Diffsue irradiance
	GLuint prefilteredMap; // specular filtered map.
	GLuint prefilteredL0MipMap;
	uint32_t numPrefilteredMipLevels;
	pvr::utils::StructuredBufferView uboView;
	GLuint uboBuffer;
	bool isBufferStorageExtSupported;
};

class SpherePass
{
public:
	/// <summary>initialise the sphere's program</summary>
	/// <param name="assetProvider">Asset provider for loading assets from disk.</param>
	void init(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" }, { "TANGENT", "tangent" } };

		std::vector<const char*> defines;
		defines.push_back("INSTANCING");
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		program =
			pvr::utils::createShaderProgram(assetProvider, PBRVertShaderFileName, PBRFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(assetProvider.getAssetStream(SphereSceneFileName)));
		pvr::utils::appendSingleBuffersFromModel(*scene, vbos, ibos);
		vertexConfiguration = createInputAssemblyFromMesh(scene->getMesh(0), vertexBindings, ARRAY_SIZE(vertexBindings));
	}

	/// <summary>Destructor for the sphere pass</summary>
	~SpherePass()
	{
		gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
		gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
		gl::DeleteProgram(program);
	}

	/// <summary>Renders the sphere scene</summary>
	void render()
	{
		debugThrowOnApiError("begin Render Sphere Scene");
		gl::UseProgram(program);
		debugThrowOnApiError("bind sphere pass program");

		for (uint32_t node = 0; node < scene->getNumMeshNodes(); ++node)
		{
			uint32_t meshId = scene->getMeshNode(node).getObjectId();
			gl::BindBuffer(GL_ARRAY_BUFFER, vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[meshId]);

			const pvr::assets::Mesh& mesh = scene->getMesh(meshId);
			for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
			{
				auto& attrib = vertexConfiguration.attributes[i];
				auto& binding = vertexConfiguration.bindings[0];
				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}
			debugThrowOnApiError("Render Node (before draw)");
			gl::DrawElementsInstanced(GL_TRIANGLES, mesh.getNumFaces() * 3, pvr::utils::convertToGles(mesh.getFaces().getDataType()), nullptr, NumInstances);

			for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
			{
				auto& attrib = vertexConfiguration.attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
			debugThrowOnApiError("Render Node (after draw)");
		}
	}

private:
	pvr::assets::ModelHandle scene;
	GLuint program;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	pvr::utils::VertexConfiguration vertexConfiguration;
};

class HelmetPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		scene = pvr::assets::Model::createWithReader(pvr::assets::GltfReader(assetProvider.getAssetStream(HelmetSceneFileName), assetProvider));

		pvr::utils::appendSingleBuffersFromModel(*scene, vbos, ibos);

		// Load the texture
		loadTextures(assetProvider);

		createProgram(assetProvider, srgbFramebuffer);
	}

	GLuint getProgram()
	{
		return program;
	}

	pvr::assets::ModelHandle& getScene()
	{
		return scene;
	}

	GLuint getAlbedoMap()
	{
		return textures[0];
	}

	GLuint getMetallicRoughnessMap()
	{
		return textures[1];
	}

	GLuint getNormalMap()
	{
		return textures[2];
	}

	GLuint getEmissiveMap()
	{
		return textures[3];
	}

	~HelmetPass()
	{
		if (program)
		{
			gl::DeleteProgram(program);
		}
		if (vbos.size())
		{
			gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
			vbos.clear();
		}
		if (ibos.size())
		{
			gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
			ibos.clear();
		}

		gl::DeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
	}

	void render()
	{
		// bind the albedo texture
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, textures[0]);

		// bind the metallic roughness texture
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, textures[1]);

		// bind the normal texture
		gl::ActiveTexture(GL_TEXTURE2);
		gl::BindTexture(GL_TEXTURE_2D, textures[2]);

		// bind the emissive map
		gl::ActiveTexture(GL_TEXTURE3);
		gl::BindTexture(GL_TEXTURE_2D, textures[3]);

		// render the helmet
		gl::UseProgram(program);
		// The scene has only on material.
		auto& material = scene->getMaterial(0);
		pvr::assets::Model::Material::GLTFMetallicRoughnessSemantics pbrMetallicRoughness(material);
		gl::Uniform2f(0, pbrMetallicRoughness.getMetallicity(), pbrMetallicRoughness.getRoughness());
		const glm::vec4 baseColor = pbrMetallicRoughness.getBaseColor();
		gl::Uniform3f(1, baseColor.r, baseColor.g, baseColor.b);

		for (uint32_t i = 0; i < scene->getNumMeshNodes(); ++i)
		{
			renderMesh(i);
		}
	}

private:
	void loadTextures(pvr::IAssetProvider& assetProvider)
	{
		for (uint32_t i = 0; i < scene->getNumTextures(); ++i)
		{
			pvr::Stream::ptr_type stream = assetProvider.getAssetStream(scene->getTexture(i).getName());
			pvr::Texture tex = pvr::textureLoad(stream, pvr::TextureFileFormat::PVR);
			textures.push_back(pvr::utils::textureUpload(tex, false, true).image);

			bool isCubemap = tex.getNumFaces() > 1;
			bool hasMipMaps = tex.getNumMipMapLevels() > 1;

			GLenum targetType;
			GLenum minFilter;

			if (isCubemap)
			{
				targetType = GL_TEXTURE_CUBE_MAP;
			}
			else
			{
				targetType = GL_TEXTURE_2D;
			}

			if (hasMipMaps)
			{
				minFilter = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				minFilter = GL_LINEAR;
			}

			gl::BindTexture(targetType, textures[i]);
			gl::TexParameteri(targetType, GL_TEXTURE_MIN_FILTER, minFilter);
			gl::TexParameteri(targetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl::TexParameteri(targetType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl::TexParameteri(targetType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			gl::TexParameteri(targetType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
	}

	void createProgram(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Create the PBR program
		std::vector<const char*> defines;
		defines.push_back("MATERIAL_TEXTURES");
		defines.push_back("NORMAL_MAP");
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		program =
			pvr::utils::createShaderProgram(assetProvider, PBRVertShaderFileName, PBRFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));
		const pvr::assets::Mesh& mesh = scene->getMesh(0);
		const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" }, { "TANGENT", "tangent" } };
		vertexConfiguration = createInputAssemblyFromMesh(mesh, vertexBindings, ARRAY_SIZE(vertexBindings));
	}

	void renderMesh(uint32_t meshNodeId)
	{
		debugThrowOnApiError("ERROR: renderMesh begin");
		const pvr::assets::Model::Node* pNode = &scene->getMeshNode(meshNodeId);
		const uint32_t meshId = pNode->getObjectId();
		// Gets pMesh referenced by the pNode
		const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

		// bind the vertex and index buffer
		gl::BindBuffer(GL_ARRAY_BUFFER, vbos[meshId]);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[meshId]);
		debugThrowOnApiError("ERROR: Failed to bind vertex and index buffer");

		// set the vertex attribute pointers
		for (size_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
		{
			auto& attrib = vertexConfiguration.attributes[i];
			auto& binding = vertexConfiguration.bindings[0];

			gl::EnableVertexAttribArray(attrib.index);
			gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
				reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			debugThrowOnApiError("ERROR");
		}
		gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

		for (uint32_t i = 0; i < vertexConfiguration.attributes.size(); ++i)
		{
			auto& attrib = vertexConfiguration.attributes[i];
			gl::DisableVertexAttribArray(attrib.index);
		}
		debugThrowOnApiError("ERROR");
	}

	pvr::assets::ModelHandle scene;
	pvr::utils::VertexConfiguration vertexConfiguration;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	GLuint program;
	std::vector<GLuint> textures;
};

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OpenGLESIBL : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::EglContext context;

		GLuint uboStatic; // static UBO
		GLuint uboPerFrame; // static UBO
		GLuint uboPerModel;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		SkyboxPass skyboxPass;
		SpherePass spherePass;
		HelmetPass helmetPass;

		GLuint brdfLUT;

		DeviceResources() {}
		~DeviceResources()
		{
			gl::DeleteBuffers(1, &uboStatic);
			gl::DeleteBuffers(1, &uboPerFrame);
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::StructuredBufferView _uboPerSceneBufferView;
	pvr::utils::StructuredBufferView _uboPerModelBufferView;
	pvr::utils::StructuredBufferView _uboPerFrameBufferView;
	std::vector<char> _uboDynamicData;
	std::vector<char> _uboModelData;
	std::vector<char> _uboMaterialData;
	glm::mat4 _projMtx;
	pvr::TPSCamera _camera;
	SceneMode _currentScene;
	bool _sceneChanged = false;
	bool _pause = false;
	float _cameraLookAngle = 0.0f;
	bool _isBufferStorageExtSupported;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void generateBRDFLUT(pvr::Texture& outTexture);
	void createUbo();
	void updateUbo(SceneMode scene);
	void setDefaultOpenglState();

	virtual void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		case pvr::SimplifiedInput::Left:
		{
			uint32_t currentScene = static_cast<uint32_t>(_currentScene);
			currentScene -= 1;
			currentScene = currentScene % static_cast<uint32_t>(SceneMode::NumScenes);
			_currentScene = static_cast<SceneMode>(currentScene);
			_sceneChanged = true;
			break;
		}
		case pvr::SimplifiedInput::Right:
		{
			uint32_t currentScene = static_cast<uint32_t>(_currentScene);
			currentScene += 1;
			currentScene = currentScene % static_cast<uint32_t>(SceneMode::NumScenes);
			_currentScene = static_cast<SceneMode>(currentScene);
			_sceneChanged = true;
			break;
		}
		case pvr::SimplifiedInput::Action1:
		{
			_pause = !_pause;
			break;
		}
		case pvr::SimplifiedInput::ActionClose:
		{
			this->exitShell();
			break;
		}
		default:
		{
			break;
		}
		}
	}
};

/// <summary>
/// Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
/// context is lost, initApplication() will not be called again.
/// </summary>
pvr::Result OpenGLESIBL::initApplication()
{
	_currentScene = SceneMode::Helmet;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIBL::quitApplication()
{
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIBL::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	_deviceResources->context = pvr::createEglContext();

	// Create the context. The minimum OpenGLES version must be OpenGL ES 3.0
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3);

	// We make use of GL_EXT_buffer_storage wherever possible
	_isBufferStorageExtSupported = gl::isGlExtensionSupported("GL_EXT_buffer_storage");

	// Initialise the uirenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("ImageBasedLighting");
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / Right: to change the scene\n"
															   "Action 1: Enable/Disable Animation\n");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->skyboxPass.init(*this, LoadIrradianceMap, LoadPrefilteredMap, getBackBufferColorspace() == pvr::ColorSpace::sRGB, _isBufferStorageExtSupported);
	_deviceResources->helmetPass.init(*this, getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->spherePass.init(*this, getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// set the view port dimension back.
	gl::Viewport(0, 0, getWidth(), getHeight());

	// create the static ubo
	createUbo();
	updateUbo(_currentScene);

	if (LoadBRDFLUT)
	{
		_deviceResources->brdfLUT = pvr::utils::textureUpload(*this, BrdfLUTTexFile, false);
	}
	else
	{
		pvr::Texture brdfLut;
		generateBRDFLUT(brdfLut);
		_deviceResources->brdfLUT = pvr::utils::textureUpload(brdfLut, false, true).image;
	}

	// set the BRDF LUT texture sampler
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->brdfLUT);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	debugThrowOnApiError("ERROR");

	if (isScreenRotated())
	{
		_projMtx = pvr::math::perspectiveFov(
			pvr::Api::OpenGLES31, glm::radians(45.f), static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), 0.1f, 2000.f, glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspectiveFov(pvr::Api::OpenGLES31, glm::radians(45.f), static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), 0.1f, 2000.f);
	}

	// setup the camera
	_camera.setDistanceFromTarget(60.f);
	_camera.setHeight(10.f);
	setDefaultOpenglState();
	return pvr::Result::Success;
}

void OpenGLESIBL::setDefaultOpenglState()
{
	gl::DepthMask(GL_TRUE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
}

void OpenGLESIBL::updateUbo(SceneMode scene)
{
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerModel);

	void* mappedMemory = nullptr;
	if (!_isBufferStorageExtSupported)
	{
		mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, _uboPerModelBufferView.getSize(), GL_MAP_WRITE_BIT);
		_uboPerModelBufferView.pointToMappedMemory(mappedMemory);
	}

	if (scene == SceneMode::Helmet)
	{
		_uboPerModelBufferView.getElement(0, 0, static_cast<uint32_t>(scene)).setValue(glm::eulerAngleXY(glm::radians(0.f), glm::radians(120.f)) * glm::scale(glm::vec3(22.0f)));
	}
	else
	{
		_uboPerModelBufferView.getElement(0, 0, static_cast<uint32_t>(scene)).setValue(glm::scale(glm::vec3(4.5f)));
	}

	static float emissiveScale = 0.0f;
	static float emissiveStrength = 1.;
	emissiveStrength += .15f;
	if (emissiveStrength >= glm::pi<float>())
	{
		emissiveStrength = 0.0f;
	}

	emissiveScale = std::abs(glm::cos(emissiveStrength)) + .75f;
	_uboPerModelBufferView.getElement(1, 0, static_cast<uint32_t>(scene)).setValue(emissiveScale);
	if (!_isBufferStorageExtSupported)
	{
		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIBL::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESIBL::renderFrame()
{
	debugThrowOnApiError("Begin Frame");

	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (_sceneChanged)
	{
		updateUbo(_currentScene);
	}

	if (!_pause)
	{
		_cameraLookAngle += 0.15f;
	}

	if (_cameraLookAngle >= 360.0f)
	{
		_cameraLookAngle = _cameraLookAngle - 360.f;
	}
	if (!_pause)
	{
		_camera.setTargetLookAngle(_cameraLookAngle);
	}

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerFrame);
	void* mappedMemory = nullptr;
	if (!_isBufferStorageExtSupported)
	{
		mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_uboPerFrameBufferView.getSize(), GL_MAP_WRITE_BIT);
		_uboPerFrameBufferView.pointToMappedMemory(mappedMemory);
	}
	const glm::mat4 viewProj = _projMtx * _camera.getViewMatrix();
	_uboPerFrameBufferView.getElement(0).setValue(_camera.getCameraPosition());
	_uboPerFrameBufferView.getElement(1).setValue(viewProj);
	if (!_isBufferStorageExtSupported)
	{
		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}

	// render the skybox
	_deviceResources->skyboxPass.render(viewProj, _camera.getCameraPosition());

	gl::Enable(GL_DEPTH_TEST);

	// bind the diffuse irradiance map
	gl::ActiveTexture(GL_TEXTURE4);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getDiffuseIrradianceMap());

	// bind the specular irradiance map
	gl::ActiveTexture(GL_TEXTURE5);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getPrefilteredMap());

	// bind the brdf lut texture
	gl::ActiveTexture(GL_TEXTURE6);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->brdfLUT);

	// bind the specular irradiance map
	gl::ActiveTexture(GL_TEXTURE7);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->skyboxPass.getPrefilteredMipMap());

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, _deviceResources->uboStatic, 0, _uboPerSceneBufferView.getSize());
	debugThrowOnApiError("ERROR");

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 1, _deviceResources->uboPerFrame, 0, _uboPerFrameBufferView.getSize());
	debugThrowOnApiError("ERROR");

	gl::BindBufferRange(GL_UNIFORM_BUFFER, 2, _deviceResources->uboPerModel, _uboPerModelBufferView.getDynamicSliceOffset(static_cast<uint32_t>(_currentScene)),
		_uboPerModelBufferView.getDynamicSliceSize());
	debugThrowOnApiError("ERROR");

	if (_currentScene == SceneMode::Helmet)
	{
		_deviceResources->helmetPass.render();
	}
	else
	{
		_deviceResources->spherePass.render();
	}

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

// Generate BRDF Look up table using CPU. This is normally done offline and the code is left here for educational purpose.
void OpenGLESIBL::generateBRDFLUT(pvr::Texture& outTexture)
{
	pvr::assets::generateBRDFLUT(outTexture);
	pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(BrdfLUTTexFile, "w");
	pvr::assetWriters::TextureWriterPVR writerPVR;
	writerPVR.openAssetStream(std::move(fileStream));
	writerPVR.writeAsset(outTexture);
}

void OpenGLESIBL::createUbo()
{
	debugThrowOnApiError("ERROR");
	{
		// Static ubo
		// The following elements are static therefore they get updated once in the initial step.
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("lightDir", pvr::GpuDatatypes::vec3);
		memDesc.addElement("numPrefilteredMipLevels", pvr::GpuDatatypes::uinteger);
		_uboPerSceneBufferView.init(memDesc);
		gl::GenBuffers(1, &_deviceResources->uboStatic);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboStatic);
		gl::BufferData(GL_UNIFORM_BUFFER, _uboPerSceneBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);
		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		void* memory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_uboPerSceneBufferView.getSize(), GL_MAP_WRITE_BIT);
		_uboPerSceneBufferView.pointToMappedMemory(memory);
		_uboPerSceneBufferView.getElement(0).setValue(lightDir);
		_uboPerSceneBufferView.getElement(1).setValue(_deviceResources->skyboxPass.getNumPrefilteredMipLevels());
		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}
	debugThrowOnApiError("ERROR");

	// Model
	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("ModelMatrix", pvr::GpuDatatypes::mat4x4);
		memDesc.addElement("emissiveScale", pvr::GpuDatatypes::Float);
		GLint uniformAlignment = 0;
		gl::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformAlignment);
		_uboPerModelBufferView.initDynamic(memDesc, 2, pvr::BufferUsageFlags::UniformBuffer, uniformAlignment);
		_uboModelData.resize(_uboPerModelBufferView.getSize());

		gl::GenBuffers(1, &_deviceResources->uboPerModel);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerModel);
		gl::BufferData(GL_UNIFORM_BUFFER, _uboPerModelBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		if (_isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->uboPerModel);
			gl::ext::BufferStorageEXT(
				GL_COPY_READ_BUFFER, (GLsizei)_uboPerModelBufferView.getSize(), nullptr, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, _uboPerModelBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_uboPerModelBufferView.pointToMappedMemory(memory);
		}
	}

	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("camPos", pvr::GpuDatatypes::vec3);
		memDesc.addElement("VPMatrix", pvr::GpuDatatypes::mat4x4);
		_uboPerFrameBufferView.init(memDesc);
		_uboDynamicData.resize(_uboPerFrameBufferView.getSize());
		gl::GenBuffers(1, &_deviceResources->uboPerFrame);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboPerFrame);
		gl::BufferData(GL_UNIFORM_BUFFER, _uboPerFrameBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		if (_isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, _deviceResources->uboPerFrame);
			gl::ext::BufferStorageEXT(
				GL_COPY_READ_BUFFER, (GLsizei)_uboPerFrameBufferView.getSize(), nullptr, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, _uboPerFrameBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			_uboPerFrameBufferView.pointToMappedMemory(memory);
		}
	}
	debugThrowOnApiError("ERROR");
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new OpenGLESIBL());
}
