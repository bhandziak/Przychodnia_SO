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

void handlePatient(Patient *patient, char* doctorStr,int doctorID ,PublicVars* globalVars_adres );

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

    int doctorOffset = 0;
    if(arg1 == 5){
        arg1 = 0;
        doctorOffset = 5;
    }

    doctorType doctorID = (doctorType)arg1;
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
    char *doctorStr = doctor_name[doctorID];

    // global vars and const
    int globalConst_memid;
    int globalVars_memid;
    ContsVars* globalConst_adres = (ContsVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ContsVars),&globalConst_memid);
    PublicVars* globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    // normal queue

    create_fifo_queue(fifo_name);

    int fifo_queue_doctor = open_read_only_fifo(fifo_name);

    // VIP queue

    create_fifo_queue(fifo_name_vip);

    int fifo_queue_doctor_vip = open_read_only_fifo(fifo_name_vip);

    struct stat fifo_doctor_stat; // stan fifo do lekarza (zwykła kolejka)

    while(1){
        // sprawdzenie stanu fifo doctor (czy plik istnieje)
        if (fstat(fifo_queue_doctor, &fifo_doctor_stat) == -1) {
            perror("Problem z odczytaniem statystyk fifo lekarz.\n");
            continue;
        }
        if(fifo_doctor_stat.st_nlink == 0){
            printf("LEKARZ (%s): nie ma podłączonych procesów do fifo - zamykam proces \n", doctorStr);
            break;
        }

        // najpierw vip, potem inni
        if(read(fifo_queue_doctor_vip, &patient, sizeof(Patient)) > 0){
            handlePatient(&patient, doctorStr, doctorID, globalVars_adres);
        }else if(read(fifo_queue_doctor, &patient, sizeof(Patient)) > 0){
            handlePatient(&patient, doctorStr, doctorID, globalVars_adres);
        }else{
            sleep(1);
        }
    }

    printf("LEKARZ (%s): koniec\n", doctorStr);

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    close(fifo_queue_doctor);
    //unlink(fifo_name);
    close(fifo_queue_doctor_vip);
    //unlink(fifo_name_vip);
    return 0;
}

void handlePatient(Patient *patient, char* doctorStr,int doctorID ,PublicVars* globalVars_adres ){
    printf("LEKARZ (%s)(wolnych miejsc - %d): Obsługuję pacjenta %d...\n", doctorStr,globalVars_adres->X_free[doctorID] ,patient->pid );

    patientState* patient_state = przydziel_adres_pamieci_pacjent(patient);

    sleep(10);
    semafor_open(patient->semid);
}