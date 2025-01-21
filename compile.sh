#!/bin/bash

#ipcrm -a
# ipcs

gcc -pthread pacjent.c common_def.c -o pacjent;
gcc rejestracja.c common_def.c -o rejestracja;
gcc lekarz.c common_def.c -o lekarz;
gcc -pthread dyrektor.c common_def.c -o dyrektor;