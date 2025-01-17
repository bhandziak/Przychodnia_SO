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
#include <signal.h>

#include <time.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

patientState* patient_state;
PublicVars* globalVars_adres;
Patient patient;

int global_semid;

int fifo_oknienko;
int fifo_queues_doctor[6];
int fifo_queues_doctor_vip[6];

doctorType choosePatientType();
bool selectPatientVIP();
void evacuate(int sig);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }

    int numOfPatients = atoi(argv[1]);

    
    while (access(FIFO_REJESTRACJA, F_OK) == -1) {
        printf("PACJENT: Czekam na utworzenie kolejki FIFO_REJESTRACJA...\n");
        sleep(3);
    }

    // obsługa sygnału 2
    
    signal(SIGUSR2, evacuate);

    // write only FIFO
    int fifo_oknienko = open_write_only_fifo(FIFO_REJESTRACJA);

    int fifo_queues_doctor[queues_doctor_count];
    int fifo_queues_doctor_vip[queues_doctor_count];

    for(int i = 0; i < queues_doctor_count-1; i++){
        char* fifo_doctor_name = fifo_queue_doctor[i];
        char* fifo_doctor_name_vip = fifo_queue_VIP_doctor[i];
        printf("Otwieram FIFO %s ... \n",fifo_doctor_name);
        printf("Otwieram FIFO %s ... \n",fifo_doctor_name_vip);
        fifo_queues_doctor[i]= open_write_only_fifo(fifo_doctor_name);
        fifo_queues_doctor_vip[i]= open_write_only_fifo(fifo_doctor_name_vip);
    }

    // global const and vars

    int globalConst_memid;
    int globalVars_memid;
    
    ConstVars* globalConst_adres = (ConstVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ConstVars),&globalConst_memid);
    globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);

    global_semid = globalConst_adres->idsemVars;
    int raport_semid = globalConst_adres->idsemRaport;

    // createPatients
    for(int i=0; i < numOfPatients; i++){
        if(fork() == 0){ // DODAĆ sprawdzenie max procesów w systemie
            srand(getpid());
            // send PID
            patient.pid = getpid();
            patient.doctor = choosePatientType();
            patient.vip = selectPatientVIP();
            patient.count = (patient.doctor == PEDIATRIA) ? 2 : 1;
            char *vipStatusStr = (patient.vip) ? "VIP" : "";

            char* doctorStr = doctor_name[patient.doctor];
            strcpy(patient.doctorStr, doctorStr);

            //tworzenie semaforow dla kazdego z pacjentow
            utworz_nowy_semafor_pacjent(&patient);

            utworz_pamiec_pacjent(&patient);
            patient_state = przydziel_adres_pamieci_pacjent(&patient);

            // pacjent stoi przed przychodnią
            // dodanie pacjenta do strefy zewnętrznej
            semafor_close(global_semid);

            printf("PACJENT %s: %d (%s) Właśnie przyszedłem do przychodni. \n",vipStatusStr ,patient.pid, patient.doctorStr);

            appendToArrayInt(globalVars_adres->outsidePatientPID,&globalVars_adres->outsidePatientPIDsize ,patient.pid);

            semafor_open(global_semid);

            *patient_state = OUTSIDE;

            sleep(2);

            // czy lekarze mają wolne terminy?
            semafor_close(global_semid);
            if(sumIntArray(globalVars_adres->X_free, DOCTOR_COUNT) <= 0){
                // nie
                printf("PACJENT %s: %d (%s) Wszyscy lekarze nie mają wolnych terminów. \n",vipStatusStr ,patient.pid, patient.doctorStr);

                // usunięcie pacjenta ze strefy zewnętrznej
                removeFromArrayInt(globalVars_adres->outsidePatientPID,&globalVars_adres->outsidePatientPIDsize ,patient.pid);
                semafor_open(global_semid);
                goHomePatient(&patient,patient_state);
            }
            semafor_open(global_semid);
            // tak maja wolne terminy

            // czy osiągnieto limit osób w przychodni?
            while(1){
                semafor_close(global_semid);

                // nie (jest miejsce) wyjdź z pętli
                if(globalVars_adres->people_free_count - patient.count >= 0){
                    globalVars_adres->people_free_count -= patient.count;
                    semafor_open(global_semid);
                    break;
                }
                printf("PACJENT %s: %d (%s) Osiągnięto LIMIT N (pojemności przychodni)! czekam ...\n",vipStatusStr ,patient.pid, patient.doctorStr);
                semafor_open(global_semid);
                sleep(1);
            }

            *patient_state = REGISTER;

            // kolejka do rejestracji
            // przeniesienie pacjenta do strefy rejestracji

            semafor_close(global_semid);

            appendToArrayInt(globalVars_adres->registerZonePatientPID,&globalVars_adres->registerZonePatientPIDsize ,patient.pid);
            removeFromArrayInt(globalVars_adres->outsidePatientPID,&globalVars_adres->outsidePatientPIDsize ,patient.pid);
            globalVars_adres->register_count+= patient.count;

            semafor_open(global_semid);
            printf("PACJENT semid(%d) %s: %d (%s) stoję w kolejce do rejestracji...\n",patient.semid,vipStatusStr ,patient.pid, patient.doctorStr);

            write_fifo_patient(&patient, fifo_oknienko);

            // czekaj na odp od przychodni
            semafor_close(patient.semid);

            if(*patient_state == REGISTER_SUCCESS){
                printf("PACJENT %s: %d (%s) zostałem zajestrowany.\n",vipStatusStr ,patient.pid, patient.doctorStr);

                // przeniesienie pacjenta do strefy gabinetów lekarskich
                *patient_state = patient.doctor + 4;
                semafor_close(global_semid);

                appendToArrayInt(globalVars_adres->doctorZonePatientPID, &globalVars_adres->doctorZonePatientPIDsize,patient.pid);
                removeFromArrayInt(globalVars_adres->registerZonePatientPID, &globalVars_adres->registerZonePatientPIDsize ,patient.pid);

                semafor_open(global_semid);

                printf("PACJENT %s: %d (%s) stoję w kolejce do lekarza.\n",vipStatusStr ,patient.pid,  patient.doctorStr);
                if(patient.vip){
                    write_fifo_patient(&patient, fifo_queues_doctor_vip[patient.doctor]);
                    // czekaj na lekarza
                    semafor_close(patient.semid);

                    //close(fifo_queues_doctor_vip[patient.doctor]);
                }else{
                    write_fifo_patient(&patient, fifo_queues_doctor[patient.doctor]);
                    // czekaj na lekarza
                    semafor_close(patient.semid);

                    //close(fifo_queues_doctor[patient.doctor]);
                }
                
                // dodatkowa rejestracja do specjalisty
                if(*patient_state > 20){
                    patient.doctor = *patient_state - 20;

                    doctorStr = doctor_name[patient.doctor];
                    strcpy(patient.doctorStr, doctorStr);

                    printf("PACJENT %s: %d (%s) stoję w kolejce do lekarza.\n",vipStatusStr ,patient.pid,  patient.doctorStr);


                    if(patient.vip){
                        write_fifo_patient(&patient, fifo_queues_doctor_vip[patient.doctor]);
                        // czekaj na lekarza
                        semafor_close(patient.semid);
                    }else{
                        write_fifo_patient(&patient, fifo_queues_doctor[patient.doctor]);
                        // czekaj na lekarza
                        semafor_close(patient.semid);
                    }
                }

                // usunięcie pacjenta ze strefy gabinetów lekarskich
                semafor_close(global_semid);

                removeFromArrayInt(globalVars_adres->doctorZonePatientPID,&globalVars_adres->doctorZonePatientPIDsize ,patient.pid);

                semafor_open(global_semid);

            }else if(*patient_state == REGISTER_FAIL){
                // usunięcie pacjenta ze strefy rejestracji
                semafor_close(global_semid);

                removeFromArrayInt(globalVars_adres->registerZonePatientPID, &globalVars_adres->registerZonePatientPIDsize,patient.pid);

                semafor_open(global_semid);

                printf("PACJENT %s: %d (%s) odrzucono moją rejestrację (brak terminów).\n", vipStatusStr,patient.pid, patient.doctorStr);
            }

            semafor_close(global_semid);

            globalVars_adres->people_free_count+=patient.count;

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
        close(fifo_queues_doctor[i]);
        close(fifo_queues_doctor_vip[i]);
        char* fifo_doctor_name = fifo_queue_doctor[i];
        char* fifo_doctor_name_vip = fifo_queue_VIP_doctor[i];
        unlink(fifo_doctor_name);
        unlink(fifo_doctor_name_vip);
    }

    odlacz_pamiec(globalConst_adres);
    odlacz_pamiec(globalVars_adres);

    usun_pamiec(globalConst_memid);
    usun_pamiec(globalVars_memid);

    usun_semafor(global_semid);
    usun_semafor(raport_semid);
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

bool selectPatientVIP(){
    int r = rand() % 100;
    if( r < 20){
        return true;
    }
    return false;
}

void evacuate(int sig){
    // usunięcie pacjenta z pamięci dzielonej
    if(*patient_state == REGISTER || *patient_state == REGISTER_SUCCESS){
        semafor_close(global_semid);

        globalVars_adres->register_count -= patient.count;
        if(globalVars_adres->register_count < 0) globalVars_adres->register_count = 0;

        removeFromArrayInt(globalVars_adres->registerZonePatientPID, &globalVars_adres->registerZonePatientPIDsize ,patient.pid);

        semafor_open(global_semid);
    }else if(*patient_state != OUTSIDE && *patient_state != GO_HOME){
        semafor_close(global_semid);

        removeFromArrayInt(globalVars_adres->doctorZonePatientPID,&globalVars_adres->doctorZonePatientPIDsize ,patient.pid);

        semafor_open(global_semid);
        
        // zamknięcie fifo
        if(patient.vip){
            close(fifo_queues_doctor_vip[patient.doctor]);
        }else{
            close(fifo_queues_doctor[patient.doctor]);
        }
    }

    close(fifo_oknienko);
    
    // zmiana stanu
    *patient_state = GO_HOME;
    printf("PACJENT: %d (%s) ewakuuję się!\n",patient.pid, patient.doctorStr);

    // zwolnienie miejsca
    semafor_close(global_semid);

    globalVars_adres->people_free_count+=patient.count;

    semafor_open(global_semid);

    // go home
    goHomePatient(&patient, patient_state);
}