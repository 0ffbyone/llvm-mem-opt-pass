#!/bin/sh

TEST=$1

#InferFunctionAttributes
opt -load-pass-plugin ./build/libMemOpt.so -passes="mem-opt" ./tests/IR/"${TEST}" -S -o ./tests/IROptimized/"${TEST}"
