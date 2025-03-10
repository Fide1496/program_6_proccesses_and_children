#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define PACKET_SIZE 3
#define DOUBLE_SIZE sizeof(double)
static pid_t pPid;

int checkError(int val, const char *msg) {
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void signalHandler(int sig){
    char response;

    if(sig == SIGINT){
        fflush(stdin);
        printf("\nAre you sure you want to exit (Y/n)? \n");
        scanf(" %c", &response);

        if (response == 'Y' || response == 'y') {
            if (pPid>0){
                kill(pPid, SIGTERM);
            }
            exit(EXIT_SUCCESS);
        }
    }
    if (sig == SIGCHLD) {
        printf("Child has exited\n");
        while(waitpid(-1, NULL, WNOHANG) > 0);
        exit(EXIT_SUCCESS);
    }
    if (sig == SIGUSR1) {
        printf("Warning! Roll outside of bounds\n");
    }
    if (sig == SIGUSR2) {
        printf("Warning! Pitch outside of bounds\n");
    }
    if(sig == SIGTERM){
        printf("Parent has died, child terminating...\n");
        exit(EXIT_SUCCESS);
    }
}

int main(){

    pid_t childPID;
    const char *input_file = "angl.dat";
    sigset_t blockSet, oldSet;

    sigemptyset(&blockSet);
    sigaddset(&blockSet, SIGINT);

    // Nanosleep variables
    struct timespec ts;
    ts.tv_sec=1;
    ts.tv_nsec=0;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;

    // Register signals in parent
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    pPid = getpid();

    int input_fd = checkError(open(input_file, O_RDONLY), "Open angl.dat\n");
    double buffer[PACKET_SIZE*DOUBLE_SIZE];
    
    // Blocking sigint before forking so it doesn't get called twice
    sigprocmask(SIG_BLOCK, &blockSet, &oldSet);

    // Creates child processes
    switch(childPID = fork())
    {
    case -1:
        perror("fork\n");
        exit(EXIT_FAILURE);
    case 0:

        struct sigaction sa_child;
        sigemptyset(&sa_child.sa_mask);
        sa_child.sa_handler = signalHandler;
        sa_child.sa_flags = 0;
        
        // Handle SIGTERM in child
        sigaction(SIGTERM, &sa_child, NULL);


        // Handles reading from data file
        while (read(input_fd, buffer, DOUBLE_SIZE * PACKET_SIZE) == DOUBLE_SIZE * PACKET_SIZE)
        {

            double roll = buffer[0];
            double pitch = buffer[1];
            double yaw = buffer[2];
        
            if(roll < -20 || roll > 20) {
                kill(getppid(), SIGUSR1);  
            }
            if(pitch < -20 || pitch > 20) {
                kill(getppid(), SIGUSR2);  
            }
            
            // Pause for 1 second between each loop
            nanosleep(&ts, NULL); 
        }
        
        
        printf("Child process done. Exiting...\n");
        exit(EXIT_SUCCESS);

    default:

        // Handles signals from the child to the parent
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = signalHandler;
        sa.sa_flags = 0;

        // Register signals in parent
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGCHLD, &sa, NULL);
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);
        pPid = childPID;
        sigprocmask(SIG_SETMASK, &oldSet, NULL);

        while(1) {
            pause();  
        }
    }

    exit(EXIT_SUCCESS);
}