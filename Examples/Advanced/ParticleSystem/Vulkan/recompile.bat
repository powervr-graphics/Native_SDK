@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat FloorVertShader_vk.vsh FloorVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat FragShader_vk.fsh FragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat ParticleFragShader_vk.fsh ParticleFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat ParticleSolver_vk.csh ParticleSolver_vk.csh.spv comp
call ..\..\..\..\Builds\Spir-V\compile.bat ParticleVertShader_vk.vsh ParticleVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat VertShader_vk.vsh VertShader_vk.vsh.spv vert
