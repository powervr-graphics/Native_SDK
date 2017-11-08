@echo off
call ..\..\..\..\Builds\Spir-V\compile.bat Object.vsh Object.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat Shadow.vsh Shadow.vsh.spv vert
call ..\..\..\..\Builds\Spir-V\compile.bat Solid.fsh Solid.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat Plant.fsh Plant.fsh.spv frag
call ..\..\..\..\Builds\Spir-V\compile.bat Shadow.fsh Shadow.fsh.spv frag
