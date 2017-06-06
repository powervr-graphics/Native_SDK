/*!
\brief Implementation of methods of the PFXReader class.
\file PVRAssets/FileIO/PFXReader.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/PFXParser.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/IO/FileStream.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVRCore/Texture.h"
#include <set>

namespace pvr {
namespace assets {
namespace pfx {


namespace {
using namespace ::pvr::types;

ImageDataFormat getFormat(const pugi::xml_attribute& attr)
{
	struct ImageFormat
	{
		StringHash name;
		ImageDataFormat fmt;
	};
	static const ImageFormat bufferFormats[] =
	{
	//	ImageFormat{ StringHash("r5g6b5_unorm"),			ImageDataFormat(PixelFormat::RGB_565,     false) },
	//	ImageFormat{ StringHash("a1r5g6b5_unorm"),     ImageDataFormat(PixelFormat,  VariableType::   false) },

		ImageFormat{ StringHash("r8_unorm"),				ImageDataFormat(PixelFormat::R_8, VariableType::UnsignedByteNorm,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8_uint"),					ImageDataFormat(PixelFormat::R_8, VariableType::UnsignedByte,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8_sint"),					ImageDataFormat(PixelFormat::R_8,  VariableType::SignedByte,      types::ColorSpace::lRGB) },

		ImageFormat{ StringHash("r8g8_unorm"),				ImageDataFormat(PixelFormat::RG_88, VariableType::UnsignedByteNorm,       types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8_uint"),				ImageDataFormat(PixelFormat::RG_88,  VariableType::UnsignedByte,      types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8_sint"),				ImageDataFormat(PixelFormat::RG_88, VariableType::SignedByte,       types::ColorSpace::lRGB) },

		ImageFormat{ StringHash("r8g8b8a8_unorm"),			ImageDataFormat(PixelFormat::RGBA_8888, VariableType::UnsignedByteNorm,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_uint"),			ImageDataFormat(PixelFormat::RGBA_8888, VariableType::UnsignedByte,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_sint"),			ImageDataFormat(PixelFormat::RGBA_8888, VariableType::SignedByte,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_unorm_srgb"),		ImageDataFormat(PixelFormat::RGBA_8888, VariableType::UnsignedByteNorm,types::ColorSpace::sRGB) },

		ImageFormat{ StringHash("b8g8r8a8_unorm"),			ImageDataFormat(PixelFormat::BGRA_8888, VariableType::UnsignedByteNorm,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("b8g8r8a8_unorm_srgb"),		ImageDataFormat(PixelFormat::BGRA_8888, VariableType::UnsignedByteNorm,types::ColorSpace::sRGB) },

		ImageFormat{ StringHash("a8b8g8r8_unorm"),			ImageDataFormat(PixelFormat::ABGR_8888, VariableType::UnsignedByteNorm,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_uint"),			ImageDataFormat(PixelFormat::ABGR_8888, VariableType::UnsignedByte,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_sint"),			ImageDataFormat(PixelFormat::ABGR_8888, VariableType::SignedByteNorm,types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_unorm_srgb"),		ImageDataFormat(PixelFormat::ABGR_8888, VariableType::UnsignedByteNorm,types::ColorSpace::sRGB) },

		//ImageFormat{ StringHash("a2b10g10r10_unorm"),		ImageDataFormat(PixelFormat::RGBA_8888, VariableType::,types::ColorSpace::lRGB) },
		//ImageFormat{ StringHash("a2b10g10r10_uint"),		ImageDataFormat(PixelFormat::R_8,        false) },

		ImageFormat{ StringHash("r16_uint"),				ImageDataFormat(PixelFormat::R_16, VariableType::UnsignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16_sint"),				ImageDataFormat(PixelFormat::R_16, VariableType::SignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16_sfloat"),				ImageDataFormat(PixelFormat::R_16, VariableType::SignedFloat, types::ColorSpace::lRGB) },

		ImageFormat{ StringHash("r16g16_uint"),				ImageDataFormat(PixelFormat::RG_1616, VariableType::UnsignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16_sint"),				ImageDataFormat(PixelFormat::RG_1616, VariableType::SignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16_sfloat"),			ImageDataFormat(PixelFormat::RG_1616, VariableType::SignedFloat, types::ColorSpace::lRGB) },

		ImageFormat{ StringHash("r16g16b16a16_uint"),		ImageDataFormat(PixelFormat::RGBA_16161616, VariableType::UnsignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16b16a16_sint"),		ImageDataFormat(PixelFormat::RGBA_16161616, VariableType::SignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16b16a16_sfloat"),		ImageDataFormat(PixelFormat::RGBA_16161616, VariableType::SignedFloat, types::ColorSpace::lRGB) },


		ImageFormat{ StringHash("r32_uint"),				ImageDataFormat(PixelFormat::R_32, VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32_sint"),				ImageDataFormat(PixelFormat::R_32, VariableType::SignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32_sfloat"),				ImageDataFormat(PixelFormat::R_32, VariableType::SignedFloat, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_uint"),				ImageDataFormat(PixelFormat::RG_3232, VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_sint"),				ImageDataFormat(PixelFormat::RG_3232, VariableType::SignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_sfloat"),			ImageDataFormat(PixelFormat::RG_3232, VariableType::SignedFloat, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_uint"),		ImageDataFormat(PixelFormat::RGBA_32323232, VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_sint"),		ImageDataFormat(PixelFormat::RGBA_32323232, VariableType::SignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_sfloat"),		ImageDataFormat(PixelFormat::RGBA_32323232, VariableType::SignedFloat, types::ColorSpace::lRGB) },

		ImageFormat{ StringHash("d16"),						ImageDataFormat(PixelFormat::Depth16,     VariableType::UnsignedShort, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("d24"),						ImageDataFormat(PixelFormat::Depth24,     VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("d24s32"),					ImageDataFormat( PixelFormat::Depth24Stencil8, VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
		ImageFormat{ StringHash("d32"),						ImageDataFormat(PixelFormat::Depth32,     VariableType::UnsignedInteger, types::ColorSpace::lRGB) },
	};

	ImageDataFormat retval;
	if (attr && attr.value())
	{
		const StringHash fmtStr(strings::toLower(attr.value()));

		for (const ImageFormat buffFmt : bufferFormats)
		{
			if (buffFmt.name == fmtStr)
			{
				retval = buffFmt.fmt;
				break;
			}
		}
		if (retval.format.getPixelTypeId() == 0)
		{
			Log(Log.Warning, "PfxParser: 'format' attribute of <texture> "
			"element was provided, but the format %s not recognized. Defaulting to RGBA8888.",attr.value());
			retval = ImageDataFormat();
		}
	}
	return retval;
}

std::map<std::string, types::GpuDatatypes::Enum> datatype_strings;

inline bool static_call_only_once_initialize_map()
{
	datatype_strings.clear();
	datatype_strings["mat2"] = types::GpuDatatypes::mat2x2;
	datatype_strings["mat2x2"] = types::GpuDatatypes::mat2x2;
	datatype_strings["mat2x3"] = types::GpuDatatypes::mat2x3;
	datatype_strings["mat2x4"] = types::GpuDatatypes::mat2x4;
	datatype_strings["mat3"] = types::GpuDatatypes::mat3x3;
	datatype_strings["mat3x2"] = types::GpuDatatypes::mat3x2;
	datatype_strings["mat3x3"] = types::GpuDatatypes::mat3x3;
	datatype_strings["mat3x4"] = types::GpuDatatypes::mat3x4;
	datatype_strings["mat4"] = types::GpuDatatypes::mat4x4;
	datatype_strings["mat4x2"] = types::GpuDatatypes::mat4x2;
	datatype_strings["mat4x3"] = types::GpuDatatypes::mat4x3;
	datatype_strings["mat4x4"] = types::GpuDatatypes::mat4x4;
	datatype_strings["vec2"] = types::GpuDatatypes::vec2;
	datatype_strings["vec3"] = types::GpuDatatypes::vec3;
	datatype_strings["vec4"] = types::GpuDatatypes::vec4;
	datatype_strings["ivec2"] = types::GpuDatatypes::ivec2;
	datatype_strings["ivec3"] = types::GpuDatatypes::ivec3;
	datatype_strings["ivec4"] = types::GpuDatatypes::ivec4;
	datatype_strings["uvec2"] = types::GpuDatatypes::uvec2;
	datatype_strings["uvec3"] = types::GpuDatatypes::uvec3;
	datatype_strings["uvec4"] = types::GpuDatatypes::uvec4;
	datatype_strings["bvec2"] = types::GpuDatatypes::bvec2;
	datatype_strings["bvec3"] = types::GpuDatatypes::bvec3;
	datatype_strings["bvec4"] = types::GpuDatatypes::bvec4;
	datatype_strings["float"] = types::GpuDatatypes::float32;
	datatype_strings["float32"] = types::GpuDatatypes::float32;
	datatype_strings["int"] = types::GpuDatatypes::integer;
	datatype_strings["int8"] = types::GpuDatatypes::integer;
	datatype_strings["int16"] = types::GpuDatatypes::integer;
	datatype_strings["int32"] = types::GpuDatatypes::integer;
	datatype_strings["uint"] = types::GpuDatatypes::uinteger;
	datatype_strings["uint8"] = types::GpuDatatypes::uinteger;
	datatype_strings["uint16"] = types::GpuDatatypes::uinteger;
	datatype_strings["uint32"] = types::GpuDatatypes::uinteger;
	datatype_strings["bool"] = types::GpuDatatypes::boolean;

	return true;
}

inline types::GpuDatatypes::Enum dataTypeFromString(const std::string& mystr)
{
	static bool initialize_map = static_call_only_once_initialize_map(); (void)initialize_map; // bypass the warning.
	auto it = datatype_strings.find(strings::toLower(mystr));

	if (it == datatype_strings.end())
	{
		Log(Log.Warning, "Unrecognized datatype [%s] reading PFX file", mystr.c_str());
	}
	return it == datatype_strings.end() ? types::GpuDatatypes::none : it->second;
}

const string uniform_str("uniform");
const string storage_str("storage");
const string uniformdynamic_str("uniformdynamic");
const string storagedynamic_str("storagedynamic");
const string dynamicuniform_str("dynamicuniform");
const string dynamicstorage_str("dynamicstorage");

inline types::DescriptorType bufferDescriptorTypeFromString(const std::string& mystr)
{
	const std::string str = strings::toLower(mystr);
	if (str == uniform_str) { return DescriptorType::UniformBuffer; }
	else if (str == storage_str) { return DescriptorType::StorageBuffer; }
	else if (str == uniformdynamic_str) { return DescriptorType::UniformBufferDynamic; }
	else if (str == storagedynamic_str) { return DescriptorType::StorageBufferDynamic; }
	else if (str == dynamicuniform_str) { return DescriptorType::UniformBufferDynamic; }
	else if (str == dynamicstorage_str) { return DescriptorType::StorageBufferDynamic; }
	return DescriptorType::UniformBuffer;
}

const string nearest_str("nearest");
const string linear_str("linear");
const string none_str("none");
inline types::SamplerFilter filterFromAttribute(const pugi::xml_attribute& attr, types::SamplerFilter default_value)
{
	types::SamplerFilter ret = default_value;
	if (!attr.empty())
	{
		const string value = strings::toLower(attr.value());
		if (value == nearest_str) { ret = types::SamplerFilter::Nearest; }
		else if (value == linear_str) { ret = types::SamplerFilter::Linear; }
		else if (value == none_str) { ret = types::SamplerFilter::None; }
	}
	return ret;
}

const string clamp_str("clamp");
const string repeat_str("repeat");
inline types::SamplerWrap wrapFromAttribute(const pugi::xml_attribute& attr, types::SamplerWrap default_value)
{
	types::SamplerWrap ret = default_value;
	if (!attr.empty())
	{
		const string value = strings::toLower(attr.value());
		if (value == clamp_str) { ret = types::SamplerWrap::Clamp; }
		else if (value == repeat_str) { ret = types::SamplerWrap::Repeat; }
	}
	return ret;
}

const string requiresUniformSemantic_str("requiresuniformsemantic");
const string requiresUniformSemanticNotPresent_str("requiresuniformsemanticnotpresent");
const string requiresUniformSemanticPresent_str("requiresuniformsemanticpresent");
const string requiresAttributeSemantic_str("requiresattributesemantic");
const string requiresAttributeSemanticPresent_str("requiresattributesemanticpresent");
const string requiresAttributeSemanticNotPresent_str("requiresattributesemanticnotpresent");
inline effect::PipelineCondition::ConditionType conditionFromAttribute(const pugi::xml_attribute& attr)
{
	effect::PipelineCondition::ConditionType ret = effect::PipelineCondition::Always;
	if (!attr.empty())
	{
		const string value = strings::toLower(attr.value());
		if (value == requiresUniformSemantic_str) { ret = effect::PipelineCondition::UniformRequired; }
		else if (value == requiresUniformSemanticNotPresent_str) { ret = effect::PipelineCondition::UniformRequiredNo; }
		else if (value == requiresAttributeSemantic_str) { ret = effect::PipelineCondition::AttributeRequired; }
		else if (value == requiresAttributeSemanticNotPresent_str) { ret = effect::PipelineCondition::AttributeRequiredNo; }
		else if (value == requiresUniformSemanticPresent_str) { ret = effect::PipelineCondition::UniformRequired; }
		else if (value == requiresAttributeSemanticPresent_str) { ret = effect::PipelineCondition::AttributeRequired; }
	}
	return ret;
}

const string vertex_str("vertex");
const string fragment_str("fragment");
const string geometry_str("geometry");
const string tessControl_str("tesscontrol");
const string tessellationControl_str("tessellationcontrol");
const string tessEvaluation_str("tessevaluation");
const string tessellationEvaluation_str("tessellationevaluation");
inline types::ShaderType shaderTypeFromString(pugi::xml_attribute& attr)
{
	types::ShaderType ret = types::ShaderType::UnknownShader;
	if (!attr.empty())
	{
		const string value = strings::toLower(attr.value());
		if (value == vertex_str) { ret = types::ShaderType::VertexShader; }
		else if (value == fragment_str) { ret = types::ShaderType::FragmentShader; }
		else if (value == geometry_str) { ret = types::ShaderType::GeometryShader; }
		else if (value == tessControl_str) { ret = types::ShaderType::TessControlShader; }
		else if (value == tessellationControl_str) { ret = types::ShaderType::TessControlShader; }
		else if (value == tessEvaluation_str) { ret = types::ShaderType::TessEvaluationShader; }
		else if (value == tessellationEvaluation_str) { ret = types::ShaderType::TessEvaluationShader; }
	}
	return ret;
}

const string model_str("model");
const string node_str("node");
const string effect_str("effect");
const string bonebatch_str("bonebatch");
const string automatic_str("automatic");
const string auto_str("auto");
inline types::VariableScope scopeFromString(const pugi::xml_attribute& attr)
{
	types::VariableScope ret = types::VariableScope::Effect;
	if (!attr.empty())
	{
		const string value = strings::toLower(attr.value());
		if (value == automatic_str || value == auto_str) { ret = types::VariableScope::Automatic; }
		else if (value == effect_str) { ret = types::VariableScope::Effect; }
		else if (value == model_str) { ret = types::VariableScope::Model; }
		else if (value == node_str) { ret = types::VariableScope::Node; }
		else if (value == bonebatch_str) { ret = types::VariableScope::BoneBatch; }
		else
		{
			Log("PFXParser: Type '%s' for buffer or uniform scope was not recognized. Valid values: 'model', 'node', 'effect'", attr.value());
		}
	}
	return ret;
}

const string blend_factor_str[] =
{
	"zero",
	"one" ,
	"srccolor" ,
	"oneminussrccolor" ,
	"dstcolor" ,
	"oneminusdstcolor" ,
	"srcalpha" ,
	"oneminussrcalpha" ,
	"dstalpha" ,
	"oneminusdstalpha" ,
	"constantcolor",
	"oneminusconstantcolor" ,
	"constantalpha" ,
	"oneminusconstantalpha" ,
	"src1color" ,
	"oneminussrc1color" ,
	"src1alpha" ,
	"oneminussrc1alpha"
};

static_assert(ARRAY_SIZE(blend_factor_str) == (uint32)types::BlendFactor::NumBlendFactor,
              "Number blendfactor strings must be same as the types::BlendFactor::NumBlendFactor");

inline types::BlendFactor blendFactorFromString(const char* val, types::BlendFactor defaultBlend)
{
	types::BlendFactor ret = defaultBlend;
	const string value = strings::toLower(val);
	for (uint32 i = 0; i < (uint32)types::BlendFactor::NumBlendFactor; ++i)
	{
		if (value == blend_factor_str[i]) { ret = types::BlendFactor(i); break ; }
	}
	return ret;
}

// BlendOps
const string blend_op_str[]  =
{
	"add", "subtract", "reversesubtract", "min", "max"
};

static_assert(ARRAY_SIZE(blend_op_str) == (uint32)types::BlendOp::NumBlendFunc,
              "Number blendop strings must be same as the types::BlendOp::NumBlendFunc");

inline types::BlendOp blendOpFromString(const pugi::xml_attribute& attr)
{
	types::BlendOp ret = types::BlendOp::Default;
	const string value = strings::toLower(attr.value());
	if (!value.empty())
	{
		if (value == blend_op_str[0])       { ret = types::BlendOp::Add; }
		else if (value == blend_op_str[1])  { ret = types::BlendOp::Subtract; }
		else if (value == blend_op_str[2])  { ret = types::BlendOp::ReverseSubtract; }
		else if (value == blend_op_str[3])  { ret = types::BlendOp::Min; }
		else if (value == blend_op_str[4])  { ret = types::BlendOp::Max; }
		else
		{
			Log("PFXParser: Type '%s' for BlendOp as not recognized. using the default %s", attr.value(), blend_op_str[(uint32)ret].c_str());
		}
	}
	return ret;
}

types::ColorChannel blendChannelWriteMaskFromString(const pugi::xml_attribute& attr)
{
	if (strlen(attr.value()) == 0) { return types::ColorChannel::All; }
	const std::string value(strings::toLower(attr.value()));

	if (value == "none") { return types::ColorChannel::None; }

	types::ColorChannel bits = types::ColorChannel(0);
	if (value.find_first_of('r') != string::npos) { bits |= types::ColorChannel::R; }
	if (value.find_first_of('g') != string::npos) { bits |= types::ColorChannel::G; }
	if (value.find_first_of('b') != string::npos) { bits |= types::ColorChannel::B; }
	if (value.find_first_of('a') != string::npos) { bits |= types::ColorChannel::A; }
	return bits;
}

const char* comparison_mode_str[] =
{
	"never", "less", "equal", "lequal", "greater", "notequal", "gequal", "always", "none"
};
static_assert(ARRAY_SIZE(comparison_mode_str) == (uint32)ComparisonMode::NumComparisonMode,
              "Number comparison_mode_str strings must be same as the ComparisonMode::NumComparisonMode");

inline types::ComparisonMode comparisonModeFromString(const char* value, types::ComparisonMode dflt)
{
	const string val(strings::toLower(value));
	types::ComparisonMode rtn = dflt;
	for (uint32 i = 0; i < (uint32)types::ComparisonMode::NumComparisonMode; ++i)
	{
		if (strcmp(val.c_str(), comparison_mode_str[i]) == 0) { rtn = types::ComparisonMode(i); break ; }
	}
	return rtn;
}

void addTextures(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	for (auto it = begin; it != end; ++it)
	{
		effect::TextureDefinition tex;
		tex.name = it->attribute("name") ? it->attribute("name").value() : StringHash();
		tex.path = it->attribute("path") ? it->attribute("path").value() : StringHash();
		tex.height = it->attribute("height") ? it->attribute("height").as_int() : 0;
		tex.width = it->attribute("width") ? it->attribute("width").as_int() : 0;
		tex.fmt =  getFormat(it->attribute("format"));
		effect.addTexture(std::move(tex));
	}
}

void addEntryToBuffer(effect::BufferDefinition& buffer, pugi::xml_node& entry_node)
{
	effect::BufferDefinition::Entry entry;
	entry.arrayElements = 1;
	if (entry_node.attribute("arrayElements"))
	{
		entry.arrayElements = entry_node.attribute("arrayElements").as_int();
	}
	entry.semantic = entry_node.attribute("semantic").value();
	entry.dataType = dataTypeFromString(entry_node.attribute("dataType").value());
	buffer.entries.push_back(std::move(entry));
}

void addBuffers(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	for (auto it = begin; it != end; ++it)
	{
		effect::BufferDefinition buff;
		buff.name = it->attribute("name") ? it->attribute("name").value() : StringHash();

		buff.scope = it->attribute("scope") ? scopeFromString(it->attribute("scope")) : types::VariableScope::Effect;

		buff.multibuffering = it->attribute("multibuffering").as_bool();

		for (auto child = it->children().begin(); child != it->children().end(); ++child)
		{
			addEntryToBuffer(buff, *child);
		}
		effect.addBuffer(std::move(buff));
	}
}

Stream::ptr_type getStream(const std::string& filename, IAssetProvider* assetProvider)
{
	if (assetProvider)
	{
		return assetProvider->getAssetStream(filename);
	}
	else
	{
		return Stream::ptr_type(new FileStream(filename, "r"));
	}
}

bool addFileCodeSourceToVector(std::vector<char>& shaderSource, const char* filename, IAssetProvider* assetProvider)
{
	Stream::ptr_type str = getStream(filename, assetProvider);
	if (!str.get() || !str->open() || !str->isReadable())
	{
		Log("PfxParser: Could not open shader file stream [%s] - File not found", filename);
		return false;
	}
	return str->readIntoBuffer(shaderSource);
}

void addShaderCodeToVectors(const StringHash& /*name*/, types::ShaderType shaderType,
                            std::map<StringHash, std::pair<types::ShaderType, std::vector<char>/**/>/**/>& versionedShaders,
                            const pugi::xml_node& node, const StringHash& apiVersion, bool isFile,  bool addToAll, IAssetProvider* assetProvider)
{
	//The next two lines will select either running the for-loop just once for the value matching versionedShaders,
	//or for all values in versionedShaders (i.e. was "apiVersion" nothing?)
	//Precondition: addToAll is true, or apiVersion exists in versionedShaders.
	const char* node_value = isFile ? NULL : node.child_value();
	std::vector<char>& rawData_vector = versionedShaders[apiVersion].second;
	versionedShaders[apiVersion].first = shaderType;

	size_t initial_size = rawData_vector.size();
	size_t value_size = node_value ? strlen(node_value) : 0;

	if (isFile)
	{
		if (node.attribute("path"))
		{
			//Append the data to the "main" node's vector
			addFileCodeSourceToVector(rawData_vector, node.attribute("path").value(), assetProvider);
		}
		else
		{
			Log(Log.Warning, "PfxParser: Found <file> element in <shader>, but no 'path' attribute."
			    " Skipping. Syntax should be <file path=\"pathname...\".");
		}
	}
	else
	{
		//Append the data to the node's vector
		rawData_vector.resize(rawData_vector.size() + value_size);
		memcpy(rawData_vector.data() + initial_size, node_value, value_size);
	}

	//Append to everything if requested
	if (addToAll)
	{
		for (auto it = versionedShaders.begin(); it != versionedShaders.end(); ++it)
		{
			if (&it->second.second == &rawData_vector) { continue; } // Skip yourself!
			//Append the additional data to the node's vector
			it->second.second.resize(it->second.second.size() + value_size);
			memcpy(it->second.second.data() + (it->second.second.size() - value_size), rawData_vector.data() + initial_size, value_size);
		}
	}
}

