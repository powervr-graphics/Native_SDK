#!/bin/bash
../../../../Builds/Spir-V/compile.sh Object.vsh Object.vsh.spv vert
../../../../Builds/Spir-V/compile.sh Plant.fsh Plant.fsh.spv frag
../../../../Builds/Spir-V/compile.sh Solid.fsh Solid.fsh.spv frag
../../../../Builds/Spir-V/compile.sh Shadow.vsh Shadow.vsh.spv vert
../../../../Builds/Spir-V/compile.sh Shadow.fsh Shadow.fsh.spv frag