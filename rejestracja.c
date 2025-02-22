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
#include <signal.h>

#include <stdbool.h>

void evacuatePatients(int sig);

int fifo_oknienko;
bool evacuateFlag = false;

int raport_semid;
int global_semid;
FILE* file_raport;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }
    int arg1 = atoi(argv[1]);
    if(arg1 < 0 || arg1 > 1){
        printf("Nieprawidlowa wartosc 1 argumentu.\n");
        return 1;
    }
    
    int NUMER_OKIENKA = arg1+1;
    bool okienko2_isopen = false;

    Patient patient;

    // obsługa sygnału 2
    signal(SIGUSR2, evacuatePatients);

    // plik raport.txt
    file_raport = fopen("raport.txt", "a");
    
    // create FIFO
    create_fifo_queue(FIFO_REJESTRACJA);

    // read only FIFO
    fifo_oknienko = open_read_only_fifo(FIFO_REJESTRACJA);

    // global vars and const
    int globalConst_memid;
    int globalVars_memid;
    ConstVars* globalConst_adres = (ConstVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ConstVars),&globalConst_memid);
    PublicVars* globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    global_semid = globalConst_adres->idsemVars;
    raport_semid = globalConst_adres->idsemRaport;

    // przypisanie PID do pamięci dzielonej
    semafor_close(global_semid);

    globalVars_adres->registerPID[NUMER_OKIENKA-1] = getpid();

    semafor_open(global_semid);

    struct stat fifo_okienko_stat; // stan fifo okienko
    // read 
    while(1){
        // sprawdzenie stanu fifo okienko (czy plik istnieje)
        if (fstat(fifo_oknienko, &fifo_okienko_stat) == -1) {
            perror("Problem z odczytaniem statystyk fifo okienko.\n");
            continue;
        }
        if(fifo_okienko_stat.st_nlink == 0){
            printf("OKIENKO nr. %d: nie ma podłączonych procesów do fifo - zamykam proces \n", NUMER_OKIENKA);
            break;
        }
        // sprawdzenia dla 2 okienka
        if(NUMER_OKIENKA == 2){
            
            semafor_close(global_semid);

            // logika okienka 2 
            if(globalVars_adres->register_count >= globalConst_adres->N / 2 && !okienko2_isopen ){
                // open
                printf("OKIENKO nr %d OTWIERA SIE: jest %d pacjentów w kolejce (N/2 = %d)\n", NUMER_OKIENKA, globalVars_adres->register_count, globalConst_adres->N / 2);
                okienko2_isopen = true;
                semafor_open(global_semid);
            } else if(globalVars_adres->register_count >= globalConst_adres->N / 3 && okienko2_isopen){
                printf("OKIENKO nr %d JEST OTWARTE: jest %d pacjentów w kolejce (N/3 = %d)\n", NUMER_OKIENKA, globalVars_adres->register_count, globalConst_adres->N / 3);
                semafor_open(global_semid);
            } else if (okienko2_isopen)
            {
                printf("OKIENKO nr %d ZAMYKA SIE: jest %d pacjentów w kolejce (N/3 = %d)\n", NUMER_OKIENKA, globalVars_adres->register_count, globalConst_adres->N / 3);
                okienko2_isopen = false;
                semafor_open(global_semid);
                continue;
            }else{
                //printf("OKIENKO: jest %d pacjentów w kolejce \n", globalVars_adres->register_count);
                semafor_open(global_semid);
                sleep(1);
                continue;
            }
        }

        if(read(fifo_oknienko, &patient, sizeof(patient)) > 0){

            semafor_close(global_semid);
            globalVars_adres->register_count -= patient.count;
            semafor_open(global_semid);

            patientState* patient_state = przydziel_adres_pamieci_pacjent(&patient);

            // przychodnia się zamyka
            if(globalVars_adres->time >= globalConst_adres->Tk){
                printf("OKIENKO nr %d: Przychodnia zamknięta - wypiszę skierowanie ... %d (%s)\n",NUMER_OKIENKA ,patient.pid, patient.doctorStr);

                semafor_close(raport_semid);
                fprintf(file_raport,"OKIENKO nr %d: %d skierowanie do %s, wystawił (%d)\n",NUMER_OKIENKA ,patient.pid, patient.doctorStr, getpid());
                semafor_open(raport_semid);

                *patient_state = REGISTER_FAIL;
                semafor_open(patient.semid);
                continue;
            }

            printf("OKIENKO nr %d: Rejestruję ... %d (%s)\n",NUMER_OKIENKA ,patient.pid, patient.doctorStr);
            sleep(5);
            

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
              }else{
                *patient_state = REGISTER_FAIL;
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
            
            semafor_open(patient.semid);

        }else{
          sleep(1);
        }
    }


    globalVars_adres->registerPID[NUMER_OKIENKA-1] = -1;


    printf("OKIENKO nr. %d: koniec\n",NUMER_OKIENKA );
    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    close(fifo_oknienko);
    fclose(file_raport);
    return 0;
}

// Funkcja czyszcząca kolejkę fifo
// Dodatkowo wysyła sygnał 2 do pacjenta

void evacuatePatients(int sig){
  Patient patient;
  evacuateFlag = true;
  while(read(fifo_oknienko, &patient, sizeof(patient)) > 0){
    kill(patient.pid, SIGUSR2);
  }
}