void addShaders(effect::Effect& theEffect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end, IAssetProvider* assetProvider)
{
	//For each shader element, we will create one per version...
	for (auto shader = begin; shader != end; ++shader)
	{
		std::map<StringHash, std::pair<types::ShaderType, std::vector<char>/**/>/**/> versionedShaders;
		StringHash shaderName;

		types::ShaderType shaderType = types::ShaderType::UnknownShader;
		//Get its name
		for (auto it2 = shader->attributes_begin(); it2 != shader->attributes_end(); ++it2)
		{
			if (string(it2->name()) == string("name")) { shaderName = it2->value(); }
			if (string(it2->name()) == string("type"))
			{
				shaderType = shaderTypeFromString(*it2);
			}
		}
		if (shaderType == types::ShaderType::UnknownShader)
		{
			Log("PFXReader: Shader with name [%s] was defined without the [type] attribute, or value was unrecognised.", shaderName.c_str());
			continue;
		}
		if (shaderName.empty())
		{
			Log("PFXReader: <shader> element did not have a [name] attribute, and will be skipped as it will not be possible to be referenced by other elements.");
			continue;
		}


		//Generate a list of api versions iterating every child element of the shader.
		//Should be either <file> or <code>, and may or may not contain an apiversion attribute.
		//For now, we create the list of apiversions.
		for (auto child = shader->children().begin(); child != shader->children().end(); ++child)
		{
			const auto& apiVersionAttr = child->attribute("apiVersion");
			if (apiVersionAttr)
			{
				versionedShaders[apiVersionAttr.value()];
			}
			else
			{
				versionedShaders[StringHash()];
			}
		}
		//All valid values have now been added to the vector. Now we concatenate all of them that are
		//either global or belong to the same apiversion.
		for (auto child = shader->children().begin(); child != shader->children().end(); ++child)
		{
			const auto& apiVersionAttr = child->attribute("apiVersion");
			bool isFile = string(child->name()) == string("file"), isCode = string(child->name()) == string("code");

			if (isFile || isCode)    //NoCode!
			{
				addShaderCodeToVectors(
				  shaderName,
				  shaderType,
				  versionedShaders,
				  *child,
				  apiVersionAttr ? StringHash(apiVersionAttr.value()) : StringHash(),
				  isFile,
				  apiVersionAttr.empty(),
				  assetProvider);
			}
			else
			{
				Log(Log.Warning, "PfxParser: Found node that was neither <code> nor <file> while parsing a <shader>. Skipping.");
			}
		}
		//One last bit! Actually add them to the effect. Note - they are character arrays without null-terminators...
		for (auto entry = versionedShaders.begin(); entry != versionedShaders.end(); ++entry)
		{
			theEffect.addShader(entry->first, effect::Shader(StringHash(shaderName), entry->second.first,
			                    string(entry->second.second.begin(), entry->second.second.end())));
		}
	}
}

