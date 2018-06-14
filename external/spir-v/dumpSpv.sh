#!/bin/bash

#usage example 
#   dumpSpv vshader_vert.spv vshader

set -e 
shader_file_in=$1
shader_file_out=$2
shader_id="${shader_file_out}"
out="${shader_file_out}.h"
pushd ./
echo $out
echo "#pragma once" > $out
chmod +w $out
echo "static uint32_t spv_${shader_id}[] = " >> $out
echo "{" >> $out
hexdump -v -e '/4 "    %#08x,\n"' $shader_file_in >> $out
echo "};" >> $out

echo "static VkShaderModuleCreateInfo shaderModuleCreateInfo_${shader_id} = " >> $out
echo "{" >> $out
echo "    static_cast<VkStructureType>(pvrvk::StructureType::e_SHADER_MODULE_CREATE_INFO)," >> $out
echo "    nullptr," >> $out
echo "    VkShaderModuleCreateFlags(0)," >> $out
echo "    sizeof(spv_${shader_id})," >> $out
echo "    spv_${shader_id}," >> $out
echo "};" >> $out
popd

