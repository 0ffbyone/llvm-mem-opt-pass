#!/bin/bash

function check_correctness() {
    # $1 filename
    should_be_no=false
    if [[ $filename == "no-"* ]]; then
        should_be_no=true
    fi

    if [[ ($should_be_no == true) ]] && ! [[ $(grep "YES" ./tests/res) ]]; then
        printf "[CORRECT] %s\n" "${filename}"
    elif  [[ ($should_be_no == false) ]] && [[ $(grep "YES" ./tests/res) ]]; then
        printf "[CORRECT] %s\n" "${filename}"
    else
        printf "[WRONG] %s\n" "$1"
    fi
}


if [ $# -eq 1 ] && [ "${1}" == "test" ]; then
    for filename in ./tests/IR/*.ll; do
        filename="$(basename "${filename}" "./tests/IR/")"
        opt -load-pass-plugin ./build/libMemOpt.so -passes="mem-opt" ./tests/IR/"${filename}" -S -o ./tests/IROptimized/"${filename}" > ./tests/res 2>&1

        check_correctness "${filename}"

    done


elif [ $# -eq 1 ]; then
    filename="$(basename "${1}" .ll)"
    opt -load-pass-plugin ./build/libMemOpt.so -passes="mem-opt" ./tests/IR/"${filename}".ll -S -o ./tests/IROptimized/"${filename}".ll
    echo


elif [ $# -eq 2 ] && [ "${1}" == "ir" ]; then
    NAME="$(basename "${2}" .c)"

    # С уровнем оптимизации -O0 атрибут "branch_weights" не генерируется,
    # иногда и с -O1 атрибут "branch_weights", это зависит от кода программы
    # эти вещи нужно проверять вручную
    clang -emit-llvm -S -O1 ./tests/c-sources/"${NAME}".c -o ./tests/IR/"${NAME}".ll

elif [ $# -eq 2 ] && [ "${1}" == "bin" ]; then
    NAME="$(basename "${2}" .c)"

    llc -filetype=obj ./tests/IROptimized/"${NAME}".ll -o ./tests/bin/"${NAME}".o
    clang ./tests/bin/main.o .tests/bin/"${NAME}".o -o ./tests/bin/"${NAME}"
    rm ./tests/bin/"${NAME}".o

    ./tests/bin/"${NAME}"
fi
