/*!********************************************************************************************
\File         OpenGLESPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRCore/cameras/TPSCamera.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace BufferEntryNames {
namespace PerMesh {
const char* const MVPMatrix = "mvpMatrix";
const char* const WorldMatrix = "worldMatrix";
} // namespace PerMesh

namespace Scene {
const char* const EyePosition = "eyePosition";
const char* const LightPosition = "lightPosition";
const char* const InverseViewProjectionMatrix = "inverseViewProjectionMatrix";
} // namespace Scene

namespace BloomConfig {
const char* const LuminosityThreshold = "luminosityThreshold";
} // namespace BloomConfig
} // namespace BufferEntryNames

// Bloom modes
enum class BloomMode
{
	NoBloom = 0,
	GaussianOriginal,
	GaussianLinear,
	Compute,
	HybridGaussian,
	GaussianLinearTruncated,
	Kawase,
	DualFilter,
	TentFilter,
	NumBloomModes,
	DefaultMode = GaussianLinearTruncated
};

// Titles for the various bloom modes
const std::string BloomStrings[] = { "Original Image (No Post Processing)", "Gaussian (Reference Implementation)", "Gaussian (Linear Sampling)",
	"Gaussian (Compute Sliding Average)", "Hybrid Gaussian", "Truncated Gaussian (Linear Sampling)", "Kawase", "Dual Filter", "Tent Filter" };

// Files used throughout the demo
namespace Files {
// Shader file names
const char Downsample2x2VertSrcFile[] = "Downsample2x2VertShader.vsh";
const char Downsample2x2FragSrcFile[] = "Downsample2x2FragShader.fsh";
const char Downsample4x4VertSrcFile[] = "Downsample4x4VertShader.vsh";
const char Downsample4x4FragSrcFile[] = "Downsample4x4FragShader.fsh";
const char DoubleDownsample2x2FragSrcFile[] = "DoubleDownsample2x2FragShader.fsh";
const char DualFilterDownSampleFragSrcFile[] = "DualFilterDownSampleFragShader.fsh";
const char DualFilterUpSampleFragSrcFile[] = "DualFilterUpSampleFragShader.fsh";
const char DualFilterUpSampleMergedFinalPassFragSrcFile[] = "DualFilterUpSampleMergedFinalPassFragShader.fsh";
const char DualFilterDownVertSrcFile[] = "DualFilterDownVertShader.vsh";
const char DualFilterUpVertSrcFile[] = "DualFilterUpVertShader.vsh";
const char TentFilterUpSampleVertSrcFile[] = "TentFilterUpSampleVertShader.vsh";
const char TentFilterUpSampleFragSrcFile[] = "TentFilterUpSampleFragShader.fsh";
const char TentFilterFirstUpSampleFragSrcFile[] = "TentFilterFirstUpSampleFragShader.fsh";
const char TentFilterUpSampleMergedFinalPassFragSrcFile[] = "TentFilterUpSampleMergedFinalPassFragShader.fsh";
const char GaussianComputeBlurHorizontalfSrcFile[] = "GaussianCompHorizontalShader.csh";
const char GaussianComputeBlurVerticalfSrcFile[] = "GaussianCompVerticalShader.csh";
const char GaussianHorizontalFragSrcFile[] = "GaussianHorizontalFragShader.fsh";
const char GaussianVerticalFragSrcFile[] = "GaussianVerticalFragShader.fsh";
const char GaussianVertSrcFile[] = "GaussianVertShader.vsh";
const char KawaseVertSrcFile[] = "KawaseVertShader.vsh";
const char KawaseFragSrcFile[] = "KawaseFragShader.fsh";
const char LinearGaussianEvenSamplesFragSrcFile[] = "LinearGaussianEvenSamplesFragShader.fsh";
const char LinearGaussianEvenSamplesHorizontalVertSrcFile[] = "LinearGaussianEvenSamplesHorizontalVertShader.vsh";
const char LinearGaussianEvenSamplesVerticalVertSrcFile[] = "LinearGaussianEvenSamplesVerticalVertShader.vsh";
const char LinearGaussianOddSamplesFragSrcFile[] = "LinearGaussianOddSamplesFragShader.fsh";
const char LinearGaussianOddSamplesHorizontalVertSrcFile[] = "LinearGaussianOddSamplesHorizontalVertShader.vsh";
const char LinearGaussianOddSamplesVerticalVertSrcFile[] = "LinearGaussianOddSamplesVerticalVertShader.vsh";
const char PostBloomVertShaderSrcFile[] = "PostBloomVertShader.vsh";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader.fsh";
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";
const char SkyboxFragShaderSrcFile[] = "SkyboxFragShader.fsh";
const char SkyboxVertShaderSrcFile[] = "SkyboxVertShader.vsh";
} // namespace Files

// POD scene files
const char SceneFile[] = "Satyr.pod";

// Texture files
const char StatueTexFile[] = "Marble.pvr";
const char StatueNormalMapTexFile[] = "MarbleNormalMap.pvr";
const char SkyboxTexFile[] = "MonValley_baked_lightmap.pvr";
const char DiffuseIrradianceMapTexFile[] = "DiffuseIrradianceMap.pvr";

// Various defaults
const float CameraNear = 1.0f;
const float CameraFar = 1000.0f;
const float RotateY = glm::pi<float>() / 150;
const float Fov = 0.80f;
const float BloomLumaThreshold = 0.8f;
const glm::vec3 LightPosition = glm::vec3(100.0, 50.0, 1000.0);
const float MinimumAcceptibleCoefficient = 0.0003f;
const uint8_t MaxDualFilterIteration = 10;
const uint8_t MaxKawaseIteration = 5;
const uint8_t MaxGaussianKernel = 51;
const uint8_t MaxGaussianHalfKernel = (MaxGaussianKernel - 1) / 2 + 1;

const pvr::utils::VertexBindings_Name VertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }, { "TANGENT", "inTangent" } };

enum class AttributeIndices
{
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	TangentArray = 3
};

// Handles the configurations being used in the demo controlling how the various bloom techniques will operate
namespace DemoConfigurations {
// Wrapper for a Kawase pass including the number of iterations in use and their kernel sizes
struct KawasePass
{
	uint32_t numIterations;
	uint32_t kernel[MaxKawaseIteration];
};

// A wrapper for the demo configuration at any time
struct DemoConfiguration
{
	std::pair<uint32_t, std::string> gaussianConfig;
	std::pair<uint32_t, std::string> linearGaussianConfig;
	std::pair<uint32_t, std::string> computeGaussianConfig;
	std::pair<uint32_t, std::string> truncatedLinearGaussianConfig;
	std::pair<KawasePass, std::string> kawaseConfig;
	std::pair<uint32_t, std::string> dualFilterConfig;
	std::pair<uint32_t, std::string> tentFilterConfig;
	std::pair<uint32_t, std::string> hybridConfig;
};

const uint32_t NumDemoConfigurations = 5;
const uint32_t DefaultDemoConfigurations = 2;
DemoConfiguration Configurations[NumDemoConfigurations]{ // Demo Blur Configurations
	DemoConfiguration{
		std::make_pair(5, "Kernel Size = 5 (5 + 5 taps)"), // Original Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (3 + 3 taps)"), // Linear Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (3 + 3 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 2, { 0, 0 } }, "2 Iterations: 0, 0"), // Kawase Blur
		std::make_pair(2, "Iterations = 2 (1 downsample, 1 upsample)"), // Dual Filter Blur
		std::make_pair(2, "Iterations = 2 (1 downsample, 1 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (5 taps), Vertical Truncated Gaussian (3 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(15, "Kernel Size = 15 (15 + 15 taps)"), // Original Gaussian Blur
		std::make_pair(15, "Kernel Size = 15 (8 + 8 taps)"), // Linear Gaussian Blur
		std::make_pair(15, "Kernel Size = 15 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(11, "Kernel Size = 11 (6 + 6 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 3, { 0, 0, 1 } }, "3 Iterations: 0, 0, 1"), // Kawase Blur
		std::make_pair(4, "Iterations = 4 (2 downsample, 2 upsample)"), // Dual Filter Blur
		std::make_pair(4, "Iterations = 4 (2 downsample, 2 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (15 taps), Vertical Truncated Gaussian (6 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(25, "Kernel Size = 25 (25 + 25 taps)"), // Original Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (13 + 13 taps)"), // Linear Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(17, "Kernel Size = 17 (9 + 9 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 4, { 0, 0, 1, 1 } }, "4 Iterations: 0, 0, 1, 1"), // Kawase Blur
		std::make_pair(6, "Iterations = 6 (3 downsample, 3 upsample)"), // Dual Filter Blur
		std::make_pair(6, "Iterations = 6 (3 downsample, 3 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (25 taps), Vertical Truncated Gaussian (9 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(35, "Kernel Size = 35 (35 + 35 taps)"), // Original Gaussian Blur
		std::make_pair(35, "Kernel Size = 35 (18 + 18 taps)"), // Linear Gaussian Blur
		std::make_pair(35, "Kernel Size = 35 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(21, "Kernel Size = 21 (11 + 11 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 4, { 0, 1, 1, 1 } }, "4 Iterations: 0, 1, 1, 1"), // Kawase Blur
		std::make_pair(8, "Iterations = 8 (4 downsample, 4 upsample)"), // Dual Filter Blur
		std::make_pair(8, "Iterations = 8 (4 downsample, 4 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (35 taps), Vertical Truncated Gaussian (11 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(51, "Kernel Size = 51 (51 + 51 taps)"), // Original Gaussian Blur
		std::make_pair(51, "Kernel Size = 51 (26 + 26 taps)"), // Linear Gaussian Blur
		std::make_pair(51, "Kernel Size = 51 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (13 + 13 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 5, { 0, 0, 1, 1, 2 } }, "5 Iterations: 0, 0, 1, 1, 2"), // Kawase Blur
		std::make_pair(10, "Iterations = 10 (5 downsample, 5 upsample)"), // Dual Filter Blur
		std::make_pair(10, "Iterations = 10 (5 downsample, 5 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (51 taps), Vertical Truncated Gaussian (13 taps)"), // Hybrid
	}
};
} // namespace DemoConfigurations

/// <summary>This class is added as the Debug Callback. Redirects the debug output to the Log object.</summary>
inline void GL_APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	Log(LogLevel::Debug, "%s", message);
}

/// <summary>Prints the gaussian weights and offsets provided in the vectors.</summary>
/// <param name="gaussianOffsets">The list of gaussian offsets to print.</param>
/// <param name="gaussianWeights">The list of gaussian weights to print.</param>
void printGaussianWeightsAndOffsets(std::vector<double>& gaussianOffsets, std::vector<double>& gaussianWeights)
{
	Log(LogLevel::Information, "const int maxStepCount = %u;", gaussianWeights.size());
	Log(LogLevel::Information, "const float gWeights[maxStepCount] =");
	Log(LogLevel::Information, "{");

	for (uint32_t i = 0; i < gaussianWeights.size() - 1; i++)
	{
		Log(LogLevel::Information, "%.15f,", gaussianWeights[i]);
	}

	Log(LogLevel::Information, "%.15f", gaussianWeights[gaussianWeights.size() - 1]);
	Log(LogLevel::Information, "};");

	Log(LogLevel::Information, "const float gOffsets[maxStepCount] =");
	Log(LogLevel::Information, "{");

	for (uint32_t i = 0; i < gaussianOffsets.size() - 1; i++)
	{
		Log(LogLevel::Information, "%.15f,", gaussianOffsets[i]);
	}

	Log(LogLevel::Information, "%.15f", gaussianOffsets[gaussianOffsets.size() - 1]);
	Log(LogLevel::Information, "};");
}

/// <summary>Updates the gaussian weights and offsets using the configuration provided.</summary>
/// <param name="kernelSize">The kernel size to generate gaussian weights and offsets for.</param>
/// <param name="useLinearOptimisation">Specifies whether linear sampling will be used when texture sampling using the given weights and offsets,
/// if linear sampling will be used then the weights and offsets must be adjusted accordingly.</param>
/// <param name="truncateCoefficients">Specifies whether to truncate and ignore coefficients which would provide a negligible change in the resulting blurred image.</param>
/// <param name="gaussianOffsets">The returned list of gaussian offsets (as double).</param>
/// <param name="gaussianWeights">The returned list of gaussian weights (as double).</param>
/// <param name="gaussianOffsetsFloats">The returned list of gaussian offsets (as float).</param>
/// <param name="gaussianWeightsFloats">The returned list of gaussian weights (as float).</param>
void updateGaussianWeightsAndOffsets(uint32_t kernelSize, bool useLinearOptimisation, bool truncateCoefficients, std::vector<double>& gaussianOffsets,
	std::vector<double>& gaussianWeights, std::vector<float>& gaussianOffsetsFloats, std::vector<float>& gaussianWeightsFloats)
{
	// Ensure that the kernel given is odd in size. Our utility function requires a central sampling position although this demo also caters for even kernel sizes
	assertion((kernelSize - 1) % 2 == 0);
	assertion(kernelSize <= MaxGaussianKernel);

	// clear the previous set of gaussian weights and offsets
	gaussianWeights.clear();
	gaussianOffsets.clear();
	gaussianWeightsFloats.clear();
	gaussianOffsetsFloats.clear();

	// generate a new set of weights and offsets based on the given configuration
	pvr::math::generateGaussianKernelWeightsAndOffsets(kernelSize, truncateCoefficients, useLinearOptimisation, gaussianWeights, gaussianOffsets, MinimumAcceptibleCoefficient);

	// The following can be used to print the current set of Gaussian Offsets and Weights in use and can be helpful during debugging
	/*
	if (useLinearOptimisation)
	{
	Log(LogLevel::Information, "Linear Sampling Optimized Gaussian Weights and Offsets:");
	}
	else
	{
	Log(LogLevel::Information, "Gaussian Weights and Offsets:");
	}
	printGaussianWeightsAndOffsets(gaussianOffsets, gaussianWeights);
	*/

	// Convert the Gaussian weights from double precision to floating point
	// Only store half of the kernel weights and offsets rather than the full kernel size set of weights and offsets as each side of the kernel will match the other meaning we
	// can save on the amount of data to upload and sample from in the shader
	if (gaussianWeights.size() % 2 == 0)
	{
		uint32_t halfKernelSize = static_cast<uint32_t>(gaussianWeights.size() / 2);

		gaussianWeightsFloats.resize(halfKernelSize);
		gaussianOffsetsFloats.resize(halfKernelSize);
		for (uint32_t i = halfKernelSize; i < gaussianWeights.size(); ++i)
		{
			gaussianWeightsFloats[i - halfKernelSize] = static_cast<float>(gaussianWeights[i]);
			gaussianOffsetsFloats[i - halfKernelSize] = static_cast<float>(gaussianOffsets[i]);
		}
	}
	else
	{
		uint32_t halfKernelSize = static_cast<uint32_t>((gaussianWeights.size() - 1) / 2 + 1);

		gaussianWeightsFloats.resize(halfKernelSize);
		gaussianOffsetsFloats.resize(halfKernelSize);
		for (uint32_t i = halfKernelSize - 1; i < gaussianWeights.size(); ++i)
		{
			gaussianWeightsFloats[i - (halfKernelSize - 1)] = static_cast<float>(gaussianWeights[i]);
			gaussianOffsetsFloats[i - (halfKernelSize - 1)] = static_cast<float>(gaussianOffsets[i]);
		}
	}
}

