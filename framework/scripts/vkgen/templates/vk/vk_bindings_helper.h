// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_bindings_helper_generator.py for modifications

/*
\brief Helper functions for filling vk bindings function pointer tables.
\file vk_bindings_helper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/* Corresponding to Vulkan registry file version #{{header_version}}# */

#pragma once
#include <string>
#include <cstring>
#include "pvr_openlib.h"
#include "vk_bindings.h"

namespace vk {
namespace internal {
/** DEFINE THE PLATFORM SPECIFIC LIBRARY NAME **/
#ifdef _WIN32
static const char* libName = "vulkan-1.dll";
#elif defined(TARGET_OS_MAC)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
static const char* libName = "libMoltenVK.dylib";
#else
static const char* libName = "libvulkan.dylib";
#endif
#else
static const char* libName = "libvulkan.so.1;libvulkan.so";
#endif
}
}

static inline bool initVkBindings(VkBindings *bindings)
{
	pvr::lib::LIBTYPE lib = pvr::lib::openlib(vk::internal::libName);

	memset(bindings, 0, sizeof(*bindings));

	// Load the function pointer dynamically for vkGetInstanceProcAddr
	bindings->vkGetInstanceProcAddr = pvr::lib::getLibFunctionChecked<PFN_vkGetInstanceProcAddr>(lib, "vkGetInstanceProcAddr");

	// Use vkGetInstanceProcAddr with a NULL instance to retrieve the function pointers
	bindings->vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
	bindings->vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
	bindings->vkCreateInstance = (PFN_vkCreateInstance)bindings->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
	bindings->vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");

	// Get bindings for MVKConfiguration functions when using MoltenVK.
#ifdef VK_USE_PLATFORM_MACOS_MVK
	bindings->vkGetMoltenVKConfigurationMVK = pvr::lib::getLibFunctionChecked<PFN_vkGetMoltenVKConfigurationMVK>(lib, "vkGetMoltenVKConfigurationMVK");
	bindings->vkSetMoltenVKConfigurationMVK = pvr::lib::getLibFunctionChecked<PFN_vkSetMoltenVKConfigurationMVK>(lib, "vkSetMoltenVKConfigurationMVK");
#endif

	// validate that we have all necessary function pointers to continue
	if (!bindings->vkEnumerateInstanceExtensionProperties || !bindings->vkEnumerateInstanceLayerProperties || !bindings->vkCreateInstance)
	{
		return false;
	}
	return true;
}

{%set all_groups = functions|unsorted_groupby("groups") %}

static inline void initVkInstanceBindings(VkInstance instance, VkInstanceBindings *bindings, PFN_vkGetInstanceProcAddr getInstanceProcAddr) {
	memset(bindings, 0, sizeof(*bindings));
	// Instance function pointers

{%for groups in all_groups %}
	{% set funclist = groups.list | select("is_instance_function")|list %}
	{% if funclist|length %}
#if (defined({{ groups[0]|map(attribute="name")|join(") || defined(") }}))
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#if (defined({{ groups[0]|map(attribute="ifdef")|join(") || defined(") }}))
		{% endif %}
			{%for func in funclist %}
	bindings->{{func.name}} = (PFN_{{func.name}})getInstanceProcAddr(instance, "{{func.name}}");
			{%endfor%}
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#endif // ({{ groups[0]|map(attribute="ifdef")|join(",") }})
		{% endif %}
#endif // {{ groups[0]|map(attribute="name")|join(",") }}

	{%endif%}
{%endfor%}
}

static inline void initVkDeviceBindings(VkDevice device, VkDeviceBindings *bindings, PFN_vkGetDeviceProcAddr getDeviceProcAddr) {
	memset(bindings, 0, sizeof(*bindings));
	// Device function pointers

{%for groups in all_groups %}
	{% set funclist = groups.list | select("is_device_function")|list %}
	{% if funclist|length %}
#if (defined({{ groups[0]|map(attribute="name")|join(") || defined(") }}))
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#if (defined({{ groups[0]|map(attribute="ifdef")|join(") || defined(") }}))
		{% endif %}
			{%for func in funclist %}
	bindings->{{func.name}} = (PFN_{{func.name}})getDeviceProcAddr(device, "{{func.name}}");
			{%endfor%}
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#endif // ({{ groups[0]|map(attribute="ifdef")|join(",") }})
		{% endif %}
#endif // {{ groups[0]|map(attribute="name")|join(",") }}

	{%endif%}
{%endfor%}
}
