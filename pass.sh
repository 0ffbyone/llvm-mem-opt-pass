#!/bin/sh

TEST=$1

opt -load-pass-plugin ./build/libMemOpt.so -passes="mem-opt" -disable-output ./tests/IR/"${TEST}"
