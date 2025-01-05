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


doctorType choosePatientType();

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }

    int numOfPatients = atoi(argv[1]);

    Patient patient;
    
    while (access(FIFO_REJESTRACJA, F_OK) == -1) {
        printf("PACJENT: Czekam na utworzenie kolejki FIFO...\n");
        sleep(3);
    }

    // write only FIFO
    int fifo_oknienko = open_write_only_fifo(FIFO_REJESTRACJA);


    // createPatients
    for(int i=0; i < numOfPatients; i++){
        if(fork() == 0){
            srand(getpid());
            // send PID
            patient.pid = getpid();
            patient.doctor = choosePatientType();

            //tworzenie semaforow dla kazdego z pacjentow
            utworz_nowy_semafor(&patient);

            utworz_pamiec_pacjent(&patient);
            patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);
            *patient_state = OUTSIDE;

            *patient_state = REGISTER;
            printf("PACJENT: %d (%d) stoję w kolejce do rejestracji...\n", patient.pid, patient.doctor);

            write_fifo_patient(&patient, fifo_oknienko);

            // czekaj na odp od przychodni
            semafor_close(&patient);

            if(*patient_state == REGISTER_SUCCESS){
                printf("PACJENT: %d (%d) zostałem zajestrowany.\n", patient.pid, patient.doctor);
                char* fifo_doctor_name = fifo_queue_doctor[patient.doctor];
                int fifo_queue_doctor = open_write_only_fifo(fifo_doctor_name);

                *patient_state = patient.doctor + 4;
                printf("PACJENT: %d (%d) stoję w kolejce do lekarza.\n", patient.pid, patient.doctor);
                write_fifo_patient(&patient, fifo_queue_doctor);
                // czekaj na lekarza
                semafor_close(&patient);

                close(fifo_queue_doctor);


            }else if(*patient_state == REGISTER_FAIL){
                printf("PACJENT: %d (%d) odrzucono moją rejestrację.\n", patient.pid, patient.doctor);
            }

            *patient_state = GO_HOME;
            printf("PACJENT: %d (%d) koniec - idę do domu.\n", patient.pid, patient.doctor);


            odlacz_pamiec_pacjent(&patient, patient_state);
            usun_semafor(&patient);
            close(fifo_oknienko);
            
            exit(0); 
        }
        sleep(1);
    }
    //printf("PACJENT: Wszystkie procesy pacjentów zostały utworzone.\n");

    for (int i = 0; i < numOfPatients; i++) {
        wait(NULL); 
    }
    
    close(fifo_oknienko);
    return 0;
}

doctorType choosePatientType() {
    int r = rand() % 100;
    if (r < 60) {
        return POZ;
    } else if (r < 70) {
        return KARDIOLOG;
    } else if (r < 80) {
        return OKULISTA;
    } else if (r < 90) {
        return PEDIATRIA;
    }
    return MED_PRAC;
    
}

