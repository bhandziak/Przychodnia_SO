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

#include <pthread.h>

PublicVars* globalVars_adres;
ConstVars* globalConst_adres;
bool clockIsActive = true;
void *count_time(void *args);
void *send_signal(void *args);
void displayMenu();

int main(int argc, char *argv[])
{
    ConstVars globalConst;
    int X1 = 15;
    int X2 = 10;
    int X3 = 8;
    int X4 = 8;
    int X5 = 8;

    int Xvals[] = {X1, X2, X3, X4, X5};

    memcpy(globalConst.X, Xvals, sizeof(Xvals));

    globalConst.N = 15;
    globalConst.Tp = 0;
    globalConst.Tk = 480;

    // globalConst_adres
  
    int globalConst_memid;
    globalConst.idsemVars = utworz_nowy_semafor(KEY_GLOBAL_SEMAPHORE);
    printf("SEMID GLOBANY: %d\n", globalConst.idsemVars);

    globalConst_adres = (ConstVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ConstVars),&globalConst_memid);
    *globalConst_adres = globalConst;



    // globalVars_adres

    PublicVars publicVars;

    memset(&publicVars, 0, sizeof(PublicVars));
    int Xvals2[] = {X1, X2, X3, X4, X5, X1};
    memcpy(publicVars.X_free, Xvals2, sizeof(Xvals2));
    publicVars.people_free_count = globalConst.N;

    int globalVars_memid;
    globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);
    *globalVars_adres = publicVars;



    pthread_t threadClock, threadSendSignal;
    if (pthread_create(&threadClock, NULL, count_time, NULL) != 0) {
        perror("Błąd tworzenia wątku clock\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&threadSendSignal, NULL, send_signal, NULL) != 0) {
        perror("Błąd tworzenia wątku send_signal\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(threadClock, NULL);
    pthread_join(threadSendSignal, NULL);

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    return 0;
}

void *count_time(void *args){
    int global_semid = globalConst_adres->idsemVars;
    char timeStr[10];
    while (clockIsActive)
    {
        semafor_close(global_semid);
        convertTimeToStr(globalVars_adres->time, timeStr);
        if(globalVars_adres->time >= globalConst_adres->Tk){
            printf("DYREKTOR: Jest godzina %s zamykam przychodnie\n", timeStr);
            clockIsActive = false;
            semafor_open(global_semid);
            break;
        }
        globalVars_adres->time++;
        semafor_open(global_semid);
        sleep(1);
    }
    return NULL;
}

void *send_signal(void *args){
    int global_semid = globalConst_adres->idsemVars;
    char timeStr[10];

    while (clockIsActive)
    {
        int action;
        displayMenu();
        if (scanf("%d", &action) != 1) {
            printf("Złe dane wejściowe\n");
            continue;
        }else if(action < 0 || action >6){
            printf("Złe dane wejściowe\n");
            continue;
        }
        getchar();

        semafor_close(global_semid);

        convertTimeToStr(globalVars_adres->time, timeStr);
        printf("Jest godzina %s \n", timeStr);

        semafor_open(global_semid);

        if(action == 0){
            printf("Na zewnątrz budynku: ");
            printDynamicArrayInt(globalVars_adres->outsidePatientPID, globalVars_adres->outsidePatientPIDsize);

            printf("Rejestracja: ");
            printDynamicArrayInt(globalVars_adres->registerPID, 2);

            printf("Lekarze: ");
            printDynamicArrayInt(globalVars_adres->doctorPID, 6);

            printf("Strefa rejestracji: ");
            printDynamicArrayInt(globalVars_adres->registerZonePatientPID, globalVars_adres->registerZonePatientPIDsize);

            printf("Strefa gabinetów: ");
            printDynamicArrayInt(globalVars_adres->doctorZonePatientPID, globalVars_adres->doctorZonePatientPIDsize);
        }else{
            int idDoctor = action-1;
            int pidDoctor = globalVars_adres->doctorPID[idDoctor];
            char *doctorStr = doctor_name[idDoctor];
            if(pidDoctor == -1){
                printf("%s pid(%d) już zakończył pracę\n", doctorStr, pidDoctor);
                continue;
            }
            printf("Wysyłam sygnał zakończenia pracy do %s pid(%d)...\n", doctorStr, pidDoctor);

            if (kill(pidDoctor, SIGUSR1) == -1) {
                perror("Kill Error: Błąd wysłania sygnału \n");
                return NULL;
            }
        }

        printf("\n");
        getchar();
        //usleep(100000);
        system("clear");
        
    }
    
    return NULL;
}

void displayMenu(){
    printf("Akcje dyrektora:\n");
    printf("0. Wyświetl PID'y i lokalizacje \n");

    printf("1. Zakończ pracę lekarza POZ 1 \n");
    printf("2. Zakończ pracę lekarza Kardiolog \n");
    printf("3. Zakończ pracę lekarza Okulistę \n");
    printf("4. Zakończ pracę lekarza Pediatria \n");
    printf("5. Zakończ pracę lekarza Medycyna pracy \n");
    printf("6. Zakończ pracę lekarza POZ 2 \n");

    printf("CTRL + C: Sygnał o ewakuacji \n");
}


