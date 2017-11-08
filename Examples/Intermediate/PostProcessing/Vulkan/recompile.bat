@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat BlurVertShader_vk.vsh BlurVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat BlurFragShader_vk.fsh BlurFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat PostBloomVertShader_vk.vsh PostBloomVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat FragShader_vk.fsh FragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat PreBloomVertShader_vk.vsh PreBloomVertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat VertShader_vk.vsh VertShader_vk.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat PostBloomFragShader_vk.fsh PostBloomFragShader_vk.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat PreBloomFragShader_vk.fsh PreBloomFragShader_vk.fsh.spv frag
