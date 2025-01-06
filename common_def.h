#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <sys/types.h>

#define FIFO_REJESTRACJA "fifo_rej1"
#define FIFO_REJESTRACJA2 "fifo_rej2"
#define MAX_GEN_PATIENTS 1024

#define KEY_GLOBAL_CONST 10

extern char* fifo_queue_doctor[];
extern int queues_doctor_count;
extern char* doctor_name[];

typedef enum {
    POZ, KARDIOLOG, OKULISTA, PEDIATRIA, MED_PRAC
} doctorType;

typedef enum {
    OUTSIDE, REGISTER, REGISTER_FAIL, REGISTER_SUCCESS,
    QUEUE_POZ, QUEUE_HEART, QUEUE_EYE, QUEUE_CHILD, QUEUE_WORK, QUEUE_EXAM,
    GO_HOME
} patientState;


typedef struct {
    pid_t pid;
    doctorType doctor;
    int semid;
    int memid;
} Patient;

typedef struct {
    int N; // limit osób w przychodni
    int K; // moment otwarcia 2 okienka
    int X1; // limit dla lekarzy
    int X2;
    int X3;
    int X4;
    int X5;
    int Tp; // czas otwarcia
    int Tk; // czas zamknięcia
} ContsVars;

typedef struct {
    int people_count; // liczba osób w przychodni
    int register_count; // liczba osób przy okienkach rejestracji
    int X1_c[2]; // liczba obsłużonych pacjentów dla lekarzy
    int X2_c;
    int X3_c;
    int X4_c;
    int X5_c;
    int time; // aktualny czas
    int X1_open[2]; // czy otwarty lekarz
    int X2_open;
    int X3_open;
    int X4_open;
    int X5_open;
} PublicVars;


// SEMAFORY

void utworz_nowy_semafor(Patient *patient);
void semafor_close(Patient *patient);
void semafor_open(Patient *patient);
void usun_semafor(Patient *patient);

// PAMIEC DZIELONA

void utworz_pamiec_pacjent(Patient *patient);
patientState* przydziel_adres_pamieci_pacjent(Patient *patient);
void odlacz_pamiec_pacjent(Patient *patient, patientState* state);

void* utworz_pamiec(key_t key, size_t size);
void odlacz_pamiec(void* addr, key_t key, size_t size);

// KOLEJKA FIFO

void create_fifo_queue(char* name);
int open_read_only_fifo(char* name);
int open_write_only_fifo(char* name);
int write_fifo_patient(Patient *patient, int queue);

#endif