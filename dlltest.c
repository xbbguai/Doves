//compile:
// linux:   gcc -shared -o mylib.so dlltest.c 
// macos:   g++ -shared -o mylib.so dlltest.c 


#include <stdio.h>

void OutputOne()
{
    printf("This is output one. \n");
}

void Output(char* str)
{
    printf("%s\n", str);
}