// A simple pass used for rendering our statue object
struct StatuePass
{
	GLuint program;
	GLuint albedoTexture;
	GLuint normalMapTexture;
	GLuint vao;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ibos;
	pvr::utils::VertexConfiguration vertexConfigurations;
	pvr::utils::VertexConfiguration vertexConfiguration;
	pvr::utils::StructuredBufferView structuredBufferView;
	GLuint buffer;
	void* mappedMemory;
	bool isBufferStorageExtSupported;

	// 3D Model
	pvr::assets::ModelHandle scene;

	/// <summary>Initialises the Statue pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="isBufferStorageExtSupported">True if GL_EXT_buffer_storage is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, bool isBufferStorageExtSupported)
	{
		this->isBufferStorageExtSupported = isBufferStorageExtSupported;

		// Load the scene
		pvr::assets::helper::loadModel(assetProvider, SceneFile, scene);
		pvr::utils::appendSingleBuffersFromModel(*scene, vbos, ibos);

		bindVertexSpecification(scene->getMesh(0), VertexBindings, 4, vertexConfigurations, vao, vbos[0], ibos[0]);

		// Create and Allocate Textures.
		albedoTexture = pvr::utils::textureUpload(assetProvider, StatueTexFile);
		normalMapTexture = pvr::utils::textureUpload(assetProvider, StatueNormalMapTexFile);
		createProgram(assetProvider);
		createBuffer();

		debugThrowOnApiError("StatuePass init");
	}

	/// <summary>Binds a vertex specification and creates a VertexArray for it.</summary>
	/// <param name="mesh">The pvr::assets::Mesh from which to bind its vertex specification.</param>
	/// <param name="vertexBindingsName">The vertex binding names.</param>
	/// <param name="numVertexBindings">The number of vertex binding names pointed to by vertexBindingsName.</param>
	/// <param name="vertexConfiguration">The vertex configuration specifying topology, attributes and bindings</param>
	/// <param name="vao">A created and returned Vertex Array Object.</param>
	/// <param name="vbo">A pre-created vbo.</param>
	/// <param name="ibo">A pre-created ibo.</param>
	void bindVertexSpecification(const pvr::assets::Mesh& mesh, const pvr::utils::VertexBindings_Name* const vertexBindingsName, const uint32_t numVertexBindings,
		pvr::utils::VertexConfiguration& vertexConfiguration, GLuint& vao, GLuint& vbo, GLuint& ibo)
	{
		vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(mesh, vertexBindingsName, numVertexBindings);

		gl::GenVertexArrays(1, &vao);
		gl::BindVertexArray(vao);
		gl::BindVertexBuffer(0, vbo, 0, mesh.getStride(0));
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		for (auto it = vertexConfiguration.attributes.begin(), end = vertexConfiguration.attributes.end(); it != end; ++it)
		{
			gl::EnableVertexAttribArray(it->index);
			gl::VertexAttribBinding(it->index, 0);
			gl::VertexAttribFormat(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format), static_cast<intptr_t>(it->offsetInBytes));
		}

		gl::BindVertexArray(0);
		for (auto it = vertexConfiguration.attributes.begin(), end = vertexConfiguration.attributes.end(); it != end; ++it)
		{
			gl::DisableVertexAttribArray(it->index);
		}
	}

	/// <summary>Creates any required buffers.</summary>
	void createBuffer()
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerMesh::MVPMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4);

		structuredBufferView.initDynamic(desc);

		gl::GenBuffers(1, &buffer);
		gl::BindBuffer(GL_UNIFORM_BUFFER, buffer);
		gl::BufferData(GL_UNIFORM_BUFFER, (size_t)structuredBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, buffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)structuredBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			mappedMemory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, structuredBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
			structuredBufferView.pointToMappedMemory(mappedMemory);
		}
	}

	/// <summary>Create the rendering program used for rendering the statue.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		const char* attributeNames[] = { VertexBindings[0].variableName.c_str(), VertexBindings[1].variableName.c_str(), VertexBindings[2].variableName.c_str(),
			VertexBindings[3].variableName.c_str() };
		const uint16_t attributeIndices[] = { static_cast<uint16_t>(AttributeIndices::VertexArray), static_cast<uint16_t>(AttributeIndices::NormalArray),
			static_cast<uint16_t>(AttributeIndices::TexCoordArray), static_cast<uint16_t>(AttributeIndices::TangentArray) };
		const uint32_t numAttributes = 4;

		program = pvr::utils::createShaderProgram(assetProvider, Files::VertShaderSrcFile, Files::FragShaderSrcFile, attributeNames, attributeIndices, numAttributes);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "sBaseTex"), 0);
		gl::Uniform1i(gl::GetUniformLocation(program, "sNormalMap"), 1);
		gl::Uniform1i(gl::GetUniformLocation(program, "irradianceMap"), 2);
	}

	/// <summary>Update the object animation.</summary>
	/// <param name="angle">The angle to use for rotating the statue.</param>
	/// <param name="viewProjectionMatrix">The view projection matrix to use for rendering.</param>
	void updateAnimation(const float angle, glm::mat4& viewProjectionMatrix)
	{
		// Calculate the model matrix
		const glm::mat4 mModel = glm::translate(glm::vec3(0.0f, 5.0f, 0.0f)) * glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(2.2f));

		glm::mat4 worldMatrix = mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
		glm::mat4 mvpMatrix = viewProjectionMatrix * worldMatrix;

		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, buffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)structuredBufferView.getSize(), GL_MAP_WRITE_BIT);
			structuredBufferView.pointToMappedMemory(mappedMemory);
		}

		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::MVPMatrix).setValue(mvpMatrix);
		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix).setValue(worldMatrix);

		if (!isBufferStorageExtSupported)
		{
			gl::UnmapBuffer(GL_UNIFORM_BUFFER);
		}
	}

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void renderMesh(uint32_t nodeIndex)
	{
		const uint32_t meshId = scene->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

		gl::BindVertexArray(vao);
		GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
		if (mesh.getMeshInfo().isIndexed)
		{
			auto indextype = mesh.getFaces().getDataType();
			GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
			// Indexed Triangle list
			gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(primitiveType, 0, mesh.getNumFaces() * 3);
		}
	}

	/// <summary>Renders the statue.</summary>
	/// <param name="thresholdBuffer">The bloom threshold buffer</param>
	/// <param name="thresholdBufferSize">The size of the bloom threshold buffer</param>
	/// <param name="sceneBuffer">The scene buffer</param>
	/// <param name="sceneBufferSize">The size of the scene buffer</param>
	/// <param name="irradianceMap">The irradiance map</param>
	/// <param name="samplerTrilinear">The trilinear sampler to use</param>
	/// <param name="irradianceSampler">A sampler to use for sampling the irradiance map</param>
	void render(GLuint thresholdBuffer, GLsizeiptr thresholdBufferSize, GLuint sceneBuffer, GLsizeiptr sceneBufferSize, GLuint irradianceMap, GLuint samplerTrilinear,
		GLuint irradianceSampler)
	{
		debugThrowOnApiError("StatuePass before render");
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, structuredBufferView.getSize());
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 1, sceneBuffer, 0, sceneBufferSize);
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 2, thresholdBuffer, 0, thresholdBufferSize);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindSampler(0, samplerTrilinear);
		gl::BindTexture(GL_TEXTURE_2D, albedoTexture);

		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindSampler(1, samplerTrilinear);
		gl::BindTexture(GL_TEXTURE_2D, normalMapTexture);

		gl::ActiveTexture(GL_TEXTURE2);
		gl::BindSampler(2, irradianceSampler);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

		gl::UseProgram(program);
		renderMesh(0);
		debugThrowOnApiError("StatuePass after render");
	}
};

// A simple pass used for rendering our skybox
struct SkyboxPass
{
	GLuint program;
	GLuint skyBoxTexture;

	/// <summary>Initialises the skybox pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void init(pvr::IAssetProvider& assetProvider)
	{
		loadSkyBoxTextures(assetProvider);
		createProgram(assetProvider);
	}

	/// <summary>Creates the textures used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void loadSkyBoxTextures(pvr::IAssetProvider& assetProvider)
	{
		// Load the Texture PVR file from the disk
		skyBoxTexture = pvr::utils::textureUpload(assetProvider, SkyboxTexFile);
	}

	/// <summary>Create the rendering program used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::SkyboxVertShaderSrcFile, Files::SkyboxFragShaderSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "skybox"), 0);
	}

	/// <summary>Renders the skybox.</summary>
	/// <param name="thresholdBuffer">The bloom threshold buffer</param>
	/// <param name="thresholdBufferSize">The size of the bloom threshold buffer</param>
	/// <param name="sceneBuffer">The scene buffer</param>
	/// <param name="sceneBufferSize">The size of the scene buffer</param>
	/// <param name="samplerTrilinear">The trilinear sampler to use</param>
	void render(GLuint thresholdBuffer, GLsizeiptr thresholdBufferSize, GLuint sceneBuffer, GLsizeiptr sceneBufferSize, GLuint samplerTrilinear)
	{
		debugThrowOnApiError("Skybox Pass before render");
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 0, thresholdBuffer, 0, thresholdBufferSize);
		gl::BindBufferRange(GL_UNIFORM_BUFFER, 1, sceneBuffer, 0, sceneBufferSize);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);
		gl::BindSampler(0, samplerTrilinear);

		gl::UseProgram(program);
		gl::DrawArrays(GL_TRIANGLES, 0, 6);
		debugThrowOnApiError("Skybox Pass after render");
	}
};

// A Downsample pass used for downsampling images by 1/4 x 1/4 i.e. 1/16 resolution OR 1/2 x 1/2 i.e. 1/4 resolution
// depending on whether GL_IMG_framebuffer_downsample is supported
struct DownSamplePass
{
	GLuint program;
	GLuint downsampleConfigUniformLocation;
	glm::vec2 blurConfigs[4];
	bool isIMGFramebufferDownsampleSupported;

	/// <summary>Initialises the Downsample pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="framebufferDimensions">The dimensions to use for the downsample pass. These dimensions should be 1/4 x 1/4 OR 1/2 x 1/2 the size of the source image.</param>
	/// <param name="isIMGFramebufferDownsampleSupported">Specifies whether the extension GL_IMG_framebuffer_downsample is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, const glm::ivec2& framebufferDimensions, bool isIMGFramebufferDownsampleSupported)
	{
		this->isIMGFramebufferDownsampleSupported = isIMGFramebufferDownsampleSupported;

		// A set of pre-calculated offsets to use for the downsample
		const glm::vec2 offsets[4] = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(-1.0, 1.0), glm::vec2(1.0, 1.0) };

		blurConfigs[0] = glm::vec2(1.0f / (framebufferDimensions.x * 4), 1.0f / (framebufferDimensions.y * 4)) * offsets[0];
		blurConfigs[1] = glm::vec2(1.0f / (framebufferDimensions.x * 4), 1.0f / (framebufferDimensions.y * 4)) * offsets[1];
		blurConfigs[2] = glm::vec2(1.0f / (framebufferDimensions.x * 4), 1.0f / (framebufferDimensions.y * 4)) * offsets[2];
		blurConfigs[3] = glm::vec2(1.0f / (framebufferDimensions.x * 4), 1.0f / (framebufferDimensions.y * 4)) * offsets[3];
		createProgram(assetProvider);

		debugThrowOnApiError("DownSamplePass init");
	}

	/// <summary>Create the rendering program used for downsampling the luminance texture.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		// GL_IMG_framebuffer_downsample is supported then we only need a program for downsampling 2x2
		if (isIMGFramebufferDownsampleSupported)
		{
			program = pvr::utils::createShaderProgram(assetProvider, Files::Downsample2x2VertSrcFile, Files::Downsample2x2FragSrcFile, nullptr, nullptr, 0);
		}
		// else the program needs to be able to downsample by 1/4 x 1/4
		else
		{
			program = pvr::utils::createShaderProgram(assetProvider, Files::Downsample4x4VertSrcFile, Files::Downsample4x4FragSrcFile, nullptr, nullptr, 0);
			gl::UseProgram(program);
			downsampleConfigUniformLocation = gl::GetUniformLocation(program, "downsampleConfigs");
			gl::Uniform2fv(downsampleConfigUniformLocation, 4, glm::value_ptr(blurConfigs[0]));
		}
	}

	/// <summary>Performs a downsample of the provided luminance texture.</summary>
	/// <param name="luminanceTexture">The the luminance texture to downsample</param>
	/// <param name="samplerBilinear">The bilinear sampler to use</param>
	void render(GLuint luminanceTexture, GLuint samplerBilinear)
	{
		debugThrowOnApiError("Downsample Pass before render");
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, luminanceTexture);
		gl::BindSampler(0, samplerBilinear);

		gl::UseProgram(program);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Downsample Pass after render");
	}
};

