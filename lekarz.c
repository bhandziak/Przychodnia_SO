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

enum doctorType {POZ, KARDIOLOG, OKULISTA, PEDIATRIA, MED_PRAC};

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Podano niepoprawna liczbe argumentow.\n");
        return 1;
    }
    int arg1 = atoi(argv[1]);
    if(arg1 < 0 || arg1 > MED_PRAC){
        printf("Nieprawidlowa wartosc 1 argumentu.\n");
        return 1;
    }

    enum doctorType doctor = (enum doctorType)arg1;

    switch (doctor) {
        case POZ:
            printf("Wybrano lekarza POZ.\n");
            break;
        case KARDIOLOG:
            printf("Wybrano lekarza KARDIOLOG.\n");
            break;
        case OKULISTA:
            printf("Wybrano lekarza OKULISTA.\n");
            break;
        case PEDIATRIA:
            printf("Wybrano lekarza PEDIATRIA.\n");
            break;
        case MED_PRAC:
            printf("Wybrano lekarza MED_PRAC.\n");
            break;
        default:
            printf("Nieprawidłowy typ lekarza.\n");
            return 1;
    }

    return 0;
}