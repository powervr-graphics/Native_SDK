@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat SkinnedVertShader_vk.vsh SkinnedVertShader_vk.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat DefaultVertShader_vk.vsh DefaultVertShader_vk.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat SkinnedFragShader_vk.fsh SkinnedFragShader_vk.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat DefaultFragShader_vk.fsh DefaultFragShader_vk.spv frag