#!/bin/bash

set -e 
shader_file_in=$1
shader_file_out=$2
shader_type=$3

cp $shader_file_in $shader_file_in.$shader_type

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#pushd $dst_dir
$DIR/glslangValidator -V -o $shader_file_out $shader_file_in.$shader_type
chmod +w $shader_file_out
rm $shader_file_in.$shader_type 