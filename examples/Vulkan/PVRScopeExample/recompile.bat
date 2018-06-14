@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V GraphVertShader_vk.vsh -o GraphVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V GraphFragShader_vk.fsh -o GraphFragShader_vk.fsh.spv -S frag