void addPipelineAttribute(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline,
                          pugi::xml_node& attribute_element)
{
	effect::AttributeSemantic semantic;
	semantic.dataType = dataTypeFromString(attribute_element.attribute("dataType").value());
	semantic.location = attribute_element.attribute("location").as_int();
	semantic.semantic = attribute_element.attribute("semantic").value();
	semantic.variableName = attribute_element.attribute("variable").value();
	semantic.vboBinding = attribute_element.attribute("vboBinding").as_int();
	pipeline.attributes.push_back(std::move(semantic));
}

void addPipelineUniform(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline,
                        pugi::xml_node& attribute_element)
{
	effect::UniformSemantic semantic;
	semantic.dataType = dataTypeFromString(attribute_element.attribute("dataType").value());
	semantic.arrayElements = attribute_element.attribute("arrayElements").as_int();
	if (semantic.arrayElements == 0) { semantic.arrayElements = 1; }
	semantic.semantic = attribute_element.attribute("semantic").value();
	semantic.variableName = attribute_element.attribute("variable").value();
	semantic.scope = scopeFromString(attribute_element.attribute("scope"));
	semantic.set = attribute_element.attribute("set").as_int();
	semantic.binding = attribute_element.attribute("binding").as_int();
	pipeline.uniforms.push_back(semantic);
}

