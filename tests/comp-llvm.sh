#!/bin/sh

NAME="$(basename "${1}" .c)"

# с уровнем оптимизации -O0 атрибут "branch_weights" не генерируется
clang -emit-llvm -S -O1 "${1}" -o ./IR/"${NAME}".ll

