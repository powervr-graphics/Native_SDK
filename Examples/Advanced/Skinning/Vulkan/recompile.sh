#!/bin/bash
../../../../Builds/Spir-V/compile.sh SkinnedVertShader_vk.vsh SkinnedVertShader_vk.spv vert
../../../../Builds/Spir-V/compile.sh SkinnedFragShader_vk.fsh SkinnedFragShader_vk.spv frag
../../../../Builds/Spir-V/compile.sh DefaultVertShader_vk.vsh DefaultVertShader_vk.spv vert
../../../../Builds/Spir-V/compile.sh DefaultFragShader_vk.fsh DefaultFragShader_vk.spv frag