#!/bin/bash
../../../external/spir-v/glslangValidator -V UIRendererVertShader.vsh -o UIRendererVertShader.vsh.spv -S vert
../../../external/spir-v/glslangValidator -V UIRendererFragShader.fsh -o UIRendererFragShader.fsh.spv -S frag
../../../external/spir-v/dumpSpv.sh UIRendererVertShader.vsh.spv UIRendererVertShader
../../../external/spir-v/dumpSpv.sh UIRendererFragShader.fsh.spv UIRendererFragShader
