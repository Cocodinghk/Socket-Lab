#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork failed.\n");
        return EXIT_FAILURE;
    }
    else if(pid == 0)
    {
        printf("Child process.\n");
    }
    else
    {
        printf("Parent process.\n");
    }

    return 0;
}