void addPipelineShader(effect::Effect& effect, const StringHash& apiName,
                       effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::Shader shader;
	shader.name = attribute_element.attribute("name").value();
	auto it = effect.versionedShaders[apiName].find(shader.name);
	if (it != effect.versionedShaders[apiName].end())
	{
		pipeline.shaders.push_back(&(it->second));
	}
	else
	{
		if (!apiName.empty())
		{
			Log(Log.Warning, "PFXParser: Could not find shader with name [%s] referenced in pipeline [%s] for api [%s]",
			    shader.name.c_str(), pipeline.name.c_str(), apiName.c_str());
		}
		else
		{
			Log(Log.Warning, "PFXParser: Could not find shader with name [%s] referenced in pipeline [%s] for api unspecified.",
			    shader.name.c_str(), pipeline.name.c_str());
		}
	}
}

void addPipelineBuffer(effect::Effect& effect, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	StringHash name = attribute_element.attribute("name").value();

	auto it = effect.buffers.find(name);
	if (it != effect.buffers.end())
	{
		effect::BufferRef ref;
		ref.binding = attribute_element.attribute("binding").as_int();
		ref.set = attribute_element.attribute("set").as_int();
		ref.semantic = attribute_element.attribute("semantic").value();
		ref.type = bufferDescriptorTypeFromString(attribute_element.attribute("type").value());
		ref.bufferName = name;
		it->second.allSupportedBindings = it->second.allSupportedBindings | descriptorTypeToBufferBindingUse(ref.type);
		it->second.isDynamic = pvr::types::isDescriptorTypeDynamic(ref.type);
		pipeline.buffers.push_back(ref);
	}
	else
	{
		Log("PfxParser::read: Could not find buffer definition [%s] referenced in pipeline [%d]", pipeline.name.c_str(), name.c_str());
	}
}

