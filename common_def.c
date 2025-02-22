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
#include <time.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

#include <string.h>

char* fifo_queue_doctor[] = {
    "fifo_POZ", "fifo_HEART", "fifo_EYE", "fifo_CHILD", "fifo_WORK"
};

char* fifo_queue_VIP_doctor[] = {
    "fifo_POZ_VIP", "fifo_HEART_VIP", "fifo_EYE_VIP", "fifo_CHILD_VIP", "fifo_WORK_VIP"
};

int queues_doctor_count = sizeof(fifo_queue_doctor) / sizeof(fifo_queue_doctor[0]);

char* doctor_name[] = {
    "POZ", "Kardiolog", "Okulista", "Pediatria", "Medycyna pracy"
};

// funkcja sumująca wartości z tablicy

int sumIntArray(int tab[], int lenght){
  int sum = 0;
  for(int i = 0; i < lenght; i++ ){
    sum += tab[i];
  }
  return sum;
}

// Funkcja kończąca pracę pacjenta
// IN:
//   - wskaźnik na Patient
//   - wskaźnik na stan pacjenta

void goHomePatient(Patient *patient, patientState* patient_state){
  *patient_state = GO_HOME;
  printf("PACJENT: %d (%s) koniec - idę do domu.\n", patient->pid, patient->doctorStr);

  usun_semafor(patient->semid);
  odlacz_pamiec_pacjent(patient, patient_state);
  exit(0);
}

// Funkcja zamieniająca czas na char*
// IN:
//   - czas w int
//   - wskaźnik na timeStr

void convertTimeToStr(int time, char* timeStr){
  int minutes = time % 60;
  int hours = (time / 60 + 6) % 24;

  snprintf(timeStr, 6, "%02d:%02d", hours, minutes);
}

// ------------------------ Dynamic Array

// Dodanie elementu do dynamicznej tablicy

void appendToArrayInt(int* array,int* size ,int element){
  if(*size < MAX_GEN_PATIENTS){
    array[*size] = element;
    (*size)++;
  }
  else{
    printf("Array error: przekroczono pojemność tablicy\n");
  }

}

// Usunięcie elementu z dynamicznej tablicy

void removeFromArrayInt(int* array,int* size,  int value) {
    if (*size == 0) {
        return;
    }

    int index = -1;

    for (int i = 0; i < *size; i++) {
        if (array[i] == value) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    for (int i = index; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }

    (*size)--;
}

// Drukowanie dynamicznej tablicy

void printDynamicArrayInt(int* array,int size ){
  for(int i=0; i < size; i++){
    printf("%d", array[i]);

    if(i != size - 1){
      printf(",");
    }
  }
  printf("\n");
}

// --------- SEMAFORY ------------------

// Funkcja tworzy nowy semafor dla pacjenta, który jest przypisany do procesu pacjenta.
// Wykorzystuje identyfikator PID pacjenta do wygenerowania unikalnego klucza semafora.
// IN:
//    - wskaźnik na Patient

void utworz_nowy_semafor_pacjent(Patient *patient)
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

// Funkcja tworzy nowy semafor przy użyciu podanego klucza
// IN:
//    - unikalny klucz

int utworz_nowy_semafor(key_t key){
  int semid = semget(key,1,0600|IPC_CREAT); 
  if (semid==-1)
  {
    perror("Problemy z utworzeniem nowego semafora.\n");
    exit(EXIT_FAILURE);
  }

  semctl(semid, 0, SETVAL, 1);
  return semid;
}

// Funkcja zamykająca semafor (operacja 'wait')
// IN: 
//    -semid - identyfikator semafora

void semafor_close(int semid)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=SEM_UNDO;
    zmien_sem=semop(semid,&bufor_sem,1); //1 -l.semafor
    if (zmien_sem==-1)
      {
        if(errno == EINTR){ //ubsluga bledu zatrzymania
        semafor_close(semid);
        }
        if (errno == EOVERFLOW) {
            printf("Przekroczenie zakresu wartości semafora (semid = %d)\n", semid);
        }
        else
        {
        printf("semmid %d\n",semid);
        }
      }
    else
      {
        //printf("Semafor zostal zamkniety.\n");
      }
  }

// Funkcja otwierająca semafor (operacja 'signal')
// IN: 
//    -semid - identyfikator semafora

