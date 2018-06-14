#!/bin/bash
../../../external/spir-v/glslangValidator -V ColShader_vk.vsh -o ColShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V ColShader_vk.fsh -o ColShader_vk.fsh.spv -S frag
