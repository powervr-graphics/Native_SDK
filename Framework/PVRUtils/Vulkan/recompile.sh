#!/bin/bash
../../../Builds/Spir-V/compile.sh UIRendererShader_vk.vsh UIRendererShader_vk.vsh.spv vert
../../../Builds/Spir-V/compile.sh UIRendererShader_vk.fsh UIRendererShader_vk.fsh.spv frag
../../../Builds/Spir-V/dumpSpv.sh UIRendererShader_vk.vsh.spv UIRendererShader_vk.vsh
../../../Builds/Spir-V/dumpSpv.sh UIRendererShader_vk.fsh.spv UIRendererShader_vk.fsh
