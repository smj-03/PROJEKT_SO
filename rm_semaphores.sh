#!/bin/bash

semids=$(ipcs -s | awk 'NR>3 {print $2}')

if [ -n "$semids" ]; then
    echo "$semids" | while read semid; do
        echo "Removing semaphore with semid: $semid"
        ipcrm -s $semid
    done
else
    echo "No semaphores found."
fi