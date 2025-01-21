#!/bin/bash

sleep 2

./lekarz 0 & # typy lekarzy
./lekarz 5 &
./lekarz 1 &
./lekarz 2 &
./lekarz 3 &
./lekarz 4 &

sleep 3

./pacjent 200 & # max ilość pacjentów

./rejestracja 0 & # nr okienka
sleep 3
./rejestracja 1 