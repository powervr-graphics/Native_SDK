#!/bin/bash
../../../external/spir-v/glslangValidator -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag
../../../external/spir-v/glslangValidator -V GraphVertShader_vk.vsh -o GraphVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V GraphFragShader_vk.fsh -o GraphFragShader_vk.fsh.spv -S frag
