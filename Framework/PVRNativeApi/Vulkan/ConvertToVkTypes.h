/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\ConvertToApiTypes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains conversions of pvr Enumerations to Vulkan types.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRApi/GpuCapabilities.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief	Contain functions to convert several PowerVR Framework types to their Native, Vulkan representations,
usually, from an enumeration to a vulkan type.
*******************************************************************************************************************/
namespace ConvertToVk {

/*!****************************************************************************************************************
\brief Convert to vulkan buffer usage flags
\param bufferUse Buffer binding use
\return A VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT etc)
*******************************************************************************************************************/
inline VkBufferUsageFlagBits bufferBindingUse(types::BufferBindingUse::Bits bufferUse) { return (VkBufferUsageFlagBits)bufferUse; }

/*!****************************************************************************************************************
\brief Convert to vulkan image type
\param textureDim Texture dimension
\return A VkImageType (VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D)
*******************************************************************************************************************/
inline VkImageType textureDimensionImageType(types::TextureDimension::Enum textureDim)
{
	switch (textureDim)
	{
	case types::TextureDimension::Texture1D:
		return VK_IMAGE_TYPE_1D;

	case types::TextureDimension::Texture2D:
	case types::TextureDimension::Texture2DCube:
	case types::TextureDimension::Texture2DArray:
		return VK_IMAGE_TYPE_2D;

	case types::TextureDimension::Texture3D:
	case types::TextureDimension::Texture3DArray:
		return VK_IMAGE_TYPE_3D;

	default:
		assertion(false ,  "Invalid texture dimension");
		return VK_IMAGE_TYPE_MAX_ENUM;
	}
}

/*!****************************************************************************************************************
\brief Convert to vulkan image view type
\param textureDim Texture dimension
\return A VkImageViewType (VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D etc)
*******************************************************************************************************************/
inline VkImageViewType textureDimensionImageView(types::TextureDimension::Enum textureDim)
{
	switch (textureDim)
	{
	case types::TextureDimension::Texture1D:
		return VK_IMAGE_VIEW_TYPE_1D;

	case types::TextureDimension::Texture2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case types::TextureDimension::Texture2DCube:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case types::TextureDimension::Texture2DArray:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case types::TextureDimension::Texture3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		assertion(false ,  "Invalid texture dimension");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
}

/*!****************************************************************************************************************
\brief Convert to vulkan primitive topology 
\param primitiveTopology Primitive topology
\return A VkPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP etc)
*******************************************************************************************************************/
inline VkPrimitiveTopology primitiveTopology(types::PrimitiveTopology::Enum primitiveTopology)
{

	static const VkPrimitiveTopology map [] = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	                                            VK_PRIMITIVE_TOPOLOGY_MAX_ENUM,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	                                            VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	                                            VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	                                          };
	return map[primitiveTopology];

}

/*!****************************************************************************************************************
\brief Convert to vulkan format
\param dataType Type of the data(Float32, Int32 etc)
\param width The Width of the data type
\return A VkFormat (VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT etc)
*******************************************************************************************************************/
inline VkFormat dataFormat(types::DataType::Enum dataType, uint8 width)
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

/*!****************************************************************************************************************
\brief Convert to vulkan vertex input rate
\param stepRate The step rate of the vertex input(Vertex, Instance)
\return A VkVertexInputRate (VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE)
*******************************************************************************************************************/
inline VkVertexInputRate stepRate(types::StepRate::Enum stepRate)
{
	return (stepRate == types::StepRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE);
}

/*!****************************************************************************************************************
\brief Convert to vulkan sample count
\param numSamples Number of samples
\return A VkSampleCountFlagBits (VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_2_BIT, etc)
*******************************************************************************************************************/
inline VkSampleCountFlagBits aaSamples(uint8 numSamples)
{
	return (numSamples < 8 ?
	        (numSamples < 2 ? VK_SAMPLE_COUNT_1_BIT : numSamples < 4 ? VK_SAMPLE_COUNT_2_BIT : VK_SAMPLE_COUNT_4_BIT)
	        :
	        (numSamples < 16 ? VK_SAMPLE_COUNT_8_BIT : numSamples < 32 ? VK_SAMPLE_COUNT_16_BIT : VK_SAMPLE_COUNT_32_BIT));
}

/*!****************************************************************************************************************
\brief Convert to vulkan buffer usage flag bits
\param use Buffer usage flag bits
\return A bits of VkBufferUsageFlagBits (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, etc)
*******************************************************************************************************************/
inline VkBufferUsageFlagBits bufferUsage(types::BufferBindingUse::Bits use)
{
	return (VkBufferUsageFlagBits)use;
}

/*!****************************************************************************************************************
\brief Convert to vulkan sampler mip-map mode
\param filter Mip map sampler filter
\return A VkSamplerMipmapMode (VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR)
*******************************************************************************************************************/
inline VkSamplerMipmapMode mipmapFilter(types::SamplerFilter::Enum filter)
{
	return VkSamplerMipmapMode(filter == 2 ? 0 : filter);
}

/*!****************************************************************************************************************
\brief Convert to vulkan pixel format
\param format Pixel format
\param colorSpace Color space of the format (lRGB, sRGB)
\param dataType TYpe of the data (SignedByte, SignedInteger etc)
\return A VkFormat representing the pixel format
*******************************************************************************************************************/
inline VkFormat pixelFormat(PixelFormat format, types::ColorSpace::Enum colorSpace, VariableType::Enum dataType)
{

	bool isSrgb = (colorSpace == types::ColorSpace::sRGB);
	bool isSigned = VariableType::isSigned(dataType);
	if (format.getPart().High == 0)
	{

		//Format and type == 0 for compressed textures.
		switch (format.getPixelTypeId())
		{
		case CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		case CompressedPixelFormat::PVRTCII_2bpp:
		case CompressedPixelFormat::PVRTCII_4bpp:
		case CompressedPixelFormat::ETC1:
		case CompressedPixelFormat::DXT2:
		case CompressedPixelFormat::DXT4:
			return VK_FORMAT_UNDEFINED;
		case CompressedPixelFormat::PVRTCI_4bpp_RGBA: return VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK;
		case CompressedPixelFormat::PVRTCI_4bpp_RGB: return VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK;
		case CompressedPixelFormat::SharedExponentR9G9B9E5: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		case CompressedPixelFormat::ETC2_RGB: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
		case CompressedPixelFormat::ETC2_RGBA: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
		case CompressedPixelFormat::ETC2_RGB_A1: return (isSrgb ? VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
		case CompressedPixelFormat::EAC_R11: return (isSigned ? VK_FORMAT_EAC_R11_SNORM_BLOCK : VK_FORMAT_EAC_R11_UNORM_BLOCK);
		case CompressedPixelFormat::EAC_RG11: return (isSigned ? VK_FORMAT_EAC_R11G11_SNORM_BLOCK : VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
		//Formats not supported by opengl/opengles
		case CompressedPixelFormat::BC2: return (isSrgb ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK);
		case CompressedPixelFormat::BC3: return (isSrgb ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK);
		case CompressedPixelFormat::BC4: return (isSigned ? VK_FORMAT_BC4_SNORM_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK);
		case CompressedPixelFormat::BC5: return (isSigned ? VK_FORMAT_BC5_SNORM_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK);
		case CompressedPixelFormat::BC6: return (isSigned ? VK_FORMAT_BC6H_SFLOAT_BLOCK : VK_FORMAT_BC6H_UFLOAT_BLOCK);
		case CompressedPixelFormat::BC7: return (isSrgb ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK);
		case CompressedPixelFormat::BC1:
		case CompressedPixelFormat::RGBG8888:
		case CompressedPixelFormat::GRGB8888:
		case CompressedPixelFormat::UYVY:
		case CompressedPixelFormat::YUY2:
		case CompressedPixelFormat::BW1bpp:
			return VK_FORMAT_UNDEFINED;
		}
	}
	else
	{
		bool depthOrStencil = (format.getChannelContent(0) == 'd' || format.getChannelContent(0) == 's' || format.getChannelContent(1) == 'd');
		if (depthOrStencil)
		{
			switch (format.getPixelTypeId())
			{
			case assets::GeneratePixelType1<'d', 32>::ID: return VK_FORMAT_D32_SFLOAT;
			case assets::GeneratePixelType1<'d', 24>::ID:
			case assets::GeneratePixelType2<'x', 8, 'd', 24 >::ID:
			case assets::GeneratePixelType2<'d', 24, 'x', 8 >::ID: return VK_FORMAT_D32_SFLOAT;
			case assets::GeneratePixelType1<'d', 16>::ID: return VK_FORMAT_D16_UNORM;
			case assets::GeneratePixelType2<'d', 's', 32, 8>::ID: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case assets::GeneratePixelType2<'d', 's', 24, 8>::ID: return VK_FORMAT_D24_UNORM_S8_UINT;
			case assets::GeneratePixelType2<'d', 's', 16, 8>::ID: return VK_FORMAT_D16_UNORM_S8_UINT;
			case assets::GeneratePixelType1<'s', 8>::ID: return VK_FORMAT_S8_UINT;
			}
		}
		else
		{

			switch (dataType)
			{
			case VariableType::UnsignedFloat:
				if (format.getPixelTypeId() == assets::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID)
				{
					return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				}
				break;
			case VariableType::SignedFloat:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SFLOAT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SFLOAT;
				case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_SFLOAT;
				case assets::GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_SFLOAT;
				case assets::GeneratePixelType1<'l', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case assets::GeneratePixelType1<'a', 16>::ID: return VK_FORMAT_R16_SFLOAT;
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_SFLOAT;
				case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_SFLOAT;
				case assets::GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				case assets::GeneratePixelType2<'l', 'a', 32, 32>::ID: return VK_FORMAT_R32G32_SFLOAT;
				case assets::GeneratePixelType1<'l', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				case assets::GeneratePixelType1<'a', 32>::ID: return VK_FORMAT_R32_SFLOAT;
				}
				break;
			}
			case VariableType::UnsignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM);
				case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM);
				case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID: return VK_FORMAT_R8G8_UNORM;
				case assets::GeneratePixelType1<'r', 8>::ID:
				case assets::GeneratePixelType1<'l', 8>::ID:
				case assets::GeneratePixelType1<'a', 8>::ID: return VK_FORMAT_R8_UNORM;
				case assets::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_B8G8R8A8_UNORM);
				}
			}
			case VariableType::SignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_SNORM;
				case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_SNORM;
				case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID: return VK_FORMAT_R8G8B8_SNORM;
				case assets::GeneratePixelType1<'r', 8>::ID:
				case assets::GeneratePixelType1<'l', 8>::ID:
				case assets::GeneratePixelType1<'a', 8>::ID: return VK_FORMAT_R8_SNORM;
					break;
				}
			}
			case VariableType::UnsignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_UINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_UINT;
				case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID: return VK_FORMAT_R8G8_UINT;
				case assets::GeneratePixelType1<'r', 8>::ID: return VK_FORMAT_R8_UINT;
				}
			}
			case VariableType::SignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VK_FORMAT_R8G8B8A8_SINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VK_FORMAT_R8G8B8_SINT;
				case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID: return VK_FORMAT_R8G8_SINT;
				case assets::GeneratePixelType1<'r', 8>::ID: return VK_FORMAT_R8_SINT;
				}
				break;
			}
			case VariableType::UnsignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
				case assets::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: return VK_FORMAT_R5G6B5_UNORM_PACK16;
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_UNORM;
				case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_UNORM;
				case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_UNORM;
				case assets::GeneratePixelType2<'d', 16, 's', 8>::ID: return VK_FORMAT_D16_UNORM_S8_UINT;
				case assets::GeneratePixelType1<'r', 16>::ID:
				case assets::GeneratePixelType1<'a', 16>::ID:
				case assets::GeneratePixelType1<'l', 16>::ID: return VK_FORMAT_R16_UNORM;
				}
				break;
			}
			case VariableType::SignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SNORM;
				case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SNORM;
				case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID: return VK_FORMAT_R16G16_SNORM;
				case assets::GeneratePixelType1<'r', 16>::ID:
				case assets::GeneratePixelType1<'l', 16>::ID:
				case assets::GeneratePixelType1<'a', 16>::ID: return VK_FORMAT_R16_SNORM;
				}
				break;
			}
			case VariableType::UnsignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_UINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_UINT;
				case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_UINT;
				case assets::GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_UINT;
				}
				break;
			}
			case VariableType::SignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VK_FORMAT_R16G16B16A16_SINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VK_FORMAT_R16G16B16_SINT;
				case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID: return VK_FORMAT_R16G16_SINT;
				case assets::GeneratePixelType1<'r', 16>::ID: return VK_FORMAT_R16_SINT;
				}
				break;
			}
			case VariableType::UnsignedIntegerNorm:
			{
				switch (format.getPixelTypeId())
				{
				case  assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				case  assets::GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				}
				break;
			}
			case VariableType::UnsignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_UINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_UINT;
				case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_UINT;
				case assets::GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_UINT;
				case assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				}
				break;
			}
			case VariableType::SignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VK_FORMAT_R32G32B32A32_SINT;
				case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VK_FORMAT_R32G32B32_SINT;
				case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID: return VK_FORMAT_R32G32_SINT;
				case assets::GeneratePixelType1<'r', 32>::ID: return VK_FORMAT_R32_SINT;
				}
				break;
			}
			default: {}
			}
		}
	}

	return VK_FORMAT_UNDEFINED;
}

