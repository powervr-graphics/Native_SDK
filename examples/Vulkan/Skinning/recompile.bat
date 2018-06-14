@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V SkinnedVertShader_vk.vsh -o SkinnedVertShader_vk.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V DefaultVertShader_vk.vsh -o DefaultVertShader_vk.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V SkinnedFragShader_vk.fsh -o SkinnedFragShader_vk.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V DefaultFragShader_vk.fsh -o DefaultFragShader_vk.spv -S frag