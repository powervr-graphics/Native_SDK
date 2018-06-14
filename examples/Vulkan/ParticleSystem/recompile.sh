#!/bin/bash
../../../external/spir-v/glslangValidator -V FloorVertShader_vk.vsh -o FloorVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag
../../../external/spir-v/glslangValidator -V ParticleFragShader_vk.fsh -o ParticleFragShader_vk.fsh.spv -S frag
../../../external/spir-v/glslangValidator -V ParticleSolver_vk.csh -o ParticleSolver_vk.csh.spv -S comp
../../../external/spir-v/glslangValidator -V ParticleVertShader_vk.vsh -o ParticleVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
