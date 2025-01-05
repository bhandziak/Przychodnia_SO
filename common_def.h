#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <sys/types.h>

#define FIFO_REJESTRACJA "fifo_rej1"
#define MAX_GEN_PATIENTS 1024

extern char* fifo_queue_doctor[];

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

// SEMAFORY

void utworz_nowy_semafor(Patient *patient);
void semafor_close(Patient *patient);
void semafor_open(Patient *patient);
void usun_semafor(Patient *patient);

// PAMIEC DZIELONA

void utworz_pamiec_pacjent(Patient *patient);
patientState* przydziel_adres_pamieci_pacjent(Patient *patient);
void odlacz_pamiec_pacjent(Patient *patient, patientState* state);

// KOLEJKA FIFO

void create_fifo_queue(char* name);
int open_read_only_fifo(char* name);
int open_write_only_fifo(char* name);
int write_fifo_patient(Patient *patient, int queue);

#endif