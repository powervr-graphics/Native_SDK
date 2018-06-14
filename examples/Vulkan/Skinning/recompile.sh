#!/bin/bash
../../../external/spir-v/glslangValidator -V SkinnedVertShader_vk.vsh -o SkinnedVertShader_vk.spv -S vert
../../../external/spir-v/glslangValidator -V SkinnedFragShader_vk.fsh -o SkinnedFragShader_vk.spv -S frag
../../../external/spir-v/glslangValidator -V DefaultVertShader_vk.vsh -o DefaultVertShader_vk.spv -S vert
../../../external/spir-v/glslangValidator -V DefaultFragShader_vk.fsh -o DefaultFragShader_vk.spv -S frag