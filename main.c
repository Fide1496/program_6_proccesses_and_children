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
volatile sig_atomic_t time_expired = 0;

int checkError(int val, const char *msg) {
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void signalHandler(int sig){
    char response;

    // Sigint handles control c
    if(sig == SIGINT){
        printf("\nAre you sure (Y/n)? ");
        scanf(" %c", &response);
        if (response == 'Y') {
            exit(EXIT_SUCCESS);
        }
    }
    // Handles timer expiring
    if(sig == SIGALRM){
        time_expired = 1;
        printf("\nTime's UP!\n");
    }
    if (sig == SIGUSR1){
        printf("Warning! roll outside of bounds");
    }
    if(sig == SIGUSR2){
        printf("Warning! pitch outside of bounds");
    }
    
}

void childSignalHandler(int sig){
    if(sig == SIGTERM){
        printf("Parents has died, needs to terminate");
    }
    if(sig == SIGINT){
        // Handle sig int for the child process
    }
}

int main(){


    pid_t childPID;
    const char *input_file = "angl.dat";

    // Nanosleep variables
    struct timespec ts;
    ts.tv_sec=1;
    ts.tv_nsec=0;



    // Create child signal processes here

    // Control C
    struct sigaction sa_int;
    sa_int.sa_handler = signalHandler;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    int input_fd = checkError(open(input_file, O_RDONLY), "Open angl.dat");

    double buffer[PACKET_SIZE*DOUBLE_SIZE];

    // Combine the while loop and switch statement below to get the child
    // to execute the code in the while loop

    while (read(input_fd, buffer, DOUBLE_SIZE * PACKET_SIZE) == DOUBLE_SIZE * PACKET_SIZE)
    {
        // If range conditions are met for roll and pitch, print values   
        double roll = buffer[0];
        double pitch = buffer[1];
        double yaw = buffer[2];


        if(roll > -20 && roll < 20 && pitch > -20 && pitch < 20)
        {
            printf("Values for roll and pitch inside the range -20 to 20\n");
            printf("Roll: %.2lf, Pitch: %.2lf \n", roll, pitch);
            fflush(stdout);
        }
        nanosleep(&ts, NULL);
    }

    // Creates child processes
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