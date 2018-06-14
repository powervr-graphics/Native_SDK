#!/bin/bash
../../../external/spir-v/glslangValidator -V Object.vsh -o Object.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V Plant.fsh -o Plant.fsh.spv -S frag
../../../external/spir-v/glslangValidator -V Solid.fsh -o Solid.fsh.spv -S frag
../../../external/spir-v/glslangValidator -V Shadow.vsh -o Shadow.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V Shadow.fsh -o Shadow.fsh.spv -S frag