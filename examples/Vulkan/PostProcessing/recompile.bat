@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V BlurVertShader_vk.vsh -o BlurVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V BlurFragShader_vk.fsh -o BlurFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V PostBloomVertShader_vk.vsh -o PostBloomVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V PreBloomVertShader_vk.vsh -o PreBloomVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V PostBloomFragShader_vk.fsh -o PostBloomFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V PreBloomFragShader_vk.fsh -o PreBloomFragShader_vk.fsh.spv -S frag
