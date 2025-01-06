#!/bin/bash

./dyrektor &

./lekarz 0 &
./lekarz 1 &
./lekarz 2 &
./lekarz 3 &
./lekarz 4 &

sleep 3

./pacjent 6 &

./przychodnia
