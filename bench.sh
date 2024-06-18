#!/bin/bash

# Branches to perform benchmark on:
# - main
# - compact-token
# - class-ids
# - compact-type
# - struct-of-arrays

# Programs: (in SampleTranspiledPrograms dir)
# - chibicc_combined.tea
# - chibicc_parse.tea
# - gzip.tea
# - zlib.tea

# first CLI arg is the number of iterations
num_iterations=$1

branches="main compact-token class-ids compact-type struct-of-arrays"

# Function that is called with a given branch name.
# Will compile the compiler, and then run all programs and gather results.

run_branch() {
    branch=$1
    echo "Running branch $branch"
    git checkout $branch
    make clean > /dev/null 2> /dev/null
    make > /dev/null 2> /dev/null
    for program in SampleTranspiledPrograms/*
    do
        for i in {1..$num_iterations}
        do
            echo "Running program $program iteration $i"
            Compiler/compile $program /dev/null 2> /dev/null
        done
    done
}

# Run all branches

for branch in $branches
do
    run_branch $branch
done