#!/bin/bash

gcc -I utilities -I datamining/correlation utilities/polybench.c datamining/correlation/correlation.c -o correlation_base -lm
gcc -I utilities -I linear-algebra/kernels/doitgen utilities/polybench.c linear-algebra/kernels/doitgen/doitgen.c -o doitgen_base
gcc -I utilities -I linear-algebra/solvers/dynprog utilities/polybench.c linear-algebra/solvers/dynprog/dynprog.c -o dynprog_base
gcc -I utilities -I medley/floyd-warshall utilities/polybench.c medley/floyd-warshall/floyd-warshall.c -o floyd-warshall_base
gcc -I utilities -I stencils/jacobi-2d-imper utilities/polybench.c stencils/jacobi-2d-imper/jacobi-2d-imper.c -o jacobi-2d-imper_base
