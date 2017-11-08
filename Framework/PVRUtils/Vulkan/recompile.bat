@echo off
call ..\..\Builds\Spir-V\compile.bat UIRendererShader_vk.vsh UIRendererShader_vk.vsh.spv vert
call ..\..\Builds\Spir-V\compile.bat UIRendererShader_vk.fsh UIRendererShader_vk.fsh.spv frag