/*!****************************************************************************************************************
\brief Convert to vulkan pixel format
\param format Pixel format
\param colorSpace Color space of the format (lRGB, sRGB)
\param dataType TYpe of the data (SignedByte, SignedInteger etc)
\param[out] outIsCompressedFormat Return if its a compressed format
\return A VkFormat representing the pixel format
*******************************************************************************************************************/
inline VkFormat pixelFormat(PixelFormat format, types::ColorSpace::Enum colorSpace, VariableType::Enum dataType, bool& outIsCompressedFormat)
{
	outIsCompressedFormat = (format.getPart().High == 0) && (format.getPixelTypeId() != CompressedPixelFormat::SharedExponentR9G9B9E5);
	return pixelFormat(format, colorSpace, dataType);
}

#define PVR_DECLARE_DIRECT_MAPPING(_vktype_, _frameworktype_, _name_) inline _vktype_ _name_(_frameworktype_ item){ return (_vktype_)item; }

PVR_DECLARE_DIRECT_MAPPING(VkAttachmentLoadOp, types::LoadOp::Enum, loadOp);
PVR_DECLARE_DIRECT_MAPPING(VkAttachmentStoreOp, types::StoreOp::Enum, storeOp);
PVR_DECLARE_DIRECT_MAPPING(VkLogicOp, types::LogicOp::Enum, logicOp);
PVR_DECLARE_DIRECT_MAPPING(VkBlendOp, types::BlendOp::Enum, blendOp);
PVR_DECLARE_DIRECT_MAPPING(VkBlendFactor, types::BlendFactor::Enum, blendFactor);
PVR_DECLARE_DIRECT_MAPPING(VkColorComponentFlags, types::ColorChannel::Bits, colorChannel);
PVR_DECLARE_DIRECT_MAPPING(VkCompareOp, types::ComparisonMode::Enum, compareMode);
PVR_DECLARE_DIRECT_MAPPING(VkStencilOp, types::StencilOp::Enum, stencilOp);
PVR_DECLARE_DIRECT_MAPPING(VkPolygonMode, types::FillMode::Enum, polygonMode);
PVR_DECLARE_DIRECT_MAPPING(VkCullModeFlags, types::Face::Enum, cullMode);
PVR_DECLARE_DIRECT_MAPPING(VkFrontFace, types::PolygonWindingOrder::Enum, frontFaceWinding);
PVR_DECLARE_DIRECT_MAPPING(VkSamplerAddressMode, types::SamplerWrap::Enum, samplerWrap);
PVR_DECLARE_DIRECT_MAPPING(VkFilter, types::SamplerFilter::Enum, samplerFilter)
PVR_DECLARE_DIRECT_MAPPING(VkBorderColor, types::BorderColor::Enum, borderColor)
PVR_DECLARE_DIRECT_MAPPING(VkComponentSwizzle, types::Swizzle::Enum, swizzle)
PVR_DECLARE_DIRECT_MAPPING(VkComponentSwizzle, uint8, swizzle)
PVR_DECLARE_DIRECT_MAPPING(VkImageLayout, types::ImageLayout::Enum, imageLayout)
PVR_DECLARE_DIRECT_MAPPING(VkAccessFlags, types::AccessFlags::Bits, accessFlags)
PVR_DECLARE_DIRECT_MAPPING(VkDescriptorType, types::DescriptorType::Enum, descriptorType);
PVR_DECLARE_DIRECT_MAPPING(VkShaderStageFlagBits, types::ShaderStageFlags::Bits, shaderStage);
PVR_DECLARE_DIRECT_MAPPING(VkPipelineStageFlagBits, types::PipelineStageFlags::Bits, pipelineStage);
PVR_DECLARE_DIRECT_MAPPING(VkImageAspectFlagBits, types::ImageAspect::Bits, imageAspect);
PVR_DECLARE_DIRECT_MAPPING(VkPipelineBindPoint, types::PipelineBindPoint::Enum, pipelineBindPoint);

