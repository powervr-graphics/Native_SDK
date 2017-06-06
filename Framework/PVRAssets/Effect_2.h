/*!
\brief A pvr::assets::Effect is the description of the entire rendering setup and can be used to create pvr::api
objects and use them for rendering.
\file PVRAssets/Effect.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/Model.h"
#include "PVRCore/Texture.h"
#include "PVRCore/Base/Types.h"
#include <set>

namespace pvr {
namespace types {
enum class VariableScope
{
	Unknown,
	Automatic,
	Model,
	Effect,
	Node,
	BoneBatch,
};
}


namespace assets {
namespace effect {

template<typename MyType_> class NameComparable
{
public:
	StringHash name;
	NameComparable() {}
	NameComparable(StringHash&& name): name(std::move(name)) {}
	NameComparable(const StringHash& name): name(name) {}

	typedef MyType_ MyType;
	bool operator<(const MyType& rhs) const { return name < rhs.name; }
	bool operator>(const MyType& rhs) const { return name > rhs.name; }

	bool operator>=(const MyType& rhs) const { return name >= rhs.name; }
	bool operator<=(const MyType& rhs) const { return name <= rhs.name; }

	bool operator==(const MyType& rhs) const { return name == rhs.name; }
	bool operator!=(const MyType& rhs) const { return name != rhs.name; }
};

/// <summary>Stores effect texture information.</summary>
struct TextureDefinition : public NameComparable<TextureDefinition>
{
	StringHash path;    //!< File name
	uint32 width, height; //!< texture dimension
	ImageDataFormat fmt;
	TextureDefinition() {}
	TextureDefinition(const StringHash& name, const StringHash& path, uint32 width, uint32 height, const ImageDataFormat& fmt) :
		NameComparable(name), path(path), width(width), height(height), fmt(fmt) {}
	bool isFile() { return path.empty(); }
	//uint64 flags;
};

/// <summary>Stores effect texture information.</summary>
struct TextureRef
{
	StringHash textureName;
	uint8 set;      //!< Texture number to set
	uint8 binding;      //!< Texture number to set
	StringHash variableName; //<! The variable name that this texture refers to in the shader.
	TextureRef() {}

	TextureRef(StringHash textureName, uint8 set, uint8 binding, StringHash variableName):
		textureName(textureName), set(set), binding(binding), variableName(variableName) {}
};

/// <summary>Stores effect texture information.</summary>
struct TextureReference : public TextureRef
{
	types::PackedSamplerFilter samplerFilter; //!< Sampler Filters
	types::SamplerWrap wrapS, wrapT, wrapR; //!< Either Clamp or Repeat
	StringHash semantic;  //!< The semantic from which this texture will get its value.
};

/// <summary>Store effect data from the shader block.</summary>
struct Shader : public NameComparable<Shader>
{
	std::string source; //<!Sources pairs
	types::ShaderType type;
	Shader() {}
	Shader(StringHash&& name, types::ShaderType type, std::string&& source) :
	    NameComparable<Shader>(std::move(name)), source(std::move(source)), type(type) { }
};
typedef const Shader* ShaderReference;

struct BufferDefinition : public NameComparable<BufferDefinition>
{
	struct Entry
	{
		StringHash semantic;
		types::GpuDatatypes::Enum dataType;
		uint32 arrayElements;
	};
	types::BufferBindingUse allSupportedBindings;
	bool isDynamic;
	std::vector<Entry> entries;
	types::VariableScope scope;
	bool multibuffering;
	BufferDefinition() : allSupportedBindings(types::BufferBindingUse(0)), isDynamic(false),
	    scope(types::VariableScope::Effect), multibuffering(0) {}
};

struct DescriptorRef
{
	int8 set;
	int8 binding;
	DescriptorRef(): set(0), binding(0) {}
};

struct BufferRef : DescriptorRef
{
	StringHash semantic; //<!An optional semantic to export for this entire buffer
	StringHash bufferName;
	types::DescriptorType type;
};

struct UniformSemantic : DescriptorRef
{
	StringHash semantic; //<!The name of this shader
	StringHash variableName; //<!The name of this shader
	types::GpuDatatypes::Enum dataType;
	uint32 arrayElements;
	types::VariableScope scope;
};

struct AttributeSemantic
{
	StringHash semantic; //<!The name of this shader
	StringHash variableName; //<!The name of this shader
	types::GpuDatatypes::Enum dataType;
	uint8 location;
	uint8 vboBinding;
};

struct InputAttachmentRef : DescriptorRef
{
	int8 targetIndex;
	InputAttachmentRef() : targetIndex(-1) {}
};

struct PipelineVertexBinding
{
	uint32 index;
	types::StepRate stepRate;

	PipelineVertexBinding() : stepRate(types::StepRate::Vertex) {}
	PipelineVertexBinding(uint32 index, types::StepRate stepRate) : index(index), stepRate(stepRate) {}
};

struct PipelineDefinition: public NameComparable<PipelineDefinition>
{
	std::vector<ShaderReference> shaders;
	std::vector<UniformSemantic> uniforms;//!< Effect uniforms
	std::vector<AttributeSemantic> attributes;//!< Effect attributes
	std::vector<TextureReference> textures;//!< Effect textures
	std::vector<BufferRef> buffers; //!< Effect targets};
	types::BlendingConfig blending;
	std::vector<InputAttachmentRef> inputAttachments;
	std::vector<PipelineVertexBinding> vertexBinding;
	// depth states
	bool enableDepthTest;
	bool enableDepthWrite;
	types::ComparisonMode depthCmpFunc;

	// stencil states
	bool enableStencilTest;
	types::StencilState stencilFront;
	types::StencilState stencilBack;

	// rasterization states
	types::PolygonWindingOrder windingOrder;
	types::Face cullFace;

	PipelineDefinition() : enableDepthTest(false), enableDepthWrite(true),
		depthCmpFunc(types::ComparisonMode::Less), enableStencilTest(false),
		windingOrder(types::PolygonWindingOrder::FrontFaceCCW), cullFace(types::Face::None) {}
};

struct PipelineCondition
{
	enum ConditionType
	{
		Always,
		UniformRequired,
		AttributeRequired,
		UniformRequiredNo,
		AttributeRequiredNo,
		AdditionalExport,
	} type;
	StringHash value;
};

struct PipelineReference
{
	StringHash pipelineName;
	DynamicArray<PipelineCondition> conditions;
	DynamicArray<StringHash> identifiers;
};

struct SubpassGroup
{
	StringHash name;
	DynamicArray<PipelineReference> pipelines;
};


struct Subpass
{
	enum { MaxTargets = 4, MaxInputs = 4 };
	StringHash targets[MaxTargets];
	StringHash inputs[MaxInputs];
	bool useDepthStencil;
	std::vector<SubpassGroup> groups;
};

struct Pass
{
	StringHash name;
	StringHash targetDepthStencil;
	std::vector<Subpass> subpasses;
};

struct Effect : public Asset<Effect>
{
	StringHash name;
	pvr::ContiguousMap<StringHash, string> headerAttributes;

	pvr::ContiguousMap<StringHash, pvr::ContiguousMap<StringHash, Shader>> versionedShaders; //!< Effect shaders
	pvr::ContiguousMap<StringHash, pvr::ContiguousMap<StringHash, PipelineDefinition>> versionedPipelines; //!< Effects, with their version

	pvr::ContiguousMap<StringHash, TextureDefinition> textures;//!< Effect textures
	pvr::ContiguousMap<StringHash, BufferDefinition> buffers; //!< Effect buffers

	std::vector<Pass> passes;
	mutable std::vector<StringHash> versions;

	const std::vector<StringHash>& getVersions() const
	{
		if (versions.empty())
		{
			for (auto it = versionedPipelines.begin(); it != versionedPipelines.end(); ++it)
			{
				versions.push_back(it->first);
			}
		}
		return versions;
	}

	void addVersion(const StringHash& apiName)
	{
		versionedShaders[apiName];
		versionedPipelines[apiName];
	}

	void addShader(const StringHash& apiName, Shader&& shader)
	{
		versionedShaders[apiName][shader.name] = std::move(shader);
	}
	void addShader(const StringHash& apiName, const Shader& shader)
	{
		versionedShaders[apiName][shader.name] = shader;
	}

	void addTexture(TextureDefinition&& texture)
	{
		textures[texture.name] = std::move(texture);
	}
	void addTexture(const TextureDefinition& texture)
	{
		textures[texture.name] = texture;
	}

	void addBuffer(BufferDefinition&& buffer)
	{
		buffers[buffer.name] = std::move(buffer);
	}

	void addBuffer(const BufferDefinition& buffer)
	{
		buffers[buffer.name] = buffer;
	}

	void addPipeline(const StringHash& apiName, PipelineDefinition&& pipeline)
	{
		versionedPipelines[apiName][pipeline.name] = std::move(pipeline);
		versions.clear();
	}

	void addPipeline(const StringHash& apiName, const PipelineDefinition& pipeline)
	{
		versionedPipelines[apiName][pipeline.name] = pipeline;
		versions.clear();
	}


	void clear()
	{
		name.clear();
		headerAttributes.clear();
		passes.clear();
		textures.clear();
		buffers.clear();
		versionedPipelines.clear();
		versionedShaders.clear();
	}
};

}

}// namespace assets
}// namespace pvr
