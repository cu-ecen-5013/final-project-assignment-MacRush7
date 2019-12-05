#!/bin/bash

sudo python3 Write.py && echo "Unique RFID card added...\n"

sudo python3 Read.py && echo "User detected...\n"

echo "RFID TEST SUCCESSFUL"
