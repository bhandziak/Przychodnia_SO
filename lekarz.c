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
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }
    int arg1 = atoi(argv[1]);
    if(arg1 < 0 || arg1 > MED_PRAC){
        printf("Nieprawidlowa wartosc 1 argumentu.\n");
        return 1;
    }

    doctorType doctor = (doctorType)arg1;
    Patient patient;

    switch (doctor) {
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
    char* fifo_name = fifo_queue_doctor[doctor];
    char *doctorStr = doctor_name[doctor];

    create_fifo_queue(fifo_name);

    int fifo_queue_doctor = open_read_only_fifo(fifo_name);

    while(read(fifo_queue_doctor, &patient, sizeof(Patient)) > 0) {
        utworz_nowy_semafor(&patient);
        printf("LEKARZ (%s): Obsługuję pacjenta %d \n", doctorStr, patient.pid);


        utworz_pamiec_pacjent(&patient);
        patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);

        sleep(5);

        semafor_open(&patient);

    }
    printf("LEKARZ (%s): koniec\n", doctorStr);
    close(fifo_queue_doctor);

    return 0;
}