// Developed by Masaki Kawase, Bunkasha Games
// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
// From his GDC2003 Presentation: Frame Buffer Postprocessing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
// Multiple iterations of fixed (per iteration) offset sampling
struct KawaseBlurPass
{
	GLuint program;

	// Per iteration fixed size offset
	std::vector<uint32_t> blurKernels;

	// The number of Kawase blur iterations
	uint32_t blurIterations;

	// Uniforms used for the per iteration Kawase blur configuration
	glm::vec2 configUniforms[MaxKawaseIteration][4];

	uint32_t blurredImageIndex;

	GLuint blurConfigLocation;

	glm::ivec2 framebufferDimensions;

	/// <summary>Initialises the Kawase blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="framebufferDimensions">The dimensions to use for the Kawase blur iterations.</param>
	void init(pvr::IAssetProvider& assetProvider, const glm::ivec2& framebufferDimensions)
	{
		createProgram(assetProvider);
		blurredImageIndex = -1;
		this->framebufferDimensions = framebufferDimensions;

		debugThrowOnApiError("KawaseBlurPass init");
	}

	/// <summary>Returns the blurred image for the given configuration.</summary>
	/// <returns>The blurred image for the current configuration.</returns>
	uint32_t getBlurredImageIndex()
	{
		return blurredImageIndex;
	}

	/// <summary>Update the Kawase blur configuration.</summary>
	/// <param name="iterationsOffsets">The offsets to use in the Kawase blur passes.</param>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	void updateConfig(uint32_t* iterationsOffsets, uint32_t numIterations)
	{
		// reset/clear the kernels and number of iterations
		blurKernels.clear();
		blurIterations = 0;

		// calculate texture sample offsets based on the number of iteratios and the kernel offset currently in use for the given iteration
		glm::vec2 pixelSize = glm::vec2(1.0f / framebufferDimensions.x, 1.0f / framebufferDimensions.y);

		glm::vec2 halfPixelSize = pixelSize / 2.0f;

		for (uint32_t i = 0; i < numIterations; ++i)
		{
			blurKernels.push_back(iterationsOffsets[i]);

			glm::vec2 dUV = pixelSize * glm::vec2(blurKernels[i], blurKernels[i]) + halfPixelSize;

			configUniforms[i][0] = glm::vec2(-dUV.x, dUV.y);
			configUniforms[i][1] = glm::vec2(dUV);
			configUniforms[i][2] = glm::vec2(dUV.x, -dUV.y);
			configUniforms[i][3] = glm::vec2(-dUV.x, -dUV.y);
		}
		blurIterations = numIterations;
		assertion(blurIterations <= MaxKawaseIteration);

		blurredImageIndex = !(numIterations % 2);
	}

	/// <summary>Creates the Kawase program.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider)
	{
		program = pvr::utils::createShaderProgram(assetProvider, Files::KawaseVertSrcFile, Files::KawaseFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(program);
		gl::Uniform1i(gl::GetUniformLocation(program, "sTexture"), 0);

		blurConfigLocation = gl::GetUniformLocation(program, "blurConfigs");
	}

	/// <summary>Performs a Kawase blur using the current configuration.</summary>
	/// <param name="horizontalBlurFramebuffer">The first of the ping ponged framebuffers.</param>
	/// <param name="verticalBlurFramebuffer">The second of the ping ponged framebuffers.</param>
	/// <param name="pingPong0Texture">The first of the ping ponged textures.</param>
	/// <param name="pingPong1Texture">The second of the ping ponged textures.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling from the ping-ponged images during the Kawase blur passes.</param>
	void render(GLuint horizontalBlurFramebuffer, GLuint verticalBlurFramebuffer, GLuint pingPong0Texture, GLuint pingPong1Texture, GLuint samplerBilinear)
	{
		GLuint framebuffers[2] = { horizontalBlurFramebuffer, verticalBlurFramebuffer };
		GLuint textures[2] = { pingPong0Texture, pingPong1Texture };

		// Iterate through the Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			debugThrowOnApiError("Kawase Pass before render");
			// calculate the ping pong index based on the current iteration
			uint32_t pingPongIndex = i % 2;

			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[pingPongIndex]);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, textures[pingPongIndex]);
			gl::BindSampler(0, samplerBilinear);

			gl::UseProgram(program);
			gl::Uniform2fv(blurConfigLocation, 4, glm::value_ptr(configUniforms[i][0]));
			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Kawase Pass after render");
		}
	}
};

// Developed by Marius Bj�rge (ARM)
// Bandwidth-Efficient Rendering - siggraph2015-mmg-marius
// Filters images whilst Downsampling and Upsampling
struct DualFilterBlurPass
{
	// We only need (MaxDualFilterIteration - 1) images as the first image is an input to the blur pass
	// We also special case the final pass as this requires either a different pipeline or a different descriptor set/layout

	// Special cased final pass pipeline where the final upsample pass and compositing occurs in the same pipeline. This lets us avoid an extra write to memory/read from memory pass
	GLuint finalPassProgram;
	GLuint finalPassBloomOnlyProgram;

	GLuint upSampleProgram;
	GLuint downSampleProgram;
	GLuint doubleDownSampleProgram;

	// The pre allocated framebuffers for the iterations up to MaxDualFilterIteration
	GLuint framebuffers[MaxDualFilterIteration - 1];

	// The current set of framebuffers in use for the currently selected configuration
	GLuint currentFramebuffers[MaxDualFilterIteration - 1];

	// The pre allocated image views for the iterations up to MaxDualFilterIteration
	GLuint textures[MaxDualFilterIteration - 1];

	// The current set of image views in use for the currently selected configuration
	GLuint currentTextures[MaxDualFilterIteration - 1];

	// The framebuffer dimensions for the current configuration
	std::vector<glm::vec2> currentIterationDimensions;

	// The framebuffer inverse dimensions for the current configuration
	std::vector<glm::vec2> currentIterationInverseDimensions;

	// The full set of framebuffer dimensions
	std::vector<glm::vec2> maxIterationDimensions;

	// The full set of framebuffer inverse dimensions
	std::vector<glm::vec2> maxIterationInverseDimensions;

	// The number of Dual Filter iterations currently in use
	uint32_t blurIterations;

	// The current set of uniforms for the current configuration
	glm::vec2 configUniforms[MaxDualFilterIteration][8];

	// The final full resolution framebuffer dimensions
	glm::ivec2 framebufferDimensions;

	// The color image format in use
	GLuint colorImageFormat;

	GLuint upSampleBlurConfigLocation;
	GLuint downSampleBlurConfigLocation;
	GLuint finalUpSampleBlurConfigLocation;
	GLuint finalUpSampleBlurBloomOnlyConfigLocation;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="colorImageFormat">The color image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions">The full size resolution framebuffer dimensions.</param>
	void init(pvr::IAssetProvider& assetProvider, GLuint colorImageFormat, const glm::ivec2& framebufferDimensions, bool srgbFramebuffer)
	{
		this->colorImageFormat = colorImageFormat;
		this->framebufferDimensions = framebufferDimensions;
		this->blurIterations = -1;

		createBuffers();

		// Calculate the maximum set of per iteration framebuffer dimensions
		// The maximum set will start from framebufferDimensions and allow for MaxDualFilterIteration. Note that this includes down and up sample passes
		calculateIterationDimensions();

		// Allocates the images used for each of the down/up sample passes
		allocatePingPongTextures();

		// Create the dual filter framebuffers
		createFramebuffers();

		// Create the up and down sample programs
		createPrograms(assetProvider, srgbFramebuffer);
	}

	/// <summary>Creates any required buffers - The Dual Filter blur pass doesn't use any buffers.</summary>
	virtual void createBuffers() {}

	/// <summary>Returns the blurred texture.</summary>
	/// <returns>The blurred texture.</returns>
	GLuint getBlurredTexture()
	{
		return currentTextures[blurIterations - 1];
	}

	/// <summary>Update the Dual Filter blur configuration.</summary>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	/// <param name="initial">Specifies whether the Dual Filter pass is being initialised.</param>
	void updateConfig(uint32_t numIterations, bool initial = false)
	{
		// We only update the Dual Filter configuration if the number of iterations has actually been modified
		if (numIterations != blurIterations || initial)
		{
			blurIterations = numIterations;

			assertion(blurIterations % 2 == 0);

			// Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration
			getIterationDimensions(currentIterationDimensions, currentIterationInverseDimensions, blurIterations);

			// Configure the Dual Filter uniform values based on the current Dual Filter configuration
			configureConfigUniforms();

			// Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration
			configurePingPongTextures();

			// Configure the set of Framebuffers based on the current Dual Filter configuration
			configureFramebuffers();
		}
	}

	/// <summary>Configure the set of Framebuffers based on the current Dual Filter configuration.</summary>
	virtual void configureFramebuffers()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentFramebuffers[index] = framebuffers[index];
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentFramebuffers[index] = framebuffers[i];
			index++;
		}
	}

	/// <summary>Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration.</summary>
	virtual void configurePingPongTextures()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentTextures[index] = textures[index];
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentTextures[index] = textures[i];
			index++;
		}
	}

	/// <summary>Calculate the full set of Dual Filter iteration dimensions.</summary>
	void calculateIterationDimensions()
	{
		maxIterationDimensions.resize(MaxDualFilterIteration);
		maxIterationInverseDimensions.resize(MaxDualFilterIteration);

		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		glm::ivec2 dimension =
			glm::ivec2(glm::ceil(framebufferDimensions.x / glm::pow(2, MaxDualFilterIteration / 2)), glm::ceil(framebufferDimensions.y / glm::pow(2, MaxDualFilterIteration / 2)));

		for (int32_t i = (MaxDualFilterIteration / 2) - 1; i >= 0; --i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::ivec2(glm::ceil(dimension.x * 2.0f), glm::ceil(dimension.y * 2.0f));
		}

		dimension = glm::ivec2(
			glm::ceil(framebufferDimensions.x / glm::pow(2, MaxDualFilterIteration / 2 - 1)), glm::ceil(framebufferDimensions.y / glm::pow(2, MaxDualFilterIteration / 2 - 1)));

		for (uint32_t i = MaxDualFilterIteration / 2; i < MaxDualFilterIteration - 1; ++i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::ivec2(glm::ceil(dimension.x * 2.0f), glm::ceil(dimension.y * 2.0f));
		}

		dimension = glm::ivec2(glm::ceil(framebufferDimensions.x), glm::ceil(framebufferDimensions.y));

		maxIterationDimensions[MaxDualFilterIteration - 1] = dimension;
		glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
		maxIterationInverseDimensions[MaxDualFilterIteration - 1] = inverseDimensions;
	}

	/// <summary>Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration.</summary>
	/// <param name="iterationDimensions">A returned list of dimensions to use for the current configuration.</param>
	/// <param name="iterationInverseDimensions">A returned list of inverse dimensions to use for the current configuration.</param>
	/// <param name="numIterations">Specifies the number of iterations used in the current configuration.</param>
	void getIterationDimensions(std::vector<glm::vec2>& iterationDimensions, std::vector<glm::vec2>& iterationInverseDimensions, uint32_t numIterations)
	{
		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		iterationDimensions.clear();
		iterationInverseDimensions.clear();

		for (uint32_t i = 0; i < numIterations / 2; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[i]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[i]);
		}

		uint32_t index = MaxDualFilterIteration - (numIterations / 2);
		for (uint32_t i = numIterations / 2; i < numIterations; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[index]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[index]);
			index++;
		}
	}

	/// <summary>Allocates the textures used for each of the down / up sample passes.</summary>
	virtual void allocatePingPongTextures()
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration / 2; ++i)
		{
			gl::GenTextures(1, &textures[i]);
			gl::BindTexture(GL_TEXTURE_2D, textures[i]);
			gl::TexStorage2D(GL_TEXTURE_2D, 1, colorImageFormat, static_cast<GLsizei>(maxIterationDimensions[i].x), static_cast<GLsizei>(maxIterationDimensions[i].y));
		}

		// We're able to reuse images between up/down sample passes. This can help us keep down the total number of images in flight
		uint32_t k = 0;
		for (uint32_t i = MaxDualFilterIteration / 2; i < MaxDualFilterIteration - 1; ++i)
		{
			uint32_t reuseIndex = (MaxDualFilterIteration / 2) - 1 - (k + 1);
			textures[i] = textures[reuseIndex];
			k++;
		}
	}

	/// <summary>Allocates the framebuffers used for each of the down / up sample passes.</summary>
	virtual void createFramebuffers()
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			gl::GenFramebuffers(1, &framebuffers[i]);
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[i]);
			gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(maxIterationDimensions[i].x));
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(maxIterationDimensions[i].y));
		}
	}

	/// <summary>Creates the Dual Filter programs.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	virtual void createPrograms(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		downSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterDownVertSrcFile, Files::DualFilterDownSampleFragSrcFile, nullptr, nullptr, 0);
		upSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleFragSrcFile, nullptr, nullptr, 0);

		gl::UseProgram(downSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(downSampleProgram, "sTexture"), 0);
		downSampleBlurConfigLocation = gl::GetUniformLocation(downSampleProgram, "blurConfigs");

		gl::UseProgram(upSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sTexture"), 0);
		upSampleBlurConfigLocation = gl::GetUniformLocation(upSampleProgram, "blurConfigs");

		finalPassProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleMergedFinalPassFragSrcFile, nullptr, nullptr, 0,
			defines.data(), static_cast<uint32_t>(defines.size()));

		defines.push_back("RENDER_BLOOM");
		finalPassBloomOnlyProgram = pvr::utils::createShaderProgram(assetProvider, Files::DualFilterUpVertSrcFile, Files::DualFilterUpSampleMergedFinalPassFragSrcFile, nullptr,
			nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		GLuint programs[] = { finalPassProgram, finalPassBloomOnlyProgram };

		for (uint32_t i = 0; i < 2; ++i)
		{
			gl::UseProgram(programs[i]);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sTexture"), 0);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sOffScreenTexture"), 1);
		}

		gl::UseProgram(finalPassProgram);
		finalUpSampleBlurConfigLocation = gl::GetUniformLocation(finalPassProgram, "blurConfigs");

		gl::UseProgram(finalPassBloomOnlyProgram);
		finalUpSampleBlurBloomOnlyConfigLocation = gl::GetUniformLocation(finalPassBloomOnlyProgram, "blurConfigs");
	}

	/// <summary>Configure the Dual Filter uniforms values based on the current Dual Filter configuration.</summary>
	virtual void configureConfigUniforms()
	{
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			// Downsample
			if (i < blurIterations / 2)
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				configUniforms[i][0] = glm::vec2(-dUV);
				configUniforms[i][1] = glm::vec2(dUV);
				configUniforms[i][2] = glm::vec2(dUV.x, -dUV.y);
				configUniforms[i][3] = glm::vec2(-dUV.x, dUV.y);
			}
			// Upsample
			else
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				configUniforms[i][0] = glm::vec2(-dUV.x * 2.0, 0.0);
				configUniforms[i][1] = glm::vec2(-dUV.x, dUV.y);
				configUniforms[i][2] = glm::vec2(0.0, dUV.y * 2.0);
				configUniforms[i][3] = glm::vec2(dUV.x, dUV.y);
				configUniforms[i][4] = glm::vec2(dUV.x * 2.0, 0.0);
				configUniforms[i][5] = glm::vec2(dUV.x, -dUV.y);
				configUniforms[i][6] = glm::vec2(0.0, -dUV.y * 2.0);
				configUniforms[i][7] = glm::vec2(-dUV.x, -dUV.y);
			}
		}
	}

	/// <summary>Renders the Dual Filter blur iterations based on the current configuration.</summary>
	/// <param name="luminanceTexture">The source luminance texture.</param>
	/// <param name="offscreenTexture">The offscreen texture.</param>
	/// <param name="onScreenFbo">The on screen fbo.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling Dual Filter.</param>
	virtual void render(GLuint luminanceTexture, GLuint offscreenTexture, GLuint onScreenFbo, GLuint samplerBilinear, bool renderBloomOnly)
	{
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

			if (i == 0)
			{
				debugThrowOnApiError("Dual Filter First Pass before render");
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, luminanceTexture);
				gl::BindSampler(0, samplerBilinear);

				gl::UseProgram(downSampleProgram);
				gl::Uniform2fv(downSampleBlurConfigLocation, 4, glm::value_ptr(configUniforms[i][0]));
			}
			// Special case the final Dual Filter iteration
			else if (i == blurIterations - 1)
			{
				debugThrowOnApiError("Dual Filter Final Pass before render");
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, onScreenFbo);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[blurIterations - 2]);
				gl::BindSampler(0, samplerBilinear);

				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, offscreenTexture);
				gl::BindSampler(1, samplerBilinear);

				if (renderBloomOnly)
				{
					gl::UseProgram(finalPassBloomOnlyProgram);
					gl::Uniform2fv(finalUpSampleBlurBloomOnlyConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
				}
				else
				{
					gl::UseProgram(finalPassProgram);
					gl::Uniform2fv(finalUpSampleBlurConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
				}
			}
			else
			{
				debugThrowOnApiError("Dual Filter Pass before render");
				// Down sample passes
				if (i < blurIterations / 2)
				{
					gl::UseProgram(downSampleProgram);
					gl::Uniform2fv(downSampleBlurConfigLocation, 4, glm::value_ptr(configUniforms[i][0]));
				}
				// Up sample passes
				else
				{
					gl::UseProgram(upSampleProgram);
					gl::Uniform2fv(upSampleBlurConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
				}
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
				gl::BindSampler(0, samplerBilinear);
			}

			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Dual Filter Pass after render");
		}
	}
};

