RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
END='\033[0m'

N_PASSED=0
N_TESTS=0

for test in Tests/*/; do
    echo -e "${CYAN}Running $test${END}"
    Compiler/compile $test/program.tea $test/.program.teax > $test/.compilation-output.txt --debug
    VM/vm $test/.program.teax > $test/.run-output.txt
    diff $test/output.txt $test/.run-output.txt
    N_TESTS=$((N_TESTS+1))
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}(${N_TESTS}) $test passed${END}"
        N_PASSED=$((N_PASSED+1))
    else
        echo -e "${RED}(${N_TESTS}) $test failed${END}"
    fi
done

echo -e "${CYAN}Passed $N_PASSED/$N_TESTS tests${END}"