void semafor_open(int semid)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=SEM_UNDO; //SEM_UNDO semafor pamieta ile operacji wykonal, gdy je wykone cofnie je (uwaga na przepelnienie semafora)
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
      {
        if (errno == EOVERFLOW) {
            printf("Przekroczenie zakresu wartości semafora (semid = %d)\n", semid);
        }
        printf("semmid %d\n",semid);
      }
    else
      {
        //printf("Semafor zostal otwarty.\n");
      }
  }

// Funkcja usuwająca semafor
// IN: 
//    -semid - identyfikator semafora

void usun_semafor(int semid)  
  {
    int sem;
    sem=semctl(semid,0,IPC_RMID);
    if (sem==-1)
      {
        printf("semmid %d\n",semid);
        perror("Nie mozna usunac semafora.\n");
        exit(EXIT_FAILURE);
      }
    else
      {
        //printf("PACJENT: Semafor pacjenta %d zostal usuniety : %d\n",patient->pid, sem);
      }
  }


  //------------------------PAMIEC----------------------

// Funkcja tworząca pamięć dzieloną dla stanu pacjenta
// IN: 
//  -wskaźnik Patient

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
        perror("Problemy z utworzeniem pamieci dzielonej PACJENT.\n");
        exit(EXIT_FAILURE);
    }
  }

// Funkcja przydzielająca adres pamięci dzielonej dla stanu pacjenta
// IN: 
//  -wskaźnik Patient
// OUT:
//   - adres patientState*

patientState* przydziel_adres_pamieci_pacjent(Patient *patient)
  {
    patientState* state = (patientState*)shmat(patient->memid, NULL, 0);
    if (state == (void*)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    return state;
  }

// Funkcja odłączająca pamięć dzieloną stanu pacjenta
// IN: 
//  -wskaźnik Patient
//   -adres patientState*

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

// DYREKTOR

// Funkcja tworzy pamięć dzieloną i zwraca do niej adres
// IN: 
// - key - unikalny klucz 
// - size - rozmiar struktury
// - wskaźnik do memid_p id pamięci dzielonej

void* utworz_pamiec(key_t key, size_t size, int* memid_p) {
    int memid = shmget(key, size, 0600 | IPC_CREAT);
    if (memid == -1) {
        perror("Problemy z utworzeniem SZCZEGOLNEJ pamieci dzielonej.\n");
        //exit(EXIT_FAILURE);
    }
    *memid_p = memid;

    void* addr = shmat(memid, NULL, 0);
    if (addr == (void*)(-1)) {
        perror("Problemy z adresem SZCZEGOLNEJ pamieci dzielonej\n");
        //exit(EXIT_FAILURE);
    }

    return addr;
}

// odłącza pamięć dzieloną

void odlacz_pamiec(void* addr) {
    if (shmdt(addr) == -1)
    {
        perror("Problemy z odlaczeniem SZCZEGOLNEJ pamieci dzielonej.\n");
        //exit(EXIT_FAILURE);
    }
}

// usuwa pamięć dzieloną

void usun_pamiec(int memid) {
    if (shmctl(memid, IPC_RMID, 0) == -1) {
        perror("Problemy z usunięciem SZCZEGOLNEJ pamieci dzielonej.\n");
        //exit(EXIT_FAILURE);
    }
}

// -------------------- FIFO --------------------

// tworzy kolejkę fifo na podstawie nazwy str

void create_fifo_queue(char* name){
    if (access(name, F_OK) >= 0) {
        return;
    }

    if (mkfifo(name, 0600) == -1) {
        perror("FIFO: bład tworzenia");
    }
}

// otwiera kolejkę fifo w trybie O_RDONLY | O_NONBLOCK na podstawie nazwy str

int open_read_only_fifo(char* name){
    // read only FIFO
    int fifo_queue = open(name, O_RDONLY | O_NONBLOCK);
    if (fifo_queue == -1) {
        perror("FIFO: Nie można otworzyć FIFO\n");
        exit(1);
    }
    return fifo_queue;
}

// otwiera kolejkę fifo w trybie O_WRONLY na podstawie nazwy str

int open_write_only_fifo(char* name){
    // write only FIFO
    int fifo_queue = open(name, O_WRONLY);
    if (fifo_queue == -1) {
        perror("FIFO: Nie można otworzyć FIFO\n");
        exit(1);
    }
    return fifo_queue;
}

// zapisuje struktuję pacjenta do kolejki
// IN:
//    - wskaźnik do struktury Patient
//     - deskryptor pliku FIFO

int write_fifo_patient(Patient *patient, int queue){
    if (write(queue, patient, sizeof(Patient)) == -1) {
        perror("FIFO: błąd zapisu\n");
        close(queue);
        exit(1);
    }
}