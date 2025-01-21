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

shmids=$(ipcs -m | awk 'NR>3 {print $2}')

if [ -n "$shmids" ]; then
    echo "$shmids" | while read shmid; do
        echo "Removing shared memory segment with shmid: $shmid"
        ipcrm -m $shmid
    done
else
    echo "No shared memory segments found."
fi

msqids=$(ipcs -q | awk 'NR>3 {print $2}')

if [ -n "$msqids" ]; then
    echo "$msqids" | while read msqid; do
        echo "Removing message queue with shmid: $msqid"
        ipcrm -q $msqid
    done
else
    echo "No message queues found."
fi