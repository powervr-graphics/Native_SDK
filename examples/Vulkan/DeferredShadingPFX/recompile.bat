@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V GBufferVertexShader.vsh -o GBufferVertexShader.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V GBufferFragmentShader.fsh -o GBufferFragmentShader.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V GBufferFloorVertexShader.vsh -o GBufferFloorVertexShader.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V GBufferFloorFragmentShader.fsh -o GBufferFloorFragmentShader.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V AttributelessVertexShader.vsh -o AttributelessVertexShader.vsh.spv -S vert

call ..\..\..\external\spir-v\glslangValidator.exe -V DirectionalLightFragmentShader.fsh -o DirectionalLightFragmentShader.fsh.spv -S frag

call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass1VertexShader.vsh -o PointLightPass1VertexShader.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass1FragmentShader.fsh -o PointLightPass1FragmentShader.fsh.spv -S frag

call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass2VertexShader.vsh -o PointLightPass2VertexShader.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass2FragmentShader.fsh -o PointLightPass2FragmentShader.fsh.spv -S frag

call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass3VertexShader.vsh -o PointLightPass3VertexShader.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V PointLightPass3FragmentShader.fsh -o PointLightPass3FragmentShader.fsh.spv -S frag