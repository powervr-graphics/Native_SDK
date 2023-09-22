/*!*********************************************************************************************************************
\File         OpenGLESShadows.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to generate dynamic shadows in real-time.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRCore/Log.h"
#include <fstream>
#include <iomanip>
#include <unordered_map>

#define _PI 3.14159

// Configuration
const float g_FOV = 65.0f;
const int g_ShadowMapSize = 256;
const float g_PCFBias = 0.01f;
const uint32_t g_PoissonDiskSampleCount = 8;
const float g_PoissonSamplingRadius = 9.0f;
const float g_VSMBias = 0.0005f;
const float g_EVSM2Bias = 0.03f;
const float g_EVSM4Bias = 0.03f;
const float g_VSMLightBleedReduction = 0.1f;
const float g_EVSM2LightBleedReduction = 0.0001f;
const float g_EVSM4LightBleedReduction = 0.0001f;

// Shaders
const char MeshVertShaderFileName[] = "MeshVertShader.vsh";
const char MeshFragShaderFileName[] = "MeshFragShader.fsh";
const char ShadowVertShaderFileName[] = "ShadowVertShader.vsh";
const char ShadowFragShaderFileName[] = "ShadowFragShader.fsh";
const char TriangleVertShaderFileName[] = "TriangleVertShader.vsh";
const char LightingFragShaderFileName[] = "LightingFragShader.fsh";
const char AmbientFragShaderFileName[] = "AmbientFragShader.fsh";
const char GaussianBlurHorizontalFragShaderFileName[] = "GaussianBlurHorizontalFragShader.fsh";
const char GaussianBlurVerticalFragShaderFileName[] = "GaussianBlurVerticalFragShader.fsh";

enum class ShadowType : uint32_t
{
	None,
	ShadowMapHard,
	ShadowMapPCFPoissonDisk,
	ShadowMapPCFOptimised2x2,
	ShadowMapPCFOptimised3x3,
	ShadowMapPCFOptimised5x5,
	ShadowMapPCFOptimised7x7,
	ShadowMapVSM,
	ShadowMapEVSM2,
	ShadowMapEVSM4,
	Count
};

// Scenes
const char ModelFileName[] = "GnomeToy.pod";

// Constants
const char* ShadowTypeNames[] = { "None", "Hard", "PCF Poisson Disk", "PCF Optimised 2x2", "PCF Optimised 3x3", "PCF Optimised 5x5", "PCF Optimised 7x7", "VSM", "EVSM2", "EVSM4" };
const pvr::utils::VertexBindings_Name VertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };

struct ShadowMapPass;
struct NoShadowsSample;
struct PCFShadowsSample;
struct VSMShadowsSample;
struct GaussianBlurFragmentPass;

struct Material
{
	GLuint diffuseTexture;
};

// Put all API managed objects in a struct so that we can one-line free them...
struct DeviceResources
{
	pvr::EglContext context;

	GLuint uboGlobal;

	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	std::vector<Material> materials;
	std::vector<pvr::utils::VertexConfiguration> vertexConfigurations;

	pvr::utils::StructuredBufferView uboView;

	std::shared_ptr<ShadowMapPass> shadowMapPass;
	std::shared_ptr<NoShadowsSample> noShadowsSample;
	std::shared_ptr<PCFShadowsSample> hardShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfPoissonDiskShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised2x2ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised3x3ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised5x5ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised7x7ShadowsSample;
	std::shared_ptr<VSMShadowsSample> vsmFragmentShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm2FragmentShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm4FragmentShadowsSample;

	// Fragment Gaussian Blurs
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurVSMFragmentPass;
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurEVSM2FragmentPass;
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurEVSM4FragmentPass;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	DeviceResources() {}
	~DeviceResources() {}
};

struct ShadowMapPass
{
	ShadowMapPass() {}
	~ShadowMapPass()
	{
		gl::DeleteFramebuffers(1, &_shadowMapFbo);
		gl::DeleteTextures(1, &_shadowMapTex);
	}

	/// <summary>Create the required GL state.</summary>
	/// <param name="assetProvider">reference to IAssetProvider.</param>
	void init(pvr::IAssetProvider& assetProvider)
	{
		_program = pvr::utils::createShaderProgram(assetProvider, ShadowVertShaderFileName, ShadowFragShaderFileName, nullptr, nullptr, 0, nullptr, 0);
		_modelMatLocation = gl::GetUniformLocation(_program, "ModelMat");

		// Create shadow map texture and framebuffer
		gl::GenTextures(1, &_shadowMapTex);
		gl::BindTexture(GL_TEXTURE_2D, _shadowMapTex);
		gl::TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, g_ShadowMapSize, g_ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		gl::TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		gl::GenFramebuffers(1, &_shadowMapFbo);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowMapFbo);
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMapTex, 0);
		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		gl::BindTexture(GL_TEXTURE_2D, 0);

		debugThrowOnApiError("ERROR: ShadowMapPass::init");
	}

	/// <summary>Render scene into depth texture.</summary>
	/// <param name="shell">Reference to PVR Shell</param>
	/// <param name="Scene">Scene to be rendered.</param>
	void render(pvr::assets::ModelHandle scene, DeviceResources* deviceResources)
	{
		gl::BindFramebuffer(GL_FRAMEBUFFER, _shadowMapFbo);

		gl::Viewport(0, 0, g_ShadowMapSize, g_ShadowMapSize);

		gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gl::CullFace(GL_FRONT);

		gl::UseProgram(_program);

		// Render all mesh nodes.
		for (int i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			gl::UniformMatrix4fv(_modelMatLocation, 1, GL_FALSE, glm::value_ptr(scene->getWorldMatrix(i)));

			// bind the vertex and index buffer
			gl::BindBuffer(GL_ARRAY_BUFFER, deviceResources->vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, deviceResources->ibos[meshId]);

			// set the vertex attribute pointers
			for (size_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				auto& binding = deviceResources->vertexConfigurations[i].bindings[0];

				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}

			gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

			for (uint32_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
		}

		gl::CullFace(GL_BACK);

		debugThrowOnApiError("ERROR: ShadowMapPass::render");
	}

	GLuint _program;
	GLuint _modelMatLocation;
	GLuint _shadowMapTex;
	GLuint _shadowMapFbo;
};

struct NoShadowsSample
{
	NoShadowsSample() {}
	~NoShadowsSample() {}

	/// <summary>Create the required GL state.</summary>
	/// <param name="assetProvider">reference to IAssetProvider.</param>
	void init(pvr::IAssetProvider& assetProvider)
	{
		const char* defines[] = { "SHADOW_TYPE_NONE" };

		_program = pvr::utils::createShaderProgram(assetProvider, MeshVertShaderFileName, MeshFragShaderFileName, nullptr, nullptr, 0, defines, 1);
		_modelMatLocation = gl::GetUniformLocation(_program, "ModelMat");
		_diffuseLocation = gl::GetUniformLocation(_program, "sDiffuse");

		debugThrowOnApiError("ERROR: NoShadowsSample::init");
	}

	/// <summary>Render scene without shadows.</summary>
	/// <param name="shell">Reference to PVR Shell</param>
	/// <param name="scene">Scene to be rendered.</param>
	void render(pvr::assets::ModelHandle scene, DeviceResources* deviceResources, pvr::Shell& shell)
	{
		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		gl::Viewport(0, 0, shell.getWidth(), shell.getHeight());
		gl::ClearColor(0.0f, 0.40f, .39f, 1.0f);
		gl::Disable(GL_STENCIL_TEST);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		gl::UseProgram(_program);

		// Render all mesh nodes.
		for (int i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			gl::UniformMatrix4fv(_modelMatLocation, 1, GL_FALSE, glm::value_ptr(scene->getWorldMatrix(i)));

			if (deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture != -1)
			{
				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture);
				gl::Uniform1i(_diffuseLocation, 0);
			}

			// bind the vertex and index buffer
			gl::BindBuffer(GL_ARRAY_BUFFER, deviceResources->vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, deviceResources->ibos[meshId]);

			// set the vertex attribute pointers
			for (size_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				auto& binding = deviceResources->vertexConfigurations[i].bindings[0];

				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}

			gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

			for (uint32_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
		}

		debugThrowOnApiError("ERROR: NoShadowsSample::render");
	}

	GLuint _program;
	GLuint _modelMatLocation;
	GLuint _diffuseLocation;
};

struct PCFShadowsSample
{
	PCFShadowsSample() {}
	~PCFShadowsSample() {}

	/// <summary>Create the required GL state.</summary>
	/// <param name="define">shader definition corresponding to the technique</param>
	/// <param name="assetProvider">reference to IAssetProvider.</param>
	/// <param name="shadowMapPass">Pointer to shadow map pass.</param>
	void init(const char* define, pvr::IAssetProvider& assetProvider, ShadowMapPass* shadowMapPass)
	{
		_shadowMapPass = shadowMapPass;

		const char* defines[] = { define };

		_program = pvr::utils::createShaderProgram(assetProvider, MeshVertShaderFileName, MeshFragShaderFileName, nullptr, nullptr, 0, defines, 1);
		_modelMatLocation = gl::GetUniformLocation(_program, "ModelMat");
		_diffuseLocation = gl::GetUniformLocation(_program, "sDiffuse");
		_shadowMapLocation = gl::GetUniformLocation(_program, "sShadowMap");
		_shadowParamsLocation = gl::GetUniformLocation(_program, "ShadowParams");

		debugThrowOnApiError("ERROR: PCFShadowsSample::init");
	}

	/// <summary>Render scene with PCF shadows.</summary>
	/// <param name="shadowParams">vec4 containing shadow bias parameters</param>
	/// <param name="shell">Reference to PVR Shell</param>
	/// <param name="scene">Scene to be rendered.</param>
	void render(pvr::assets::ModelHandle scene, DeviceResources* deviceResources, glm::vec4 shadowParams, pvr::Shell& shell)
	{
		_shadowMapPass->render(scene, deviceResources);

		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		gl::Viewport(0, 0, shell.getWidth(), shell.getHeight());
		gl::ClearColor(0.0f, 0.40f, .39f, 1.0f);
		gl::Disable(GL_STENCIL_TEST);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		gl::UseProgram(_program);

		// Bind shadow map from the provided shadow map pass.
		if (_shadowMapLocation != GL_INVALID_INDEX)
		{
			gl::Uniform1i(_shadowMapLocation, 0);
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _shadowMapPass->_shadowMapTex);
		}

		gl::Uniform4f(_shadowParamsLocation, shadowParams.x, shadowParams.y, shadowParams.z, shadowParams.w);

		// Render all mesh nodes.
		for (int i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			gl::UniformMatrix4fv(_modelMatLocation, 1, GL_FALSE, glm::value_ptr(scene->getWorldMatrix(i)));

			if (deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture != -1)
			{
				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture);
				gl::Uniform1i(_diffuseLocation, 1);
			}

			// bind the vertex and index buffer
			gl::BindBuffer(GL_ARRAY_BUFFER, deviceResources->vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, deviceResources->ibos[meshId]);

			// set the vertex attribute pointers
			for (size_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				auto& binding = deviceResources->vertexConfigurations[i].bindings[0];

				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}

			gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

			for (uint32_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
		}

		debugThrowOnApiError("ERROR: PCFShadowsSample::render");
	}

	ShadowMapPass* _shadowMapPass;
	GLuint _program;
	GLuint _modelMatLocation;
	GLuint _diffuseLocation;
	GLuint _shadowMapLocation;
	GLuint _shadowParamsLocation;
};

struct GaussianBlurPass
{
	virtual void render(GLuint inputShadowMap) = 0;
	virtual GLuint shadowMap() = 0;
	void computeBlurFactors()
	{
		const int n = _blurSize;
		assertion(n < 8, "blur size n > 7 not supported unless more gaussianFactors are allocated (currently 4 vec4s)");
		const float standardDeviation = sqrt(static_cast<float>(n) / 2.0f);
		const float factor1D = sqrt(1.0f / (2.0f * _PI * standardDeviation * standardDeviation));
		const float factorExp = 1.0f / (2.0f * standardDeviation * standardDeviation);

		float factorSum = 0.0f;
		int gaussianIndex = 0;
		for (int x = -n; x <= n; x++)
		{
			float factor = factor1D * exp(-(x * x) * factorExp);
			_gaussianFactors[gaussianIndex] = factor;
			factorSum += factor;
			gaussianIndex++;
		}

		// double check scaling - factors should add up to 1!
		for (int g = 0; g < gaussianIndex; g++) _gaussianFactors[g] /= factorSum;

		// fill remaining spaces
		while (gaussianIndex < _gaussianFactors.size())
		{
			_gaussianFactors[gaussianIndex] = 0.0f;
			gaussianIndex++;
		}
	}

	std::array<float, 16> _gaussianFactors;
	uint32_t _blurSize = 7;
	GLuint _blurredShadowMapTex;
};

struct GaussianBlurFragmentPass : public GaussianBlurPass
{
	~GaussianBlurFragmentPass()
	{
		gl::DeleteFramebuffers(2, &_blurredShadowMapFbo[0]);
		gl::DeleteTextures(2, &_blurredShadowMapTex[0]);
	}

	/// <summary>Create the required GL state.</summary>
	/// <param name="horizontalPassDefine">shader definition corresponding to the horizontal blur pass</param>
	/// <param name="assetProvider">reference to IAssetProvider.</param>
	void init(const char* horizontalPassDefine, pvr::IAssetProvider& assetProvider, bool fourChannel = false)
	{
		computeBlurFactors();

		const char* defines[] = { horizontalPassDefine };

		_programH = pvr::utils::createShaderProgram(assetProvider, TriangleVertShaderFileName, GaussianBlurHorizontalFragShaderFileName, nullptr, nullptr, 0, defines, 1);
		_depthLocationH = gl::GetUniformLocation(_programH, "sDepth");
		_blurSizeShadowMapSizeLocationH = gl::GetUniformLocation(_programH, "blurSizeShadowMapSize");
		_gaussianFactorsLocationH = gl::GetUniformLocation(_programH, "gaussianFactors[0]");

		_programV = pvr::utils::createShaderProgram(assetProvider, TriangleVertShaderFileName, GaussianBlurVerticalFragShaderFileName, nullptr, nullptr, 0, nullptr, 0);
		_intermediateLocationV = gl::GetUniformLocation(_programV, "sIntermediateMap");
		_blurSizeShadowMapSizeLocationV = gl::GetUniformLocation(_programV, "blurSizeShadowMapSize");
		_gaussianFactorsLocationV = gl::GetUniformLocation(_programV, "gaussianFactors[0]");

		// Create two output textures for ping ponging between
		for (int i = 0; i < 2; i++)
		{
			gl::GenTextures(1, &_blurredShadowMapTex[i]);
			gl::BindTexture(GL_TEXTURE_2D, _blurredShadowMapTex[i]);
			gl::TexImage2D(GL_TEXTURE_2D, 0, fourChannel ? GL_RGBA16F : GL_RG16F, g_ShadowMapSize, g_ShadowMapSize, 0, fourChannel ? GL_RGBA : GL_RG, GL_HALF_FLOAT, nullptr);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			// gl::TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			gl::GenFramebuffers(1, &_blurredShadowMapFbo[i]);
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _blurredShadowMapFbo[i]);
			gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurredShadowMapTex[i], 0);
		}
	}

	/// <summary>Perform a two-pass gaussian blur on the given input image.</summary>
	/// <param name="inputShadowMap">Image containing the variance shadow map to be blurred</param>
	void render(GLuint inputShadowMap) override
	{
		// Gaussian Blur Horizontal
		gl::UseProgram(_programH);

		gl::BindFramebuffer(GL_FRAMEBUFFER, _blurredShadowMapFbo[0]);
		gl::Viewport(0, 0, g_ShadowMapSize, g_ShadowMapSize);
		gl::ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gl::Disable(GL_DEPTH_TEST);
		gl::Disable(GL_CULL_FACE);

		if (_depthLocationH != GL_INVALID_INDEX)
		{
			gl::Uniform1i(_depthLocationH, 0);
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, inputShadowMap);
		}

		glm::uvec2 blurSizeShadowMapSize = glm::uvec2(_blurSize, g_ShadowMapSize);

		gl::Uniform2ui(_blurSizeShadowMapSizeLocationH, blurSizeShadowMapSize.x, blurSizeShadowMapSize.y);
		gl::Uniform4fv(_gaussianFactorsLocationH, 4, &_gaussianFactors[0]);

		// Render fullscreen triangle
		gl::DrawArrays(GL_TRIANGLES, 0, 3);

		// Gaussian Blur Vertical
		gl::UseProgram(_programV);

		gl::Uniform2ui(_blurSizeShadowMapSizeLocationV, blurSizeShadowMapSize.x, blurSizeShadowMapSize.y);
		gl::Uniform4fv(_gaussianFactorsLocationV, 4, &_gaussianFactors[0]);

		gl::BindFramebuffer(GL_FRAMEBUFFER, _blurredShadowMapFbo[1]);
		gl::Viewport(0, 0, g_ShadowMapSize, g_ShadowMapSize);
		gl::ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gl::Disable(GL_DEPTH_TEST);
		gl::Disable(GL_CULL_FACE);

		if (_intermediateLocationV != GL_INVALID_INDEX)
		{
			gl::Uniform1i(_intermediateLocationV, 0);
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _blurredShadowMapTex[0]);
		}

		// Render fullscreen triangle
		gl::DrawArrays(GL_TRIANGLES, 0, 3);

		gl::Enable(GL_DEPTH_TEST);
		gl::Enable(GL_CULL_FACE);
	}

	GLuint shadowMap() override { return _blurredShadowMapTex[1]; }

	GLuint _programH;
	GLuint _programV;
	GLuint _depthLocationH;
	GLuint _blurSizeShadowMapSizeLocationH;
	GLuint _gaussianFactorsLocationH;
	GLuint _intermediateLocationV;
	GLuint _blurSizeShadowMapSizeLocationV;
	GLuint _gaussianFactorsLocationV;
	GLuint _blurredShadowMapTex[2];
	GLuint _blurredShadowMapFbo[2];
};

struct VSMShadowsSample
{
	VSMShadowsSample() {}
	~VSMShadowsSample() {}

	/// <summary>Create the required GL state.</summary>
	/// <param name="define">shader definition corresponding to the current technique</param>
	/// <param name="assetProvider">reference to IAssetProvider.</param>
	/// <param name="shadowMapPass">Pointer to shadow map pass.</param>
	/// <param name="blurPass">Pointer to gaussian blur pass.</param>
	void init(const char* define, pvr::IAssetProvider& assetProvider, ShadowMapPass* shadowMapPass, GaussianBlurPass* blurPass)
	{
		_shadowMapPass = shadowMapPass;
		_blurPass = blurPass;

		const char* defines[] = { define };

		_program = pvr::utils::createShaderProgram(assetProvider, MeshVertShaderFileName, MeshFragShaderFileName, nullptr, nullptr, 0, defines, 1);
		_modelMatLocation = gl::GetUniformLocation(_program, "ModelMat");
		_diffuseLocation = gl::GetUniformLocation(_program, "sDiffuse");
		_shadowMapLocation = gl::GetUniformLocation(_program, "sShadowMap");
		_shadowParamsLocation = gl::GetUniformLocation(_program, "ShadowParams");

		debugThrowOnApiError("ERROR: VSMShadowsSample::init");
	}

	/// <summary>Render scene with PCF shadows.</summary>
	/// <param name="shadowParams">vec4 containing shadow bias parameters</param>
	/// <param name="shell">Reference to PVR Shell</param>
	/// <param name="scene">Scene to be rendered.</param>
	void render(pvr::assets::ModelHandle scene, DeviceResources* deviceResources, glm::vec4 shadowParams, pvr::Shell& shell)
	{
		_shadowMapPass->render(scene, deviceResources);
		_blurPass->render(_shadowMapPass->_shadowMapTex);

		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		gl::Viewport(0, 0, shell.getWidth(), shell.getHeight());
		gl::ClearColor(0.0f, 0.40f, .39f, 1.0f);
		gl::Disable(GL_STENCIL_TEST);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		gl::UseProgram(_program);

		if (_shadowMapLocation != GL_INVALID_INDEX)
		{
			gl::Uniform1i(_shadowMapLocation, 0);
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _blurPass->shadowMap());
		}

		gl::Uniform4f(_shadowParamsLocation, shadowParams.x, shadowParams.y, shadowParams.z, shadowParams.w);

		// Render all mesh nodes.
		for (int i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			gl::UniformMatrix4fv(_modelMatLocation, 1, GL_FALSE, glm::value_ptr(scene->getWorldMatrix(i)));

			if (deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture != -1)
			{
				gl::Uniform1i(_diffuseLocation, 1);
				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, deviceResources->materials[pNode->getMaterialIndex()].diffuseTexture);
			}

			// bind the vertex and index buffer
			gl::BindBuffer(GL_ARRAY_BUFFER, deviceResources->vbos[meshId]);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, deviceResources->ibos[meshId]);

			// set the vertex attribute pointers
			for (size_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				auto& binding = deviceResources->vertexConfigurations[i].bindings[0];

				gl::EnableVertexAttribArray(attrib.index);
				gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
					reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
			}

			gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), nullptr);

			for (uint32_t i = 0; i < deviceResources->vertexConfigurations[i].attributes.size(); ++i)
			{
				auto& attrib = deviceResources->vertexConfigurations[i].attributes[i];
				gl::DisableVertexAttribArray(attrib.index);
			}
		}

		debugThrowOnApiError("ERROR: VSMShadowsSample::render");
	}

	GLuint _program;
	GLuint _modelMatLocation;
	GLuint _diffuseLocation;
	GLuint _shadowMapLocation;
	GLuint _shadowParamsLocation;

	ShadowMapPass* _shadowMapPass;
	GaussianBlurPass* _blurPass;
};

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class OpenGLESShadows : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;
	glm::mat4 _projMtx;
	pvr::TPSOrbitCamera _camera;
	pvr::assets::ModelHandle _scene;
	float _frame = 0.0f;
	int32_t _selectedShadowTypeIdx = (int)ShadowType::ShadowMapPCFPoissonDisk;
	glm::vec3 _lightDir;
	float _rotation = 75.0f;
	bool _rotate = false;

	/// <summary>Flag to know whether astc iss upported by the physical device.</summary>
	bool _astcSupported;

public:
	OpenGLESShadows() {}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void updateControlsUI();
	void loadResources();
	void createPasses();
	void createUBO();
	void setDefaultOpenglState();
	void eventMappedInput(pvr::SimplifiedInput action);
};

/*!*********************************************************************************************************************
\brief  handle the input event
\param  action input actions to handle
***********************************************************************************************************************/
void OpenGLESShadows::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1: {
		_rotate = !_rotate;
		break;
	}
	case pvr::SimplifiedInput::Action2: {
		_selectedShadowTypeIdx++;
		break;
	}
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}

	updateControlsUI();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
  Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
  If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OpenGLESShadows::initApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
  If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OpenGLESShadows::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
  Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OpenGLESShadows::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	_deviceResources->context = pvr::createEglContext();

	auto& dispAttrib = getDisplayAttributes();

	dispAttrib.stencilBPP = 8;

	_deviceResources->context->init(getWindow(), getDisplay(), dispAttrib, pvr::Api::OpenGLES31);

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Shadows");
	updateControlsUI();
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_astcSupported = gl::isGlExtensionSupported("GL_KHR_texture_compression_astc_ldr");

	if (isScreenRotated())
		_projMtx = pvr::math::perspectiveFov(
			pvr::Api::OpenGLES31, glm::radians(g_FOV), static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), 0.1f, 2000.f, glm::pi<float>() * .5f);
	else
		_projMtx = pvr::math::perspectiveFov(pvr::Api::OpenGLES31, glm::radians(g_FOV), static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), 0.1f, 2000.f);

	// setup the camera
	_camera.setTargetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
	_camera.setDistanceFromTarget(150.0f);
	_camera.setInclination(25.f);

	setDefaultOpenglState();

	_lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));

	loadResources();
	createUBO();
	createPasses();

	return pvr::Result::Success;
}

