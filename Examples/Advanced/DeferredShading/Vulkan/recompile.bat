@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferVertexShader_vk.vsh GBufferVertexShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFragmentShader_vk.fsh GBufferFragmentShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFloorVertexShader_vk.vsh GBufferFloorVertexShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFloorFragmentShader_vk.fsh GBufferFloorFragmentShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat AttributelessVertexShader_vk.vsh AttributelessVertexShader_vk.vsh.spv vert

call ..\..\..\..\Builds\Spir-V\compile.bat DirectionalLightFragmentShader_vk.fsh DirectionalLightFragmentShader_vk.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass1VertexShader_vk.vsh PointLightPass1VertexShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass1FragmentShader_vk.fsh PointLightPass1FragmentShader_vk.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass2VertexShader_vk.vsh PointLightPass2VertexShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass2FragmentShader_vk.fsh PointLightPass2FragmentShader_vk.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass3VertexShader_vk.vsh PointLightPass3VertexShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass3FragmentShader_vk.fsh PointLightPass3FragmentShader_vk.fsh.spv frag
