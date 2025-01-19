#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <sys/types.h>
#include <stdbool.h>

#define FIFO_REJESTRACJA "fifo_rej1"
#define MAX_GEN_PATIENTS 1024

#define DOCTOR_COUNT 6
#define KEY_GLOBAL_CONST 10
#define KEY_GLOBAL_VARS 11

#define KEY_GLOBAL_SEMAPHORE 120
#define KEY_RAPORT_TXT 150

extern char* fifo_queue_doctor[];
extern char* fifo_queue_VIP_doctor[];
extern int queues_doctor_count;
extern char* doctor_name[];

typedef enum {
    POZ, KARDIOLOG, OKULISTA, PEDIATRIA, MED_PRAC
} doctorType;

typedef enum {
    OUTSIDE, REGISTER, REGISTER_FAIL, REGISTER_SUCCESS,
    QUEUE_POZ, QUEUE_HEART, QUEUE_EYE, QUEUE_CHILD, QUEUE_WORK, QUEUE_EXAM,
    GO_HOME,
    TAKE_EXAM,
    SECOND_DOCTOR_HEART = 21, SECOND_DOCTOR_EYE = 22, SECOND_DOCTOR_WORK = 24
} patientState;


typedef struct {
    pid_t pid;
    doctorType doctor;
    char doctorStr[50];
    int semid;
    int memid;
    bool vip;
    int count;
    int tidChild;
} Patient;

typedef struct {
    int N; // limit osób w przychodni
    int X[5]; // limit lekarzy
    int Tp; // czas otwarcia
    int Tk; // czas zamknięcia
    int idsemVars; // semafor dla pamięci dzielonej PublicVars
    int idsemRaport; // semafor dla pliku raport.txt
} ConstVars;

typedef struct {
    int people_free_count; // ilość wolnych miejsc w przychodni
    int register_count; // liczba osób przy okienkach rejestracji
    int X_free[6]; // liczba wolnych miejsc do konkretnego lekarzas
    int time; // aktualny czas

    int registerPID[2];

    int doctorPID[6];

    int outsidePatientPID[MAX_GEN_PATIENTS];
    int outsidePatientPIDsize;

    int registerZonePatientPID[MAX_GEN_PATIENTS];
    int registerZonePatientPIDsize;

    int doctorZonePatientPID[MAX_GEN_PATIENTS];
    int doctorZonePatientPIDsize;
} PublicVars;


int sumIntArray(int tab[], int lenght);
void goHomePatient(Patient *patient, patientState* patient_state);
void convertTimeToStr(int time, char* timeStr);

// DYNAMIC TABLE

void appendToArrayInt(int* array,int* size ,int element);
void removeFromArrayInt(int* array,int* size,  int value);
void printDynamicArrayInt(int* array,int size );

// SEMAFORY

void utworz_nowy_semafor_pacjent(Patient *patient);
int utworz_nowy_semafor(key_t key);
void semafor_close(int semid);
void semafor_open(int semid);
void usun_semafor(int semid);

// PAMIEC DZIELONA

void utworz_pamiec_pacjent(Patient *patient);
patientState* przydziel_adres_pamieci_pacjent(Patient *patient);
void odlacz_pamiec_pacjent(Patient *patient, patientState* state);

void* utworz_pamiec(key_t key, size_t size, int* memid_p);
void odlacz_pamiec(void* addr);
void usun_pamiec(int memid);

// KOLEJKA FIFO

void create_fifo_queue(char* name);
int open_read_only_fifo(char* name);
int open_write_only_fifo(char* name);
int write_fifo_patient(Patient *patient, int queue);

#endif