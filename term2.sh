#!/bin/bash

sleep 2

./lekarz 0 &
./lekarz 5 &
./lekarz 1 &
./lekarz 2 &
./lekarz 3 &
./lekarz 4 &

sleep 3

./pacjent 30 &

./przychodnia 0 &
sleep 3
./przychodnia 1 