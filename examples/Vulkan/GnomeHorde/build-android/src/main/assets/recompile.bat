@echo off
call ..\..\..\external\spir-v\glslangValidator.exe -V Object.vsh -o Object.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V Shadow.vsh -o Shadow.vsh.spv -S vert
call ..\..\..\external\spir-v\glslangValidator.exe -V Solid.fsh -o Solid.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V Plant.fsh -o Plant.fsh.spv -S frag
call ..\..\..\external\spir-v\glslangValidator.exe -V Shadow.fsh -o Shadow.fsh.spv -S frag