void OpenGLESShadows::setDefaultOpenglState()
{
	gl::Enable(GL_DEPTH_TEST);
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);
	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OpenGLESShadows::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OpenGLESShadows::renderFrame()
{
	debugThrowOnApiError("ERROR: begin frame");

	pvr::assets::AnimationInstance& animInst = _scene->getAnimationInstance(0);

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += static_cast<float>(getFrameTime()); // design-time target fps for animation

	if (_frame >= animInst.getTotalTimeInMs()) { _frame = 0; }

	// Sets the _scene animation to this _frame
	animInst.updateAnimation(_frame);

	ShadowType type = (ShadowType)(_selectedShadowTypeIdx % (int32_t)ShadowType::Count);

	_lightDir = glm::normalize(glm::vec3(sinf(getTime() * 0.001f), -1.0f, cosf(getTime() * 0.001f)));

	if (_rotate) _rotation += getFrameTime() * 0.05f;

	_camera.setAzimuth(_rotation);

	auto& uboView = _deviceResources->uboView;

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboGlobal);
	gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, _deviceResources->uboGlobal);

	void* uboData = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)_deviceResources->uboView.getSize(), GL_MAP_WRITE_BIT);

	uboView.pointToMappedMemory(uboData);

	const glm::mat4 viewProj = _projMtx * _camera.getViewMatrix();

	float shadowMapSize = 90.0f;

	glm::mat4 shadowProjMat = glm::ortho(-shadowMapSize, shadowMapSize, -shadowMapSize, shadowMapSize, 10.0f, 500.0f);

	shadowProjMat[1] *= -1.f;

	glm::vec3 shadowCamTargetPos = glm::vec3(0.0f);
	glm::vec3 shadowCamPos = -_lightDir * 250.0f;

	glm::mat4 shadowViewMat = glm::lookAt(shadowCamPos, shadowCamTargetPos, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 shadowMat = shadowProjMat * shadowViewMat;

	uboView.getElement(0).setValue(viewProj);
	uboView.getElement(1).setValue(_projMtx);
	uboView.getElement(2).setValue(_camera.getViewMatrix());
	uboView.getElement(3).setValue(shadowMat);
	uboView.getElement(4).setValue(glm::vec4(_lightDir, 0.0f));
	uboView.getElement(5).setValue(_camera.getViewMatrix() * glm::vec4(shadowCamPos, 1.0f));
	uboView.getElement(6).setValue(_camera.getViewMatrix() * glm::vec4(_lightDir, 0.0f));

	gl::UnmapBuffer(GL_UNIFORM_BUFFER);

	switch (type)
	{
	case ShadowType::None: {
		_deviceResources->noShadowsSample->render(_scene, _deviceResources.get(), *this);
		break;
	}
	case ShadowType::ShadowMapHard: {
		_deviceResources->hardShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapPCFPoissonDisk: {
		_deviceResources->pcfPoissonDiskShadowsSample->render(
			_scene, _deviceResources.get(), glm::vec4(g_PCFBias, g_PoissonSamplingRadius, g_PoissonDiskSampleCount, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapPCFOptimised2x2: {
		_deviceResources->pcfOptimised2x2ShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapPCFOptimised3x3: {
		_deviceResources->pcfOptimised3x3ShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapPCFOptimised5x5: {
		_deviceResources->pcfOptimised5x5ShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapPCFOptimised7x7: {
		_deviceResources->pcfOptimised7x7ShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize), *this);
		break;
	}
	case ShadowType::ShadowMapVSM: {
		_deviceResources->vsmFragmentShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_VSMBias, g_VSMLightBleedReduction, 0.0f, 0.0f), *this);
		break;
	}
	case ShadowType::ShadowMapEVSM2: {
		_deviceResources->evsm2FragmentShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_EVSM2Bias, g_EVSM2LightBleedReduction, 0.0f, 0.0f), *this);
		break;
	}
	case ShadowType::ShadowMapEVSM4: {
		_deviceResources->evsm4FragmentShadowsSample->render(_scene, _deviceResources.get(), glm::vec4(g_EVSM4Bias, g_EVSM4LightBleedReduction, 0.0f, 0.0f), *this);
		break;
	}
	}

	debugThrowOnApiError("ERROR: UI render");

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	debugThrowOnApiError("ERROR: Swap Buffers");

	_deviceResources->context->swapBuffers();

	debugThrowOnApiError("ERROR: end frame");

	return pvr::Result::Success;
}

void OpenGLESShadows::updateControlsUI()
{
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Pause\n"
															   "Action 2: Change Technique (" +
		std::string(ShadowTypeNames[_selectedShadowTypeIdx % (int32_t)ShadowType::Count]) + ")\n");
}

