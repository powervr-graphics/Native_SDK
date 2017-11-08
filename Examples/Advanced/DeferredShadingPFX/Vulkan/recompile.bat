@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferVertexShader.vsh GBufferVertexShader.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFragmentShader.fsh GBufferFragmentShader.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFloorVertexShader.vsh GBufferFloorVertexShader.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat GBufferFloorFragmentShader.fsh GBufferFloorFragmentShader.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat AttributelessVertexShader.vsh AttributelessVertexShader.vsh.spv vert

call ..\..\..\..\Builds\Spir-V\compile.bat DirectionalLightFragmentShader.fsh DirectionalLightFragmentShader.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass1VertexShader.vsh PointLightPass1VertexShader.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass1FragmentShader.fsh PointLightPass1FragmentShader.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass2VertexShader.vsh PointLightPass2VertexShader.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass2FragmentShader.fsh PointLightPass2FragmentShader.fsh.spv frag

call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass3VertexShader.vsh PointLightPass3VertexShader.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PointLightPass3FragmentShader.fsh PointLightPass3FragmentShader.fsh.spv frag