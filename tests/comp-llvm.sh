#!/bin/sh

if [ $# -eq 1 ]; then
    NAME="$(basename "${1}" .c)"

    # С уровнем оптимизации -O0 атрибут "branch_weights" не генерируется,
    # иногда и с -O1 атрибут "branch_weights", это зависит от кода программы
    # эти вещи нужно проверять вручную
    clang -emit-llvm -S -O1 "${1}" -o ./IR/"${NAME}".ll

elif [ $# -eq 2 ] && [ "${1}" = "bin" ]; then
    NAME="$(basename "${2}" .c)"

    llc -filetype=obj ./IROptimized/"${NAME}".ll -o ./bin/"${NAME}".o
    clang ./bin/main.o ./bin/"${NAME}".o -o ./bin/"${NAME}"
    rm ./bin/"${NAME}".o

    ./bin/"${NAME}"
fi

