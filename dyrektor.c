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


int main(int argc, char *argv[])
{
    ContsVars globalConst;
    globalConst.N = 30;
    globalConst.K = 5;
    globalConst.X1 = 5;
    globalConst.X2 = 5;
    globalConst.X3 = 5;
    globalConst.X4 = 5;
    globalConst.X5 = 5;

    globalConst.Tp = 0;
    globalConst.Tk = 3600;

    // create globalConst_adres
  
    ContsVars* globalConst_adres = (ContsVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ContsVars));
    *globalConst_adres = globalConst;

    printf("Test zmiennej globalnej %d\n", globalConst_adres->N);

    odlacz_pamiec(globalConst_adres,KEY_GLOBAL_CONST, sizeof(ContsVars));

    return 0;
}