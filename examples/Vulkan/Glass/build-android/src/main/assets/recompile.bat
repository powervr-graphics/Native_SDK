@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V SkyboxVertShader_vk.vsh -o SkyboxVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V SkyboxFragShader_vk.fsh -o SkyboxFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V DefaultVertShader_vk.vsh -o DefaultVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V DefaultFragShader_vk.fsh -o DefaultFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V ParaboloidVertShader_vk.vsh -o ParaboloidVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectRefractVertShader_vk.vsh -o EffectRefractVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectRefractFragShader_vk.fsh -o EffectRefractFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectVertShader_vk.vsh -o EffectReflectVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectFragShader_vk.fsh -o EffectReflectFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectChromaticDispersion_vk.vsh -o EffectChromaticDispersion_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectChromaticDispersion_vk.fsh -o EffectChromaticDispersion_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectionRefraction_vk.vsh -o EffectReflectionRefraction_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectionRefraction_vk.fsh -o EffectReflectionRefraction_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectChromDispersion_vk.vsh -o EffectReflectChromDispersion_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V EffectReflectChromDispersion_vk.fsh -o EffectReflectChromDispersion_vk.fsh.spv -S frag
