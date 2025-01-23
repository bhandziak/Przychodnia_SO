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
bool isCloseTime = false;

void *count_time(void *args);
void *send_signal(void *args);
void displayMenu();
void handleSignal2(int sig);

int main(int argc, char *argv[])
{
    ConstVars globalConst;
    int X1 = 50; // limity przyjęć dla lekarzy
    int X2 = 5;
    int X3 = 5;
    int X4 = 5;
    int X5 = 5;

    int Xvals[] = {X1, X2, X3, X4, X5};

    memcpy(globalConst.X, Xvals, sizeof(Xvals));

    globalConst.N = 30; // max pojemność budynku
    globalConst.Tp = 0; 
    globalConst.Tk = 120; // godzina zamknięcia

    // obsługa CTRL + C

    signal(SIGINT, handleSignal2);

    // tworzenie pliku dla lekarzy (raport.txt)

    FILE* file_raport;
    file_raport = fopen("raport.txt", "w");

    fclose(file_raport);

    // globalConst_adres
  
    int globalConst_memid;
    globalConst.idsemVars = utworz_nowy_semafor(KEY_GLOBAL_SEMAPHORE);
    globalConst.idsemRaport = utworz_nowy_semafor(KEY_RAPORT_TXT);

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

    // błędny input 
    if(
        X1 < 0 || X2 < 0 || X3 < 0 || X4 < 0 || X5 < 0 ||
        globalConst.N <= 0 ||
        globalConst.Tp > globalConst.Tk ||
        globalConst.Tp < 0 || globalConst.Tp < 0
    ){
        int global_semid = globalConst_adres->idsemVars;
        perror("Podano błędne dane\n");
        semafor_close(global_semid);
        globalConst_adres->Tk = 1;
        globalConst_adres->Tp = 0;
        globalVars_adres->time = 0; // ustawiam czas na zamknięcie przychodni (bezpieczne zamknięcie programu)
        semafor_open(global_semid);
    }else{

        if (pthread_create(&threadSendSignal, NULL, send_signal, NULL) != 0) {
            perror("Błąd tworzenia wątku send_signal\n");
            exit(EXIT_FAILURE);
        }
    }

    if (pthread_create(&threadClock, NULL, count_time, NULL) != 0) {
        perror("Błąd tworzenia wątku clock\n");
        exit(EXIT_FAILURE);
    }


    pthread_join(threadClock, NULL);
    pthread_join(threadSendSignal, NULL);

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);
    return 0;
}

// Funkcja obsługująca odliczanie czasu w oddzielnym wątku.
// Wykorzystuje semafory i zapisuje czas w pamięci dzielonej (atrybut time)
// IN:
//   - nieużywane
// OUT:
//  VOID
// 
//   - Jeśli semafor o `global_semid` nie istnieje to kończy pracę procesu
//   - Jeśli jest godzina zamknięcia to kończy pracę procesus

void *count_time(void *args){
    int global_semid = globalConst_adres->idsemVars;
    char timeStr[10];
    while (clockIsActive)
    {
        if(semctl(global_semid, 0, GETPID) == -1){
            clockIsActive = false;
            exit(0);
        }
        semafor_close(global_semid);
        convertTimeToStr(globalVars_adres->time, timeStr);
        if(globalVars_adres->time >= globalConst_adres->Tk && !isCloseTime){
            printf("DYREKTOR: Jest godzina %s zamykam przychodnie\n", timeStr);
            isCloseTime = true;

        }else if(globalVars_adres->time >= globalConst_adres->Tk + 3){
            clockIsActive = false;
            exit(0);
        }
        globalVars_adres->time++;
        semafor_open(global_semid);
        sleep(1);
    }
    return NULL;
}

// Funkcja obsługująca wysyłanie sygnałów 1 i wyświetlanie informacji w oddzielnym wątku.
// Obsługuje interakcję z użytkownikiem.
// IN:
//   - nieużywane
// OUT:
//     VOID
//  - obsługa niepoprawnych danych wejściowych

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
            int idDoctorStr = action == 6 ? 0 : idDoctor; 
            char *doctorStr = doctor_name[idDoctorStr];
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

// Funkcja obsługująca sygnał ewakuacji 
// Wysyła sygnały 2 do procesów pacjentów, rejestracji, lekarzy
// IN:
//   - kod sygnału
// OUT:
//     VOID

void handleSignal2(int sig){
    printf("\n Wywołuję sygnał o ewakuacji... \n");


    // sygnał dla pacjentów w rejestracji
    for(int i = 0; i < globalVars_adres->registerZonePatientPIDsize; i++){
        if(globalVars_adres->registerZonePatientPID[i] > 0){
            kill(globalVars_adres->registerZonePatientPID[i], SIGUSR2);
        }
    }
    // sygnał dla pacjentów w strefie gabinetów
    for(int i = 0; i < globalVars_adres->registerZonePatientPIDsize; i++){
        if(globalVars_adres->doctorZonePatientPID[i] > 0){
            kill(globalVars_adres->doctorZonePatientPID[i], SIGUSR2);
        }
    }
    // sygnał dla rejestracji
    for(int i = 0; i < 2; i++){
        if(globalVars_adres->registerPID[i] > 0){
            kill(globalVars_adres->registerPID[i], SIGUSR2);
        }
    }
    // sygnał dla lekarzy
    for(int i = 0; i < 6; i++){
        if(globalVars_adres->doctorPID[i] > 0){
            kill(globalVars_adres->doctorPID[i], SIGUSR2);
        }
    }

    sleep(3);
    system("clear");
    displayMenu();
}

// Funkcja wyświetlająca menu akcji dyrektora

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


