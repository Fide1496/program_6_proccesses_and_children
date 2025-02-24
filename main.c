#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>

int main(){


    pid_t childPID;

    // childPID = fork();
    // Returns a -1 on error. Returns a 0 in the child. Returns the process id for the child in the parents

    switch(childPID=fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        printf("We are in the child");
        exit(EXIT_SUCCESS);
    default:
        printf("We are in the parents, just after calling fork");
        while(1)
        {
            childPID = wait(NULL);
            if (childPID == -1){
                if(errno == ECHILD){
                    printf("Our last child has stopped. Goodbye");
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {
                perror("Wait");
                exit(EXIT_FAILURE);
            }

        }

    }

    exit(EXIT_SUCCESS);
}