@echo off
call ..\..\..\external\spir-v\glslangValidator.exe VertShader_vk.vsh VertShader_vk.spv vert
call ..\..\..\external\spir-v\glslangValidator.exe FragShader_vk.fsh FragShader_vk.spv frag