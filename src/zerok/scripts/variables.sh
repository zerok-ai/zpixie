#!/bin/bash
THIS_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export ROOT_DIR="$(dirname "$THIS_DIR")"
export OUTPUT_DIR="$ROOT_DIR/output"