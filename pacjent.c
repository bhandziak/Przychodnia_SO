#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <fcntl.h>

#include <time.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/wait.h>

#define FIFO_REJESTRACJA "fifo_rej1"

int main(int argc, char *argv[])
{
    int numOfPatients = atoi(argv[1]);
    int pacjent_pid;
    
    while (access(FIFO_REJESTRACJA, F_OK) == -1) {
        printf("PACJENT: Czekam na utworzenie kolejki FIFO...\n");
        sleep(3);
    }

    // write only FIFO
    int fifo_oknienko = open(FIFO_REJESTRACJA, O_WRONLY);
    if (fifo_oknienko == -1) {
        perror("PACJENT: Cant open FIFO\n");
        exit(1);
    }

    // createPatients
    for(int i=0; i < numOfPatients; i++){
        if(fork() == 0){
            // send PID
            pacjent_pid = getpid();
            printf("PACJENT: %d stoję w kolejce\n", pacjent_pid);

            if (write(fifo_oknienko, &pacjent_pid, sizeof(pacjent_pid)) == -1) {
                perror("write error\n");
                close(fifo_oknienko);
                exit(1);
            }
            close(fifo_oknienko);
            exit(0); 
        }
        sleep(1);
    }
    printf("PACJENT: Wszystkie procesy pacjentów zostały utworzone.\n");

    close(fifo_oknienko);
    return 0;
}