void addPipelineInputAttachment(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::InputAttachmentRef ref;
	ref.binding = attribute_element.attribute("binding").as_int();
	ref.set = attribute_element.attribute("set").as_int(-1);
	ref.targetIndex = attribute_element.attribute("targetIndex").as_int(-1);
	pipeline.inputAttachments.push_back(ref);
}

void addPipelineTexture(effect::Effect& effect, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	StringHash name = attribute_element.attribute("name").value();
	if (effect.textures.find(name) != effect.textures.end()) { return ; }


	effect::TextureReference ref;
	ref.binding = attribute_element.attribute("binding").as_int();
	ref.set = attribute_element.attribute("set").as_int(-1);
	ref.semantic = attribute_element.attribute("semantic").value();
	ref.samplerFilter = packSamplerFilter(filterFromAttribute(attribute_element.attribute("minification"), types::SamplerFilter::Nearest),
	                                      filterFromAttribute(attribute_element.attribute("magnification"), types::SamplerFilter::Nearest),
	                                      filterFromAttribute(attribute_element.attribute("mipmap"), types::SamplerFilter::None));
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_r"), types::SamplerWrap::Clamp);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_s"), types::SamplerWrap::Clamp);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_t"), types::SamplerWrap::Clamp);
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_u"), ref.wrapR);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_v"), ref.wrapS);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_w"), ref.wrapT);
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_x"), ref.wrapR);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_y"), ref.wrapS);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_z"), ref.wrapT);
	ref.variableName = attribute_element.attribute("variable").value();
	ref.textureName = name;
	pipeline.textures.push_back(ref);
}

void addPipelineBlending(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.blending.blendEnable = attribute_element.attribute("enabled").as_bool();
	pipeline.blending.srcBlendColor = blendFactorFromString(attribute_element.attribute("srcColorFactor").as_string(),
	                                  types::BlendFactor::DefaultSrcRgba);
	pipeline.blending.srcBlendAlpha = blendFactorFromString(attribute_element.attribute("srcAlphaFactor").as_string(),
	                                  types::BlendFactor::DefaultSrcRgba);
	pipeline.blending.destBlendColor = blendFactorFromString(attribute_element.attribute("dstColorFactor").as_string(),
	                                   types::BlendFactor::DefaultDestRgba);
	pipeline.blending.destBlendAlpha = blendFactorFromString(attribute_element.attribute("dstAlphaFactor").as_string(),
	                                   types::BlendFactor::DefaultDestRgba);
	pipeline.blending.blendOpColor = blendOpFromString(attribute_element.attribute("colorBlendOp"));
	pipeline.blending.blendOpAlpha = blendOpFromString(attribute_element.attribute("alphaBlendOp"));
	pipeline.blending.channelWriteMask = blendChannelWriteMaskFromString(attribute_element.attribute("writeMask"));
}


