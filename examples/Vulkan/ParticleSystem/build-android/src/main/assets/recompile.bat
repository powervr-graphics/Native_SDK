@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V FloorVertShader_vk.vsh -o FloorVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V ParticleFragShader_vk.fsh -o ParticleFragShader_vk.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V ParticleSolver_vk.csh -o ParticleSolver_vk.csh.spv -S comp
call ..\..\..\external\spir-v\glslangValidator.exe -V ParticleVertShader_vk.vsh -o ParticleVertShader_vk.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
