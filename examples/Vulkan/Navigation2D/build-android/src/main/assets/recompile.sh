#!/bin/bash
../../../external/spir-v/glslangValidator -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V AA_VertShader_vk.vsh -o AA_VertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V AA_FragShader_vk.fsh -o AA_FragShader_vk.fsh.spv -S frag