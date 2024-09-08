/*!
\brief Functionality that helps management of Vulkan extensions, such as
enumerating, enabling/disabling
\file PVRVk/ExtensionsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include "PVRVk/TypesVk.h"
#include "PVRVk/pvrvk_vulkan_wrapper.h"

namespace pvrvk {
namespace Extensions {

/// <summary>Filter the extensions</summary>
/// <param name="extensionProperties">Extension properties</param>
/// <param name="extensionsToEnable">Extensions to enable</param>
/// <returns>VulkanExtensionList</returns>
extern VulkanExtensionList filterExtensions(const std::vector<pvrvk::ExtensionProperties>& extensionProperties, const VulkanExtensionList& extensionsToEnable);

/// <summary>Get list of all supported instance extension properties</summary>
/// <param name="outExtensions">Returned extensions</param>
void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions);

/// <summary>Get list of all supported instance extension properties for a given layer</summary>
/// <param name="outExtensions">Returned extensions</param>
/// <param name="layerName">Layer from which to retrieve supported extensions</param>
void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions, const std::string& layerName);

/// <summary>Query if an Instance Extension is supported</summary>
/// <param name="extension">The extension string</param>
/// <returns>True if the instance supports the extension, otherwise false</param>
bool isInstanceExtensionSupported(const std::string& extension);
} // namespace Extensions

/// <summary>Defines a fragment shading rate as a fragment size and a bitmask of the MSAA sample counts that can be used with that fragment size.
/// Use pvrvk::PhysicalDevice::getAvailableFragmentShadingRates() to get the available fragment shading rates for a particular physical devices</summary>
class FragmentShadingRate : private VkPhysicalDeviceFragmentShadingRateKHR
{
public:
	FragmentShadingRate()
	{
		sampleCounts = static_cast<VkSampleCountFlags>(SampleCountFlags::e_NONE);
		fragmentSize = Extent2D().get();
	}
	FragmentShadingRate(VkPhysicalDeviceFragmentShadingRateKHR vkType) : VkPhysicalDeviceFragmentShadingRateKHR(vkType) {}
	FragmentShadingRate(SampleCountFlags sampleCounts, Extent2D fragmentSize)
	{
		this->sampleCounts = static_cast<VkSampleCountFlags>(sampleCounts);
		this->fragmentSize = fragmentSize.get();
	}

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>VkPhysicalDeviceFragmentShadingRateKHR struct.</returns>
	inline VkPhysicalDeviceFragmentShadingRateKHR& get() { return *this; }

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>Const VkPhysicalDeviceFragmentShadingRateKHR struct.</returns>
	inline const VkPhysicalDeviceFragmentShadingRateKHR& get() const { return *this; }

	/// <summary>Get the [x,y] fragment size that can be used for fragment shading rate functionality.</summary>
	/// <returns>[x,y] integers corresponding to a fragment size.</returns>
	inline Extent2D getFragmentSize() const { return static_cast<Extent2D>(fragmentSize); }

	/// <summary>Get a bitmask of the possible MSAA sample counts that can be used with the associated fragment size.</summary>
	/// <returns>Bitmask of sample counts.</returns>
	inline SampleCountFlags getSampleCount() const { return static_cast<SampleCountFlags>(sampleCounts); }
};

/// <summary>Set of fragment shading rate properties for a physical device</summary>
class FragmentShadingRateProperties : public VkPhysicalDeviceFragmentShadingRatePropertiesKHR
{
public:
	FragmentShadingRateProperties(void* pNext = nullptr)
	{
		sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR);
		this->pNext = pNext;
		minFragmentShadingRateAttachmentTexelSize = VkExtent2D();
		maxFragmentShadingRateAttachmentTexelSize = VkExtent2D();
		maxFragmentShadingRateAttachmentTexelSizeAspectRatio = 0;
		primitiveFragmentShadingRateWithMultipleViewports = (VkBool32) false;
		layeredShadingRateAttachments = (VkBool32) false;
		fragmentShadingRateNonTrivialCombinerOps = (VkBool32) false;
		maxFragmentSize = VkExtent2D();
		maxFragmentSizeAspectRatio = 0;
		maxFragmentShadingRateCoverageSamples = 0;
		maxFragmentShadingRateRasterizationSamples = VkSampleCountFlagBits();
		fragmentShadingRateWithShaderDepthStencilWrites = (VkBool32) false;
		fragmentShadingRateWithSampleMask = (VkBool32) false;
		fragmentShadingRateWithShaderSampleMask = (VkBool32) false;
		fragmentShadingRateWithConservativeRasterization = (VkBool32) false;
		fragmentShadingRateWithFragmentShaderInterlock = (VkBool32) false;
		fragmentShadingRateWithCustomSampleLocations = (VkBool32) false;
		fragmentShadingRateStrictMultiplyCombiner = (VkBool32) false;
	}
	FragmentShadingRateProperties(VkPhysicalDeviceFragmentShadingRatePropertiesKHR vkType) : VkPhysicalDeviceFragmentShadingRatePropertiesKHR(vkType) {}

	/// <summary>Get the location of the vulkan physical device features struct</summary>
	/// <returns>Pointer to the beginning of the vulkan struct data (the sType member)</returns>
	inline VkPhysicalDeviceFragmentShadingRatePropertiesKHR* getVkPtr()
	{
		return (VkPhysicalDeviceFragmentShadingRatePropertiesKHR*)((char*)this + offsetof(FragmentShadingRateProperties, sType));
	}

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>VkPhysicalDeviceFragmentShadingRatePropertiesKHR struct.</returns>
	inline VkPhysicalDeviceFragmentShadingRatePropertiesKHR& get() { return *this; }

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>Const VkPhysicalDeviceFragmentShadingRatePropertiesKHR struct.</returns>
	inline const VkPhysicalDeviceFragmentShadingRatePropertiesKHR& get() const { return *this; }

	/// <summary>Get pNext</summary>
	/// <returns>pNext pointer</returns>
	inline void* getPNext() const { return pNext; }

	/// <summary>Set pNext</summary>
	/// <param name="pNext">pNext pointer</param>
	FragmentShadingRateProperties& setPNext(void* pNextPointer)
	{
		this->pNext = pNextPointer;
		return *this;
	}
};

/// <summary>Base class for physical device extension features abstractions</summary>
class ExtensionFeatures
{
public:
	/// <summary>Get the location of the vulkan physical device features struct</summary>
	/// <returns>Pointer to the beginning of the vulkan struct data (the sType member)</returns>
	virtual inline void* getVkPtr() = 0;

	/// <summary>Get sType</summary>
	/// <returns>Vulkan struct type</returns>
	virtual inline StructureType getSType() const = 0;

	/// <summary>Get pNext</summary>
	/// <returns>pNext pointer</returns>
	virtual inline void* getPNext() const = 0;

	/// <summary>Set pNext</summary>
	/// <param name="pNext">pNext pointer</param>
	virtual ExtensionFeatures& setPNext(void* pNext) = 0;
};

/// <summary>List of supported fragment shading rate features for a physical device</summary>
class FragmentShadingRateFeatures : private VkPhysicalDeviceFragmentShadingRateFeaturesKHR, public ExtensionFeatures
{
public:
	FragmentShadingRateFeatures(void* pNext = nullptr)
	{
		sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR);
		this->pNext = pNext;
		pipelineFragmentShadingRate = (VkBool32) false;
		primitiveFragmentShadingRate = (VkBool32) false;
		attachmentFragmentShadingRate = (VkBool32) false;
	}

	FragmentShadingRateFeatures(VkPhysicalDeviceFragmentShadingRateFeaturesKHR vkType) : VkPhysicalDeviceFragmentShadingRateFeaturesKHR(vkType) {}

	/// <summary>Get the location of the vulkan physical device features struct</summary>
	/// <returns>Pointer to the beginning of the vulkan struct data (the sType member)</returns>
	inline void* getVkPtr() {
	    VkPhysicalDeviceFragmentShadingRateFeaturesKHR* vkFragmentShadingRateFeatures = this;
	    return vkFragmentShadingRateFeatures;
	}

	/// <summary>Get sType</summary>
	/// <returns>Vulkan struct type</returns>
	inline StructureType getSType() const { return static_cast<StructureType>(sType); }

	/// <summary>Get pNext</summary>
	/// <returns>pNext pointer</returns>
	inline void* getPNext() const { return pNext; }

	/// <summary>Set pNext</summary>
	/// <param name="pNext">pNext pointer</param>
	ExtensionFeatures& setPNext(void* pNextPointer)
	{
		this->pNext = pNextPointer;
		return *this;
	}

	/// <summary>Set pipeline fsr feature</summary>
	/// <param name"pipelineFragmentShadingRate">boolean state</param>
	inline void setPipelineFeature(bool inPipelineFragmentShadingRate) { this->pipelineFragmentShadingRate = inPipelineFragmentShadingRate; }

	/// <summary>Set primitive fsr feature</summary>
	/// <param name"primitiveFragmentShadingRate">boolean state</param>
	inline void setPrimitiveFeature(bool inPrimitiveFragmentShadingRate) { this->primitiveFragmentShadingRate = inPrimitiveFragmentShadingRate; }

	/// <summary>Set attachment fsr feature</summary>
	/// <param name"attachmentFragmentShadingRate">boolean state</param>
	inline void setAttachmentFeature(bool inAttachmentFragmentShadingRate) { this->attachmentFragmentShadingRate = inAttachmentFragmentShadingRate; }

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>VkPhysicalDeviceFragmentShadingRateFeaturesKHR struct</returns>
	inline VkPhysicalDeviceFragmentShadingRateFeaturesKHR& get() { return *this; }
	
	/// <summary>If members have been populated, returns wherever the pipeline fragment shading rate feature is enabled</summary>
	/// <returns>Bool is true if feature is enabled</returns>
	inline bool getPipelineFeature() const { return (bool)pipelineFragmentShadingRate; }

	/// <summary>If members have been populated, returns wherever the primitive fragment shading rate feature is enabled</summary>
	/// <returns>Bool is true if feature is enabled</returns>
	inline bool getPrimitiveFeature() const { return (bool)primitiveFragmentShadingRate; }

	/// <summary>If members have been populated, returns wherever the attachment fragment shading rate feature is enabled</summary>
	/// <returns>Bool is true if feature is enabled</returns>
	inline bool getAttachmentFeature() const { return (bool)attachmentFragmentShadingRate; }
};

/// <summary>List of supported ray tracing features for a physical device</summary>
class RayTracingPipelineFeatures : private VkPhysicalDeviceRayTracingPipelineFeaturesKHR, public ExtensionFeatures
{
public:
	RayTracingPipelineFeatures(void* pNext = nullptr)
	{
		sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
		this->pNext = pNext;
		rayTracingPipeline = (VkBool32)false;
		rayTracingPipelineShaderGroupHandleCaptureReplay = (VkBool32)false;
		rayTracingPipelineShaderGroupHandleCaptureReplayMixed = (VkBool32)false;
		rayTracingPipelineTraceRaysIndirect = (VkBool32)false;
		rayTraversalPrimitiveCulling = (VkBool32)false;
	}
	RayTracingPipelineFeatures(VkPhysicalDeviceRayTracingPipelineFeaturesKHR vkType) : VkPhysicalDeviceRayTracingPipelineFeaturesKHR(vkType) {}

	/// <summary>Get the location of the vulkan physical device features struct</summary>
	/// <returns>Pointer to the beginning of the vulkan struct data (the sType member)</returns>
	inline void* getVkPtr() {
	    VkPhysicalDeviceRayTracingPipelineFeaturesKHR* vkRTPipelineFeatures = this;
	    return vkRTPipelineFeatures;
	}

	/// <summary>Get sType</summary>
	/// <returns>Vulkan struct type</returns>
	inline StructureType getSType() const { return static_cast<StructureType>(sType); }

	/// <summary>Get pNext</summary>
	/// <returns>pNext pointer</returns>
	inline void* getPNext() const { return pNext; }

	/// <summary>Set pNext</summary>
	/// <param name="pNext">pNext pointer</param>
	ExtensionFeatures& setPNext(void* pNextPointer)
	{
		this->pNext = pNextPointer;
		return *this;
	}

	/// <summary>Get the vulkan struct equivilent of this class.</summary>
	/// <returns>VkPhysicalDeviceFragmentShadingRateFeaturesKHR struct</returns>
	inline VkPhysicalDeviceRayTracingPipelineFeaturesKHR& get() { return *this; }

	/// <summary>Get boolean value indicating ray tracing pipeline support</summary>
	/// <returns>Indicates whether the implementation supports the ray tracing pipeline functionality</returns>
	inline bool getRayTracingPipeline() { return static_cast<bool>(rayTracingPipeline); }
	/// <summary>Get boolean value indicating ray tracing pipeline shader group handle capture replay support</summary>
	/// <returns>Indicates whether the implementation supports saving and reusing shader group handles</returns>
	inline bool getRayTracingPipelineShaderGroupHandleCaptureReplay() { return static_cast<bool>(rayTracingPipelineShaderGroupHandleCaptureReplay); }
	/// <summary>Get boolean value indicating ray tracing pipeline shader group handle capture replay mixed support</summary>
	/// <returns>Indicates whether the implementation supports reuse of shader group handles being arbitrarily mixed with creation of non-reused shader group handles.
	/// If this is VK_FALSE, all reused shader group handles must be specified before any non-reused handles may be created.</returns>
	inline bool getRayTracingPipelineShaderGroupHandleCaptureReplayMixed() { return static_cast<bool>(rayTracingPipelineShaderGroupHandleCaptureReplayMixed); }
	/// <summary>Get boolean value indicating ray tracing pipeline indirect trace rays support</summary>
	/// <returns>Indicates whether the implementation supports indirect trace ray commands e.g. vkCmdTraceRaysIndirect</returns>
	inline bool getRayTracingPipelineTraceRaysIndirect() { return static_cast<bool>(rayTracingPipelineTraceRaysIndirect); }
	/// <summary>Get boolean value indicating ray traversal primitive culling support</summary>
	/// <returns>Indicates whether the implementation supports primitive culling during ray traversal</returns>
	inline bool getRayTraversalPrimitiveCulling() { return static_cast<bool>(rayTraversalPrimitiveCulling); }
};

} // namespace pvrvk
