#!/bin/bash

if [ ! -x $(which clang-format) ]
then
    echo "Install clang-format before running this script"
    exit 1
fi

repo_path=$(dirname $(readlink -f $0))
printf "repo_path: $repo_path\n"
cpp_files=$(find $repo_path -path $repo_path/build -prune -false -o -name *.hpp -o -name *.cpp)
printf "cpp_files:\n$cpp_files\n"

clang-format --style=file -i $cpp_files

echo "Done"
