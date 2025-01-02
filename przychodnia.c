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

#define FIFO_REJESTRACJA "fifo_rej1"
#define MAX_GEN_PATIENTS 1024

int semafor;
enum doctorType {POZ, KARDIOLOG, OKULISTA, PEDIATRIA, MED_PRAC};

struct Patient {
    pid_t pid;
    enum doctorType doctor;
    int semid;
};

static void utworz_nowy_semafor(struct Patient *patient);
static void semafor_close(struct Patient *patient);
static void semafor_open(struct Patient *patient);
static void usun_semafor(struct Patient *patient);

int main(int argc, char *argv[])
{
    struct Patient patient;

    

    // create FIFO
    if (mkfifo(FIFO_REJESTRACJA, 0600) == -1) {
        perror("PRZYCHODNIA: FIFO error");
    }

    // read only FIFO
    int fifo_oknienko = open(FIFO_REJESTRACJA, O_RDONLY);
    if (fifo_oknienko == -1) {
        perror("PRZYCHODNIA: Cant open FIFO");
        exit(1);
    }

    // read PID
    while(read(fifo_oknienko, &patient, sizeof(patient)) > 0){
        utworz_nowy_semafor(&patient);
        printf("PRZYCHODNIA: Rejestruję ... %d (%d)\n", patient.pid, patient.doctor);
        sleep(3);
        semafor_open(&patient);
    }
    
    close(fifo_oknienko);
    return 0;
}


// --------- SEMAFORY ------------------

static void utworz_nowy_semafor(struct Patient *patient)
  {
    key_t key = ftok("/tmp", patient->pid);
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    patient->semid=semget(key,1,0600|IPC_CREAT); //do projektu nalezy podac najnizsze prawa
    if (semafor==-1)
      {
        perror("Nie moglem utworzyc nowego semafora.\n");
        exit(EXIT_FAILURE);
      }
  }

  static void semafor_close(struct Patient *patient)
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

static void semafor_open(struct Patient *patient)
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

static void usun_semafor(struct Patient *patient)  
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
        printf("PACJENT: Semafor pacjenta %d zostal usuniety : %d\n",patient->pid, sem);
      }
  }