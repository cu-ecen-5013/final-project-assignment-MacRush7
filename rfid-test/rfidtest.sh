#!/bin/bash

python3 Write.py && echo "Unique RFID card added...\n"
sleep 3
python3 Read.py && echo "User detected...\n"
echo "RFID TEST SUCCESSFUL"
