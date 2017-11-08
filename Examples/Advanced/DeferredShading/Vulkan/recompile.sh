#!/bin/bash
../../../../Builds/Spir-V/compile.sh GBufferVertexShader_vk.vsh GBufferVertexShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh GBufferFragmentShader_vk.fsh GBufferFragmentShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh GBufferFloorVertexShader_vk.vsh GBufferFloorVertexShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh GBufferFloorFragmentShader_vk.fsh GBufferFloorFragmentShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh AttributelessVertexShader_vk.vsh AttributelessVertexShader_vk.vsh.spv vert

../../../../Builds/Spir-V/compile.sh DirectionalLightFragmentShader_vk.fsh DirectionalLightFragmentShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass1VertexShader_vk.vsh PointLightPass1VertexShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass1FragmentShader_vk.fsh PointLightPass1FragmentShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass2VertexShader_vk.vsh PointLightPass2VertexShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass2FragmentShader_vk.fsh PointLightPass2FragmentShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass3VertexShader_vk.vsh PointLightPass3VertexShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass3FragmentShader_vk.fsh PointLightPass3FragmentShader_vk.fsh.spv frag
