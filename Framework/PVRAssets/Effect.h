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
#include <string>
namespace pvr {
namespace types {
/// <summary>Enum values for the various variable types supported by Semantics.</summary>
enum class SemanticDataType
{
	Mat2,//!< 2x2 matrix
	Mat3,//!< 3x3 matrix
	Mat4,//!< 4x4 matrix
	Vec2,//!< 2d vector
	Vec3,//!< 3d vector
	Vec4,//!< 4d vector
	IVec2,//!< 2d integer vector
	IVec3,//!< 3d integer vector
	IVec4,//!< 4d integer vector
	BVec2,//!< 2d bool vector
	BVec3,//!< 3d bool vector
	BVec4,//!< 4d bool vector
	Float,//!< float
	Int1,//!< integer
	Bool1,//!< bool

	Count,//!< number of supported semantic type
	None,

	// Conceptual data types
	RGB,//!< Semantic RGB
	RGBA//!< Semantic RGBA
};

/// <summary>Enumeration of the type of render required for an effect.</summary>
enum class EffectPassType
{
	Null,   //!< Null pass
	Camera,   //!< Camera
	PostProcess,  //!< Post-process
	EnvMapCube, //!< Environment cube-map
	EnvMapSph,  //!< Environment sphere map
	Count   //!< Number of supported pass
};


/// <summary>Enumeration Describes the type of different Effect Passes.</summary>
enum class EffectPassView
{
	Current,      //!< The scene's active camera is used
	PodCamera,    //!< The specified camera is used
	None        //!< No specified view
};

/// <summary>Enum values for defining whether a variable is float, integer or bool.</summary>
enum class EffectDefaultDataInternalType
{
	Float,//!< Float
	Integer,//!< Integer
	Boolean//!< Boolean
};
}

namespace assets {


/// <summary>Stores effect texture information.</summary>
struct EffectTexture
{
	StringHash name;      //!< Name of texture.
	StringHash fileName;    //!< File name
	uint8 number;     //!< Texture number to set
	uint8 unit;     //!< Texture binding unit
	types::SamplerFilter minFilter, magFilter, mipFilter; //!< Sampler Filters
	types::SamplerWrap wrapS, wrapT, wrapR; //!< Either Clamp or Repeat
	uint32 width, height; //!< texture dimension
	uint64 flags;
	bool renderToTexture; //!< render to the texture
};

/// <summary>Stores type information for a default data type.</summary>
struct EffectSemanticDefaultDataTypeInfo
{
	types::SemanticDataType type; //!< Semantic data type
	const char8* name;
	uint32 numDataItems;    //!< Number of data types
	types::EffectDefaultDataInternalType internalType;    //!< Data internal type

	const static EffectSemanticDefaultDataTypeInfo& getSemanticDefaultTypeInfo(types::SemanticDataType semanticDfltType);
};

/// <summary>Stores a Semantic value. Union of different possible values. Support up to 64 bytes.</summary>
struct EffectSemanticData
{
	union
	{
		float32     dataF32[16];    //!< Float
		int32     dataI32[16];    //!< Interger
		bool      dataBool[64];   //!< bool
		char      dataChar[64];   //!< char
	};
	types::SemanticDataType type;

	EffectSemanticData() : type(types::SemanticDataType::None) {}
};

/// <summary>Stores information about a semantic.</summary>
struct EffectSemantic
{
	std::string variableName; //!< The variable name as used in the shader-language code
	StringHash semantic; //!< For example: LIGHTPOSITION
	EffectSemanticData sDefaultValue;     //!< Default value
};

/// <summary>Store effect data from the shader block.</summary>
struct EffectShader
{
	StringHash name;
	bool useFileName;//!< Use shader file name
	std::string glslFile;//!< Glsl source file
	std::string  glslBinFile;//!< Glsl binary file
	std::string glslCode;//!< Glsl code
	std::string glslBin;//!< Glsl binary
	uint32 glslBinSize;//!< Glsl binary size
	uint32 firstLineNumPos; //!< Line number in the text file where this code began; use to correct line-numbers in compiler errors
	uint32 lastLineNumPos;  //!< The final line number of the GLSL block.
};

/// <summary>Stores a buffer type and name for a render target.</summary>
typedef std::pair<std::string, std::string> EffectTargetPair;

template<typename T_>
struct SemanticComparator
{
	StringHash semantic;
	SemanticComparator(const StringHash& semantic) : semantic(semantic) { }
	bool operator()(const T_& effectSemantic)const { return semantic == effectSemantic.semantic; }
};

template<>
struct SemanticComparator<EffectTexture>
{
	StringHash semantic;
	SemanticComparator(const StringHash& semantic) : semantic(semantic) { }
	bool operator()(const EffectTexture& effectTex)const { return semantic == effectTex.name; }
};

/// <summary>Represents the information of an entire Effect, all information required to set up rendering of a Mesh in a
/// graphics API, such as number and type of textures, attributes, shader variables etc.</summary>
/// <remarks>Models can have effects. Can be used to generate API objects for rendering.</remarks>
struct Effect : public Asset<Effect>
{
public:
	string annotation;
	EffectShader vertexShader;//!< Effect vertex shader
	EffectShader fragmentShader;//!< Effect fragment shader
	std::vector<EffectSemantic> uniforms;//!< Effect uniforms
	std::vector<EffectSemantic> attributes;//!< Effect attributes
	std::vector<EffectTexture> textures;//!< Effect textures
	std::vector<EffectTargetPair> targets; //!< Effect targets
	uint32 numDefaultSemantics;//!< Number of default semantics stored in the effect
	assets::Model::Material material;//!< Effect material
	glm::vec4 viewport;             //!< Effect viewport
	std::string fileName;//!< effect file name
	Effect() {}
	Effect(const std::string& effectName) { material.setEffectName(effectName); }
	const assets::Model::Material& getMaterial()const { return material; }

	/// <summary>Return an id of uniform semantic. return -1 if the semantic not found</summary>
	/// <param name="semantic">a semantic</param>
	/// <returns>semantic id</returns>
	int32 getUniformSemanticId(const StringHash& semantic)const
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), SemanticComparator<EffectSemantic>(semantic));
		if (it == uniforms.end()) { return -1; }
		return (int32)(it - uniforms.begin());
	}

	/// <summary>Return an id to asset semantic. return -1 if the semantic not found</summary>
	/// <param name="semantic">a semantic</param>
	/// <returns>semantic id</returns>
	int32 getAssetSemanticId(const StringHash& semantic)const
	{
		auto it = std::find_if(attributes.begin(), attributes.end(), SemanticComparator<EffectSemantic>(semantic));
		if (it == attributes.end()) { return -1; }
		return (int32)(it - attributes.begin());
	}

	/// <summary>Return an id to texture semantic. return -1 if the semantic not found</summary>
	/// <param name="semantic">a semantic</param>
	/// <returns>semantic id</returns>
	int32 getTextureSemanticId(const StringHash& semantic)const
	{
		auto it = std::find_if(textures.begin(), textures.end(), SemanticComparator<EffectTexture>(semantic));
		if (it == textures.end()) { return -1; }
		return (int32)(it - textures.begin());
	}

private:

};
}// namespace assets
}// namespace pvr