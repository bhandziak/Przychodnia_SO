#!/bin/bash

#ipcrm -a
# ipcs

gcc -pthread pacjent.c common_def.c -o pacjent;
gcc przychodnia.c common_def.c -o przychodnia;
gcc lekarz.c common_def.c -o lekarz;
gcc -pthread dyrektor.c common_def.c -o dyrektor;