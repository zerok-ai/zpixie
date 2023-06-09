#!/bin/bash
THIS_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $THIS_DIR/variables.sh

cd $OUTPUT_DIR
cmake ..
cmake --build .
cd $ROOT_DIR