#!/bin/bash

gnome-terminal -- bash -c "./dyrektor; echo 'Press Enter to exit'; read"

gnome-terminal -- bash -c "./term2.sh; exec bash"
