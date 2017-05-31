<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\ConvertToVkTypes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains conversions of pvr Enumerations to Vulkan types.
***********************************************************************************************************************/
=======
/*!
\brief Contains conversions of pvr Enumerations to Vulkan types.
\file PVRNativeApi/Vulkan/ConvertToVkTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRCore/Base/ComplexTypes.h"
#include "PVRCore/Texture.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"

namespace pvr {

extern bool use_old_pvrtc_vulkan_enums;

<<<<<<< HEAD
namespace api {
/*!****************************************************************************************************************
\brief  Contain functions to convert several PowerVR Framework types to their Native, Vulkan representations,
usually, from an enumeration to a vulkan type.
*******************************************************************************************************************/
namespace ConvertToVk {

/*!****************************************************************************************************************
\brief Convert to vulkan buffer usage flags
\param bufferUse Buffer binding use
\return A VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT etc)
*******************************************************************************************************************/
inline VkBufferUsageFlagBits bufferBindingUse(types::BufferBindingUse bufferUse) { return (VkBufferUsageFlagBits)bufferUse; }

/*!****************************************************************************************************************
\brief Convert to vulkan image type
\param textureDimension Texture dimension
\return A VkImageType (VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D)
*******************************************************************************************************************/
=======
namespace nativeVk {
/// <summary>Contain functions to convert several PowerVR Framework types to their Native, Vulkan representations,
/// usually, from an enumeration to a vulkan type.</summary>
namespace ConvertToVk {

/// <summary>Convert to vulkan buffer usage flags</summary>
/// <param name="bufferUse">Buffer binding use</param>
/// <returns>A VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT etc)
/// </returns>
inline VkBufferUsageFlagBits bufferBindingUse(types::BufferBindingUse bufferUse) { return (VkBufferUsageFlagBits)bufferUse; }

/// <summary>Convert to vulkan image type</summary>
/// <param name="textureDimension">Texture dimension</param>
/// <returns>A VkImageType (VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D)</returns>
>>>>>>> 1776432f... 4.3
inline VkImageType textureViewTypeToImageBaseType(types::ImageViewType textureDimension)
{
	switch (textureDimension)
	{
	case types::ImageViewType::ImageView1D:
		return VK_IMAGE_TYPE_1D;

	case types::ImageViewType::ImageView2D:
	case types::ImageViewType::ImageView2DCube:
	case types::ImageViewType::ImageView2DArray:
		return VK_IMAGE_TYPE_2D;

	case types::ImageViewType::ImageView3D:
	case types::ImageViewType::ImageView3DArray:
		return VK_IMAGE_TYPE_3D;

	default:
		assertion(false ,  "Invalid texture dimension");
		return VK_IMAGE_TYPE_MAX_ENUM;
	}
}

inline VkImageViewType imageBaseTypeToTexViewType(types::ImageBaseType baseType, uint32 numArrayLayers,
    bool isCubeMap)
{
	// if it is a cube map it has to be 2D Texture base
	if (isCubeMap && baseType != types::ImageBaseType::Image2D)
	{
		assertion(baseType == types::ImageBaseType::Image2D, "Cubemap texture must be 2D");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
	// array must be atleast 1
	if (!numArrayLayers)
	{
		assertion(false, "Number of array layers must be greater than equal to 0");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
	// if it is array it must be 1D or 2D texture base
<<<<<<< HEAD
	if (numArrayLayers && baseType > types::ImageBaseType::Image2D)
=======
	if ((numArrayLayers > 1) && (baseType > types::ImageBaseType::Image2D))
>>>>>>> 1776432f... 4.3
	{
		assertion(false, "1D and 2D image type supports array texture");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}

	VkImageViewType vkType[] =
	{
		VK_IMAGE_VIEW_TYPE_1D,
		VK_IMAGE_VIEW_TYPE_1D_ARRAY,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		VK_IMAGE_VIEW_TYPE_3D ,
		VK_IMAGE_VIEW_TYPE_CUBE,
		VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
	};
	if (isCubeMap)
	{
		numArrayLayers = (numArrayLayers > 6) * 6;
	}
	return vkType[((uint32)baseType * 2) + (isCubeMap ? 3 : 0) + (numArrayLayers > 1 ? 1 : 0)];
}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan image view type
\param texDimemsion Texture dimension
\return A VkImageViewType (VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D etc)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan image view type</summary>
/// <param name="texDimemsion">Texture dimension</param>
/// <returns>A VkImageViewType (VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D etc)</returns>
>>>>>>> 1776432f... 4.3
inline VkImageViewType textureViewType(types::ImageViewType texDimemsion)
{
	switch (texDimemsion)
	{
	case types::ImageViewType::ImageView1D:
		return VK_IMAGE_VIEW_TYPE_1D;

	case types::ImageViewType::ImageView2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case types::ImageViewType::ImageView2DCube:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case types::ImageViewType::ImageView2DArray:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case types::ImageViewType::ImageView3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		assertion(false ,  "Invalid texture dimension");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan primitive topology
\param primitiveTopology Primitive topology
\return A VkPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP etc)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan primitive topology</summary>
/// <param name="primitiveTopology">Primitive topology</param>
/// <returns>A VkPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
/// etc)</returns>
>>>>>>> 1776432f... 4.3
inline VkPrimitiveTopology primitiveTopology(types::PrimitiveTopology primitiveTopology)
{

	static const VkPrimitiveTopology map [] = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	                                            VK_PRIMITIVE_TOPOLOGY_MAX_ENUM,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	                                            VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_MAX_ENUM
	                                          };
	return map[(uint32)primitiveTopology];

}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan format
\param dataType Type of the data(Float32, Int32 etc)
\param width The Width of the data type
\return A VkFormat (VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT etc)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan format</summary>
/// <param name="dataType">Type of the data(Float32, Int32 etc)</param>
/// <param name="width">The Width of the data type</param>
/// <returns>A VkFormat (VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT etc)</returns>
>>>>>>> 1776432f... 4.3
inline VkFormat dataFormat(types::DataType dataType, uint8 width)
{
	static const VkFormat Float32[] = { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };
	static const VkFormat Int32[] = { VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT };
	static const VkFormat UInt32[] = { VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT };
	static const VkFormat Int8[] = { VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8A8_SINT };
	static const VkFormat Int8Norm[] = { VK_FORMAT_R8_SNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8A8_SNORM };
	static const VkFormat Int16[] = { VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16A16_SINT };
	static const VkFormat Int16Norm[] = { VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16A16_SNORM };
	static const VkFormat UInt8[] = { VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8A8_UINT };
	static const VkFormat UInt8Norm[] = { VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM };
	static const VkFormat UInt16[] = { VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16A16_UINT };
	static const VkFormat UInt16Norm[] = { VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM };
	switch (dataType)
	{
	case pvr::types::DataType::Float32: return Float32[width - 1];
	case pvr::types::DataType::Int16: return Int16[width - 1];
	case pvr::types::DataType::Int16Norm: return Int16Norm[width - 1];
	case pvr::types::DataType::Int8: return Int8[width - 1];
	case pvr::types::DataType::Int8Norm: return Int8Norm[width - 1];
	case pvr::types::DataType::UInt8: return UInt8[width - 1];
	case pvr::types::DataType::UInt8Norm: return UInt8Norm[width - 1];
	case pvr::types::DataType::UInt16: return UInt16[width - 1];
	case pvr::types::DataType::UInt16Norm: return UInt16Norm[width - 1];
	case pvr::types::DataType::Int32: return Int32[width - 1];
	case pvr::types::DataType::UInt32: return UInt32[width - 1];
	case pvr::types::DataType::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
	case pvr::types::DataType::UBYTE4: return VK_FORMAT_R8G8B8A8_UINT;
	case pvr::types::DataType::DEC3N:  return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	case pvr::types::DataType::Fixed16_16: return VK_FORMAT_R16G16_SNORM;
	case pvr::types::DataType::ABGR: return VK_FORMAT_A8B8G8R8_UNORM_PACK32;

	//case pvr::types::DataType::ARGB: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
	case pvr::types::DataType::Custom:
	case pvr::types::DataType::None:
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan vertex input rate
\param stepRate The step rate of the vertex input(Vertex, Instance)
\return A VkVertexInputRate (VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan vertex input rate</summary>
/// <param name="stepRate">The step rate of the vertex input(Vertex, Instance)</param>
/// <returns>A VkVertexInputRate (VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE)</returns>
>>>>>>> 1776432f... 4.3
inline VkVertexInputRate stepRate(types::StepRate stepRate)
{
	return (stepRate == types::StepRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE);
}

/// <summary>Convert to vulkan sample count</summary>
/// <param name="numSamples">Number of samples</param>
/// <returns>A VkSampleCountFlagBits (VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_2_BIT, etc)</returns>
inline VkSampleCountFlagBits aaSamples(uint8 numSamples)
{
	return (numSamples < 8 ?
	        (numSamples < 2 ? VK_SAMPLE_COUNT_1_BIT : numSamples < 4 ? VK_SAMPLE_COUNT_2_BIT : VK_SAMPLE_COUNT_4_BIT)
	        :
	        (numSamples < 16 ? VK_SAMPLE_COUNT_8_BIT : numSamples < 32 ? VK_SAMPLE_COUNT_16_BIT : VK_SAMPLE_COUNT_32_BIT));
}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan buffer usage flag bits
\param use Buffer usage flag bits
\return A bits of VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, etc)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan buffer usage flag bits</summary>
/// <param name="use">Buffer usage flag bits</param>
/// <returns>A bits of VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
/// etc)</returns>
>>>>>>> 1776432f... 4.3
inline VkBufferUsageFlagBits bufferUsage(types::BufferBindingUse use)
{
	return (VkBufferUsageFlagBits)use;
}

<<<<<<< HEAD
//inline VkMemoryPropertyFlagBits bufferUse(types::BufferUse::Bits use)
//{
//	VkMemoryPropertyFlagBits vkFlagBits[] =
//	{
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
//		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//		VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
//		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
//	};
//
//	VkMemoryPropertyFlags returnVkBits = 0;
//	if (use == types::BufferUse::DEFAULT)
//	{
//		returnVkBits |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//	}
//	if (use == types::BufferUse::DYNAMIC)
//	{
//		returnVkBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
//	}
//	if (use == types::BufferUse::STAGING)
//	{
//		returnVkBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
//	}
//	if ((use & types::BufferUse::CPU_READ) || (use & types::BufferUse::CPU_WRITE))
//	{
//		returnVkBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
//	}
//	return (VkMemoryPropertyFlagBits)returnVkBits;
//}



/*!****************************************************************************************************************
\brief Convert to vulkan sampler mip-map mode
\param filter Mip map sampler filter
\return A VkSamplerMipmapMode (VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR)
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan sampler mip-map mode</summary>
/// <param name="filter">Mip map sampler filter</param>
/// <returns>A VkSamplerMipmapMode (VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR)</returns>
>>>>>>> 1776432f... 4.3
inline VkSamplerMipmapMode mipmapFilter(types::SamplerFilter filter)
{
	return VkSamplerMipmapMode((uint32)filter & 1); //Nearest = Nearest, Linear = Linear, None = Nearest, Cubic = linear
}

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan pixel format
\param format Pixel format
\param colorSpace Color space of the format (lRGB, sRGB)
\param dataType Type of the data (SignedByte, SignedInteger etc)
\return A VkFormat representing the pixel format
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">Type of the data (SignedByte, SignedInteger etc)</param>
/// <returns>A VkFormat representing the pixel format</returns>
>>>>>>> 1776432f... 4.3
inline VkFormat pixelFormat(PixelFormat format, types::ColorSpace colorSpace, VariableType dataType)
{

#ifndef VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA
#define VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA -0x40000001
#define VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA -0x40000002
#endif

	bool isSrgb = (colorSpace == types::ColorSpace::sRGB);
	bool isSigned = isVariableTypeSigned(dataType);
	if (format.getPart().High == 0) // IS COMPRESSED FORMAT!
	{
		//Format and type == 0 for compressed textures.
		switch (format.getPixelTypeId())
		{
		//PVRTC

		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB: //fall through
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA: return (isSrgb ? VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp: return (isSrgb ? VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG);
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp: return (isSrgb ? VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG);

		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB: return use_old_pvrtc_vulkan_enums ? (VkFormat)VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA : isSrgb ? VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA: return use_old_pvrtc_vulkan_enums ? (VkFormat)VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA : isSrgb ? VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;

		//OTHER COMPRESSED
		case (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		case (uint64)CompressedPixelFormat::ETC2_RGB: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ETC2_RGBA: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ETC2_RGB_A1: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::EAC_R11: return (isSigned ? VK_FORMAT_EAC_R11_SNORM_BLOCK : VK_FORMAT_EAC_R11_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::EAC_RG11: return (isSigned ? VK_FORMAT_EAC_R11G11_SNORM_BLOCK : VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
		//Formats not supported by opengl/opengles
		case (uint64)CompressedPixelFormat::BC2: return (isSrgb ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::BC3: return (isSrgb ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::BC4: return (isSigned ? VK_FORMAT_BC4_SNORM_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::BC5: return (isSigned ? VK_FORMAT_BC5_SNORM_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::BC6: return (isSigned ? VK_FORMAT_BC6H_SFLOAT_BLOCK : VK_FORMAT_BC6H_UFLOAT_BLOCK);
		case (uint64)CompressedPixelFormat::BC7: return (isSrgb ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_10x10: return (isSrgb ? VK_FORMAT_ASTC_10x10_SRGB_BLOCK : VK_FORMAT_ASTC_10x10_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_10x5: return (isSrgb ? VK_FORMAT_ASTC_10x5_SRGB_BLOCK : VK_FORMAT_ASTC_10x5_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_10x6: return (isSrgb ? VK_FORMAT_ASTC_10x6_SRGB_BLOCK : VK_FORMAT_ASTC_10x6_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_10x8: return (isSrgb ? VK_FORMAT_ASTC_10x8_SRGB_BLOCK : VK_FORMAT_ASTC_10x8_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_12x10: return (isSrgb ? VK_FORMAT_ASTC_12x10_SRGB_BLOCK : VK_FORMAT_ASTC_12x10_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_12x12: return (isSrgb ? VK_FORMAT_ASTC_12x12_SRGB_BLOCK : VK_FORMAT_ASTC_12x12_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_4x4: return (isSrgb ? VK_FORMAT_ASTC_4x4_SRGB_BLOCK : VK_FORMAT_ASTC_4x4_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_5x4: return (isSrgb ? VK_FORMAT_ASTC_5x4_SRGB_BLOCK : VK_FORMAT_ASTC_5x4_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_5x5: return (isSrgb ? VK_FORMAT_ASTC_5x5_SRGB_BLOCK : VK_FORMAT_ASTC_5x5_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_6x5: return (isSrgb ? VK_FORMAT_ASTC_6x5_SRGB_BLOCK : VK_FORMAT_ASTC_6x5_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_8x5: return (isSrgb ? VK_FORMAT_ASTC_8x5_SRGB_BLOCK : VK_FORMAT_ASTC_8x5_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_8x6: return (isSrgb ? VK_FORMAT_ASTC_8x6_SRGB_BLOCK : VK_FORMAT_ASTC_8x6_UNORM_BLOCK);
		case (uint64)CompressedPixelFormat::ASTC_8x8: return (isSrgb ? VK_FORMAT_ASTC_8x8_SRGB_BLOCK : VK_FORMAT_ASTC_8x8_UNORM_BLOCK);

#define UNSUPPORTED_FORMAT(fmt) case (uint64)CompressedPixelFormat::fmt: return VK_FORMAT_UNDEFINED;

			///////// UNSUPPORTED FORMATS
			UNSUPPORTED_FORMAT(ETC1);
			UNSUPPORTED_FORMAT(DXT2)
			UNSUPPORTED_FORMAT(DXT4)
			UNSUPPORTED_FORMAT(BC1)
			UNSUPPORTED_FORMAT(RGBG8888)
			UNSUPPORTED_FORMAT(GRGB8888)
			UNSUPPORTED_FORMAT(UYVY)
			UNSUPPORTED_FORMAT(YUY2)
			UNSUPPORTED_FORMAT(BW1bpp)
			UNSUPPORTED_FORMAT(ASTC_3x3x3)
			UNSUPPORTED_FORMAT(ASTC_4x3x3)
			UNSUPPORTED_FORMAT(ASTC_4x4x3)
			UNSUPPORTED_FORMAT(ASTC_4x4x4)
			UNSUPPORTED_FORMAT(ASTC_5x4x4)
			UNSUPPORTED_FORMAT(ASTC_5x5x4)
			UNSUPPORTED_FORMAT(ASTC_5x5x5)
			UNSUPPORTED_FORMAT(ASTC_6x5x5)
			UNSUPPORTED_FORMAT(ASTC_6x6x5)
			UNSUPPORTED_FORMAT(ASTC_6x6x6)
#undef UNSUPPORTED_FORMAT
		}
	}
	else
	{
		bool depthOrStencil = (format.getChannelContent(0) == 'd' || format.getChannelContent(0) == 's' || format.getChannelContent(1) == 'd');
		if (depthOrStencil)
		{
			switch (format.getPixelTypeId())
			{
<<<<<<< HEAD
			case assets::GeneratePixelType1<'d', 32>::ID: return VK_FORMAT_D32_SFLOAT;
			case assets::GeneratePixelType1<'d', 24>::ID:
			case assets::GeneratePixelType2<'x', 8, 'd', 24>::ID:
			case assets::GeneratePixelType2<'d', 24, 'x', 8>::ID: return VK_FORMAT_D32_SFLOAT;
			case assets::GeneratePixelType1<'d', 16>::ID: return VK_FORMAT_D16_UNORM;
			case assets::GeneratePixelType2<'d', 's', 32, 8>::ID: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case assets::GeneratePixelType2<'d', 's', 24, 8>::ID: return VK_FORMAT_D24_UNORM_S8_UINT;
			case assets::GeneratePixelType2<'d', 's', 16, 8>::ID: return VK_FORMAT_D16_UNORM_S8_UINT;
			case assets::GeneratePixelType1<'s', 8>::ID: return VK_FORMAT_S8_UINT;
=======
			case GeneratePixelType1<'d', 32>::ID: return VK_FORMAT_D32_SFLOAT;
			case GeneratePixelType1<'d', 24>::ID:
			case GeneratePixelType2<'x', 8, 'd', 24>::ID:
			case GeneratePixelType2<'d', 24, 'x', 8>::ID: return VK_FORMAT_D32_SFLOAT;
			case GeneratePixelType1<'d', 16>::ID: return VK_FORMAT_D16_UNORM;
			case GeneratePixelType2<'d', 's', 32, 8>::ID: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case GeneratePixelType2<'d', 's', 24, 8>::ID: return VK_FORMAT_D24_UNORM_S8_UINT;
			case GeneratePixelType2<'d', 's', 16, 8>::ID: return VK_FORMAT_D16_UNORM_S8_UINT;
			case GeneratePixelType1<'s', 8>::ID: return VK_FORMAT_S8_UINT;
>>>>>>> 1776432f... 4.3
			}
		}
		else
		{

			switch (dataType)
			{
			case VariableType::UnsignedFloat:
				if (format.getPixelTypeId() == GeneratePixelType3<'b', 'g', 'r', 10, 11, 11>::ID)
				{
					return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				}
				break;
			case VariableType::SignedFloat:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SFLOAT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_SFLOAT;
				case GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_SFLOAT;
				case GeneratePixelType1<'l', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case GeneratePixelType1<'a', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_SFLOAT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_SFLOAT;
				case GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				case GeneratePixelType2<'l', 'a', 32, 32>::ID: return VK_FORMAT_R32G32_SFLOAT;
				case GeneratePixelType1<'l', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				case GeneratePixelType1<'a', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				}
				break;
			}
			case VariableType::UnsignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM);
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM);
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return VK_FORMAT_R8G8_UNORM;
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return VK_FORMAT_R8_UNORM;
				case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_B8G8R8A8_UNORM);
				}
			}
			case VariableType::SignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_SNORM;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return VK_FORMAT_R8G8B8_SNORM;
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return VK_FORMAT_R8_SNORM;
					break;
				}
			}
			case VariableType::UnsignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_UINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return VK_FORMAT_R8G8_UINT;
				case GeneratePixelType1<'r', 8>::ID: return VK_FORMAT_R8_UINT;
				}
			}
			case VariableType::SignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_SINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return VK_FORMAT_R8G8_SINT;
				case GeneratePixelType1<'r', 8>::ID: return VK_FORMAT_R8_SINT;
				}
				break;
			}
			case VariableType::UnsignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
				case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: return VK_FORMAT_R5G6B5_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_UNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_UNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_UNORM;
				case GeneratePixelType2<'d', 16, 's', 8>::ID: return VK_FORMAT_D16_UNORM_S8_UINT;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID: return VK_FORMAT_R16_UNORM;
				}
				break;
			}
			case VariableType::SignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_SNORM;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID: return VK_FORMAT_R16_SNORM;
				}
				break;
			}
			case VariableType::UnsignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_UINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_UINT;
				case GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_UINT;
				}
				break;
			}
			case VariableType::SignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_SINT;
				case GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_SINT;
				}
				break;
			}
			case VariableType::UnsignedIntegerNorm:
			{
				switch (format.getPixelTypeId())
				{
				case  GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				case  GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				}
				break;
			}
			case VariableType::UnsignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_UINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_UINT;
				case GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_UINT;
				case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				}
				break;
			}
			case VariableType::SignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_SINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_SINT;
				case GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_SINT;
				}
				break;
			}
			default: {}
			}
		}
	}

	return VK_FORMAT_UNDEFINED;
}


<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan pixel format
\param format Image Data format
\return A VkFormat representing the pixel format
*******************************************************************************************************************/
inline VkFormat pixelFormat(const ImageDataFormat& format)
{
	return pixelFormat(format.format, format.colorSpace, format.dataType);
}


/*!****************************************************************************************************************
\brief Convert to vulkan pixel format
\param format Pixel format
\param colorSpace Color space of the format (lRGB, sRGB)
\param dataType TYpe of the data (SignedByte, SignedInteger etc)
\param[out] outIsCompressedFormat Return if its a compressed format
\return A VkFormat representing the pixel format
*******************************************************************************************************************/
inline VkFormat pixelFormat(PixelFormat format, types::ColorSpace colorSpace, VariableType dataType, bool& outIsCompressedFormat)
{
=======
/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Image Data format</param>
/// <returns>A VkFormat representing the pixel format</returns>
inline VkFormat pixelFormat(const ImageDataFormat& format)
{
	return pixelFormat(format.format, format.colorSpace, format.dataType);
}


/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">TYpe of the data (SignedByte, SignedInteger etc)</param>
/// <param name="outIsCompressedFormat">Return if its a compressed format</param>
/// <returns>A VkFormat representing the pixel format</returns>
inline VkFormat pixelFormat(PixelFormat format, types::ColorSpace colorSpace, VariableType dataType, bool& outIsCompressedFormat)
{
>>>>>>> 1776432f... 4.3
	outIsCompressedFormat = (format.getPart().High == 0) && (format.getPixelTypeId() != (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5);
	return pixelFormat(format, colorSpace, dataType);
}

#define PVR_DECLARE_DIRECT_MAPPING(_vktype_, _frameworktype_, _name_) inline _vktype_ _name_(_frameworktype_ item){ return (_vktype_)item; }

PVR_DECLARE_DIRECT_MAPPING(VkAttachmentLoadOp, types::LoadOp, loadOp);
PVR_DECLARE_DIRECT_MAPPING(VkAttachmentStoreOp, types::StoreOp, storeOp);
PVR_DECLARE_DIRECT_MAPPING(VkLogicOp, types::LogicOp, logicOp);
PVR_DECLARE_DIRECT_MAPPING(VkBlendOp, types::BlendOp, blendOp);
PVR_DECLARE_DIRECT_MAPPING(VkBlendFactor, types::BlendFactor, blendFactor);
PVR_DECLARE_DIRECT_MAPPING(VkColorComponentFlags, types::ColorChannel, colorChannel);
PVR_DECLARE_DIRECT_MAPPING(VkCompareOp, types::ComparisonMode, compareMode);
PVR_DECLARE_DIRECT_MAPPING(VkStencilOp, types::StencilOp, stencilOp);
PVR_DECLARE_DIRECT_MAPPING(VkPolygonMode, types::FillMode, polygonMode);
PVR_DECLARE_DIRECT_MAPPING(VkCullModeFlags, types::Face, cullMode);
PVR_DECLARE_DIRECT_MAPPING(VkFrontFace, types::PolygonWindingOrder, frontFaceWinding);
PVR_DECLARE_DIRECT_MAPPING(VkSamplerAddressMode, types::SamplerWrap, samplerWrap);
PVR_DECLARE_DIRECT_MAPPING(VkFilter, types::SamplerFilter, samplerFilter);
PVR_DECLARE_DIRECT_MAPPING(VkBorderColor, types::BorderColor, borderColor);
PVR_DECLARE_DIRECT_MAPPING(VkComponentSwizzle, types::Swizzle, swizzle);
PVR_DECLARE_DIRECT_MAPPING(VkComponentSwizzle, uint8, swizzle);
PVR_DECLARE_DIRECT_MAPPING(VkImageLayout, types::ImageLayout, imageLayout);
PVR_DECLARE_DIRECT_MAPPING(VkAccessFlags, types::AccessFlags, accessFlags);
PVR_DECLARE_DIRECT_MAPPING(VkDescriptorType, types::DescriptorType, descriptorType);
PVR_DECLARE_DIRECT_MAPPING(VkShaderStageFlagBits, types::ShaderStageFlags, shaderStage);
PVR_DECLARE_DIRECT_MAPPING(VkPipelineStageFlagBits, types::PipelineStageFlags, pipelineStage);
PVR_DECLARE_DIRECT_MAPPING(VkImageAspectFlagBits, types::ImageAspect, imageAspect);
PVR_DECLARE_DIRECT_MAPPING(VkPipelineBindPoint, types::PipelineBindPoint, pipelineBindPoint);
PVR_DECLARE_DIRECT_MAPPING(VkImageUsageFlagBits, types::ImageUsageFlags, imageUsageFlags);
PVR_DECLARE_DIRECT_MAPPING(VkSampleCountFlagBits, types::SampleCount, sampleCount);
<<<<<<< HEAD
/*!****************************************************************************************************************
\brief Convert to vulkan image sub-resource range
\param area Image sub-resource range
\return A VkImageSubresourceRange
*******************************************************************************************************************/
=======
/// <summary>Convert to vulkan image sub-resource range</summary>
/// <param name="area">Image sub-resource range</param>
/// <returns>A VkImageSubresourceRange</returns>
>>>>>>> 1776432f... 4.3
inline VkImageSubresourceRange imageSubResourceRange(const types::ImageSubresourceRange& area)
{
	return VkImageSubresourceRange
	{
		(VkImageAspectFlags)imageAspect(area.aspect),
		area.mipLevelOffset,
		area.numArrayLevels,
		area.arrayLayerOffset,
		area.numMipLevels
	};
}

inline VkImageSubresourceLayers imageSubresourceLayers(const types::ImageSubResourceLayers& imageLayers)
{
	return VkImageSubresourceLayers
	{
		(VkImageAspectFlags)ConvertToVk::imageAspect(imageLayers.aspect),
		imageLayers.mipLevelOffset,
		imageLayers.arrayLayerOffset,
		imageLayers.numArrayLayers,
	};
<<<<<<< HEAD
}


inline VkImageBlit imageBlit(const types::ImageBlitRange& range)
{
	return VkImageBlit
	{
		ConvertToVk::imageSubresourceLayers(range.srcSubResource),
		// srcOffsets
		{
			VkOffset3D{range.srcOffset[0].offsetX, range.srcOffset[0].offsetY, range.srcOffset[0].offsetZ},
			VkOffset3D{range.srcOffset[1].offsetX, range.srcOffset[1].offsetY, range.srcOffset[1].offsetZ}
		},
		ConvertToVk::imageSubresourceLayers(range.dstSubResource),
		// dstOffsets
		{
			VkOffset3D{range.dstOffset[0].offsetX, range.dstOffset[0].offsetY, range.dstOffset[0].offsetZ },
			VkOffset3D{range.dstOffset[1].offsetX, range.dstOffset[1].offsetY, range.dstOffset[1].offsetZ }
		}
	};
}

=======
}


inline VkImageBlit imageBlit(const types::ImageBlitRange& range)
{
	return VkImageBlit
	{
		ConvertToVk::imageSubresourceLayers(range.srcSubResource),
		// srcOffsets
		{
			VkOffset3D{range.srcOffset[0].offsetX, range.srcOffset[0].offsetY, range.srcOffset[0].offsetZ},
			VkOffset3D{range.srcOffset[1].offsetX, range.srcOffset[1].offsetY, range.srcOffset[1].offsetZ}
		},
		ConvertToVk::imageSubresourceLayers(range.dstSubResource),
		// dstOffsets
		{
			VkOffset3D{range.dstOffset[0].offsetX, range.dstOffset[0].offsetY, range.dstOffset[0].offsetZ },
			VkOffset3D{range.dstOffset[1].offsetX, range.dstOffset[1].offsetY, range.dstOffset[1].offsetZ }
		}
	};
}

>>>>>>> 1776432f... 4.3
inline VkBufferImageCopy bufferImageCopy(const types::BufferImageCopy& region)
{
	return VkBufferImageCopy
	{
		region.bufferOffset,
		region.bufferRowLength,
		region.bufferImageHeight,
		imageSubresourceLayers(region.imageSubResource),
		VkOffset3D{ int32_t(region.imageOffset.x), int32_t(region.imageOffset.y), int32_t(region.imageOffset.z) },
		VkExtent3D{ uint32_t(region.imageExtent.x), uint32_t(region.imageExtent.y), uint32_t(region.imageExtent.z) },
	};
}


}// ConvertToVulkan

namespace ConvertFromVulkan {
/// <summary>Convert from image format to framework ImageDataFormats</summary>
/// <param name="format">Vulkan image data format</param>
/// <returns>A ImageDataFormat</returns>
inline ImageDataFormat imageDataFormat(VkFormat format)
{
	ImageDataFormat fmt;
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_SRGB: fmt.colorSpace = types::ColorSpace::sRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_B8G8R8A8_SRGB: fmt.colorSpace = types::ColorSpace::sRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedShortNorm; fmt.format = GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID;
		break;
	case VK_FORMAT_D16_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedShortNorm; fmt.format = GeneratePixelType1<'d', 16>::ID;
		break;
	case VK_FORMAT_D16_UNORM_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedIntegerNorm; fmt.format = PixelFormat::Depth16Stencil8;
		break;
	case VK_FORMAT_D24_UNORM_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedIntegerNorm; fmt.format = PixelFormat::Depth24Stencil8;
		break;
	case VK_FORMAT_D32_SFLOAT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = PixelFormat::Depth32;
		break;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = PixelFormat::Depth32Stencil8;
		break;
	case VK_FORMAT_X8_D24_UNORM_PACK32: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = PixelFormat::Depth24;
		break;

	default: assertion(0, "UNIMPLEMENTED FORMAT - JUST ADD SPECIFIED ENTRY");
	}
	return fmt;
}
}

}// namespace api
}//namespace pvr

//!\endcond