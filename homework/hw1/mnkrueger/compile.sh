#!/bin/bash

# file: compile.sh
# author: Matt Krueger (mnkrueger@uiowa.edu)
# description: compiler for 'print_args_envs.c' and 'shell.c'

echo "compiling 'print_args_envs.c'..."
gcc -o print_args_envs print_args_envs.c

echo "compiling 'shell.c'..."
gcc -o shell -std=gnu99 shell.c

echo "done!"
