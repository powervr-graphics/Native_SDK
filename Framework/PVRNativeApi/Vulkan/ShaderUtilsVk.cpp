#include "PVRNativeApi/ShaderUtils.h"
#include "PVRCore/IGraphicsContext.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
namespace pvr {
namespace utils {

bool loadShader(const native::HContext_& context, Stream& shaderData, types::ShaderType shaderType, types::ShaderBinaryFormat binaryFormat, native::HShader_& outShader, const ApiCapabilities* contextCapabilities)
{
	assertion(false, "Not implemented yet");
	Log(Log.Error, "BinaryFormat Not supported");
	return false;
}


bool loadShader(const native::HContext_& context, const Stream& shaderSource, types::ShaderType shaderType, const char* const* defines, uint32 numDefines, native::HShader_& outShader, const ApiCapabilities* apiCapabilities)
{
	if (!shaderSource.isopen() && !shaderSource.open())
	{
		Log(Log.Error, "loadShader: Could not open the shaderSource stream");
		return false;
	}
	if (outShader.handle)
	{
		pvr::Log(Log.Warning,
		         "loadShader: Generated shader passed to loadShader. Deleting reference to avoid leaking a preexisting shader object.");
		vk::DestroyShaderModule(context.device, outShader.handle, NULL);
		outShader.handle = 0;
	}

	std::vector<uint32> shaderSrc;
	shaderSrc.resize(shaderSource.getSize());
	shaderSrc = shaderSource.readToEnd<uint32>();
	if (shaderSrc.empty()) { return false; }
	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = NULL;
	shaderModuleCreateInfo.codeSize = shaderSrc.size();
	shaderModuleCreateInfo.pCode = shaderSrc.data();
	shaderModuleCreateInfo.flags = 0;
	return pvr::vkIsSuccessful(vk::CreateShaderModule(context.device, &shaderModuleCreateInfo, NULL, &outShader.handle), "Shader Creation Failed") ? true : false;
}

void destroyShader(const native::HContext_& context, native::HShader_& shader)
{
	vk::DestroyShaderModule(context.device, shader.handle, NULL);
}

}
}