// Presented in "Next Generation Post Processing In Call Of Duty Advanced Warfare" by Jorge Jimenez
// Filters whilst Downsampling and Upsampling
// Downsamples:
//	Used for preventing aliasing artifacts
//		A = downsample2(FullRes)
//		B = downsample2(A)
//		C = downsample2(B)
//		D = downsample2(C)
//		E = downsample2(D)
// Upsamples:
//	Used for image quality and smooth results
//	Upsampling progressively using bilinear filtering is equivalent to bi-quadratic b-spline filtering
//	We do the sum with the previous mip as we upscale
//		E' = E
//		D' = D + blur(E')
//		C' = C + blur(D')
//		B' = B + blur(C')
//		A' = A + blur(B')
//	The tent filter (3x3) - uses a radius parameter : 1 2 1
//												      2 4 2 * 1/16
//													  1 2 1
// Described here: http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
// We make use of the DualFilterBlurPass as these passes share many similarities
struct DownAndTentFilterBlurPass : public DualFilterBlurPass
{
	// The down sample image views
	GLuint downSampledTextures[MaxDualFilterIteration / 2];
	GLuint doubleDownSampleFramebuffers[MaxDualFilterIteration / 4];
	GLuint firstUpSampleProgram;
	bool isIMGFramebufferDownsampleSupported;
	uint32_t blurScale;
	// Defines a scale to use for offsetting the tent offsets
	glm::vec2 tentScale;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="colorImageFormat">The color image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions">The full size resolution framebuffer dimensions.</param>
	/// <param name="isIMGFramebufferDownsampleSupported">Specifies whether the extension GL_IMG_framebuffer_downsample is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, GLuint colorImageFormat, const glm::ivec2& framebufferDimensions, bool isIMGFramebufferDownsampleSupported, bool srgbFramebuffer)
	{
		this->isIMGFramebufferDownsampleSupported = isIMGFramebufferDownsampleSupported;
		if (this->isIMGFramebufferDownsampleSupported)
		{
			// If isIMGFramebufferDownsampleSupported is supported then 2x2 downsample must be supported
			this->blurScale = 2;
		}
		else
		{
			this->blurScale = -1;
		}
		tentScale = glm::vec2(3.0f, 3.0f);
		DualFilterBlurPass::init(assetProvider, colorImageFormat, framebufferDimensions, srgbFramebuffer);
	}

	/// <summary>Allocates the framebuffers used for each of the down / up sample passes.</summary>
	void createFramebuffers() override
	{
		DualFilterBlurPass::createFramebuffers();

		// If GL_IMG_framebuffer_downsample is supported then we make use of the extension to achieve what we're coining as a double downsample pass
		if (isIMGFramebufferDownsampleSupported)
		{
			uint32_t doubleDownsampleIndex = 0;
			for (uint32_t i = 0; i < (MaxDualFilterIteration / 2) - ((MaxDualFilterIteration / 2) % 2); i = i + 2)
			{
				gl::GenFramebuffers(1, &doubleDownSampleFramebuffers[doubleDownsampleIndex]);
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, doubleDownSampleFramebuffers[doubleDownsampleIndex]);
				gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
				gl::ext::FramebufferTexture2DDownsampleIMG(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[i + 1], 0, blurScale, blurScale);
				gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(maxIterationDimensions[i].x));
				gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(maxIterationDimensions[i].y));
				pvr::utils::checkFboStatus();
				doubleDownsampleIndex++;
			}
		}
	}

	void configureFramebuffers() override
	{
		uint32_t index = 0;
		if (isIMGFramebufferDownsampleSupported && (blurIterations / 2) >= 2)
		{
			// make use of the double downsample framebuffers
			uint32_t doubleDownsampleIndex = 0;
			bool needsExtraDownSample = blurIterations / 2 % 2 == 1;
			for (; index < (blurIterations / 2) - needsExtraDownSample; index = index + 2)
			{
				currentFramebuffers[index] = doubleDownSampleFramebuffers[doubleDownsampleIndex];
				doubleDownsampleIndex++;
			}
			// Add any other non double down sample framebuffers to finish the downsamples
			for (; index < blurIterations / 2; ++index)
			{
				currentFramebuffers[index] = framebuffers[index];
			}
		}
		else
		{
			for (; index < blurIterations / 2; ++index)
			{
				currentFramebuffers[index] = framebuffers[index];
			}
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentFramebuffers[index] = framebuffers[i];
			index++;
		}
	}

	void allocatePingPongTextures() override
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			gl::GenTextures(1, &textures[i]);
			gl::BindTexture(GL_TEXTURE_2D, textures[i]);
			gl::TexStorage2D(GL_TEXTURE_2D, 1, colorImageFormat, static_cast<GLsizei>(maxIterationDimensions[i].x), static_cast<GLsizei>(maxIterationDimensions[i].y));
		}
	}

	void configurePingPongTextures() override
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentTextures[index] = textures[index];
			downSampledTextures[index] = currentTextures[index];
		}

		for (uint32_t i = 0; i < (blurIterations / 2) - 1; ++i)
		{
			currentTextures[index] = textures[(MaxDualFilterIteration - (blurIterations / 2)) + i];
			index++;
		}
	}

	virtual void createPrograms(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer) override
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		downSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::Downsample2x2VertSrcFile, Files::Downsample2x2FragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(downSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(downSampleProgram, "sTexture"), 0);

		// Create a special case double downsample program if GL_IMG_framebuffer_downsample is supported
		if (isIMGFramebufferDownsampleSupported)
		{
			doubleDownSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::Downsample2x2VertSrcFile, Files::DoubleDownsample2x2FragSrcFile, nullptr, nullptr, 0);
			gl::UseProgram(doubleDownSampleProgram);
			gl::Uniform1i(gl::GetUniformLocation(doubleDownSampleProgram, "sTexture"), 0);
		}

		firstUpSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::Downsample2x2VertSrcFile, Files::TentFilterFirstUpSampleFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(firstUpSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(firstUpSampleProgram, "sCurrentBlurredImage"), 0);

		upSampleProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(upSampleProgram);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sCurrentBlurredImage"), 0);
		gl::Uniform1i(gl::GetUniformLocation(upSampleProgram, "sDownsampledCurrentMipLevel"), 1);
		upSampleBlurConfigLocation = gl::GetUniformLocation(upSampleProgram, "upSampleConfigs");

		finalPassProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleMergedFinalPassFragSrcFile, nullptr,
			nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		defines.push_back("RENDER_BLOOM");
		finalPassBloomOnlyProgram = pvr::utils::createShaderProgram(assetProvider, Files::TentFilterUpSampleVertSrcFile, Files::TentFilterUpSampleMergedFinalPassFragSrcFile,
			nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		GLuint programs[] = { finalPassProgram, finalPassBloomOnlyProgram };

		for (uint32_t i = 0; i < 2; ++i)
		{
			gl::UseProgram(programs[i]);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sCurrentBlurredImage"), 0);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sDownsampledCurrentMipLevel"), 1);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sOffScreenTexture"), 2);
		}

		gl::UseProgram(finalPassProgram);
		finalUpSampleBlurConfigLocation = gl::GetUniformLocation(finalPassProgram, "upSampleConfigs");

		gl::UseProgram(finalPassBloomOnlyProgram);
		finalUpSampleBlurBloomOnlyConfigLocation = gl::GetUniformLocation(finalPassBloomOnlyProgram, "upSampleConfigs");
	}

	virtual void configureConfigUniforms() override
	{
		const glm::vec2 offsets[8] = { glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, -1.0), glm::vec2(0.0, -1.0),
			glm::vec2(-1.0, -1.0), glm::vec2(-1.0, 0.0) };

		for (uint32_t i = 0; i < blurIterations; i++)
		{
			configUniforms[i][0] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[0] * tentScale;
			configUniforms[i][1] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[1] * tentScale;
			configUniforms[i][2] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[2] * tentScale;
			configUniforms[i][3] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[3] * tentScale;
			configUniforms[i][4] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[4] * tentScale;
			configUniforms[i][5] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[5] * tentScale;
			configUniforms[i][6] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[6] * tentScale;
			configUniforms[i][7] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[7] * tentScale;
		}
	}

	virtual void render(GLuint luminanceTexture, GLuint offscreenTexture, GLuint onScreenFbo, GLuint samplerBilinear, bool renderBloomOnly) override
	{
		uint32_t downsampledIndex = 1;

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindSampler(0, samplerBilinear);

		uint32_t i = 0;

		// if true we should attempt a double downsample (If GL_IMG_framebuffer_downsample is
		// supported and if there are enough downsample passes to take advantage of a double downsample)
		if (isIMGFramebufferDownsampleSupported && (blurIterations / 2) >= 2)
		{
			uint32_t doubleDownsampleIterations = (blurIterations / 2) / 2;

			GLenum doubleDownsampleDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

			bool needsExtraDownSample = blurIterations / 2 % 2 == 1;

			for (; i < doubleDownsampleIterations; i++)
			{
				debugThrowOnApiError("Tent Filter Double Downsample Pass before render");
				uint32_t iterationIndex = i * 2;
				gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[iterationIndex].x), static_cast<GLsizei>(currentIterationDimensions[iterationIndex].y));

				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, doubleDownSampleFramebuffers[i]);
				gl::DrawBuffers(2, doubleDownsampleDrawBuffers);
				gl::Clear(GL_COLOR_BUFFER_BIT);
				gl::UseProgram(doubleDownSampleProgram);

				if (iterationIndex == 0)
				{
					gl::BindTexture(GL_TEXTURE_2D, luminanceTexture);
				}
				else
				{
					gl::BindTexture(GL_TEXTURE_2D, currentTextures[iterationIndex - 1]);
				}

				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			}

			// We may need to perform a single (0 or 1) extra single downsamples
			if (needsExtraDownSample)
			{
				debugThrowOnApiError("Tent Filter Downsample Pass before render");
				i = (blurIterations / 2) - 1;

				gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::DrawBuffers(1, drawBuffers);
				gl::Clear(GL_COLOR_BUFFER_BIT);
				gl::UseProgram(downSampleProgram);

				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);

				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			}
		}
		// if GL_IMG_framebuffer_downsample is not supported then carry out the traditional downsample passes
		else
		{
			for (; i < blurIterations / 2; i++)
			{
				debugThrowOnApiError("Tent Filter Downsample Pass before render");
				gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::DrawBuffers(1, drawBuffers);
				gl::Clear(GL_COLOR_BUFFER_BIT);
				gl::UseProgram(downSampleProgram);

				if (i == 0)
				{
					gl::BindTexture(GL_TEXTURE_2D, luminanceTexture);
				}
				else
				{
					gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
				}

				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			}
		}

		// Up sample passes
		i = blurIterations / 2;

		for (; i < blurIterations; i++)
		{
			gl::Viewport(0, 0, static_cast<GLsizei>(currentIterationDimensions[i].x), static_cast<GLsizei>(currentIterationDimensions[i].y));

			// final pass
			if (i == blurIterations - 1)
			{
				debugThrowOnApiError("Tent Filter Final Up sample Pass before render");
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, onScreenFbo);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
				gl::BindSampler(0, samplerBilinear);

				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, downSampledTextures[0]);
				gl::BindSampler(1, samplerBilinear);

				gl::ActiveTexture(GL_TEXTURE2);
				gl::BindTexture(GL_TEXTURE_2D, offscreenTexture);
				gl::BindSampler(2, samplerBilinear);

				if (renderBloomOnly)
				{
					gl::UseProgram(finalPassBloomOnlyProgram);
					gl::Uniform2fv(finalUpSampleBlurBloomOnlyConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
				}
				else
				{
					gl::UseProgram(finalPassProgram);
					gl::Uniform2fv(finalUpSampleBlurConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
				}
			}
			// first upsample
			else if (i == blurIterations / 2)
			{
				debugThrowOnApiError("Tent Filter First Up sample Pass before render");
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::DrawBuffers(1, drawBuffers);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindSampler(0, samplerBilinear);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);

				gl::UseProgram(firstUpSampleProgram);
			}
			// up sample
			else
			{
				debugThrowOnApiError("Tent Filter Up sample Pass before render");
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFramebuffers[i]);
				gl::DrawBuffers(1, drawBuffers);
				gl::Clear(GL_COLOR_BUFFER_BIT);

				gl::ActiveTexture(GL_TEXTURE0);
				gl::BindTexture(GL_TEXTURE_2D, currentTextures[i - 1]);
				gl::BindSampler(0, samplerBilinear);

				gl::ActiveTexture(GL_TEXTURE1);
				gl::BindTexture(GL_TEXTURE_2D, downSampledTextures[blurIterations / 2 - 1 - downsampledIndex]);
				gl::BindSampler(1, samplerBilinear);
				downsampledIndex++;

				gl::UseProgram(upSampleProgram);
				gl::Uniform2fv(upSampleBlurConfigLocation, 8, glm::value_ptr(configUniforms[i][0]));
			}

			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			debugThrowOnApiError("Tent Filter Pass after render");
		}
	}
};

