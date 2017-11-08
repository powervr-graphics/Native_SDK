/*!
\brief The PVRVk Sampler class.
\file PVRVk/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {


/// <summary>Object describing the state of a sampler. Also used to construct a full-blown Sampler object.</summary>
struct SamplerCreateInfo
{
	/// <summary>Default constructor.</summary>
	/// <summary>Creates a Sampler with following configs. Can be edited after construction magnification filter: nearest,
	/// minifictation filter: nearest, mipmap filter: linear,wrap UVW: Repeat no comparison, no lod-bias, no
	/// anisotropy.</summary>
	SamplerCreateInfo() :
		magFilter(VkFilter::e_LINEAR), minFilter(VkFilter::e_NEAREST),
		mipMapMode(VkSamplerMipmapMode::e_LINEAR), wrapModeU(VkSamplerAddressMode::e_REPEAT),
		wrapModeV(VkSamplerAddressMode::e_REPEAT), wrapModeW(VkSamplerAddressMode::e_REPEAT),
		compareOp(VkCompareOp::e_NEVER), compareOpEnable(false),
		enableAnisotropy(false), anisotropyMaximum(1.0f), lodBias(0.0f), lodMinimum(0.0f),
		lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(VkBorderColor::e_FLOAT_TRANSPARENT_BLACK)
	{}

	/// <summary>Constructor that sets the filters. Set the filtering mode explicitly. Wrap. Creates a Sampler with
	/// wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.</summary>
	/// <param name="miniFilter">Minification filter: Nearest or Linear</param>
	/// <param name="magniFilter">Magnification filter: Nearest or Linear</param>
	/// <param name="mipMapFilter">Mipmap interpolation filter: Nearest or Linear</param>
	/// <param name="wrapModeU">The wrapping mode for the U coordinate</param>
	/// <param name="wrapModeV">The wrapping mode for the V coordinate</param>
	/// <param name="wrapModeW">The wrapping mode for the W coordinate</param>
	SamplerCreateInfo(
	  VkFilter magniFilter,
	  VkFilter  miniFilter,
	  VkSamplerMipmapMode mipMapFilter = VkSamplerMipmapMode::e_LINEAR,
	  VkSamplerAddressMode wrapModeU = VkSamplerAddressMode::e_REPEAT,
	  VkSamplerAddressMode wrapModeV = VkSamplerAddressMode::e_REPEAT,
	  VkSamplerAddressMode wrapModeW = VkSamplerAddressMode::e_REPEAT) :
		magFilter(magniFilter), minFilter(miniFilter),
		mipMapMode(mipMapFilter), wrapModeU(wrapModeU), wrapModeV(wrapModeV),
		wrapModeW(wrapModeW), compareOp(VkCompareOp::e_NEVER), compareOpEnable(false),
		enableAnisotropy(false), anisotropyMaximum(1.0f),
		lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(VkBorderColor::e_FLOAT_TRANSPARENT_BLACK)
	{}

	VkFilter        magFilter; //!< Texture Magnification interpolation filter. Nearest or Linear. Default Nearest.
	VkFilter        minFilter; //!< Texture Minification interpolation filter. Nearest or Linear. Default Nearest.
	VkSamplerMipmapMode    mipMapMode; //!< Texture Mipmap interpolation filter. Nearest, Linear or None (for no mipmaps). Default Linear.
	VkSamplerAddressMode   wrapModeU; //!< Texture Wrap mode, U (x) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	VkSamplerAddressMode   wrapModeV; //!< Texture Wrap mode, V (y) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	VkSamplerAddressMode   wrapModeW; //!< Texture Wrap mode, W (z) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	VkCompareOp            compareOp; //!< Comparison mode for comparison samplers. Default None.
	bool                        compareOpEnable; //!< Enable the compare op
	bool                        enableAnisotropy; //!< Enable anisotropic filtering
	float                     anisotropyMaximum; //!< Texture anisotropic filtering. Default 0.
	float                     lodBias;           //!< Texture level-of-detail bias (bias of mipmap to select). Default 0.
	float                     lodMinimum;        //!< Texture minimum level-of-detail (mipmap). Default 0.
	float                     lodMaximum;        //!< Texture maximum level-of-detail (mipmap). Default 0.
	bool                        unnormalizedCoordinates; //!< Texture Coordinates are Un-Normalized
	VkBorderColor          borderColor;          //!< In case of a border address mode, the border color
};

namespace impl {
/// <summary>SamplerVk_ implementation that wraps the vulkan sampler</summary>
class Sampler_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Sampler_)

	/// <summary>Get sampler create info (const)</summary>
	/// <returns>SamplerCreateInfo&</returns>
	const SamplerCreateInfo& getCreateInfo()const
	{
		return _createInfo;
	}

	/// <summary>Get vulkan object</summary>
	/// <returns>VkSampler</returns>
	const VkSampler& getNativeObject()const
	{
		return _vkSampler;
	}

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Sampler_(const DeviceWeakPtr& device) :
		_device(device), _vkSampler(VK_NULL_HANDLE) {}

	bool init(const pvrvk::SamplerCreateInfo& desc);

	/// <summary>destructor</summary>
	~Sampler_() { destroy(); }

	void destroy()
	{
		if (_vkSampler != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				vk::DestroySampler(_device->getNativeObject(), _vkSampler, nullptr);
				_vkSampler = VK_NULL_HANDLE;
				_device.reset();
			}
			else
			{
				reportDestroyedAfterContext("Sampler");
			}
		}
	}

	DeviceWeakPtr _device;
	VkSampler _vkSampler;
	SamplerCreateInfo _createInfo;
};
}
}
