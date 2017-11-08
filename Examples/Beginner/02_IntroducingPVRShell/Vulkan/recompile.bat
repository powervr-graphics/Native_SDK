@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat FragShader_vk.fsh FragShader_vk.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat VertShader_vk.vsh VertShader_vk.spv vert