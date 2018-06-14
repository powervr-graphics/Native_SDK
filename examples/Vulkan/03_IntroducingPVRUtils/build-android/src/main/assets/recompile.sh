#!/bin/bash
../../../external/spir-v/glslangValidator -V FragShader_vk.fsh -o FragShader_vk.spv -S frag
../../../external/spir-v/glslangValidator -V VertShader_vk.vsh -o VertShader_vk.spv -S vert