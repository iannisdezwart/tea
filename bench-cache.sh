#!/bin/bash

branches="main compact-token class-ids compact-type struct-of-arrays"

run_branch() {
    branch=$1
    git checkout $branch > /dev/null 2> /dev/null
    make clean > /dev/null 2> /dev/null
    ENV_FLAGS="-DCALLGRIND" make > /dev/null 2> /dev/null
    for program in SampleTranspiledPrograms/*
    do
        valgrind_output=$(valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes --collect-atstart=no --instr-atstart=no Compiler/compile $program /dev/null 2>&1)
        i_refs=$(echo "$valgrind_output" | grep "I   refs:" | awk '{gsub(/,/, "", $4); print $4}')
        i1_misses=$(echo "$valgrind_output" | grep "I1  misses:" | awk '{gsub(/,/, "", $4); print $4}')
        d_refs=$(echo "$valgrind_output" | grep "D   refs:" | awk '{gsub(/,/, "", $4); print $4}')
        d1_misses=$(echo "$valgrind_output" | grep "D1  misses:" | awk '{gsub(/,/, "", $4); print $4}')
        lld_misses=$(echo "$valgrind_output" | grep "LLd misses:" | awk '{gsub(/,/, "", $4); print $4}')
        ll_refs=$(echo "$valgrind_output" | grep "LL refs:" | awk '{gsub(/,/, "", $4); print $4}')
        ll_misses=$(echo "$valgrind_output" | grep "LL misses:" | awk '{gsub(/,/, "", $4); print $4}')
        echo "$branch,$program,$i_refs,$i1_misses,$d_refs,$d1_misses,$lld_misses,$ll_refs,$ll_misses"
    done
}

# Run all branches

echo "branch,program,i_refs,i1_misses,d_refs,d1_misses,lld_misses,ll_refs,ll_misses"

for branch in $branches
do
    run_branch $branch
done