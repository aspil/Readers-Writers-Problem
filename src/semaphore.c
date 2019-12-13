#include "../include/semaphore.h"

int SEM_INIT(key_t key, int n_sems, int val){
    union semun sem_un;
    int sem_id;

    if ((key < 0) || (n_sems <= 0)) {
        printf("SEM_INIT() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    if ((sem_id = semget(key, n_sems, 0666 | IPC_CREAT)) < 0) {
        printf("semget() used in SEM_INIT() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    sem_un.val = val;
    for(int i = 0; i < n_sems; i++){
        if (semctl(sem_id,i,SETVAL,sem_un) < 0) {
            printf("semctl() used in SEM_INIT() returned error %d. Exiting...\n", errno);
            exit(-1);
        }
    }
    return sem_id;
}
void SEM_DEL(int sem_id, int n_sems){
    if(sem_id < 0) {
        printf("SEM_DEL() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    if(semctl(sem_id, n_sems, IPC_RMID, 0) < 0) {
        printf("semctl() used in SEM_DEL() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
}

void SEM_DOWN(int sem_id,int sem_num){
    struct sembuf sem_b;
    if((sem_id < 0) || (sem_num < 0)) {
        printf("SEM_DOWN() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    sem_b.sem_num = sem_num;
    sem_b.sem_op = -1;
    sem_b.sem_flg = 0;

    if (semop(sem_id, &sem_b, 1) < 0) {
        printf("semop() used in SEM_DOWN() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
}
void SEM_UP(int sem_id, int sem_num){
    struct sembuf sem_b;
    if((sem_id < 0) || (sem_num < 0)) {
        printf("SEM_UP() used in SEM_UP() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    sem_b.sem_num = sem_num;
    sem_b.sem_op = 1;
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) < 0) {
        printf("semop() used in SEM_UP() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
}
