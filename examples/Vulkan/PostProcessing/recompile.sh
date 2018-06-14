#!/bin/bash
../../../external/spir-v/glslangValidator -V BlurVertShader_vk.vsh -o BlurVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V BlurFragShader_vk.fsh -o BlurFragShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V PostBloomVertShader_vk.vsh -o PostBloomVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V PostBloomFragShader_vk.fsh -o PostBloomFragShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V PreBloomVertShader_vk.vsh -o PreBloomVertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V PreBloomFragShader_vk.fsh -o PreBloomFragShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V VertShader_vk.vsh -o VertShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V FragShader_vk.fsh -o FragShader_vk.fsh.spv -S frag


