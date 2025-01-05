#include "common_def.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/wait.h>

char* fifo_queue_doctor[] = {
    "fifo_POZ", "fifo_HEART", "fifo_EYE", "fifo_CHILD", "fifo_WORK", "fifo_EXAM"
};

// --------- SEMAFORY ------------------

void utworz_nowy_semafor(Patient *patient)
  {
    key_t key = ftok("/tmp", patient->pid);
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    patient->semid=semget(key,1,0600|IPC_CREAT); //do projektu nalezy podac najnizsze prawa
    if (patient->semid==-1)
      {
        perror("Nie moglem utworzyc nowego semafora.\n");
        exit(EXIT_FAILURE);
      }
  }

void semafor_close(Patient *patient)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(patient->semid,&bufor_sem,1); //1 -l.semafor
    if (zmien_sem==-1)
      {
        if(errno == EINTR){ //ubsluga bledu zatrzymania
        semafor_close(patient);
        }
        else
        {
        perror("Nie moglem zamknac semafora.\n");
        exit(EXIT_FAILURE);
        }
      }
    else
      {
        //printf("Semafor zostal zamkniety.\n");
      }
  }

void semafor_open(Patient *patient)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=SEM_UNDO; //SEM_UNDO semafor pamieta ile operacji wykonal, gdy je wykone cofnie je (uwaga na przepelnienie semafora)
    zmien_sem=semop(patient->semid,&bufor_sem,1);
    if (zmien_sem==-1) 
      {
        perror("Nie moglem otworzyc semafora.\n");
        exit(EXIT_FAILURE);
      }
    else
      {
        //printf("Semafor zostal otwarty.\n");
      }
  }

void usun_semafor(Patient *patient)  
  {
    int sem;
    sem=semctl(patient->semid,0,IPC_RMID);
    if (sem==-1)
      {
        perror("Nie mozna usunac semafora.\n");
        exit(EXIT_FAILURE);
      }
    else
      {
        //printf("PACJENT: Semafor pacjenta %d zostal usuniety : %d\n",patient->pid, sem);
      }
  }


  //------------------------PAMIEC----------------------


void utworz_pamiec_pacjent(Patient *patient)
  {
    key_t key = ftok("/tmp", patient->pid);
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

	patient->memid=shmget(key, sizeof(patientState), 0600 | IPC_CREAT);
    if (patient->memid==-1) 
    {
        perror("Problemy z utworzeniem pamieci dzielonej.\n");
        exit(EXIT_FAILURE);
    }
  }

patientState* przydziel_adres_pamieci_pacjent(Patient *patient)
  {
    patientState* state = (patientState*)shmat(patient->memid, NULL, 0);
    if (state == (void*)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    return state;
  }

void odlacz_pamiec_pacjent(Patient *patient, patientState* state)
  {
    int disconnect1=shmctl(patient->memid,IPC_RMID,0);

    int disconnect2=shmdt(state);
    if (disconnect1==-1 || disconnect2==-1)
    {
        perror("Problemy z odlaczeniem pamieci dzielonej.\n");
        exit(EXIT_FAILURE);
    }
  }


  // -------------------- FIFO --------------------

void create_fifo_queue(char* name){
    if (mkfifo(name, 0600) == -1) {
        perror("FIFO: bład tworzenia");
    }
}

int open_read_only_fifo(char* name){
    // read only FIFO
    int fifo_queue = open(name, O_RDONLY);
    if (fifo_queue == -1) {
        perror("FIFO: Nie można otworzyć FIFO\n");
        exit(1);
    }
    return fifo_queue;
}

int open_write_only_fifo(char* name){
    // write only FIFO
    int fifo_queue = open(name, O_WRONLY);
    if (fifo_queue == -1) {
        perror("FIFO: Nie można otworzyć FIFO\n");
        exit(1);
    }
    return fifo_queue;
}

int write_fifo_patient(Patient *patient, int queue){
    if (write(queue, patient, sizeof(Patient)) == -1) {
        perror("FIFO: błąd zapisu\n");
        close(queue);
        exit(1);
    }
}