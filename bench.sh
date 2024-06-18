#!/bin/bash

# first CLI arg is the number of iterations, 50 by default
num_iterations=${1:-50}

branches="main compact-token class-ids compact-type struct-of-arrays"

# Function that is called with a given branch name.
# Will compile the compiler, and then run all programs and gather results.

run_branch() {
    branch=$1
    git checkout $branch > /dev/null 2> /dev/null
    make clean > /dev/null 2> /dev/null
    make > /dev/null 2> /dev/null
    for program in SampleTranspiledPrograms/*
    do
        for i in $(seq 1 $num_iterations)
        do
            output=$(nice -n -20 Compiler/compile $program /dev/null 2> /dev/null)
            type_time=$(echo $output | grep -o "Type checking took [0-9]*" | grep -o "[0-9]*")
            codegen_time=$(echo $output | grep -o "Code generation took [0-9]*" | grep -o "[0-9]*")
            echo "$branch,$program,$i,$type_time,$codegen_time"
        done
    done
}

# Run all branches

echo "branch,program,iteration,type_time,codegen_time"

for branch in $branches
do
    run_branch $branch
done