/*!****************************************************************************************************************
\brief Convert to vulkan image sub-resource range
\param area Image sub-resource range
\return A VkImageSubresourceRange
*******************************************************************************************************************/
inline VkImageSubresourceRange imageSubResourceRange(const types::ImageSubresourceRange& area)
{
	VkImageSubresourceRange retval;
	retval.aspectMask = imageAspect(area.aspect);
	retval.baseArrayLayer = area.arrayLayerOffset;
	retval.baseMipLevel = area.mipLevelOffset;
	retval.layerCount = area.numArrayLevels;
	retval.levelCount = area.numMipLevels;
	return retval;
}
}// ConvertToVulkan

namespace ConvertFromVulkan {
/*!****************************************************************************************************************
\brief Convert from image format to framework ImageDataFormats
\param format Vulkan image data format
\return A ImageDataFormat
*******************************************************************************************************************/
inline ImageDataFormat imageDataFormat(VkFormat format)
{
	ImageDataFormat fmt;
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_SRGB: fmt.colorSpace = types::ColorSpace::sRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = assets::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_B8G8R8A8_SRGB: fmt.colorSpace = types::ColorSpace::sRGB; fmt.dataType = VariableType::UnsignedByteNorm; fmt.format = assets::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedShortNorm; fmt.format = assets::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID;
		break;
	case VK_FORMAT_D16_UNORM: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedShortNorm; fmt.format = assets::GeneratePixelType1<'d', 16>::ID;
		break;
	case VK_FORMAT_D16_UNORM_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedIntegerNorm; fmt.format = assets::GeneratePixelType2<'d', 's', 16, 8>::ID;
		break;
	case VK_FORMAT_D24_UNORM_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedIntegerNorm; fmt.format = assets::GeneratePixelType2<'d', 's', 24, 8>::ID;
		break;
	case VK_FORMAT_D32_SFLOAT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = assets::GeneratePixelType1<'d', 32>::ID;
		break;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = assets::GeneratePixelType2<'d', 32, 's', 8>::ID;
		break;
	case VK_FORMAT_X8_D24_UNORM_PACK32: fmt.colorSpace = types::ColorSpace::lRGB; fmt.dataType = VariableType::UnsignedFloat; fmt.format = assets::GeneratePixelType1<'d', 24>::ID;
		break;

	default: assertion(0, "UNIMPLEMENTED FORMAT - JUST ADD SPECIFIED ENTRY");
	}
	return fmt;
}
}

}// namespace api
}//namespace pvr
