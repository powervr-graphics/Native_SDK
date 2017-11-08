@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat ColShader_vk.vsh ColShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat ColShader_vk.fsh ColShader_vk.fsh.spv frag

