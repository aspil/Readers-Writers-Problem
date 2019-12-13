#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
/* Initialize a semaphore set. */
int SEM_INIT(key_t, int, int);
/* Delete semaphores */
void SEM_DEL(int, int);
void SEM_DOWN(int, int);
void SEM_UP(int, int);
