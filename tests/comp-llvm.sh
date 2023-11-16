#!/bin/sh

NAME="$(basename "${1}" .c)"

# С уровнем оптимизации -O0 атрибут "branch_weights" не генерируется,
# иногда и с -O1 атрибут "branch_weights", это зависит от кода программы
# эти вещи нужно проверять вручную
clang -emit-llvm -S -O1 "${1}" -o ./IR/"${NAME}".ll

