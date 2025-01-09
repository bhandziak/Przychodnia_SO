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

    int fifo_queues_doctor[queues_doctor_count];

    for(int i = 0; i < queues_doctor_count-1; i++){
        char* fifo_doctor_name = fifo_queue_doctor[i];
        printf("Otwieram FIFO %s ... \n",fifo_doctor_name);
        fifo_queues_doctor[i]= open_write_only_fifo(fifo_doctor_name);
    }

    // global const and vars

    int globalConst_memid;
    int globalVars_memid;
    ContsVars* globalConst_adres = (ContsVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ContsVars),&globalConst_memid);
    PublicVars* globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    int global_semid = globalConst_adres->idsemVars;

    // createPatients
    for(int i=0; i < numOfPatients; i++){
        if(fork() == 0){
            srand(getpid());
            // send PID
            patient.pid = getpid();
            patient.doctor = choosePatientType();

            char* doctorStr = doctor_name[patient.doctor];
            strcpy(patient.doctorStr, doctorStr);

            //tworzenie semaforow dla kazdego z pacjentow
            utworz_nowy_semafor_pacjent(&patient);

            utworz_pamiec_pacjent(&patient);
            patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);
            *patient_state = OUTSIDE;

            // czy lekarze mają wolne terminy?
            if(sumIntArray(globalVars_adres->X_free, DOCTOR_COUNT) <= 0){
                // nie
                goHomePatient(&patient,patient_state);
            }
            
            // tak maja wolne terminy

            // czy osiągnieto limit osób w przychodni?
            // POTENCJALNE PROBLEMY Z SYNCHRONIZACJA
            semafor_close(global_semid);
            printf("Odczytuje LIMIT: %d ", globalVars_adres->people_free_count);
            if(globalVars_adres->people_free_count <= 0){
                // tak
                // TYMCZASOWO GO HOME
                // Powinna być kolejka fifo_outside
                printf("Osiągnięto LIMIT N (pojemności przychodni)! ");
                semafor_open(global_semid);
                goHomePatient(&patient,patient_state);
            }
            // nie
            globalVars_adres->people_free_count--;
            semafor_open(global_semid);

            *patient_state = REGISTER;
            printf("PACJENT: %d (%s) stoję w kolejce do rejestracji...\n", patient.pid, patient.doctorStr);

            write_fifo_patient(&patient, fifo_oknienko);

            // czekaj na odp od przychodni
            semafor_close(patient.semid);

            if(*patient_state == REGISTER_SUCCESS){
                printf("PACJENT: %d (%s) zostałem zajestrowany.\n", patient.pid, patient.doctorStr);


                *patient_state = patient.doctor + 4;
                printf("PACJENT: %d (%s) stoję w kolejce do lekarza.\n", patient.pid,  patient.doctorStr);
                write_fifo_patient(&patient, fifo_queues_doctor[patient.doctor]);
                // czekaj na lekarza
                semafor_close(patient.semid);

                close(fifo_queues_doctor[patient.doctor]);


            }else if(*patient_state == REGISTER_FAIL){
                printf("PACJENT: %d (%s) odrzucono moją rejestrację.\n", patient.pid, patient.doctorStr);
            }

            semafor_close(global_semid);

            globalVars_adres->people_free_count++;

            semafor_open(global_semid);


            close(fifo_oknienko);
            goHomePatient(&patient, patient_state);
            
            //exit(0); 
        }
        if(i > 2){
            sleep(3);
        }else{
            sleep(1);
        }
        
    }
    printf("PACJENT: koniec generowania pajentów.\n");

    for (int i = 0; i < numOfPatients; i++) {
        wait(NULL); 
    }
    
    close(fifo_oknienko);
    unlink(FIFO_REJESTRACJA);
    for(int i = 0; i < queues_doctor_count-1; i++){
        char* fifo_doctor_name = fifo_queue_doctor[i];
        close(fifo_queues_doctor[i]);
        unlink(fifo_doctor_name);
    }

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);

    usun_pamiec(globalConst_memid);
    usun_pamiec(globalVars_memid);

    usun_semafor(global_semid);
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

