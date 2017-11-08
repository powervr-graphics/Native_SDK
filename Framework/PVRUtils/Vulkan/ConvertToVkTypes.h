/*!
\brief Contains conversions of pvr Enumerations to Vulkan types.
\file PVRUtils/Vulkan/ConvertToVkTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
///**********************************************************************
///     NOTE
/// ThisFile has functions to:
/// convert from enums in pvr namespace to Vulkan Enums
/// Convert struct/ class from pvr type to struct/class
///**********************************************************************
#include "PVRVk/TypesVk.h"
#include "PVRCore/Base/ComplexTypes.h"
#include "PVRCore/Texture.h"
#include "PVRVk/BindingsVk.h"
namespace pvr {
namespace utils {
/// <summary>Contain functions to convert several PowerVR Framework types to their Native, Vulkan representations,
/// usually, from an enumeration to a vulkan type.</summary>
#define PVR_DECLARE_DIRECT_MAPPING(_vktype_, _frameworktype_) inline _vktype_ convertToVk(_frameworktype_ item){ return (_vktype_)item; }
PVR_DECLARE_DIRECT_MAPPING(VkPrimitiveTopology, ::pvr::PrimitiveTopology);
PVR_DECLARE_DIRECT_MAPPING(VkBufferUsageFlags, ::pvr::BufferUsageFlags);
PVR_DECLARE_DIRECT_MAPPING(VkBlendOp, ::pvr::BlendOp);
PVR_DECLARE_DIRECT_MAPPING(VkColorComponentFlags, ::pvr::ColorChannelFlags);
PVR_DECLARE_DIRECT_MAPPING(VkBlendFactor, ::pvr::BlendFactor);
PVR_DECLARE_DIRECT_MAPPING(VkStencilOp, ::pvr::StencilOp);
PVR_DECLARE_DIRECT_MAPPING(VkSamplerAddressMode, ::pvr::SamplerAddressMode);
PVR_DECLARE_DIRECT_MAPPING(VkFilter, ::pvr::Filter);
PVR_DECLARE_DIRECT_MAPPING(VkSamplerMipmapMode, ::pvr::SamplerMipmapMode);
PVR_DECLARE_DIRECT_MAPPING(VkCompareOp, ::pvr::CompareOp);
PVR_DECLARE_DIRECT_MAPPING(VkImageAspectFlags, ::pvr::ImageAspectFlags);
PVR_DECLARE_DIRECT_MAPPING(VkImageType, ::pvr::ImageType);
PVR_DECLARE_DIRECT_MAPPING(VkDescriptorType, ::pvr::DescriptorType);

inline VkIndexType convertToVk(IndexType type)
{
	return type == IndexType::IndexType16Bit ? VkIndexType::e_UINT16 : VkIndexType::e_UINT32;
}

/// <summary>Convert to vulkan image view type</summary>
/// <param name="texDimemsion">Texture dimension</param>
/// <returns>A VkImageViewType (VkImageViewType::e_1D, VkImageViewType::e_2D etc)</returns>
inline VkImageViewType convertToVk(ImageViewType texDimemsion)
{
	switch (texDimemsion)
	{
	case ImageViewType::ImageView1D:
		return VkImageViewType::e_1D;

	case ImageViewType::ImageView2D:
		return VkImageViewType::e_2D;
	case ImageViewType::ImageView2DCube:
		return VkImageViewType::e_CUBE;
	case ImageViewType::ImageView2DArray:
		return VkImageViewType::e_2D_ARRAY;
	case ImageViewType::ImageView3D:
		return VkImageViewType::e_3D;
	default:
		assertion(false,  "Invalid texture dimension");
		return VkImageViewType::e_MAX_ENUM;
	}
}

/// <summary>Convert to vulkan vertex input rate</summary>
/// <param name="stepRate">The step rate of the vertex input(Vertex, Instance)</param>
/// <returns>A VkVertexInputRate (VkVertexInputRate::e_VERTEX, VkVertexInputRate::e_INSTANCE)</returns>
inline VkVertexInputRate convertToVk(StepRate stepRate)
{
	return (stepRate == StepRate::Vertex ? VkVertexInputRate::e_VERTEX : VkVertexInputRate::e_INSTANCE);
}

/// <summary>Convert to vulkan vertex input rate</summary>
/// <param name="stepRate">The step rate of the vertex input(Vertex, Instance)</param>
/// <returns>A VkVertexInputRate (VkVertexInputRate::e_VERTEX, VkVertexInputRate::e_INSTANCE)</returns>
inline pvrvk::DataType convertToVk(DataType dataType)
{
	return (pvrvk::DataType)dataType;
}


/// <summary>Convert to vulkan sample count</summary>
/// <param name="numSamples">Number of samples</param>
/// <returns>A VkSampleCountFlags (VkSampleCountFlags::e_1_BIT, VkSampleCountFlags::e_2_BIT, etc)</returns>
inline VkSampleCountFlags convertToVkNumSamples(uint8_t numSamples)
{
	return (numSamples < 8 ?
	        (numSamples < 2 ? VkSampleCountFlags::e_1_BIT : numSamples < 4 ? VkSampleCountFlags::e_2_BIT : VkSampleCountFlags::e_4_BIT)
	        :
	        (numSamples < 16 ? VkSampleCountFlags::e_8_BIT : numSamples < 32 ? VkSampleCountFlags::e_16_BIT : VkSampleCountFlags::e_32_BIT));
}

/// <summary>Convert to vulkan sampler mip-map mode</summary>
/// <param name="filter">Mip map sampler filter</param>
/// <returns>A VkSamplerMipmapMode (VkSamplerMipmapMode::e_NEAREST, VkSamplerMipmapMode::e_LINEAR)</returns>
inline VkSamplerMipmapMode convertToVkMipmapMode(Filter filter)
{
	return VkSamplerMipmapMode(static_cast<uint32_t>(filter) & 1); //Nearest = Nearest, Linear = Linear, None = Nearest, Cubic = linear
}

/// <summary>Convert to vulkan format</summary>
/// <param name="dataType">Type of the data(Float32, Int32 etc)</param>
/// <param name="width">The Width of the data type</param>
/// <returns>A VkFormat (VkFormat::e_R32_SFLOAT, VkFormat::e_R32G32_SFLOAT etc)</returns>
inline VkFormat convertToVkVertexInputFormat(DataType dataType, uint8_t width)
{
	static const VkFormat Float32[] = { VkFormat::e_R32_SFLOAT, VkFormat::e_R32G32_SFLOAT, VkFormat::e_R32G32B32_SFLOAT, VkFormat::e_R32G32B32A32_SFLOAT };
	static const VkFormat Int32[] = { VkFormat::e_R32_SINT, VkFormat::e_R32G32_SINT, VkFormat::e_R32G32B32_SINT, VkFormat::e_R32G32B32A32_SINT };
	static const VkFormat UInt32[] = { VkFormat::e_R32_UINT, VkFormat::e_R32G32_UINT, VkFormat::e_R32G32B32_UINT, VkFormat::e_R32G32B32A32_UINT };
	static const VkFormat Int8[] = { VkFormat::e_R8_SINT, VkFormat::e_R8G8_SINT, VkFormat::e_R8G8B8_SINT, VkFormat::e_R8G8B8A8_SINT };
	static const VkFormat Int8Norm[] = { VkFormat::e_R8_SNORM, VkFormat::e_R8G8_SNORM, VkFormat::e_R8G8B8_SNORM, VkFormat::e_R8G8B8A8_SNORM };
	static const VkFormat Int16[] = { VkFormat::e_R16_SINT, VkFormat::e_R16G16_SINT, VkFormat::e_R16G16B16_SINT, VkFormat::e_R16G16B16A16_SINT };
	static const VkFormat Int16Norm[] = { VkFormat::e_R16_SNORM, VkFormat::e_R16G16_SNORM, VkFormat::e_R16G16B16_SNORM, VkFormat::e_R16G16B16A16_SNORM };
	static const VkFormat UInt8[] = { VkFormat::e_R8_UINT, VkFormat::e_R8G8_UINT, VkFormat::e_R8G8B8_UINT, VkFormat::e_R8G8B8A8_UINT };
	static const VkFormat UInt8Norm[] = { VkFormat::e_R8_UNORM, VkFormat::e_R8G8_UNORM, VkFormat::e_R8G8B8_UNORM, VkFormat::e_R8G8B8A8_UNORM };
	static const VkFormat UInt16[] = { VkFormat::e_R16_UINT, VkFormat::e_R16G16_UINT, VkFormat::e_R16G16B16_UINT, VkFormat::e_R16G16B16A16_UINT };
	static const VkFormat UInt16Norm[] = { VkFormat::e_R16_UNORM, VkFormat::e_R16G16_UNORM, VkFormat::e_R16G16B16_UNORM, VkFormat::e_R16G16B16A16_UNORM };
	switch (dataType)
	{
	case DataType::Float32: return Float32[width - 1];
	case DataType::Int16: return Int16[width - 1];
	case DataType::Int16Norm: return Int16Norm[width - 1];
	case DataType::Int8: return Int8[width - 1];
	case DataType::Int8Norm: return Int8Norm[width - 1];
	case DataType::UInt8: return UInt8[width - 1];
	case DataType::UInt8Norm: return UInt8Norm[width - 1];
	case DataType::UInt16: return UInt16[width - 1];
	case DataType::UInt16Norm: return UInt16Norm[width - 1];
	case DataType::Int32: return Int32[width - 1];
	case DataType::UInt32: return UInt32[width - 1];
	case DataType::RGBA: return VkFormat::e_R8G8B8A8_UNORM;
	case DataType::UBYTE4: return VkFormat::e_R8G8B8A8_UINT;
	case DataType::DEC3N:  return VkFormat::e_A2R10G10B10_UNORM_PACK32;
	case DataType::Fixed16_16: return VkFormat::e_R16G16_SNORM;
	case DataType::ABGR: return VkFormat::e_A8B8G8R8_UNORM_PACK32;
	case DataType::Custom:
	case DataType::None:
	default:
		return VkFormat::e_UNDEFINED;
	}
}

/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">Type of the data (SignedByte, SignedInteger etc)</param>
/// <returns>A VkFormat representing the pixel format</returns>
inline VkFormat convertToVkPixelFormat(PixelFormat format, ColorSpace colorSpace, VariableType dataType)
{
	bool isSrgb = (colorSpace == ColorSpace::sRGB);
	bool isSigned = isVariableTypeSigned(dataType);
	if (format.getPart().High == 0) // IS COMPRESSED FORMAT!
	{
		//VkFormat and type == 0 for compressed textures.
		switch (format.getPixelTypeId())
		{
		//PVRTC

		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB): //fall through
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA): return (isSrgb ? VkFormat::e_PVRTC1_2BPP_SRGB_BLOCK_IMG : VkFormat::e_PVRTC1_2BPP_UNORM_BLOCK_IMG);
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp): return (isSrgb ? VkFormat::e_PVRTC2_2BPP_SRGB_BLOCK_IMG : VkFormat::e_PVRTC2_2BPP_UNORM_BLOCK_IMG);
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp): return (isSrgb ? VkFormat::e_PVRTC2_4BPP_SRGB_BLOCK_IMG : VkFormat::e_PVRTC2_4BPP_UNORM_BLOCK_IMG);

		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB): return isSrgb ? VkFormat::e_PVRTC1_4BPP_SRGB_BLOCK_IMG : VkFormat::e_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA): return isSrgb ? VkFormat::e_PVRTC1_4BPP_SRGB_BLOCK_IMG : VkFormat::e_PVRTC1_4BPP_UNORM_BLOCK_IMG;

		//OTHER COMPRESSED
		case static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5): return VkFormat::e_E5B9G9R9_UFLOAT_PACK32;
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB): return (isSrgb ? VkFormat::e_ETC2_R8G8B8_SRGB_BLOCK : VkFormat::e_ETC2_R8G8B8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA): return (isSrgb ? VkFormat::e_ETC2_R8G8B8A8_SRGB_BLOCK : VkFormat::e_ETC2_R8G8B8A8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1): return (isSrgb ? VkFormat::e_ETC2_R8G8B8A1_SRGB_BLOCK : VkFormat::e_ETC2_R8G8B8A1_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11): return (isSigned ? VkFormat::e_EAC_R11_SNORM_BLOCK : VkFormat::e_EAC_R11_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11): return (isSigned ? VkFormat::e_EAC_R11G11_SNORM_BLOCK : VkFormat::e_EAC_R11G11_UNORM_BLOCK);

		case static_cast<uint64_t>(CompressedPixelFormat::BC2): return (isSrgb ? VkFormat::e_BC2_SRGB_BLOCK : VkFormat::e_BC2_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC3): return (isSrgb ? VkFormat::e_BC3_SRGB_BLOCK : VkFormat::e_BC3_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC4): return (isSigned ? VkFormat::e_BC4_SNORM_BLOCK : VkFormat::e_BC4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC5): return (isSigned ? VkFormat::e_BC5_SNORM_BLOCK : VkFormat::e_BC5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC6): return (isSigned ? VkFormat::e_BC6H_SFLOAT_BLOCK : VkFormat::e_BC6H_UFLOAT_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC7): return (isSrgb ? VkFormat::e_BC7_SRGB_BLOCK : VkFormat::e_BC7_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x10): return (isSrgb ? VkFormat::e_ASTC_10x10_SRGB_BLOCK : VkFormat::e_ASTC_10x10_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x5): return (isSrgb ? VkFormat::e_ASTC_10x5_SRGB_BLOCK : VkFormat::e_ASTC_10x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x6): return (isSrgb ? VkFormat::e_ASTC_10x6_SRGB_BLOCK : VkFormat::e_ASTC_10x6_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x8): return (isSrgb ? VkFormat::e_ASTC_10x8_SRGB_BLOCK : VkFormat::e_ASTC_10x8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x10): return (isSrgb ? VkFormat::e_ASTC_12x10_SRGB_BLOCK : VkFormat::e_ASTC_12x10_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x12): return (isSrgb ? VkFormat::e_ASTC_12x12_SRGB_BLOCK : VkFormat::e_ASTC_12x12_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4): return (isSrgb ? VkFormat::e_ASTC_4x4_SRGB_BLOCK : VkFormat::e_ASTC_4x4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4): return (isSrgb ? VkFormat::e_ASTC_5x4_SRGB_BLOCK : VkFormat::e_ASTC_5x4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5): return (isSrgb ? VkFormat::e_ASTC_5x5_SRGB_BLOCK : VkFormat::e_ASTC_5x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5): return (isSrgb ? VkFormat::e_ASTC_6x5_SRGB_BLOCK : VkFormat::e_ASTC_6x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x5): return (isSrgb ? VkFormat::e_ASTC_8x5_SRGB_BLOCK : VkFormat::e_ASTC_8x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x6): return (isSrgb ? VkFormat::e_ASTC_8x6_SRGB_BLOCK : VkFormat::e_ASTC_8x6_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x8): return (isSrgb ? VkFormat::e_ASTC_8x8_SRGB_BLOCK : VkFormat::e_ASTC_8x8_UNORM_BLOCK);

#define UNSUPPORTED_FORMAT(fmt) case static_cast<uint64_t>(CompressedPixelFormat::fmt): return VkFormat::e_UNDEFINED;

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
			case GeneratePixelType1<'d', 32>::ID: return VkFormat::e_D32_SFLOAT;
			case GeneratePixelType1<'d', 24>::ID:
			case GeneratePixelType2<'x', 8, 'd', 24>::ID:
			case GeneratePixelType2<'d', 24, 'x', 8>::ID: return VkFormat::e_D32_SFLOAT;
			case GeneratePixelType1<'d', 16>::ID: return VkFormat::e_D16_UNORM;
			case GeneratePixelType2<'d', 's', 32, 8>::ID: return VkFormat::e_D32_SFLOAT_S8_UINT;
			case GeneratePixelType2<'d', 's', 24, 8>::ID: return VkFormat::e_D24_UNORM_S8_UINT;
			case GeneratePixelType2<'d', 's', 16, 8>::ID: return VkFormat::e_D16_UNORM_S8_UINT;
			case GeneratePixelType1<'s', 8>::ID: return VkFormat::e_S8_UINT;
			}
		}
		else
		{

			switch (dataType)
			{
			case VariableType::UnsignedFloat:
				if (format.getPixelTypeId() == GeneratePixelType3<'b', 'g', 'r', 10, 11, 11>::ID)
				{
					return VkFormat::e_B10G11R11_UFLOAT_PACK32;
				}
				break;
			case VariableType::SignedFloat:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VkFormat::e_R16G16B16A16_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VkFormat::e_R16G16B16_SFLOAT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VkFormat::e_R16G16_SFLOAT;
				case GeneratePixelType1<'r', 16>::ID: return VkFormat::e_R16_SFLOAT;
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VkFormat::e_R16G16_SFLOAT;
				case GeneratePixelType1<'l', 16>::ID: return VkFormat::e_R16_SFLOAT;
				case GeneratePixelType1<'a', 16>::ID: return VkFormat::e_R16_SFLOAT;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VkFormat::e_R32G32B32A32_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VkFormat::e_R32G32B32_SFLOAT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VkFormat::e_R32G32_SFLOAT;
				case GeneratePixelType1<'r', 32>::ID: return VkFormat::e_R32_SFLOAT;
				case GeneratePixelType2<'l', 'a', 32, 32>::ID: return VkFormat::e_R32G32_SFLOAT;
				case GeneratePixelType1<'l', 32>::ID: return VkFormat::e_R32_SFLOAT;
				case GeneratePixelType1<'a', 32>::ID: return VkFormat::e_R32_SFLOAT;
				}
				break;
			}
			case VariableType::UnsignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VkFormat::e_R8G8B8A8_SRGB : VkFormat::e_R8G8B8A8_UNORM);
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return (isSrgb ? VkFormat::e_R8G8B8_SRGB : VkFormat::e_R8G8B8_UNORM);
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return VkFormat::e_R8G8_UNORM;
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return VkFormat::e_R8_UNORM;
				case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? VkFormat::e_B8G8R8A8_SNORM : VkFormat::e_B8G8R8A8_UNORM);
				case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return VkFormat::e_R4G4B4A4_UNORM_PACK16;
				}
			}
			case VariableType::SignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VkFormat::e_R8G8B8A8_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VkFormat::e_R8G8B8_SNORM;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return VkFormat::e_R8G8B8_SNORM;
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return VkFormat::e_R8_SNORM;
					break;
				}
			}
			case VariableType::UnsignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VkFormat::e_R8G8B8A8_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VkFormat::e_R8G8B8_UINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return VkFormat::e_R8G8_UINT;
				case GeneratePixelType1<'r', 8>::ID: return VkFormat::e_R8_UINT;
				}
			}
			case VariableType::SignedByte:
			{

				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return VkFormat::e_R8G8B8A8_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return VkFormat::e_R8G8B8_SINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return VkFormat::e_R8G8_SINT;
				case GeneratePixelType1<'r', 8>::ID: return VkFormat::e_R8_SINT;
				}
				break;
			}
			case VariableType::UnsignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return VkFormat::e_R4G4B4A4_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID: return VkFormat::e_R5G5B5A1_UNORM_PACK16;
				case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: return VkFormat::e_R5G6B5_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VkFormat::e_R16G16B16A16_UNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VkFormat::e_R16G16B16_UNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VkFormat::e_R16G16_UNORM;
				case GeneratePixelType2<'d', 16, 's', 8>::ID: return VkFormat::e_D16_UNORM_S8_UINT;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID: return VkFormat::e_R16_UNORM;
				}
				break;
			}
			case VariableType::SignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VkFormat::e_R16G16B16A16_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VkFormat::e_R16G16B16_SNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return VkFormat::e_R16G16_SNORM;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID: return VkFormat::e_R16_SNORM;
				}
				break;
			}
			case VariableType::UnsignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VkFormat::e_R16G16B16A16_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VkFormat::e_R16G16B16_UINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VkFormat::e_R16G16_UINT;
				case GeneratePixelType1<'r', 16>::ID: return VkFormat::e_R16_UINT;
				}
				break;
			}
			case VariableType::SignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return VkFormat::e_R16G16B16A16_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return VkFormat::e_R16G16B16_SINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return VkFormat::e_R16G16_SINT;
				case GeneratePixelType1<'r', 16>::ID: return VkFormat::e_R16_SINT;
				}
				break;
			}
			case VariableType::UnsignedIntegerNorm:
			{
				switch (format.getPixelTypeId())
				{
				case  GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				case  GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VkFormat::e_A2B10G10R10_UNORM_PACK32;
				}
				break;
			}
			case VariableType::UnsignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VkFormat::e_R32G32B32A32_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VkFormat::e_R32G32B32_UINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VkFormat::e_R32G32_UINT;
				case GeneratePixelType1<'r', 32>::ID: return VkFormat::e_R32_UINT;
				case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return VkFormat::e_A2B10G10R10_UINT_PACK32;
				}
				break;
			}
			case VariableType::SignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return VkFormat::e_R32G32B32A32_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return VkFormat::e_R32G32B32_SINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return VkFormat::e_R32G32_SINT;
				case GeneratePixelType1<'r', 32>::ID: return VkFormat::e_R32_SINT;
				}
				break;
			}
			default: {}
			}
		}
	}

	return VkFormat::e_UNDEFINED;
}

inline PackedSamplerFilter packSamplerFilter(VkFilter mini, VkFilter magni, VkSamplerMipmapMode mip)
{
	return PackedSamplerFilter((PackedSamplerFilter)mini + ((PackedSamplerFilter)magni << 2) + ((PackedSamplerFilter)mip << 4));
}

inline void unpackSamplerFilter(PackedSamplerFilter packed, VkFilter& mini, VkFilter& magni, VkSamplerMipmapMode& mip)
{
	mini = (VkFilter)(packed & 3);
	magni = (VkFilter)((packed >> 2) & 3);
	mip = (VkSamplerMipmapMode)(packed >> 4);
}

/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Image Data format</param>
/// <returns>A VkFormat representing the pixel format</returns>
inline VkFormat convertToVk(const ImageDataFormat& format)
{
	return convertToVkPixelFormat(format.format, format.colorSpace, format.dataType);
}

/// <summary>Convert to vulkan pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">TYpe of the data (SignedByte, SignedInteger etc)</param>
/// <param name="outIsCompressedFormat">Return if its a compressed format</param>
/// <returns>A VkFormat representing the pixel format</returns>
inline VkFormat convertToVkPixelFormat(
  PixelFormat format, ColorSpace colorSpace,
  VariableType dataType, bool& outIsCompressedFormat)
{
	outIsCompressedFormat = (format.getPart().High == 0) && (format.getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5));
	return convertToVkPixelFormat(format, colorSpace, dataType);
}

inline pvrvk::StencilOpState convertToVk(const StencilState& op)
{
	return pvrvk::StencilOpState(convertToVk(op.opDepthPass), convertToVk(op.opDepthFail),
	                             convertToVk(op.opStencilFail), convertToVk(op.compareOp), op.compareMask, op.writeMask, op.reference);
}

inline pvrvk::PipelineColorBlendAttachmentState convertToVk(const BlendingConfig& config)
{
	return pvrvk::PipelineColorBlendAttachmentState(
	         config.blendEnable,
	         convertToVk(config.srcBlendColor),
	         convertToVk(config.dstBlendColor),
	         convertToVk(config.srcBlendAlpha),
	         convertToVk(config.dstBlendAlpha),
	         convertToVk(config.blendOpColor),
	         convertToVk(config.blendOpAlpha),
	         convertToVk(config.channelWriteMask)
	       );
}

inline pvrvk::VertexInputAttributeDescription convertToVk(const VertexAttributeInfo& info, uint32_t binding)
{
	return pvrvk::VertexInputAttributeDescription(info.index, binding,
	       convertToVkVertexInputFormat(info.format, info.width), info.offsetInBytes);
}

inline pvrvk::VertexInputBindingDescription convertToVk(const VertexInputBindingInfo& info)
{
	return pvrvk::VertexInputBindingDescription(info.bindingId, info.strideInBytes, convertToVk(info.stepRate));
}

inline pvrvk::Extent3D convertToVk(const Extent3D& extent)
{
	return pvrvk::Extent3D{extent.width, extent.height, extent.depth};
}

inline pvrvk::Extent2D convertToVk(const pvrvk::Extent2D& extent)
{
	return pvrvk::Extent2D{extent.width, extent.height};
}

inline pvrvk::Offset3D convertToVk(const Offset3D& extent)
{
	return pvrvk::Offset3D{extent.x, extent.y, extent.z};
}


inline pvrvk::Offset2D convertToVk(const Offset2D& extent)
{
	return pvrvk::Offset2D{extent.x, extent.y};
}


}// namespace api
}//namespace pvr

//!\endcond
