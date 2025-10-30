/**
 * @file print_args_envs.c
 * @author Matt Krueger (mnkrueger@uiowa.edu)
 * @brief Homework 1 Task 1
 * @version 0.1
 * @date 2025-09-30
 */
#include <stdio.h>

/**
 * @brief simple program that prints the arguments and environment variables 1-per-line
 * 
 * Example Output:
 *                  ./printargsandenv
 *                   argument1
 *                   argument2
 *                   XDG VTNR=7
 *                   ORBIT SOCKETDIR=/tmp/orbit-antoniob
 *                   XDG SESSION ID=c1
 *
 * @param argc argument count
 * @param argv argument vector
 * @param env  environment pointer array
 * @return int 
 */
int main(int argc, char *argv[], char *env[])
{
    for (int i=0; i<argc; i++){
        printf("%s\n", argv[i]);
    }

    int i=0;
    while (env[i] != NULL){
        printf("%s\n", env[i]);
        i++;
    }

    return 0;
}