inline types::StencilOp stencilOpFromString(const std::string& str, types::StencilOp dflt)
{
	if (str == "keep")                  { return types::StencilOp::Keep; }
	else if (str == "zero")             { return types::StencilOp::Zero; }
	else if (str == "replace")          { return types::StencilOp::Replace; }
	else if (str == "incrementclamp")   { return types::StencilOp::IncrementClamp; }
	else if (str == "decrementclamp")   { return types::StencilOp::DecrementClamp; }
	else if (str == "invert")           { return types::StencilOp::Invert; }
	else if (str == "incrementwrap")    { return types::StencilOp::IncrementWrap; }
	else if (str == "decrementwrap")    { return types::StencilOp::DecrementWrap; }
	else                                { return dflt; }
}

void addPipelineDepthStencil(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline,
                             pugi::xml_node& attribute_element)
{
	//--- Depth
	pipeline.depthCmpFunc = comparisonModeFromString(attribute_element.attribute("depthFunc").as_string(""),
	                        types::ComparisonMode::DefaultDepthFunc);

	pipeline.enableDepthTest = attribute_element.attribute("depthTest").as_bool("false");
	pipeline.enableDepthWrite = attribute_element.attribute("depthWrite").as_bool("true");

	pipeline.enableStencilTest = attribute_element.attribute("stencilTest").as_bool("true");

	//---- Stencil, check for common
	pipeline.stencilFront.opDepthFail = stencilOpFromString(strings::toLower(
	                                      attribute_element.attribute("stencilOpDepthFail").as_string("")), types::StencilOp::Keep);

	pipeline.stencilFront.opDepthPass = stencilOpFromString(strings::toLower(
	                                      attribute_element.attribute("stencilOpDepthPass").as_string("")), types::StencilOp::Keep);

	pipeline.stencilFront.opStencilFail = stencilOpFromString(strings::toLower(
	                                        attribute_element.attribute("stencilOpStencilFail").as_string("")), types::StencilOp::Keep);

	pipeline.stencilFront.compareMask = attribute_element.attribute("stencilCompareMask").as_uint(0xff);
	pipeline.stencilFront.writeMask = attribute_element.attribute("stencilWriteMask").as_uint(0xff);
	pipeline.stencilFront.reference = attribute_element.attribute("stencilReference").as_uint(0);
	pipeline.stencilFront.compareOp = comparisonModeFromString(attribute_element.attribute(
	                                    "stencilFunc").as_string(""), types::ComparisonMode::DefaultStencilFunc);

	pipeline.stencilBack =  pipeline.stencilFront;

	//---- Stencil, now check for explicit case, overwrite if necessary
	// stencil front
	pipeline.stencilFront.opDepthFail = stencilOpFromString(strings::toLower(
	                                      attribute_element.attribute("stencilOpDepthFailFront").as_string("")), pipeline.stencilFront.opDepthFail);

	pipeline.stencilFront.opDepthPass = stencilOpFromString(strings::toLower(
	                                      attribute_element.attribute("stencilOpDepthPassFront").as_string("")), pipeline.stencilFront.opDepthPass);

	pipeline.stencilFront.opStencilFail = stencilOpFromString(strings::toLower(
	                                        attribute_element.attribute("stencilOpStencilFailFront").as_string("")), pipeline.stencilFront.opStencilFail);

	pipeline.stencilFront.compareMask = attribute_element.attribute("stencilCompareMaskFront").as_uint(pipeline.stencilFront.compareMask);
	pipeline.stencilFront.writeMask = attribute_element.attribute("stencilWriteMaskFront").as_uint(pipeline.stencilFront.writeMask);
	pipeline.stencilFront.reference = attribute_element.attribute("stencilReferenceFront").as_uint(pipeline.stencilFront.reference);
	pipeline.stencilFront.compareOp = comparisonModeFromString(attribute_element.attribute("stencilFunc").as_string(""),
	                                  pipeline.stencilFront.compareOp);

	// stencil back
	pipeline.stencilBack.opDepthFail = stencilOpFromString(strings::toLower(
	                                     attribute_element.attribute("stencilOpDepthFailBack").as_string("")), pipeline.stencilBack.opDepthFail);

	pipeline.stencilBack.opDepthPass = stencilOpFromString(strings::toLower(
	                                     attribute_element.attribute("stencilOpDepthPassBack").as_string("")), pipeline.stencilBack.opDepthPass);

	pipeline.stencilBack.opStencilFail = stencilOpFromString(strings::toLower(
	                                       attribute_element.attribute("stencilOpStencilFailBack").as_string("")), pipeline.stencilBack.opStencilFail);

	pipeline.stencilBack.compareMask = attribute_element.attribute("stencilCompareMaskBack").as_uint(pipeline.stencilBack.compareMask);
	pipeline.stencilBack.writeMask = attribute_element.attribute("stencilWriteMaskBack").as_uint(pipeline.stencilBack.writeMask);
	pipeline.stencilBack.reference = attribute_element.attribute("stencilReferenceBack").as_uint(pipeline.stencilBack.reference);
	pipeline.stencilBack.compareOp = comparisonModeFromString(attribute_element.attribute("stencilFunc").as_string(""),
	                                 pipeline.stencilBack.compareOp);
}


inline types::Face faceFromString(const std::string& str, types::Face defaultFace)
{
	if (str.empty()) { return defaultFace; }
	else if ("none" == str) { return types::Face::None; }
	else if ("front" == str) { return types::Face::Front;}
	else if ("back" == str) { return types::Face::Back;}
	else if ("frontback" == str || "front_and_back" == str || "frontandback" == str)
	{
		return types::Face::FrontBack;
	}
	return defaultFace;
}

inline types::StepRate stepRateFromString(const char* str, types::StepRate defaultStepRate)
{
	const std::string str_l(strings::toLower(str));
	if (str_l == "vertex") { return types::StepRate::Vertex; }
	else if (str_l == "instance") { return types::StepRate::Instance; }
	return defaultStepRate;
}

inline types::PolygonWindingOrder polygonWindingOrderFromString(const std::string& str)
{
	if (str == "cw" || str == "clockwise") { return types::PolygonWindingOrder::FrontFaceCW; }
	else if (str == "ccw" || str == "counterclockwise") {  return types::PolygonWindingOrder::FrontFaceCCW; }
	return types::PolygonWindingOrder::FrontFaceCCW;
}

void addPipelineRasterization(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.cullFace = faceFromString(attribute_element.attribute("faceCulling").as_string(), types::Face::Default);
	pipeline.windingOrder = polygonWindingOrderFromString(attribute_element.attribute("frontFaceWinding").as_string("ccw"));
}

void addPipelineVertexInputBinding(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.vertexBinding.push_back(assets::effect::PipelineVertexBinding(
	                                   attribute_element.attribute("index").as_uint(), stepRateFromString(
	                                     attribute_element.attribute("stepRate").as_string(""), types::StepRate::Vertex)));
}

