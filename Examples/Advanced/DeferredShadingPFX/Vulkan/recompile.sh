#!/bin/bash
../../../../Builds/Spir-V/compile.sh GBufferVertexShader.vsh GBufferVertexShader.vsh.spv vert
../../../../Builds/Spir-V/compile.sh GBufferFragmentShader.fsh GBufferFragmentShader.fsh.spv frag

../../../../Builds/Spir-V/compile.sh GBufferFloorVertexShader.vsh GBufferFloorVertexShader.vsh.spv vert
../../../../Builds/Spir-V/compile.sh GBufferFloorFragmentShader.fsh GBufferFloorFragmentShader.fsh.spv frag

../../../../Builds/Spir-V/compile.sh AttributelessVertexShader.vsh AttributelessVertexShader.vsh.spv vert

../../../../Builds/Spir-V/compile.sh DirectionalLightFragmentShader.fsh DirectionalLightFragmentShader.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass1VertexShader.vsh PointLightPass1VertexShader.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass1FragmentShader.fsh PointLightPass1FragmentShader.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass2VertexShader.vsh PointLightPass2VertexShader.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass2FragmentShader.fsh PointLightPass2FragmentShader.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PointLightPass3VertexShader.vsh PointLightPass3VertexShader.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PointLightPass3FragmentShader.fsh PointLightPass3FragmentShader.fsh.spv frag 