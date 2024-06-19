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
    ENV_FLAGS="-DMEASURE_MEM" make > /dev/null 2> /dev/null
    for program in SampleTranspiledPrograms/*
    do
        for i in $(seq 1 $num_iterations)
        do
            rm -f /tmp/pipe
            mkfifo /tmp/pipe
            exec 3<>/tmp/pipe
            Compiler/compile $program /dev/null >/dev/null </tmp/pipe 2>/dev/null & pid=$!
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_pr_1=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_pr_1=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            # before parsing
            echo "" >&3
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_pr_2=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_pr_2=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            vsz_pr=$((vsz_pr_2 - vsz_pr_1))
            rss_pr=$((rss_pr_2 - rss_pr_1))
            # after parsing
            echo "" >&3
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_tc_1=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_tc_1=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            # before type checking
            echo "" >&3
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_tc_2=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_tc_2=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            vsz_tc=$((vsz_tc_2 - vsz_tc_1))
            rss_tc=$((rss_tc_2 - rss_tc_1))
            # after type checking
            echo "" >&3
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_cg_1=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_cg_1=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            # before code generation
            echo "" >&3
            sleep 0.5
            pidstat_output=$(pidstat -h -r -u -v -p $pid)
            vsz_cg_2=$(echo "$pidstat_output" | awk 'NR==4 {print $12}')
            rss_cg_2=$(echo "$pidstat_output" | awk 'NR==4 {print $13}')
            vsz_cg=$((vsz_cg_2 - vsz_cg_1))
            rss_cg=$((rss_cg_2 - rss_cg_1))
            # after code generation
            echo "" >&3
            echo "$branch,$program,$i,$vsz_pr,$rss_pr,$vsz_tc,$rss_tc,$vsz_cg,$rss_cg"
            wait $pid
            exec 3>&-
        done
    done
}

# Run all branches

echo "branch,program,iteration,checkpoint,vsz_pr,rss_pr,vsz_tc,rss_ts,vsz_cg,rss_cg"

for branch in $branches
do
    run_branch $branch
done