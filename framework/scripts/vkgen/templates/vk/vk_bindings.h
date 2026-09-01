// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_bindings_generator.py for modifications

/*
\brief Vk Vulkan function pointers.
\file vk_bindings.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/* Corresponding to Vulkan registry file version #{{header_version}}# */

#pragma once
#ifndef VK_PROTOTYPES
#define VK_NO_PROTOTYPES 1
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
#include "MoltenVK/vk_mvk_moltenvk.h"
#else
#include "vulkan/vulkan.h"
#endif

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);

// Vulkan function pointer table
typedef struct VkBindings_
{
	// ---- Before using Vulkan, an application must initialize it by loading the Vulkan commands, and creating a VkInstance object.

	// ---- This function table provides the functions necessary for achieving this.
{%for func in functions | select("is_special_function") %}
	PFN_{{ func.name }} {{ func.name }};
{%endfor%}

// ---- Functions required for MoltenVK MVKConfiguration functionality.
#ifdef VK_USE_PLATFORM_MACOS_MVK
	PFN_vkSetMoltenVKConfigurationMVK vkSetMoltenVKConfigurationMVK;
	PFN_vkGetMoltenVKConfigurationMVK vkGetMoltenVKConfigurationMVK;
#endif

} VkBindings;

{%set all_groups = functions|unsorted_groupby("groups") %}

// Instance function pointers
typedef struct VkInstanceBindings_ {
	// Manually add in vkGetPhysicalDeviceProcAddr entry
	PFN_GetPhysicalDeviceProcAddr vkGetPhysicalDeviceProcAddr;

{%for groups in all_groups %}
	{% set funclist = groups.list | select("is_instance_function")|list%}
	{% if funclist|length %}
#if (defined({{ groups[0]|map(attribute="name")|join(") || defined(") }}))
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#if (defined({{ groups[0]|map(attribute="ifdef")|join(") || defined(") }}))
		{% endif %}
			{%for func in funclist %}
	PFN_{{ func.name }} {{ func.name }};
			{%endfor%}
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#endif // ({{ groups[0]|map(attribute="ifdef")|join(",") }})
		{% endif %}
#endif // {{ groups[0]|map(attribute="name")|join(",") }}

	{%endif%}
{%endfor%}


} VkInstanceBindings;
// Device function pointers
typedef struct VkDeviceBindings_ {

{%for groups in all_groups %}
	{% set funclist = groups.list | select("is_device_function")|list%}
	{% if funclist|length %}
#if (defined({{ groups[0]|map(attribute="name")|join(") || defined(") }}))
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#if (defined({{ groups[0]|map(attribute="ifdef")|join(") || defined(") }}))
		{% endif %}
			{%for func in funclist %}
	PFN_{{ func.name }} {{ func.name }};
			{%endfor%}
		{% if groups[0]|map(attribute="ifdef")|select("string")|list|length %}
#endif // ({{ groups[0]|map(attribute="ifdef")|join(",") }})
		{% endif %}
#endif // {{ groups[0]|map(attribute="name")|join(",") }}

	{%endif%}
{%endfor%}
} VkDeviceBindings;