// A Gaussian Blur Pass
struct GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines
	GLuint horizontalProgram;
	GLuint verticalProgram;

	// Gaussian offsets and weights
	std::vector<double> gaussianOffsets;
	std::vector<double> gaussianWeights;

	std::vector<float> gaussianOffsetsFloats;
	std::vector<float> gaussianWeightsFloats;

	// stores details regarding the Gaussian blur configuration currently in use
	glm::vec4 blurConfig;

	// the size of the gaussian kernel currently in use
	uint32_t kernelSize;
	uint32_t ssboSize;

	GLuint bloomConfigBuffer;

	void* mappedMemory;

	bool isBufferStorageExtSupported;

	/// <summary>Initialises the Gaussian blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="blurFramebufferDimensions">The dimensions used for the blur.</param>
	/// <param name="isBufferStorageExtSupported">True if GL_EXT_buffer_storage is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, const glm::ivec2& blurFramebufferDimensions, bool isBufferStorageExtSupported)
	{
		this->isBufferStorageExtSupported = isBufferStorageExtSupported;
		createBuffer();
		blurConfig = glm::vec4(1.0f / blurFramebufferDimensions.x, 1.0f / blurFramebufferDimensions.y, 0.0f, 0.0f);

		createPrograms(assetProvider);

		debugThrowOnApiError("GaussianBlurPass init");
	}

	/// <summary>Updates the kernel configuration currently in use.</summary>
	/// <param name="kernelSizeConfig">The kernel size.</param>
	/// <param name="useLinearOptimisation">Specifies whether the offsets and weights used for the current kernel size should be modified
	/// (optimised) for use with linear sampling.</param>
	/// <param name="truncateCoefficients">Provides us with an efficient and convenient mechanism for achieving blurs approximating
	/// blurs with larger kernel sizes Note that using making use "truncateCoefficients" can result in a loss of a low value high
	/// precision tail in the blur. Depending on the scene/image being blurred the effect of using "truncateCoefficients" may not be
	/// particularly visible however in some cases it may be required that high precision low value tails are present - it may be artistically
	/// required that no loss is observed. Using a traditional 8 bit per channel color buffer we could very well get away with the loss of the
	/// very low value tails. Using a HDR 16 bit per channel color buffer we may get away with ignoring negligible coefficients depending on
	/// the minimum coefficient we deem to be non-negligible. For the most accurate of blur then we would recommened against its use however
	/// if speed is of the utmost importance and if you can get away with the quality degradation then ignoring negligible coefficients
	/// is a great way to reduce the number of texture samples. We will leave this decision to the reader..</param>
	virtual void updateKernelConfig(uint32_t kernelSizeConfig, bool useLinearOptimisation, bool truncateCoefficients)
	{
		kernelSize = kernelSizeConfig;
		updateGaussianWeightsAndOffsets(kernelSize, useLinearOptimisation, truncateCoefficients, gaussianOffsets, gaussianWeights, gaussianOffsetsFloats, gaussianWeightsFloats);

		blurConfig.z = static_cast<float>(gaussianOffsetsFloats.size());
	}

	/// <summary>Updates the kernel buffers for the specified swapchain index.</summary>
	virtual void updateKernelBuffer()
	{
		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, bloomConfigBuffer);
			mappedMemory = gl::MapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, (size_t)ssboSize, GL_MAP_WRITE_BIT);
		}

		memcpy(static_cast<char*>(mappedMemory), &blurConfig, pvr::getSize(pvr::GpuDatatypes::vec4));
		memcpy(static_cast<char*>(mappedMemory) + pvr::getSize(pvr::GpuDatatypes::vec4), gaussianWeightsFloats.data(), pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel);
		memcpy(static_cast<char*>(mappedMemory) + pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel,
			gaussianOffsetsFloats.data(), pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel);

		if (!isBufferStorageExtSupported)
		{
			gl::UnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
	}

	/// <summary>Creates any required buffers.</summary>
	virtual void createBuffer()
	{
		ssboSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel * 2);

		gl::GenBuffers(1, &bloomConfigBuffer);
		gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, bloomConfigBuffer);
		gl::BufferData(GL_SHADER_STORAGE_BUFFER, (size_t)ssboSize, nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, bloomConfigBuffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)ssboSize, 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			mappedMemory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, ssboSize, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
		}
	}

	/// <summary>Creates the Gaussian blur programs.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	virtual void createPrograms(pvr::IAssetProvider& assetProvider)
	{
		horizontalProgram = pvr::utils::createShaderProgram(assetProvider, Files::GaussianVertSrcFile, Files::GaussianHorizontalFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(horizontalProgram);
		gl::Uniform1i(gl::GetUniformLocation(horizontalProgram, "sTexture"), 0);
		verticalProgram = pvr::utils::createShaderProgram(assetProvider, Files::GaussianVertSrcFile, Files::GaussianVerticalFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(verticalProgram);
		gl::Uniform1i(gl::GetUniformLocation(verticalProgram, "sTexture"), 0);
	}

	/// <summary>Renders Dual Filter blur iterations based on the current configuration.</summary>
	/// <param name="downsampledTexture">The source downsampled luminance texture.</param>
	/// <param name="horizontallyBlurredTexture">The texture which will contain the horizontal Gaussian blur.</param>
	/// <param name="horizontalBlurFramebuffer">The framebuffer to use in the horizontal Gaussian blur.</param>
	/// <param name="verticalBlurFramebuffer">The framebuffer to use in the vertical Gaussian blur.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	virtual void render(GLuint downsampledTexture, GLuint horizontallyBlurredTexture, GLuint horizontalBlurFramebuffer, GLuint verticalBlurFramebuffer, GLuint samplerBilinear)
	{
		debugThrowOnApiError("Gaussian Blur Pass before render");
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, horizontalBlurFramebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bloomConfigBuffer);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, downsampledTexture);
		gl::BindSampler(0, samplerBilinear);

		gl::UseProgram(horizontalProgram);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindTexture(GL_TEXTURE_2D, horizontallyBlurredTexture);
		gl::UseProgram(verticalProgram);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Gaussian Blur Pass after render");
	}
};

// A Compute shader based Gaussian Blur Pass
struct ComputeBlurPass : public GaussianBlurPass
{
	// For our compute shader based Gaussian blur we duplicate the Gaussian weights so that we don't need special shader logic to handle buffer overruns
	std::vector<float> duplicatedGaussianWeightsFloats;

	virtual void createBuffer()
	{
		ssboSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianKernel * 2);

