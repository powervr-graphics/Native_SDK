@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat VertShader_vk.vsh VertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat FragShader_vk.fsh FragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat CompShader_vk.csh CompShader_vk.csh.spv comp