typedef void (*pfn_add_element)(effect::Effect&, const StringHash&, effect::PipelineDefinition&, pugi::xml_node&);

void addElementsToPipelines(effect::Effect& effect, std::map<StringHash, effect::PipelineDefinition>& pipelines,
                            pugi::xml_node& pipe_element, pfn_add_element adder)
{
	if (pipe_element.attribute("apiVersion"))
	{
		const auto& apiversion = pipe_element.attribute("apiVersion").value();
		adder(effect, apiversion, pipelines[apiversion], pipe_element);
	}
	else
	{
		for (auto versions = pipelines.begin(); versions != pipelines.end(); ++versions)
		{
			adder(effect, versions->first, versions->second, pipe_element);
		}
	}
}


const StringHash empty_str("");
bool processPipeline(effect::Effect& effect, pugi::xml_node& pipe_element, const StringHash& name)
{
	std::map<StringHash, effect::PipelineDefinition> pipelines;
	typedef pugi::xml_named_node_iterator pugi_node_iterator;
	pipelines[empty_str].name = name;

	for (auto it = pipe_element.children().begin(); it != pipe_element.children().end(); ++it)
	{
		if (it->attribute("apiVersion"))
		{
			pipelines[it->attribute("apiVersion").value()].name = name;
		}
	}

	for (auto it = effect.getVersions().begin(); it != effect.getVersions().end(); ++it)
	{
		pipelines[it->c_str()].name = name;
	}

	//add attributes
	for (auto it = pipe_element.children("attribute").begin(); it != pipe_element.children("attribute").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineAttribute);
	}

	//add uniforms
	for (auto it = pipe_element.children("uniform").begin(); it != pipe_element.children("uniform").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineUniform);
	}

	//add shaders
	for (auto it = pipe_element.children("shader").begin(); it != pipe_element.children("shader").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineShader);
	}

	//add buffers
	for (auto it = pipe_element.children("buffer").begin(); it != pipe_element.children("buffer").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineBuffer);
	}

	//add textures
	for (auto it = pipe_element.children("texture").begin(); it != pipe_element.children("texture").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineTexture);
	}

	//add input attachments
	for (auto it = pipe_element.children("inputattachment").begin(); it != pipe_element.children("inputattachment").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineInputAttachment);
	}


	// add the blending
	for (auto it =  pipe_element.children("blending").begin(); it != pipe_element.children("blending").end() ; ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineBlending);
	}

	// add the depth stencil
	// add a default if depthStencil children not found

	for (auto it = pipe_element.children("depthstencil").begin(); it != pipe_element.children("depthstencil").end() ; ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineDepthStencil);
	}


	// add the raster states
	// add defaults if rasterization children not found
	if (pipe_element.children("rasterization").begin() == pipe_element.children("rasterization").end())
	{
		addElementsToPipelines(effect, pipelines, pipe_element, &addPipelineRasterization);
	}
	else
	{
		for (auto it = pipe_element.children("rasterization").begin(); it != pipe_element.children("rasterization").end() ; ++it)
		{
			addElementsToPipelines(effect, pipelines, *it, &addPipelineRasterization);
		}
	}

	// add the pipeline binding
	for (auto it = pipe_element.children("vbobinding").begin(); it != pipe_element.children("vbobinding").end() ; ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineVertexInputBinding);
	}

	for (auto it = pipelines.begin(); it != pipelines.end(); ++it)
	{
		effect.versionedPipelines[it->first][it->second.name] = it->second;
	}
	return true;
}

void addPipelines(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	//Each pipeline element...
	for (auto pipe_element = begin; pipe_element != end; ++pipe_element)
	{
		StringHash pipelineName;

		//Get its name
		for (auto it2 = pipe_element->attributes_begin(); it2 != pipe_element->attributes_end(); ++it2)
		{
			if (string(it2->name()) == string("name")) { pipelineName = it2->value(); }
		}
		processPipeline(effect, *pipe_element, pipelineName);
	}
}

void addSubpassGroup(effect::SubpassGroup& outGroup, pugi::xml_node& subpassgroup_element)
{
	uint32 pipe_counter = 0;
	outGroup.name = subpassgroup_element.attribute("name").as_string("");
	outGroup.pipelines.resize(static_cast<pvr::uint32>(subpassgroup_element.select_nodes("pipeline").size()));
	pipe_counter = 0;
	for (auto pipeline = subpassgroup_element.children("pipeline").begin();
	     pipeline != subpassgroup_element.children("pipeline").end(); ++pipeline)
	{
		effect::PipelineReference& ref = outGroup.pipelines[pipe_counter++];
		ref.pipelineName = pipeline->attribute("name").value();
		int32 counter = 0;
		auto condition_begin = pipeline->children("condition").begin();
		auto condition_end = pipeline->children("condition").end();
		//unfortunately no operator "-" exists for those iterators - they are not random access, so we'll traverse twice. No big deal.
		for (auto conditions = condition_begin; conditions != condition_end; ++conditions) { ++counter; }
		ref.conditions.resize(counter);
		counter = 0;
		for (auto condition = pipeline->children("condition").begin(); condition != pipeline->children("condition").end(); ++condition)
		{
			ref.conditions[counter].type = conditionFromAttribute(condition->attribute("type"));
			ref.conditions[counter++].value = condition->attribute("name").value();
		}

		counter = 0;
		auto identifiers_begin = pipeline->children("exportIdentifier").begin();
		auto identifiers_end = pipeline->children("exportIdentifier").end();
		for (auto identifier = identifiers_begin; identifier != identifiers_end; ++identifier) { ++counter; }
		ref.identifiers.resize(counter);
		counter = 0;
		for (auto identifier = identifiers_begin; identifier != identifiers_end; ++identifier)
		{
			ref.identifiers[counter++] = identifier->attribute("name").value();
		}
	}
}

void addSubpass(effect::Subpass& outSubPass, pugi::xml_node& subpass_element)
{
	//-----------------------------------------
	// render targets
	outSubPass.targets[0] = StringHash(subpass_element.attribute("target0").as_string("default"));
	outSubPass.targets[1] = StringHash(subpass_element.attribute("target1").value());
	outSubPass.targets[2] = StringHash(subpass_element.attribute("target2").value());
	outSubPass.targets[3] = StringHash(subpass_element.attribute("target3").value());

	//-----------------------------------------
	// inputs
	outSubPass.inputs[0] = StringHash(subpass_element.attribute("input0").value());
	outSubPass.inputs[1] = StringHash(subpass_element.attribute("input1").value());
	outSubPass.inputs[2] = StringHash(subpass_element.attribute("input2").value());
	outSubPass.inputs[3] = StringHash(subpass_element.attribute("input3").value());
	outSubPass.useDepthStencil = subpass_element.attribute("usesDepthStencil").as_bool(true);

	//----
	// if there is no subpassgroup then add one
	// else process all the subgroups
	if (subpass_element.children("subpassgroup").begin() == subpass_element.children("subpassgroup").end())
	{
		outSubPass.groups.resize(1);
		addSubpassGroup(outSubPass.groups[0], subpass_element);
	}
	else
	{
		outSubPass.groups.resize(subpass_element.select_nodes("subpassgroup").size());
		uint32 groupIndex = 0;
		for (auto walk = subpass_element.children("subpassgroup").begin();
		     walk != subpass_element.children("subpassgroup").end(); ++walk, ++groupIndex)
		{
			addSubpassGroup(outSubPass.groups[groupIndex], *walk);
		}
	}

}

