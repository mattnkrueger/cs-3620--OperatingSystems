/**
 * @file task2.c
 * @author Matt Krueger (mnkrueger@uiowa.edu)
 * @brief homework 3, task 2 | Operating Systems, Fall 2025 | University of Iowa
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
 * @brief program that essentially performs a reversal of a path passed (that lives on disk) using mmap
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    if (argc != 2) return -1;

    // open the file passed; if error ==> exit
    const char *path = argv[1];
    int fd = open(path, O_RDWR);
    if (fd < 0) return -1;

    // get the size of needed for mmap
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        return -1;
    }

    size_t size = (size_t)sb.st_size;
    if (size == 0) 
    {
        close(fd);      // if no content, nothing to reverse ==> success
        return 0;
    }

    // map the file into the process's virtual mem
    char *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // reverse the file content in-place (in memory mapped region; not stack!) T: O(n), M: O(1) 
    for (size_t i = 0; i < size / 2; i++)
    {
        char temp = addr[i];
        addr[i] = addr[size-1-i]; 
        addr[size-1-i] = temp;
    }

    // cleanup
    msync(addr, size, MS_SYNC); // flush for persistence
    munmap(addr, size);         // remove mapping
    close(fd);                  // close the file
    return -0;
}