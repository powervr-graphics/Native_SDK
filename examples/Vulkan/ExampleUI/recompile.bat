@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V ColShader_vk.vsh -o ColShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V ColShader_vk.fsh -o ColShader_vk.fsh.spv -S frag