void addPass(effect::Effect& effect, pugi::xml_node& pass_element)
{
	effect.passes.resize(effect.passes.size() + 1);
	effect::Pass& pass = effect.passes.back();

	pass.name = pass_element.attribute("name").as_string("");
	pass.targetDepthStencil = pass_element.attribute("targetDepthStencil").as_string("");//TODO


	// do the subpasses
	auto subpass_begin = pass_element.children("subpass").begin();
	auto subpass_end = pass_element.children("subpass").end();
	uint32 size = static_cast<pvr::uint32>(pass_element.select_nodes("subpass").size());
	size = (size ? size : 1);
	pass.subpasses.resize(size);

	// if we have no supass then create one.
	if (subpass_begin == subpass_end)
	{
		addSubpass(pass.subpasses[0], pass_element);
	}
	else
	{
		effect::Subpass* subpass = &pass.subpasses[0];
		for (auto it = subpass_begin; it != subpass_end; ++it)
		{
			addSubpass(*subpass, *it); ++subpass;
		}
	}
}

void addEffects(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	//Each effect element...
	for (auto effect_element = begin; effect_element != end; ++effect_element)
	{
		//Get its name
		for (auto it2 = effect_element->attributes_begin(); it2 != effect_element->attributes_end(); ++it2)
		{
			if (string(it2->name()) == string("name")) { effect.name = it2->value(); }
		}

		auto pass_begin = effect_element->children("pass").begin();
		auto pass_end = effect_element->children("pass").end();
		if (pass_begin == pass_end) //If there is only one pass, it is allowed skip the "pass" elements and put the rest straight into the pass.
		{
			addPass(effect, *effect_element);
		}
		else
		{
			for (auto pass = pass_begin; pass != pass_end; ++pass)
			{
				addPass(effect, *pass);
			}
		}
	}
}

void findVersions(effect::Effect& effect, std::set<StringHash>& apiversions, pugi::xml_node root)
{
	for (auto it = root.children().begin(); it != root.children().end(); ++it)
	{
		if (it->attribute("apiVersion"))
		{
			apiversions.insert(it->attribute("apiVersion").value());
		}
		findVersions(effect, apiversions, *it);
	}
}

void addVersions(effect::Effect& effect, pugi::xml_node root)
{
	std::set<StringHash> apiversions;
	apiversions.insert("");

	findVersions(effect, apiversions, root);

	for (auto it = apiversions.begin(); it != apiversions.end(); ++it)
	{
		effect.addVersion(*it);
	}
}

}
/// <summary>Constructor. The OSManager is used to load files in a platform-specific way. If the OSManager is NULL, then
/// only a FileStreams from the current directory will be attempted to be loaded.</summary>
PfxParser::PfxParser(const std::string& pfxFilename, IAssetProvider* assetProvider) : assetProvider(assetProvider)
{
	if (!assetProvider)
	{
		Log(Log.Warning, "PfxParser: Asset provider was not passed on construction, so a fallback path that can only "
		    "create FileStreams is used. This is not enough to function on many mobile platforms. "
		    "You should pass the Application class (itself deriving from pvr::Shell, which is an IAssetProvider "
		    "as the asset provider, otherwise consider writing a custom pvr::IAssetProvider.");
	}

	Stream::ptr_type stream(getStream(pfxFilename, assetProvider));
	if (stream.get())
	{
		newAssetStream(stream);
	}
	else
	{
		Log("PfxParser: PFX Filename [%s] was not be found", pfxFilename.c_str());
	}
}

/// <summary>Constructor. The OSManager is used to load pfxFilename and any shader files in a platform-specific way. If the
/// OSManager is NULL, then only FileStreams will be attempted to be loaded.</summary>
PfxParser::PfxParser(Stream::ptr_type pfxStream, IAssetProvider* assetProvider) : assetProvider(assetProvider)
{
	if (!assetProvider)
	{
		Log(Log.Warning, "PfxParser: Asset provider was not passed on construction, so a fallback path that can only "
		    "create FileStreams is used. This is not enough to function on many mobile platforms. "
		    "You should pass the Application class (itself deriving from pvr::Shell, which is an IAssetProvider "
		    "as the asset provider, otherwise consider writing a custom pvr::IAssetProvider.");
	}
	if (pfxStream.get())
	{
		newAssetStream(pfxStream);
	}
	else
	{
		Log("PfxParser: PFX stream provided was not open.");
	}
}

bool PfxParser::readNextAsset(effect::Effect& asset)
{
	asset.clear();
	std::vector<char> v = _assetStream->readToEnd<char>();

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer_inplace(v.data(), v.size());

	if (result.status != pugi::xml_parse_status::status_ok || !doc || !doc.root())
	{
		Log("Failed to parse PFX file - not valid XML"); return false;
	}
	if (!doc.root().first_child() || string(doc.root().first_child().name()) != string("pfx"))
	{
		Log("Failed to parse PFX file: root <pfx> element not found");
	}

	const auto& root = doc.root().first_child();
	auto textures = root.children("texture");
	auto shaders = root.children("shader");
	auto buffers = root.children("buffer");
	auto pipelines = root.children("pipeline");
	auto effects = root.children("effect");

	//*** Load header attributes ***//
	for (auto it = root.attributes_begin(); it != root.attributes_end(); ++it)
	{
		asset.headerAttributes[it->name()] = it->value();
	}

	//*** Load Textures ***//
	addVersions(asset, root); //Pre-process a list of all different version flavors.
	addTextures(asset, textures.begin(), textures.end());
	addBuffers(asset, buffers.begin(), buffers.end());
	addShaders(asset, shaders.begin(), shaders.end(), assetProvider);
	addPipelines(asset, pipelines.begin(), pipelines.end());
	addEffects(asset, effects.begin(), effects.end());
	return true;
}

}

}
}
//!\endcond