void OpenGLESShadows::loadResources()
{
	_scene = pvr::assets::loadModel(*this, ModelFileName);
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);

	_deviceResources->vertexConfigurations.resize(_scene->getNumMeshes());

	for (int i = 0; i < _scene->getNumMeshes(); i++)
	{
		_deviceResources->vertexConfigurations[i] = createInputAssemblyFromMesh(_scene->getMesh(i), VertexBindings, ARRAY_SIZE(VertexBindings));
	}

	_deviceResources->materials.resize(_scene->getNumMaterials());

	for (uint32_t i = 0; i < _scene->getNumMaterials(); ++i)
	{
		if (_scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() == static_cast<uint32_t>(-1)) { continue; }

		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		std::string textureName = _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName();
		pvr::assets::helper::getTextureNameWithExtension(textureName, _astcSupported);

		// Load the diffuse texture map
		_deviceResources->materials[i].diffuseTexture = pvr::utils::textureUpload(*this, textureName.c_str());
	}

	debugThrowOnApiError("ERROR: OpenGLESShadows::loadResources");
}

void OpenGLESShadows::createPasses()
{
	_deviceResources->shadowMapPass = std::make_shared<ShadowMapPass>();
	_deviceResources->shadowMapPass->init(*this);

	_deviceResources->noShadowsSample = std::make_shared<NoShadowsSample>();
	_deviceResources->noShadowsSample->init(*this);

	_deviceResources->hardShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->hardShadowsSample->init("SHADOW_TYPE_HARD", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->pcfPoissonDiskShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfPoissonDiskShadowsSample->init("SHADOW_TYPE_PCF_POISSON_DISK", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised2x2ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised2x2ShadowsSample->init("SHADOW_TYPE_PCF_OPTIMISED_2x2", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised3x3ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised3x3ShadowsSample->init("SHADOW_TYPE_PCF_OPTIMISED_3x3", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised5x5ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised5x5ShadowsSample->init("SHADOW_TYPE_PCF_OPTIMISED_5x5", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised7x7ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised7x7ShadowsSample->init("SHADOW_TYPE_PCF_OPTIMISED_7x7", *this, _deviceResources->shadowMapPass.get());

	_deviceResources->gaussianBlurVSMFragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurVSMFragmentPass->init("SHADOW_TYPE_VSM", *this);

	_deviceResources->gaussianBlurEVSM2FragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurEVSM2FragmentPass->init("SHADOW_TYPE_EVSM2", *this);

	_deviceResources->gaussianBlurEVSM4FragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurEVSM4FragmentPass->init("SHADOW_TYPE_EVSM4", *this, true);

	_deviceResources->vsmFragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->vsmFragmentShadowsSample->init("SHADOW_TYPE_VSM", *this, _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurVSMFragmentPass.get());

	_deviceResources->evsm2FragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm2FragmentShadowsSample->init("SHADOW_TYPE_EVSM2", *this, _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM2FragmentPass.get());

	_deviceResources->evsm4FragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm4FragmentShadowsSample->init("SHADOW_TYPE_EVSM4", *this, _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM4FragmentPass.get());
}

void OpenGLESShadows::createUBO()
{
	pvr::utils::StructuredMemoryDescription viewDesc;

	viewDesc.addElement("ViewProjMat", pvr::GpuDatatypes::mat4x4);
	viewDesc.addElement("ProjMat", pvr::GpuDatatypes::mat4x4);
	viewDesc.addElement("ViewMat", pvr::GpuDatatypes::mat4x4);
	viewDesc.addElement("ShadowMat", pvr::GpuDatatypes::mat4x4);
	viewDesc.addElement("LightDir", pvr::GpuDatatypes::vec4);
	viewDesc.addElement("LightPosVS", pvr::GpuDatatypes::vec4);
	viewDesc.addElement("LightDirVS", pvr::GpuDatatypes::vec4);

	_deviceResources->uboView.init(viewDesc);

	gl::GenBuffers(1, &_deviceResources->uboGlobal);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboGlobal);
	gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_deviceResources->uboView.getSize()), nullptr, GL_DYNAMIC_DRAW);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESShadows>(); }
