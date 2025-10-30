/**
 * @file task1.c
 * @author Matt Krueger (mnkrueger@uiowa.edu)
 * @brief homework 3, task 1 | Operating Systems, Fall 2025 | University of Iowa
 * @version 0.1
 * @date 2025-10-24
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**
 * @brief converts the user passed arg (integer) into hexidecimal form 
 * 
 * @param argv number to convert
 * @param out_addr ptr to address of the converted number 
 * @return int 
 */
int parse_hex(char const **argv, unsigned long *out_addr) 
{
    char *end = NULL;
    unsigned long hex = strtoul(argv[1], &end, 16);
    if (end == argv[1] || *end != '\0' || hex > 0x7fffffffffffffffUL) return -1;    
    *out_addr = hex;
    return 0;
}

/**
 * @brief entry point
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    if (argc != 2) return -1;

    // convert hex (address is stored at req)
    unsigned long req = 0;
    if (parse_hex(argv, &req) != 0) return -1;

    // get path to virtual addresses of this process's id
    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/maps", (long)getpid());

    // opens the virtual addresses and search for the passed arg
    // this is T: O(n), M: O(1) search. could benefit from binary search (assuming that the addresses are ordered)
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        unsigned long start, end;
        if (sscanf(line, "%lx-%lx", &start, &end) == 2)
        {
            // included in region
            if (req >= start && req < end)
            {
                unsigned char *p = (unsigned char *)req;
                printf("%02x\n", *p);
                fclose(fp);
                return 0;
            }
        }

    }
    fclose(fp);
    return -1;      // not included
}