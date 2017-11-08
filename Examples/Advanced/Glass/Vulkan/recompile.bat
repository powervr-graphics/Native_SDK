@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat SkyboxVertShader_vk.vsh SkyboxVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat SkyboxFragShader_vk.fsh SkyboxFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat DefaultVertShader_vk.vsh DefaultVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat DefaultFragShader_vk.fsh DefaultFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat ParaboloidVertShader_vk.vsh ParaboloidVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectRefractVertShader_vk.vsh EffectRefractVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectRefractFragShader_vk.fsh EffectRefractFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectVertShader_vk.vsh EffectReflectVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectFragShader_vk.fsh EffectReflectFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat EffectChromaticDispersion_vk.vsh EffectChromaticDispersion_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectChromaticDispersion_vk.fsh EffectChromaticDispersion_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectionRefraction_vk.vsh EffectReflectionRefraction_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectionRefraction_vk.fsh EffectReflectionRefraction_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectChromDispersion_vk.vsh EffectReflectChromDispersion_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat EffectReflectChromDispersion_vk.fsh EffectReflectChromDispersion_vk.fsh.spv frag
