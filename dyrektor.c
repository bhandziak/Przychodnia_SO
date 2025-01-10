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
    int X1 = 3;
    int X2 = 1;
    int X3 = 1;
    int X4 = 1;
    int X5 = 1;

    int Xvals[] = {X1, X2, X3, X4, X5};

    memcpy(globalConst.X, Xvals, sizeof(Xvals));

    globalConst.N = 15;
    globalConst.Tp = 0;
    globalConst.Tk = 3600;

    // globalConst_adres
  
    int globalConst_memid;
    globalConst.idsemVars = utworz_nowy_semafor(KEY_GLOBAL_SEMAPHORE);

    ContsVars* globalConst_adres = (ContsVars*)utworz_pamiec(KEY_GLOBAL_CONST, sizeof(ContsVars),&globalConst_memid);
    *globalConst_adres = globalConst;


    odlacz_pamiec(globalConst_adres);

    // globalVars_adres

    PublicVars publicVars;
    memset(&publicVars, 0, sizeof(PublicVars));
    int Xvals2[] = {X1, X2, X3, X4, X5, X1};
    memcpy(publicVars.X_free, Xvals2, sizeof(Xvals2));
    publicVars.people_free_count = globalConst.N;

    int globalVars_memid;
    PublicVars* globalVars_adres = (PublicVars*)utworz_pamiec(KEY_GLOBAL_VARS, sizeof(PublicVars), &globalVars_memid);
    *globalVars_adres = publicVars;


    odlacz_pamiec(globalVars_adres);
    return 0;
}