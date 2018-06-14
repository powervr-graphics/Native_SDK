#!/bin/bash
../../../external/spir-v/glslangValidator -V GBufferVertexShader_vk.vsh -o GBufferVertexShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V GBufferFragmentShader_vk.fsh -o GBufferFragmentShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V GBufferFloorVertexShader_vk.vsh -o GBufferFloorVertexShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V GBufferFloorFragmentShader_vk.fsh -o GBufferFloorFragmentShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V AttributelessVertexShader_vk.vsh -o AttributelessVertexShader_vk.vsh.spv -S vert

../../../external/spir-v/glslangValidator -V DirectionalLightFragmentShader_vk.fsh -o DirectionalLightFragmentShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V PointLightPass1VertexShader_vk.vsh -o PointLightPass1VertexShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V PointLightPass1FragmentShader_vk.fsh -o PointLightPass1FragmentShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V PointLightPass2VertexShader_vk.vsh -o PointLightPass2VertexShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V PointLightPass2FragmentShader_vk.fsh -o PointLightPass2FragmentShader_vk.fsh.spv -S frag

../../../external/spir-v/glslangValidator -V PointLightPass3VertexShader_vk.vsh -o PointLightPass3VertexShader_vk.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V PointLightPass3FragmentShader_vk.fsh -o PointLightPass3FragmentShader_vk.fsh.spv -S frag
