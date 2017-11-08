@echo off
copy %1 %1.%3

%~dp0/glslangValidator.exe -V -o %2 %1.%3

del %1.%3