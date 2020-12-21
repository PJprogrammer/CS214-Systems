#!/bin/bash
NUM_TESTS=$(ls -d1 tests/test* | wc -l)

# Compile program
make clean
make all

# Run program
for ((i=1; i<=NUM_TESTS; i++))
do
	echo "Test Case $i:"
	./detector "tests/test$i"
done
