#!/bin/bash
../../../../Builds/Spir-V/compile.sh BlurVertShader_vk.vsh BlurVertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh BlurFragShader_vk.fsh BlurFragShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PostBloomVertShader_vk.vsh PostBloomVertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PostBloomFragShader_vk.fsh PostBloomFragShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh PreBloomVertShader_vk.vsh PreBloomVertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh PreBloomFragShader_vk.fsh PreBloomFragShader_vk.fsh.spv frag

../../../../Builds/Spir-V/compile.sh VertShader_vk.vsh VertShader_vk.vsh.spv vert
../../../../Builds/Spir-V/compile.sh FragShader_vk.fsh FragShader_vk.fsh.spv frag