		gl::GenBuffers(1, &bloomConfigBuffer);
		gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, bloomConfigBuffer);
		gl::BufferData(GL_SHADER_STORAGE_BUFFER, (size_t)ssboSize, nullptr, GL_DYNAMIC_DRAW);

		// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
		if (isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_COPY_READ_BUFFER, bloomConfigBuffer);
			gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)ssboSize, 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

			mappedMemory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, ssboSize, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
		}
	}

	virtual void updateKernelBuffer()
	{
		if (!isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, bloomConfigBuffer);
			mappedMemory = gl::MapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, (size_t)ssboSize, GL_MAP_WRITE_BIT);
		}

		memcpy(static_cast<char*>(mappedMemory), &blurConfig, pvr::getSize(pvr::GpuDatatypes::vec4));
		memcpy(static_cast<char*>(mappedMemory) + pvr::getSize(pvr::GpuDatatypes::vec4), duplicatedGaussianWeightsFloats.data(),
			pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianKernel * 2);

		if (!isBufferStorageExtSupported)
		{
			gl::UnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
	}

	virtual void updateKernelConfig(uint32_t kernelSizeConfig, bool useLinearOptimisation, bool truncateCoefficients)
	{
		kernelSize = kernelSizeConfig;
		updateGaussianWeightsAndOffsets(kernelSize, useLinearOptimisation, truncateCoefficients, gaussianOffsets, gaussianWeights, gaussianOffsetsFloats, gaussianWeightsFloats);

		duplicatedGaussianWeightsFloats.clear();

		blurConfig.z = static_cast<float>(gaussianWeights.size());

		for (uint32_t duplications = 0; duplications < 2; duplications++)
		{
			for (uint32_t i = 0; i < gaussianWeightsFloats.size(); i++)
			{
				duplicatedGaussianWeightsFloats.push_back(gaussianWeightsFloats[gaussianWeightsFloats.size() - 1 - i]);
			}

			for (uint32_t i = 1; i < gaussianWeightsFloats.size(); i++)
			{
				duplicatedGaussianWeightsFloats.push_back(gaussianWeightsFloats[i]);
			}
		}
	}

	virtual void createPrograms(pvr::IAssetProvider& assetProvider)
	{
		horizontalProgram = pvr::utils::createComputeShaderProgram(assetProvider, Files::GaussianComputeBlurHorizontalfSrcFile);
		verticalProgram = pvr::utils::createComputeShaderProgram(assetProvider, Files::GaussianComputeBlurVerticalfSrcFile);
	}

	using GaussianBlurPass::render;
	virtual void render(
		GLuint downsampledTexture, GLuint horizontallyBlurredTexture, GLuint horizontalBlurFramebuffer, GLuint verticalBlurFramebuffer, const glm::ivec2& blurFramebufferDimensions)
	{
		debugThrowOnApiError("Compute Gaussian Blur Pass before render");
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bloomConfigBuffer);

		// horizontal
		{
			// We Execute the Compute shader, we bind the input and output texture.
			gl::UseProgram(horizontalProgram);

			gl::BindImageTexture(0, downsampledTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			gl::BindImageTexture(1, horizontallyBlurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(blurFramebufferDimensions.y / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// vertical
		{
			gl::UseProgram(verticalProgram);

			gl::BindImageTexture(0, horizontallyBlurredTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			gl::BindImageTexture(1, downsampledTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(blurFramebufferDimensions.x / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		debugThrowOnApiError("Compute Gaussian Blur Pass after render");
	}
};

// A Linear sampler optimised Gaussian Blur Pass
struct LinearGaussianBlurPass : public GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines to handle special cases where the number of samples will be an even number
	GLuint evenSampleHorizontalProgram;
	GLuint evenSampleVerticalProgram;

	virtual void createPrograms(pvr::IAssetProvider& assetProvider) override
	{
		horizontalProgram =
			pvr::utils::createShaderProgram(assetProvider, Files::LinearGaussianOddSamplesHorizontalVertSrcFile, Files::LinearGaussianOddSamplesFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(horizontalProgram);
		gl::Uniform1i(gl::GetUniformLocation(horizontalProgram, "sTexture"), 0);
		verticalProgram =
			pvr::utils::createShaderProgram(assetProvider, Files::LinearGaussianOddSamplesVerticalVertSrcFile, Files::LinearGaussianOddSamplesFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(verticalProgram);
		gl::Uniform1i(gl::GetUniformLocation(verticalProgram, "sTexture"), 0);

		evenSampleHorizontalProgram =
			pvr::utils::createShaderProgram(assetProvider, Files::LinearGaussianEvenSamplesHorizontalVertSrcFile, Files::LinearGaussianEvenSamplesFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(evenSampleHorizontalProgram);
		gl::Uniform1i(gl::GetUniformLocation(evenSampleHorizontalProgram, "sTexture"), 0);
		evenSampleVerticalProgram =
			pvr::utils::createShaderProgram(assetProvider, Files::LinearGaussianEvenSamplesVerticalVertSrcFile, Files::LinearGaussianEvenSamplesFragSrcFile, nullptr, nullptr, 0);
		gl::UseProgram(evenSampleVerticalProgram);
		gl::Uniform1i(gl::GetUniformLocation(evenSampleVerticalProgram, "sTexture"), 0);
	}

	virtual void render(GLuint downsampledTexture, GLuint horizontallyBlurredTexture, GLuint horizontalBlurFramebuffer, GLuint verticalBlurFramebuffer, GLuint samplerBilinear) override
	{
		debugThrowOnApiError("Linear Gaussian Blur Pass before render");
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, horizontalBlurFramebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bloomConfigBuffer);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, downsampledTexture);
		gl::BindSampler(0, samplerBilinear);

		// appropriately handle odd vs even numbers of taps
		if (gaussianWeights.size() % 2 == 0)
		{
			gl::UseProgram(evenSampleHorizontalProgram);
		}
		else
		{
			gl::UseProgram(horizontalProgram);
		}
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer);
		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::BindTexture(GL_TEXTURE_2D, horizontallyBlurredTexture);
		if (gaussianWeights.size() % 2 == 0)
		{
			gl::UseProgram(evenSampleVerticalProgram);
		}
		else
		{
			gl::UseProgram(verticalProgram);
		}
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		debugThrowOnApiError("Linear Gaussian Blur Pass after render");
	}
};

// A Hybrid Gaussian Blur pass making use of a horizontal Compute shader pass followed by a Fragment based Vertical Gaussian Blur Pass
struct HybridGaussianBlurPass
{
	// The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources
	ComputeBlurPass* computeBlurPass;
	// The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources
	LinearGaussianBlurPass* linearBlurPass;

	/// <summary>A minimal initialisation function as no extra resources are created for this type of blur pass and instead we make use of the compute and fragment based passes.</summary>
	/// <param name="computeBlurPass">The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources.</param>
	/// <param name="linearBlurPass">The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources.</param>
	void init(ComputeBlurPass* computeBlurPass, LinearGaussianBlurPass* linearBlurPass)
	{
		this->computeBlurPass = computeBlurPass;
		this->linearBlurPass = linearBlurPass;
	}

	/// <summary>Renders A hybrid gaussian blur based on the current configuration.</summary>
	/// <param name="downsampledTexture">The source downsampled luminance texture.</param>
	/// <param name="horizontallyBlurredTexture">The texture which will contain the horizontal Gaussian blur.</param>
	/// <param name="horizontalBlurFramebuffer">The framebuffer to use in the horizontal Gaussian blur.</param>
	/// <param name="verticalBlurFramebuffer">The framebuffer to use in the vertical Gaussian blur.</param>
	/// <param name="blurFramebufferDimensions">The dimensions used for the blur.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	void render(GLuint downsampledTexture, GLuint horizontallyBlurredTexture, GLuint horizontalBlurFramebuffer, GLuint verticalBlurFramebuffer,
		const glm::ivec2& blurFramebufferDimensions, GLuint samplerBilinear)
	{
		debugThrowOnApiError("Hybrid Gaussian Blur Pass before render");
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->computeBlurPass->bloomConfigBuffer);

		// horizontal
		{
			// We Execute the Compute shader, we bind the input and output texture.
			gl::UseProgram(this->computeBlurPass->horizontalProgram);

			gl::BindImageTexture(0, downsampledTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			gl::BindImageTexture(1, horizontallyBlurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			gl::DispatchCompute(static_cast<uint32_t>(glm::ceil(blurFramebufferDimensions.y / 32.0f)), 1, 1);
			gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// vertical
		{
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, verticalBlurFramebuffer);
			gl::Clear(GL_COLOR_BUFFER_BIT);

			gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->linearBlurPass->bloomConfigBuffer);

			gl::BindTexture(GL_TEXTURE_2D, horizontallyBlurredTexture);
			gl::BindSampler(0, samplerBilinear);
			if (this->linearBlurPass->gaussianWeights.size() % 2 == 0)
			{
				gl::UseProgram(this->linearBlurPass->evenSampleVerticalProgram);
			}
			else
			{
				gl::UseProgram(this->linearBlurPass->verticalProgram);
			}
			gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		}
		debugThrowOnApiError("Hybrid Gaussian Blur Pass after render");
	}
};

// Post bloom composition pass
struct PostBloomPass
{
	GLuint defaultProgram;
	GLuint bloomOnlyProgram;
	GLuint offscreenOnlyProgram;
	void* mappedMemory;
	bool isBufferStorageExtSupported;

	/// <summary>Initialises the Post Bloom pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="isBufferStorageExtSupported">True if GL_EXT_buffer_storage is supported.</param>
	void init(pvr::IAssetProvider& assetProvider, bool isBufferStorageExtSupported, bool srgbFramebuffer)
	{
		this->isBufferStorageExtSupported = isBufferStorageExtSupported;
		createProgram(assetProvider, srgbFramebuffer);

		debugThrowOnApiError("PostBloomPass init");
	}

	/// <summary>Creates the Kawase program.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	void createProgram(pvr::IAssetProvider& assetProvider, bool srgbFramebuffer)
	{
		// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
		std::vector<const char*> defines;
		if (srgbFramebuffer)
		{
			defines.push_back("FRAMEBUFFER_SRGB");
		}

		defaultProgram = pvr::utils::createShaderProgram(
			assetProvider, Files::PostBloomVertShaderSrcFile, Files::PostBloomFragShaderSrcFile, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		defines.push_back("RENDER_BLOOM");
		bloomOnlyProgram = pvr::utils::createShaderProgram(
			assetProvider, Files::PostBloomVertShaderSrcFile, Files::PostBloomFragShaderSrcFile, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		defines[defines.size() - 1] = "RENDER_OFFSCREEN";
		offscreenOnlyProgram = pvr::utils::createShaderProgram(
			assetProvider, Files::PostBloomVertShaderSrcFile, Files::PostBloomFragShaderSrcFile, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

		GLuint programs[] = { defaultProgram, bloomOnlyProgram, offscreenOnlyProgram };

		for (uint32_t i = 0; i < 3; ++i)
		{
			gl::UseProgram(programs[i]);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sBlurTexture"), 0);
			gl::Uniform1i(gl::GetUniformLocation(programs[i], "sOriginalTexture"), 1);
		}
	}

	/// <summary>Renders the post bloom composition pass.</summary>
	/// <param name="blurTexture">The bloomed luminance texture.</param>
	/// <param name="originalTexture">The original HDR texture.</param>
	/// <param name="samplerBilinear">The sampler object to use when sampling.</param>
	void render(GLuint blurTexture, GLuint originalTexture, GLuint samplerBilinear, bool renderBloomOnly, bool renderOffScreenOnly)
	{
		debugThrowOnApiError("Post Bloom Pass before render");
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, blurTexture);
		gl::BindSampler(0, samplerBilinear);
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, originalTexture);
		gl::BindSampler(1, samplerBilinear);

		if (renderOffScreenOnly)
		{
			gl::UseProgram(offscreenOnlyProgram);
		}
		else if (renderBloomOnly)
		{
			gl::UseProgram(bloomOnlyProgram);
		}
		else
		{
			gl::UseProgram(defaultProgram);
		}

		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		debugThrowOnApiError("Post Bloom Pass after render");
	}
};

/*!********************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************/
class OpenGLESPostProcessing : public pvr::Shell
{
	pvr::EglContext _context;

	// Framebuffers
	GLuint _offScreenFramebuffer;
	GLuint _offScreenNoDownsampleFramebuffer;
	GLuint _blurFramebuffers[2];
	GLuint _computeBlurFramebuffers[2];

	// Textures
	GLuint _luminanceTexture;
	GLuint _downSampledLuminanceTexture;
	GLuint _offScreenColorTexture;

	GLuint _pingPongTextures[2];
	GLuint _computePingPongTextures[2];

	GLuint _diffuseIrradianceTexture;

	// Samplers
	GLuint _samplerNearest;
	GLuint _samplerBilinear;
	GLuint _samplerTrilinear;
	GLuint _irradianceSampler;

	GLuint _depthStencilTexture;

	// UIRenderers used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// Buffers
	pvr::utils::StructuredBufferView _sceneBufferView;
	GLuint _sceneBuffer;

	pvr::utils::StructuredBufferView _bloomThresholdBufferView;
	GLuint _bloomThresholdBuffer;

	SkyboxPass _skyBoxPass;
	StatuePass _statuePass;
	PostBloomPass _postBloomPass;

	// Blur Passes
	GaussianBlurPass _gaussianBlurPass;
	LinearGaussianBlurPass _linearGaussianBlurPass;
	LinearGaussianBlurPass _truncatedLinearGaussianBlurPass;

	DualFilterBlurPass _dualFilterBlurPass;
	DownAndTentFilterBlurPass _downAndTentFilterBlurPass;
	ComputeBlurPass _computeBlurPass;
	HybridGaussianBlurPass _hybridGaussianBlurPass;

	KawaseBlurPass _kawaseBlurPass;

	DownSamplePass _downsamplePass;

	GLenum _luminanceColorFormat;
	GLenum _computeLuminanceColorFormat;
	GLenum _offscreenColorFormat;

	glm::ivec2 _blurFramebufferDimensions;
	glm::vec2 _blurInverseFramebufferDimensions;
	uint32_t _blurScale;
	uint32_t _IMGFramebufferScale;

	bool _animateObject;
	bool _animateCamera;
	float _objectAngleY;
	float _cameraAngle;
	pvr::TPSCamera _camera;
	float _logicTime;
	float _modeSwitchTime;
	bool _isManual;
	float _modeDuration;

	glm::vec3 _lightPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;

	BloomMode _blurMode;

	bool _useThreshold;

	uint32_t _currentDemoConfiguration;

	bool _isIMGFramebufferDownsampleSupported;
	bool _isBufferStorageExtSupported;
	bool _mustUpdateDemoConfig;

	float _bloomLumaThreshold;

	bool _renderOnlyBloom;

	std::string _currentBlurString;

	GLenum _drawBuffers[1];
	GLenum _mrtDrawBuffers[2];

public:
	OpenGLESPostProcessing() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createBuffers();
	void createSceneBuffer();
	void createBloomThresholdBuffer();
	void getDownScaleFactor(GLint& xDownscale, GLint& yDownscale);
	void createBlurFramebuffers();
	void createSamplers();
	void allocatePingPongTextures();
	void createOffScreenFramebuffers();
	void createUiRenderer();
	void updateBlurDescription();
	void renderUI();
	void updateDemoConfigs();
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateBloomConfiguration();
	void updateAnimation();
	void updateDynamicSceneData();
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::initApplication()
{
	this->setStencilBitsPerPixel(0);

	_animateObject = true;
	_animateCamera = false;
	_lightPosition = LightPosition;
	_useThreshold = true;
	_objectAngleY = 0.0f;
	_cameraAngle = 240.0f;
	_camera.setDistanceFromTarget(200.f);
	_camera.setHeight(-15.f);
	_blurScale = 4;
	_IMGFramebufferScale = -1;
	_logicTime = 0.0f;
	_modeSwitchTime = 0.0f;
	_isManual = false;
	_modeDuration = 1.5f;

	_drawBuffers[0] = GL_COLOR_ATTACHMENT0;
	_mrtDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	_mrtDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

	_isIMGFramebufferDownsampleSupported = false;
	_isBufferStorageExtSupported = false;

	// Handle command line arguments including "blurmode", "blursize" and "bloom"
	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intBloomMode = -1;
	if (commandOptions.getIntOption("-blurmode", intBloomMode))
	{
		if (intBloomMode > static_cast<int32_t>(BloomMode::NumBloomModes))
		{
			_blurMode = BloomMode::DefaultMode;
		}
		else
		{
			_isManual = true;
			_blurMode = static_cast<BloomMode>(intBloomMode);
		}
	}
	else
	{
		_blurMode = BloomMode::DefaultMode;
	}

	int32_t intConfigSize = -1;
	if (commandOptions.getIntOption("-blursize", intConfigSize))
	{
		if (intConfigSize > static_cast<int32_t>(DemoConfigurations::NumDemoConfigurations))
		{
			_currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations;
		}
		else
		{
			_isManual = true;
			_currentDemoConfiguration = intConfigSize;
		}
	}
	else
	{
		_currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations;
	}

	_renderOnlyBloom = false;
	commandOptions.getBoolOptionSetTrueIfPresent("-bloom", _renderOnlyBloom);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	debugThrowOnApiError("InitView Begin");

	if (gl::isGlExtensionSupported("GL_KHR_debug"))
	{
		gl::ext::DebugMessageCallbackKHR(&debugCallback, NULL);
	}

	// Check for GL_IMG_framebuffer_downsample support
	if (gl::isGlExtensionSupported("GL_IMG_framebuffer_downsample"))
	{
		_isIMGFramebufferDownsampleSupported = true;
	}

	// Determine the extent of the support for GL_IMG_framebuffer_downsample
	if (_isIMGFramebufferDownsampleSupported)
	{
		GLint xDownscale, yDownscale;
		getDownScaleFactor(xDownscale, yDownscale);

		Log("Using GL_IMG_framebuffer_downsample");
		Log("Chosen Downsampling factor: %i, %i", xDownscale, yDownscale);

		_IMGFramebufferScale = static_cast<uint32_t>(xDownscale);
	}

	// Ensure the extension GL_EXT_color_buffer_float is supported
	if (!gl::isGlExtensionSupported("GL_EXT_color_buffer_float"))
	{
		setExitMessage("GL_EXT_color_buffer_float is not supported.");
		return pvr::Result::UnknownError;
	}

	// We make use of GL_EXT_buffer_storage wherever possible
	_isBufferStorageExtSupported = gl::isGlExtensionSupported("GL_EXT_buffer_storage");

	_luminanceColorFormat = GL_R16F;
	// Only a subset of formats have support for Image Load Store.
	// A subset of these also support linear filtering.
	// We use GL_RGBA16F as it has support for both
	_computeLuminanceColorFormat = GL_RGBA16F;
	_offscreenColorFormat = GL_RGBA16F;

	// calculate the frame buffer width and heights
	_blurFramebufferDimensions = glm::ivec2(this->getWidth() / _blurScale, this->getHeight() / _blurScale);
	_blurInverseFramebufferDimensions = glm::vec2(1.0f / _blurFramebufferDimensions.x, 1.0f / _blurFramebufferDimensions.y);

	// Calculates the projection matrices
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix =
			pvr::math::perspectiveFov(_context->getApiVersion(), Fov, static_cast<float>(getHeight()), static_cast<float>(getWidth()), CameraNear, CameraFar, glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(_context->getApiVersion(), Fov, static_cast<float>(getWidth()), static_cast<float>(getHeight()), CameraNear, CameraFar);
	}

	// create demo buffers
	createBuffers();

	_diffuseIrradianceTexture = pvr::utils::textureUpload(*this, DiffuseIrradianceMapTexFile);

	// Allocate two images to use which can be "ping-ponged" between when applying various filters/blurs
	// Pass 1: Read From 1, Render to 0
	// Pass 2: Read From 0, Render to 1
	allocatePingPongTextures();

	// Create the HDR offscreen framebuffers
	createOffScreenFramebuffers();

	// Create the samplers used for various texture sampling
	createSamplers();

	_statuePass.init(*this, _isBufferStorageExtSupported);
	_skyBoxPass.init(*this);

	createBlurFramebuffers();

	_downsamplePass.init(*this, _blurFramebufferDimensions, _isIMGFramebufferDownsampleSupported);
	_postBloomPass.init(*this, _isBufferStorageExtSupported, getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// Initialise the Blur Passes
	// Gaussian Blurs
	{
		_gaussianBlurPass.init(*this, _blurFramebufferDimensions, _isBufferStorageExtSupported);

		_linearGaussianBlurPass.init(*this, _blurFramebufferDimensions, _isBufferStorageExtSupported);

		_truncatedLinearGaussianBlurPass.init(*this, _blurFramebufferDimensions, _isBufferStorageExtSupported);

		_computeBlurPass.init(*this, _blurFramebufferDimensions, _isBufferStorageExtSupported);

		_hybridGaussianBlurPass.init(&_computeBlurPass, &_truncatedLinearGaussianBlurPass);
	}

	// Kawase Blur
	{
		_kawaseBlurPass.init(*this, _blurFramebufferDimensions);
	}

	// Dual Filter Blur
	{
		_dualFilterBlurPass.init(*this, _luminanceColorFormat, glm::ivec2(this->getWidth(), this->getHeight()), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	}

	// Down Sample and Tent filter blur pass
	{
		_downAndTentFilterBlurPass.init(*this, _luminanceColorFormat, glm::ivec2(this->getWidth(), this->getHeight()), _isIMGFramebufferDownsampleSupported,
			getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	}

	// initalise the UI Renderers
	createUiRenderer();

	// Initially update the bloom configurations
	updateDemoConfigs();

	// signals that the demo configuration and associated buffers need updating
	_mustUpdateDemoConfig = true;

	gl::BindFramebuffer(GL_FRAMEBUFFER, _context->getOnScreenFbo());
	gl::UseProgram(0);

	gl::Disable(GL_BLEND);
	gl::Disable(GL_STENCIL_TEST);

	gl::Enable(GL_DEPTH_TEST); // depth test
	gl::DepthMask(GL_TRUE); // depth write enabled
	gl::DepthFunc(GL_LESS);

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_FRONT);
	gl::FrontFace(GL_CW);

	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::ClearDepthf(1.0f);
	gl::ClearStencil(0);

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESPostProcessing::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	// update dynamic buffers
	updateDynamicSceneData();

	// Set the viewport for full screen rendering
	gl::Viewport(0, 0, getWidth(), getHeight());

	// Bind the offscreen framebuffer appropriately
	// Note that the DualFilter and TentFilter take care of their own downsampling
	if (_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter)
	{
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenNoDownsampleFramebuffer);
	}
	else
	{
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebuffer);
	}

	// Setup the set of draw buffers
	gl::DrawBuffers(2, _mrtDrawBuffers);

	// Clear the color and depth
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Perform Scene rendering
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	gl::DepthFunc(GL_LESS);
	_statuePass.render(
		_bloomThresholdBuffer, _bloomThresholdBufferView.getSize(), _sceneBuffer, _sceneBufferView.getSize(), _diffuseIrradianceTexture, _samplerTrilinear, _irradianceSampler);

	gl::DepthFunc(GL_LEQUAL);
	_skyBoxPass.render(_bloomThresholdBuffer, _bloomThresholdBufferView.getSize(), _sceneBuffer, _sceneBufferView.getSize(), _samplerTrilinear);

	// Disable depth testing, from this point onwards we don't need depth
	gl::Disable(GL_DEPTH_TEST);

	{
		std::vector<GLenum> invalidateAttachments;
		invalidateAttachments.push_back(GL_DEPTH_ATTACHMENT);
		invalidateAttachments.push_back(GL_STENCIL_ATTACHMENT);
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
	}

	// Set draw buffers for rendering to only a single attachment
	gl::DrawBuffers(1, _drawBuffers);

	if (_blurMode != BloomMode::NoBloom)
	{
		// Perform a downsample if the bloom mode is not DualFilter or TentFilter
		if (!(_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter))
		{
			gl::Viewport(0, 0, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y);

			if (_blurMode == BloomMode::Compute || _blurMode == BloomMode::HybridGaussian)
			{
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _computeBlurFramebuffers[1]);
			}
			else
			{
				gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _blurFramebuffers[1]);
			}
			gl::Clear(GL_COLOR_BUFFER_BIT);

			// If isIMGFramebufferDownsampleSupported is supported 2x2 downsample must be supported
			// At this point _downSampledLuminanceTexture already contains a downsampled image but if the scale is 2x2 then we must still downsample the image again
			if (_isIMGFramebufferDownsampleSupported && _IMGFramebufferScale == 2)
			{
				_downsamplePass.render(_downSampledLuminanceTexture, _samplerBilinear);
			}
			else
			{
				_downsamplePass.render(_luminanceTexture, _samplerBilinear);
			}
		}

		// Render the bloom
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_gaussianBlurPass.render(_pingPongTextures[1], _pingPongTextures[0], _blurFramebuffers[0], _blurFramebuffers[1], _samplerNearest);
			break;
		}

		case (BloomMode::GaussianLinear):
		{
			_linearGaussianBlurPass.render(_pingPongTextures[1], _pingPongTextures[0], _blurFramebuffers[0], _blurFramebuffers[1], _samplerBilinear);
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_truncatedLinearGaussianBlurPass.render(_pingPongTextures[1], _pingPongTextures[0], _blurFramebuffers[0], _blurFramebuffers[1], _samplerBilinear);
			break;
		}
		case (BloomMode::Compute):
		{
			_computeBlurPass.render(_computePingPongTextures[1], _computePingPongTextures[0], _blurFramebuffers[0], _blurFramebuffers[1], _blurFramebufferDimensions);
			break;
		}
		case (BloomMode::Kawase):
		{
			_kawaseBlurPass.render(_blurFramebuffers[0], _blurFramebuffers[1], _pingPongTextures[1], _pingPongTextures[0], _samplerBilinear);
			break;
		}
		case (BloomMode::DualFilter):
		{
			_dualFilterBlurPass.render(_luminanceTexture, _offScreenColorTexture, _context->getOnScreenFbo(), _samplerBilinear, _renderOnlyBloom);
			break;
		}
		case (BloomMode::TentFilter):
		{
			_downAndTentFilterBlurPass.render(_luminanceTexture, _offScreenColorTexture, _context->getOnScreenFbo(), _samplerBilinear, _renderOnlyBloom);
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			_hybridGaussianBlurPass.render(
				_computePingPongTextures[1], _computePingPongTextures[0], _computeBlurFramebuffers[0], _blurFramebuffers[1], _blurFramebufferDimensions, _samplerBilinear);
			break;
		}
		default:
			throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}
	}

	// If Dual or Tent filter then the composition is taken care of during the final up sample
	if (_blurMode != BloomMode::DualFilter && _blurMode != BloomMode::TentFilter)
	{
		GLuint blurredTexture = -1;

		// Ensure the post bloom pass uses the correct blurred image for the current blur mode
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			blurredTexture = _pingPongTextures[1];
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			blurredTexture = _pingPongTextures[1];
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			blurredTexture = _pingPongTextures[1];
			break;
		}
		case (BloomMode::Compute):
		{
			blurredTexture = _computePingPongTextures[1];
			break;
		}
		case (BloomMode::Kawase):
		{
			blurredTexture = _pingPongTextures[_kawaseBlurPass.getBlurredImageIndex()];
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			blurredTexture = _pingPongTextures[1];
			break;
		}
		case (BloomMode::DualFilter):
		{
			blurredTexture = _dualFilterBlurPass.getBlurredTexture();
			break;
		}
		case (BloomMode::TentFilter):
		{
			blurredTexture = _downAndTentFilterBlurPass.getBlurredTexture();
			break;
		}
		case (BloomMode::NoBloom):
		{
			if (_isIMGFramebufferDownsampleSupported)
			{
				blurredTexture = _downSampledLuminanceTexture;
			}
			else
			{
				blurredTexture = _luminanceTexture;
			}
			break;
		}
		default:
			throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}

		gl::Viewport(0, 0, getWidth(), getHeight());

		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _context->getOnScreenFbo());
		gl::Clear(GL_COLOR_BUFFER_BIT);
		_postBloomPass.render(blurredTexture, _offScreenColorTexture, _samplerBilinear, _renderOnlyBloom, _blurMode == BloomMode::NoBloom);
	}

	renderUI();

	{
		std::vector<GLenum> invalidateAttachments;
		invalidateAttachments.push_back(GL_DEPTH);
		invalidateAttachments.push_back(GL_STENCIL);
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
	}

	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::releaseView()
{
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESPostProcessing::quitApplication()
{
	return pvr::Result::Success;
}

/// <summary>Creates The UI renderer.</summary>
void OpenGLESPostProcessing::createUiRenderer()
{
	_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_uiRenderer.getDefaultTitle()->commitUpdates();
	_uiRenderer.getDefaultControls()->setText("Left / right: Blur Mode\n"
											  "Up / Down: Blur Size\n"
											  "Action 1: Enable/Disable Bloom\n"
											  "Action 2: Enable/Disable Animation\n");
	_uiRenderer.getDefaultControls()->commitUpdates();

	updateBlurDescription();
	_uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_uiRenderer.getDefaultDescription()->commitUpdates();

	debugThrowOnApiError("createUiRenderer");
}

/// <summary>Updates the description for the currently used blur technique.</summary>
void OpenGLESPostProcessing::updateBlurDescription()
{
	switch (_blurMode)
	{
	case (BloomMode::NoBloom):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)];
		break;
	}
	case (BloomMode::GaussianOriginal):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig.second;
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig.second;
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.second;
		break;
	}
	case (BloomMode::Compute):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.second;
		break;
	}
	case (BloomMode::DualFilter):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig.second;
		break;
	}
	case (BloomMode::TentFilter):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].tentFilterConfig.second;
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].hybridConfig.second;
		break;
	}
	case (BloomMode::Kawase):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.second;
		break;
	}
	default:
		throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
	}

	Log(LogLevel::Information, "Current blur mode: \"%s\"", BloomStrings[static_cast<int32_t>(_blurMode)].c_str());
	Log(LogLevel::Information, "Current blur size configiuration: \"%u\"", _currentDemoConfiguration);
}

