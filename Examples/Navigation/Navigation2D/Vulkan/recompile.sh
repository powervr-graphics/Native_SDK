#!/bin/bash
../../../../Builds/Spir-V/compile.sh VertShader_vk.vsh VertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh FragShader_vk.fsh FragShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh AA_VertShader_vk.vsh AA_VertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh AA_FragShader_vk.fsh AA_FragShader_vk.fsh.spv frag