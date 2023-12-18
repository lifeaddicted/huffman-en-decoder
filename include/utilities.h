#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <cstdint>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/*******************************************************************************
* functionality:
        compare two files' deference, return the num of deferent bytes
* args:
* 		pathname of two files
*******************************************************************************/
uint32_t comp_2files(const char *pathname1, const char *pathname2)
{
    FILE *fp1 = fopen(pathname1, "r");
    FILE *fp2 = fopen(pathname2, "r");
    char buf1[1024] = {0};
    char buf2[1024] = {0};
    fclose(fp1);
    fclose(fp2);
}

#endif