/// <summary>Creates the main scene buffer.</summary>
void OpenGLESPostProcessing::createSceneBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::Scene::InverseViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::Scene::EyePosition, pvr::GpuDatatypes::vec3);
	desc.addElement(BufferEntryNames::Scene::LightPosition, pvr::GpuDatatypes::vec3);

	_sceneBufferView.init(desc);

	gl::GenBuffers(1, &_sceneBuffer);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _sceneBuffer);
	gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_sceneBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

	// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
	if (_isBufferStorageExtSupported)
	{
		gl::BindBuffer(GL_COPY_READ_BUFFER, _sceneBuffer);
		gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)_sceneBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

		void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, _sceneBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
		_sceneBufferView.pointToMappedMemory(memory);
	}
}

/// <summary>Creates the bloom threshold buffer.</summary>
void OpenGLESPostProcessing::createBloomThresholdBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::BloomConfig::LuminosityThreshold, pvr::GpuDatatypes::Float);

	_bloomThresholdBufferView.init(desc);

	gl::GenBuffers(1, &_bloomThresholdBuffer);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _bloomThresholdBuffer);
	gl::BufferData(GL_UNIFORM_BUFFER, (size_t)_bloomThresholdBufferView.getSize(), nullptr, GL_DYNAMIC_DRAW);

	// if GL_EXT_buffer_storage is supported then map the buffer upfront and never upmap it
	if (_isBufferStorageExtSupported)
	{
		gl::BindBuffer(GL_COPY_READ_BUFFER, _bloomThresholdBuffer);
		gl::ext::BufferStorageEXT(GL_COPY_READ_BUFFER, (GLsizei)_bloomThresholdBufferView.getSize(), 0, GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

		void* memory = gl::MapBufferRange(GL_COPY_READ_BUFFER, 0, _bloomThresholdBufferView.getSize(), GL_MAP_WRITE_BIT_EXT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
		_bloomThresholdBufferView.pointToMappedMemory(memory);
	}
}

/// <summary>Creates main application buffers.</summary>
void OpenGLESPostProcessing::createBuffers()
{
	createSceneBuffer();
	createBloomThresholdBuffer();

	// update the bloom threshold buffer
	if (_useThreshold)
	{
		// This threshold values controls the minimum luminosity value any fragment must have to be used as part of the bloom
		_bloomLumaThreshold = BloomLumaThreshold;
	}
	else
	{
		_bloomLumaThreshold = 0.0;
	}

	void* mappedMemory = nullptr;
	if (!_isBufferStorageExtSupported)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _bloomThresholdBuffer);
		mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_bloomThresholdBufferView.getSize(), GL_MAP_WRITE_BIT);
		_bloomThresholdBufferView.pointToMappedMemory(mappedMemory);
	}

	_bloomThresholdBufferView.getElementByName(BufferEntryNames::BloomConfig::LuminosityThreshold).setValue(_bloomLumaThreshold);

	if (!_isBufferStorageExtSupported)
	{
		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}

	debugThrowOnApiError("createBuffers");
}

