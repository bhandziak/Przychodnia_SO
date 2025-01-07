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

    // global vars and const
    int globalConst_memid;
    int globalVars_memid;
    ContsVars* globalConst_adres = (ContsVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ContsVars),&globalConst_memid);
    PublicVars* globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    int global_semid = globalConst_adres->idsemVars;
    // read 
    while(read(fifo_oknienko, &patient, sizeof(patient)) > 0){
        printf("PRZYCHODNIA: Rejestruję ... %d (%s)\n", patient.pid, patient.doctorStr);
        utworz_nowy_semafor_pacjent(&patient);


        utworz_pamiec_pacjent(&patient);
        patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);

        // czy dany lekarz ma wolne terminy?
        semafor_close(global_semid);
        if(patient.doctor == POZ){
          // POZ
          if(globalVars_adres->X_free[0] + globalVars_adres->X_free[5] > 0){
            if(globalVars_adres->X_free[0] < globalVars_adres->X_free[5]){
              globalVars_adres->X_free[5]--;
            }else{
              globalVars_adres->X_free[0]--;
            }
            *patient_state = REGISTER_SUCCESS;
          }
        }else{
          // Specjaliści
          if(globalVars_adres->X_free[patient.doctor] > 0){
            *patient_state = REGISTER_SUCCESS;
            globalVars_adres->X_free[patient.doctor]--; 
          }else{
            *patient_state = REGISTER_FAIL;
          }
        }
        semafor_open(global_semid);

        sleep(3);
        semafor_open(patient.semid);
    }
    
    printf("PRZYCHODNIA: koniec\n");
    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    close(fifo_oknienko);
    return 0;
}

