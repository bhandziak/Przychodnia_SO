#!/usr/bin/env bash

sleep 0.3

./lekarz 0 & # typy lekarzy
./lekarz 5 &
./lekarz 1 &
./lekarz 2 &
./lekarz 3 &
./lekarz 4 &

sleep 0.3

./pacjent 50 & # max ilość pacjentów

./rejestracja 0 & # nr okienka

./rejestracja 1 