/// <summary>Allocates the various ping pong textures used throughout the demo.</summary>
void OpenGLESPostProcessing::allocatePingPongTextures()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		gl::GenTextures(1, &_pingPongTextures[i]);
		gl::BindTexture(GL_TEXTURE_2D, _pingPongTextures[i]);
		gl::TexStorage2D(GL_TEXTURE_2D, 1, _luminanceColorFormat, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
	}

	debugThrowOnApiError("allocatePingPongTextures");

	for (uint32_t i = 0; i < 2; ++i)
	{
		gl::GenTextures(1, &_computePingPongTextures[i]);
		gl::BindTexture(GL_TEXTURE_2D, _computePingPongTextures[i]);
		gl::TexStorage2D(GL_TEXTURE_2D, 1, _computeLuminanceColorFormat, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
	}

	debugThrowOnApiError("allocateComputePingPongImages");
}

/// <summary>Creates the various samplers used throughout the demo.</summary>
void OpenGLESPostProcessing::createSamplers()
{
	gl::GenSamplers(1, &_samplerTrilinear);
	gl::GenSamplers(1, &_irradianceSampler);
	gl::GenSamplers(1, &_samplerBilinear);
	gl::GenSamplers(1, &_samplerNearest);

	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerTrilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_irradianceSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_irradianceSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_irradianceSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_irradianceSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_irradianceSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_samplerNearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	debugThrowOnApiError("createSamplers");
}

/// <summary>Create the framebuffers which will be used in the various bloom passes.</summary>
void OpenGLESPostProcessing::createBlurFramebuffers()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		gl::GenFramebuffers(1, &_blurFramebuffers[i]);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _blurFramebuffers[i]);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pingPongTextures[i], 0);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _blurFramebufferDimensions.x);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _blurFramebufferDimensions.y);
		pvr::utils::checkFboStatus();
	}

	debugThrowOnApiError("createBlurFramebuffers init");

	for (uint32_t i = 0; i < 2; ++i)
	{
		gl::GenFramebuffers(1, &_computeBlurFramebuffers[i]);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _computeBlurFramebuffers[i]);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _computePingPongTextures[i], 0);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _blurFramebufferDimensions.x);
		gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _blurFramebufferDimensions.y);
		pvr::utils::checkFboStatus();
	}

	debugThrowOnApiError("createComputeBlurFramebuffers init");
}

/// <summary>Determine the maximum down scale factor supported by the GL_IMG_framebuffer_downsample extension.</summary>
/// <param name="xDownscale">The maximum x downscale factor.</param>
/// <param name="yDownscale">The maximum y downscale factor.</param>
void OpenGLESPostProcessing::getDownScaleFactor(GLint& xDownscale, GLint& yDownscale)
{
	Log("Supported Downsampling factors:");

	xDownscale = 1;
	yDownscale = 1;

	// Query the number of available scales
	GLint numScales;
	gl::GetIntegerv(GL_NUM_DOWNSAMPLE_SCALES_IMG, &numScales);

	// 2 scale modes are supported as minimum, so only need to check for
	// better than 2x2 if more modes are exposed.
	if (numScales > 2)
	{
		// Try to select most aggressive scaling.
		GLint bestScale = 1;
		GLint tempScale[2];
		GLint i;
		for (i = 0; i < numScales; ++i)
		{
			gl::GetIntegeri_v(GL_DOWNSAMPLE_SCALES_IMG, i, tempScale);

			Log("	Downsampling factor: %i, %i", tempScale[0], tempScale[1]);

			// If the scaling is more aggressive, update our x/y scale values.
			if (tempScale[0] * tempScale[1] > bestScale)
			{
				xDownscale = tempScale[0];
				yDownscale = tempScale[1];
			}
		}
	}
	else
	{
		xDownscale = 2;
		yDownscale = 2;
	}
}

/// <summary>Create the offscreen framebuffers and various attachments used in the application.</summary>
void OpenGLESPostProcessing::createOffScreenFramebuffers()
{
	// Offscreen texture
	gl::GenTextures(1, &_offScreenColorTexture);
	gl::BindTexture(GL_TEXTURE_2D, _offScreenColorTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, _offscreenColorFormat, getWidth(), getHeight());

	debugThrowOnApiError("createOffScreenFramebuffers - created offscreen color texture");

	// Source luminance texture
	gl::GenTextures(1, &_luminanceTexture);
	gl::BindTexture(GL_TEXTURE_2D, _luminanceTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, _luminanceColorFormat, getWidth(), getHeight());

	debugThrowOnApiError("createOffScreenFramebuffers - created luminance texture");

	// Downsampled luminance texture
	if (_isIMGFramebufferDownsampleSupported)
	{
		gl::GenTextures(1, &_downSampledLuminanceTexture);
		gl::BindTexture(GL_TEXTURE_2D, _downSampledLuminanceTexture);
		gl::TexStorage2D(GL_TEXTURE_2D, 1, _luminanceColorFormat, getWidth() / _IMGFramebufferScale, getHeight() / _IMGFramebufferScale);

		debugThrowOnApiError("createOffScreenFramebuffers - created downsample luminance texture");
	}

	gl::GenTextures(1, &_depthStencilTexture);
	gl::BindTexture(GL_TEXTURE_2D, _depthStencilTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, getWidth(), getHeight());

	debugThrowOnApiError("createOffScreenFramebuffers - created depth stencil texture");

	// Fbo used for the offscreen rendering
	{
		gl::GenFramebuffers(1, &_offScreenFramebuffer);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenFramebuffer);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _offScreenColorTexture, 0);

		if (_isIMGFramebufferDownsampleSupported)
		{
			gl::ext::FramebufferTexture2DDownsampleIMG(
				GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _downSampledLuminanceTexture, 0, _IMGFramebufferScale, _IMGFramebufferScale);
		}
		else
		{
			gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _luminanceTexture, 0);
		}

		debugThrowOnApiError("createOffScreenFramebuffers - created offscreen Framebuffer");

		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthStencilTexture, 0);
		pvr::utils::checkFboStatus();
	}

	// Fbo used when using Bloom passes which take care of their own downsampling
	{
		gl::GenFramebuffers(1, &_offScreenNoDownsampleFramebuffer);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _offScreenNoDownsampleFramebuffer);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _offScreenColorTexture, 0);
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _luminanceTexture, 0);

		debugThrowOnApiError("createOffScreenNoDownsampleFramebuffers - created offscreen Framebuffer");

		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthStencilTexture, 0);
		pvr::utils::checkFboStatus();
	}

	debugThrowOnApiError("createOffScreenFramebuffers");
}

/// <summary>Update the various dynamic scene data used in the application.</summary>
void OpenGLESPostProcessing::updateDynamicSceneData()
{
	// Update object animations
	updateAnimation();

	// Update the animation data used in the statue pass
	_statuePass.updateAnimation(_objectAngleY, _viewProjectionMatrix);

	{
		void* mappedMemory = nullptr;
		if (!_isBufferStorageExtSupported)
		{
			gl::BindBuffer(GL_UNIFORM_BUFFER, _sceneBuffer);
			mappedMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (size_t)_sceneBufferView.getSize(), GL_MAP_WRITE_BIT);
			_sceneBufferView.pointToMappedMemory(mappedMemory);
		}

		_sceneBufferView.getElementByName(BufferEntryNames::Scene::InverseViewProjectionMatrix).setValue(glm::inverse(_viewProjectionMatrix));
		_sceneBufferView.getElementByName(BufferEntryNames::Scene::EyePosition).setValue(_camera.getCameraPosition());
		_sceneBufferView.getElementByName(BufferEntryNames::Scene::LightPosition).setValue(_lightPosition);

		if (!_isBufferStorageExtSupported)
		{
			gl::UnmapBuffer(GL_UNIFORM_BUFFER);
		}
	}

	if (_mustUpdateDemoConfig)
	{
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_gaussianBlurPass.updateKernelBuffer();
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			_linearGaussianBlurPass.updateKernelBuffer();
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_truncatedLinearGaussianBlurPass.updateKernelBuffer();
			break;
		}
		case (BloomMode::Compute):
		{
			_computeBlurPass.updateKernelBuffer();
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			_hybridGaussianBlurPass.linearBlurPass->updateKernelBuffer();
			_hybridGaussianBlurPass.computeBlurPass->updateKernelBuffer();
			break;
		}
		default:
			break;
		}

		_mustUpdateDemoConfig = false;
	}
}

/// <summary>Update the animations for the current frame.</summary>
void OpenGLESPostProcessing::updateAnimation()
{
	if (_animateCamera)
	{
		_cameraAngle += 0.15f;

		if (_cameraAngle >= 360.0f)
		{
			_cameraAngle = _cameraAngle - 360.f;
		}
	}

	_camera.setTargetLookAngle(_cameraAngle);

	_viewMatrix = _camera.getViewMatrix();
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;

	if (_animateObject)
	{
		_objectAngleY += RotateY * 0.03f * getFrameTime();
	}

	float dt = getFrameTime() * 0.001f;
	_logicTime += dt;
	if (_logicTime > 10000000)
	{
		_logicTime = 0;
	}

	if (!_isManual)
	{
		if (_logicTime > _modeSwitchTime + _modeDuration)
		{
			_modeSwitchTime = _logicTime;

			if (_blurMode != BloomMode::NoBloom)
			{
				// Increase the demo configuration
				_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
			}
			// Change to the next bloom mode
			if (_currentDemoConfiguration == 0 || _blurMode == BloomMode::NoBloom)
			{
				uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
				currentBlurMode += 1;
				currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
				_blurMode = static_cast<BloomMode>(currentBlurMode);
			}

			updateBloomConfiguration();
		}
	}
}

/// <summary>Update the demo configuration in use. Calculates gaussian weights and offsets, images being used, framebuffers being used etc.</summary>
void OpenGLESPostProcessing::updateDemoConfigs()
{
	switch (_blurMode)
	{
	case (BloomMode::GaussianOriginal):
	{
		_gaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig.first, false, false);
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_linearGaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig.first, true, false);
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_truncatedLinearGaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.first, true, true);
		break;
	}
	case (BloomMode::Kawase):
	{
		_kawaseBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.first.kernel,
			DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.first.numIterations);
		break;
	}
	case (BloomMode::Compute):
	{
		_computeBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.first, false, false);
		break;
	}
	case (BloomMode::DualFilter):
	{
		_dualFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig.first);
		break;
	}
	case (BloomMode::TentFilter):
	{
		_downAndTentFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].tentFilterConfig.first);
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_truncatedLinearGaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.first, true, true);
		_computeBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.first, false, false);
		break;
	}
	default:
		break;
	}
	debugThrowOnApiError("updateDemoConfigs");
}

/// <summary>Update the bloom configuration.</summary>
void OpenGLESPostProcessing::updateBloomConfiguration()
{
	updateDemoConfigs();

	updateBlurDescription();
	_uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_uiRenderer.getDefaultDescription()->commitUpdates();

	_mustUpdateDemoConfig = true;
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
void OpenGLESPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	switch (e)
	{
	case pvr::SimplifiedInput::Up:
	{
		_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Down:
	{
		if (_currentDemoConfiguration == 0)
		{
			_currentDemoConfiguration = DemoConfigurations::NumDemoConfigurations;
		}
		_currentDemoConfiguration = (_currentDemoConfiguration - 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Left:
	{
		uint32_t currentBloomMode = static_cast<uint32_t>(_blurMode);
		currentBloomMode -= 1;
		currentBloomMode = (currentBloomMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBloomMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Right:
	{
		uint32_t currentBloomMode = static_cast<uint32_t>(_blurMode);
		currentBloomMode += 1;
		currentBloomMode = (currentBloomMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBloomMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::ActionClose:
	{
		this->exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1:
	{
		_renderOnlyBloom = !_renderOnlyBloom;
		break;
	}
	case pvr::SimplifiedInput::Action2:
	{
		_animateObject = !_animateObject;
		break;
	}
	default:
	{
		break;
	}
	}
}

/// <summary>Render the UI.</summary>
void OpenGLESPostProcessing::renderUI()
{
	_uiRenderer.beginRendering();
	_uiRenderer.getSdkLogo()->render();
	_uiRenderer.getDefaultTitle()->render();
	_uiRenderer.getDefaultControls()->render();
	_uiRenderer.getDefaultDescription()->render();
	_uiRenderer.endRendering();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new OpenGLESPostProcessing());
}
