#include "common_def.h"

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



int main(int argc, char *argv[])
{
    Patient patient;
    
    // create FIFO
    create_fifo_queue(FIFO_REJESTRACJA);

    // read only FIFO
    int fifo_oknienko = open_read_only_fifo(FIFO_REJESTRACJA);
    char *doctorStr = doctor_name[patient.doctor];

    // read 
    while(read(fifo_oknienko, &patient, sizeof(patient)) > 0){
        printf("PRZYCHODNIA: Rejestruję ... %d (%s)\n", patient.pid, doctorStr);
        utworz_nowy_semafor(&patient);


        utworz_pamiec_pacjent(&patient);
        patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);

        if(patient.doctor == POZ){
          *patient_state = REGISTER_SUCCESS;
        }else{
          *patient_state = REGISTER_FAIL;
        }

        sleep(3);
        semafor_open(&patient);
    }
    
    printf("PRZYCHODNIA: koniec\n");
    close(fifo_oknienko);
    return 0;
}

