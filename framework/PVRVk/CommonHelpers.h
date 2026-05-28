/*!
\brief Contains generic functions used in multiple places through PVRVk
\file PVRVk/TypesVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include <vector>

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <aclapi.h>
#include <dxgi1_2.h>
#endif

namespace pvrvk {

/// <summary>Sets an element in a vector at a specified index. If the index is outside the allocated size of the vector,
/// create new elements using their default constructor to fill the vector with up to the specified index.</summary>
/// <typeparam name="T">Type of element to be inserted to the vector.</typeparam>
/// <param name="index">Index to insert newElement into.</param>
/// <param name="newElement">The new value/element to be inserted to the vector.</param>
/// <param name="elements">The vector you want to insert the new element into.</param>
template<typename T>
static void setElementAtIndex(const uint32_t index, const T& newElement, std::vector<T>& elements)
{
	size_t numElements = elements.size();
	if (index > numElements)
	{
		elements.reserve(index + 1u); // so we don't do 2 dynamic allocations
		elements.resize(index); // don't need to initialize elements[index], assumes you want default initializer
	}
	if (index >= numElements)
	{ elements.emplace_back(newElement); }
	else
	{ elements[index] = newElement; }
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
/// <summary>Class required when building certain Vulkan objects which can be shared with other APIs, like OpenCL.</summary>
class WindowsExportMemorySecurity
{
public:
	void init()
	{
		securityDescriptor = (PSECURITY_DESCRIPTOR*)calloc(1, SECURITY_DESCRIPTOR_MIN_LENGTH + 2 * sizeof(void**));
		if (InitializeSecurityDescriptor(securityDescriptor, SECURITY_DESCRIPTOR_REVISION) == 0)
		{
			// handle error
		}

		sid = (PSID*)((PBYTE)securityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);
		SID_IDENTIFIER_AUTHORITY sid_identifier = SECURITY_WORLD_SID_AUTHORITY;
		if (AllocateAndInitializeSid(&sid_identifier, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, sid) == 0)
		{
			// handle error
		}

		EXPLICIT_ACCESS explicitAccess;
		memset((void*)&explicitAccess, 0, sizeof(explicitAccess));
		explicitAccess.grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
		explicitAccess.grfAccessMode = SET_ACCESS;
		explicitAccess.grfInheritance = INHERIT_ONLY;
		explicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		explicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		explicitAccess.Trustee.ptstrName = (LPTSTR)*sid;

		acl = (PACL*)((PBYTE)sid + sizeof(PSID*));
		if (SetEntriesInAcl(1, &explicitAccess, nullptr, acl) != ERROR_SUCCESS)
		{
			// handle error
		}
		if (SetSecurityDescriptorDacl(securityDescriptor, TRUE, *acl, FALSE) == 0)
		{
			// handle error
		}

		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		securityAttributes.lpSecurityDescriptor = securityDescriptor;
		securityAttributes.bInheritHandle = TRUE;
	}

	~WindowsExportMemorySecurity()
	{
		free(securityDescriptor);
	}

	const SECURITY_ATTRIBUTES* getSecurityAttributes() const { return &securityAttributes; }

protected:
	PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
	PSID* sid = nullptr;
	PACL* acl = nullptr;
	SECURITY_ATTRIBUTES securityAttributes = {};
};
#endif

} // namespace pvrvk