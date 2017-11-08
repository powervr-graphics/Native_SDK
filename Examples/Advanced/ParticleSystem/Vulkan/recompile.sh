#!/bin/bash
../../../../Builds/Spir-V/compile.sh FloorVertShader_vk.vsh FloorVertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh FragShader_vk.fsh FragShader_vk.fsh.spv frag
../../../../Builds/Spir-V/compile.sh ParticleFragShader_vk.fsh ParticleFragShader_vk.fsh.spv frag
../../../../Builds/Spir-V/compile.sh ParticleSolver_vk.csh ParticleSolver_vk.csh.spv comp
../../../../Builds/Spir-V/compile.sh ParticleVertShader_vk.vsh ParticleVertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh VertShader_vk.vsh VertShader_vk.vsh.spv vert
