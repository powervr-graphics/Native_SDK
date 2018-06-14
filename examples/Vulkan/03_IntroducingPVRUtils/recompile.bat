@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V VertShader_vk.vsh -o VertShader_vk.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V FragShader_vk.fsh -o FragShader_vk.spv -S frag