#!/bin/bash

for ((i = 0 ; i <= 9 ; i++)); do
  /usr/bin/time -o output_shell.txt -a -f "%e" ./correlation_base
done
for ((i = 0 ; i <= 9 ; i++)); do
  /usr/bin/time -o output_shell.txt -a -f "%e" ./doitgen_base
done
for ((i = 0 ; i <= 9 ; i++)); do
  /usr/bin/time -o output_shell.txt -a -f "%e" ./dynprog_base
done
for ((i = 0 ; i <= 9 ; i++)); do
  /usr/bin/time -o output_shell.txt -a -f "%e" ./floyd-warshall_base
done
for ((i = 0 ; i <= 9 ; i++)); do
  /usr/bin/time -o output_shell.txt -a -f "%e" ./jacobi-2d-imper_base
done
