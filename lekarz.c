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
#include <stdbool.h>
#include <signal.h>

void handlePatient(Patient *patient );
void endOfWork(int sig);

int doctorOffset;
doctorType doctorID;
int globalVars_memid;
PublicVars* globalVars_adres;
ConstVars* globalConst_adres;
int global_semid;

int fifo_queue_doctor_id;
int fifo_queue_doctor_id_vip;
char *doctorStr;

volatile sig_atomic_t endOfWorkFlag = 0;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }
    int arg1 = atoi(argv[1]);
    if(arg1 < 0 || arg1 > DOCTOR_COUNT){
        printf("Nieprawidlowa wartosc 1 argumentu.\n");
        return 1;
    }

    doctorOffset = 0;
    if(arg1 == 5){
        arg1 = 0;
        doctorOffset = 5;
    }

    // obsługa sygnału 1
    signal(SIGUSR1, endOfWork);

    doctorID = (doctorType)arg1;
    Patient patient;

    switch (doctorID) {
        case POZ:
            printf("Wybrano lekarza POZ.\n");
            break;
        case KARDIOLOG:
            printf("Wybrano lekarza KARDIOLOG.\n");
            break;
        case OKULISTA:
            printf("Wybrano lekarza OKULISTA.\n");
            break;
        case PEDIATRIA:
            printf("Wybrano lekarza PEDIATRIA.\n");
            break;
        case MED_PRAC:
            printf("Wybrano lekarza MED_PRAC.\n");
            break;
        default:
            printf("Nieprawidłowy typ lekarza.\n");
            return 1;
    }
    char* fifo_name = fifo_queue_doctor[doctorID];
    char* fifo_name_vip = fifo_queue_VIP_doctor[doctorID];
    doctorStr = doctor_name[doctorID];

    // global vars and const
    int globalConst_memid;
    globalConst_adres = (ConstVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ConstVars),&globalConst_memid);
    globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    // normal queue

    create_fifo_queue(fifo_name);

    fifo_queue_doctor_id = open_read_only_fifo(fifo_name);

    // VIP queue

    create_fifo_queue(fifo_name_vip);

    fifo_queue_doctor_id_vip = open_read_only_fifo(fifo_name_vip);

    global_semid = globalConst_adres->idsemVars;
    // przypisanie PID do pamięci dzielonej
    semafor_close(global_semid);

    globalVars_adres->doctorPID[doctorID+doctorOffset] = getpid();

    semafor_open(global_semid);

    struct stat fifo_doctor_stat; // stan fifo do lekarza (zwykła kolejka)

    while(1){
        // sprawdzenie stanu fifo doctor (czy plik istnieje)
        if (fstat(fifo_queue_doctor_id, &fifo_doctor_stat) == -1) {
            perror("Problem z odczytaniem statystyk fifo lekarz.\n");
            continue;
        }
        if(fifo_doctor_stat.st_nlink == 0){
            printf("LEKARZ (%s): nie ma podłączonych procesów do fifo - zamykam proces \n", doctorStr);
            break;
        }

        // najpierw vip, potem inni
        if(read(fifo_queue_doctor_id_vip, &patient, sizeof(Patient)) > 0){
            handlePatient(&patient);
        }else if(read(fifo_queue_doctor_id, &patient, sizeof(Patient)) > 0){
            handlePatient(&patient);
        }else{
            sleep(1);
        }

        if (endOfWorkFlag) {
            printf("LEKARZ (%s): Otrzymano sygnał zakończenia pracy, kończę proces.\n", doctorStr);
            break;
        }
    }

    globalVars_adres->doctorPID[doctorID+doctorOffset] = -1;

    printf("LEKARZ (%s): koniec\n", doctorStr);

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    close(fifo_queue_doctor_id);
    //unlink(fifo_name);
    close(fifo_queue_doctor_id_vip);
    //unlink(fifo_name_vip);
    return 0;
}

void handlePatient(Patient *patient){
    printf("LEKARZ (%s)(wolnych miejsc - %d): Obsługuję pacjenta %d...\n", doctorStr,globalVars_adres->X_free[doctorID] ,patient->pid );

    patientState* patient_state = przydziel_adres_pamieci_pacjent(patient);

    sleep(15);
    semafor_open(patient->semid);
}

void endOfWork(int sig){
    Patient patient;

    printf("--------- LEKARZ %s (%d): Otrzymałem sygnał 1 ------------\n",doctorStr, getpid());

    endOfWorkFlag = 1;

    if(doctorID == POZ){
        semafor_close(global_semid);
        globalVars_adres->X_free[doctorID+doctorOffset] = 0;
        globalVars_adres->doctorPID[doctorID+doctorOffset] = -1;
        semafor_open(global_semid);

        // drugi lekarz też skończył pracę
        if(globalVars_adres->doctorPID[0] < 0 && globalVars_adres->doctorPID[5] < 0 ){

            while (read(fifo_queue_doctor_id_vip, &patient, sizeof(Patient)) > 0)
            {
                printf("LEKARZ: %d skierowanie do %s, wystawił (%d)\n", patient.pid, doctorStr, getpid());
                semafor_open(patient.semid);
            }

            while (read(fifo_queue_doctor_id, &patient, sizeof(Patient)) > 0)
            {
                printf("LEKARZ: %d skierowanie do %s, wystawił (%d)\n", patient.pid, doctorStr, getpid());
                semafor_open(patient.semid);
            }
            
        }

        printf("LEKARZ (%s): koniec\n", doctorStr);

        odlacz_pamiec(globalConst_adres);
        odlacz_pamiec(globalVars_adres);

        close(fifo_queue_doctor_id);
        close(fifo_queue_doctor_id_vip);

    }else{
        semafor_close(global_semid);
        globalVars_adres->X_free[doctorID] = 0;
        globalVars_adres->doctorPID[doctorID] = -1;
        semafor_open(global_semid);

        while (read(fifo_queue_doctor_id_vip, &patient, sizeof(Patient)) > 0)
        {
            printf("LEKARZ: %d skierowanie do %s, wystawił (%d)\n", patient.pid, doctorStr, getpid());
            semafor_open(patient.semid);
        }

        while (read(fifo_queue_doctor_id, &patient, sizeof(Patient)) > 0)
        {
            printf("LEKARZ: %d skierowanie do %s, wystawił (%d)\n", patient.pid, doctorStr, getpid());
            semafor_open(patient.semid);
        }
                

        printf("LEKARZ (%s): koniec\n", doctorStr);

        odlacz_pamiec(globalConst_adres);
        odlacz_pamiec(globalVars_adres);

        close(fifo_queue_doctor_id);
        close(fifo_queue_doctor_id_vip);

        //exit(0);
    }
}