#!/bin/bash

if [ $# -eq 1 ] && [ "${1}" == "test" ]; then
    for filename in ./IR/*.ll; do
        filename="$(basename "${filename}" "./IR/")"
        opt -load-pass-plugin ../build/libMemOpt.so -passes="mem-opt" ./IR/"${filename}" -S -o ./IROptimized/"${filename}" 2> res
        should_be_no=false
        if [[ $filename == "no-"* ]]; then
            should_be_no=true
        fi

        #echo -n $filename " " $should_be_no " "
        #cat res
        #echo

        if [[ ($should_be_no == true) && ($(cat res) == "NO") ]]; then
            printf "[CORRECT] %s\n" "${filename}"
            #echo
        elif  [[ ($should_be_no == false) && ($(cat res) == "YES") ]]; then
            printf "[CORRECT] %s\n" "${filename}"
            #echo
        else
            printf "[WRONG] %s\n" "${filename}"
            #echo
        fi

    done

elif [ $# -eq 1 ]; then
    NAME="$(basename "${1}" .c)"

    # С уровнем оптимизации -O0 атрибут "branch_weights" не генерируется,
    # иногда и с -O1 атрибут "branch_weights", это зависит от кода программы
    # эти вещи нужно проверять вручную
    clang -emit-llvm -S -O1 "${1}" -o ./IR/"${NAME}".ll

elif [ $# -eq 2 ] && [ "${1}" == "bin" ]; then
    NAME="$(basename "${2}" .c)"

    llc -filetype=obj ./IROptimized/"${NAME}".ll -o ./bin/"${NAME}".o
    clang ./bin/main.o ./bin/"${NAME}".o -o ./bin/"${NAME}"
    rm ./bin/"${NAME}".o

    ./bin/"${NAME}"
fi

