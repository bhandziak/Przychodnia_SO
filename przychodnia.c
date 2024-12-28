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
    pid_t pacjent_pid;

    // create FIFO
    if (mkfifo(FIFO_REJESTRACJA, 0600) == -1) {
        perror("PRZYCHODNIA: FIFO error");
    }

    // read only FIFO
    int fifo_oknienko = open(FIFO_REJESTRACJA, O_RDONLY);
    if (fifo_oknienko == -1) {
        perror("PRZYCHODNIA: Cant open FIFO");
        exit(1);
    }

    // read PID
    while(read(fifo_oknienko, &pacjent_pid, sizeof(pacjent_pid)) > 0){
        printf("PRZYCHODNIA: Rejestruję ... %d\n", pacjent_pid);
        sleep(3);
    }
    
    close(fifo_oknienko);
    return 0;
}
