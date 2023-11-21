#!/bin/sh

TEST=$1

#InferFunctionAttributes
opt -load-pass-plugin ./build/libMemOpt.so -passes="mem-opt" -disable-output ./tests/IR/"${TEST}"
