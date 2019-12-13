#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include "../include/semaphore.h"

typedef struct shared_data{
  int readers_count;
  int writers_count;
} Entry;

double expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1-u) / lambda;
}

int main(int argc, char **argv) {
    Entry *shared_data;                                                         // Array of entries in shared memory.
    key_t s_key;                                                                // Unique key used in shmget(...) function.

    int total_peers;                                                            // Total processes that will be used.
    int total_entries;                                                          // Total entries in the shared array.
    int rw_ratio;                                                               // Readers / Writers ratio
    int total_iterations;                                                       // Total possible iterations for a process.

    pid_t current_peer;                                                         // Used later to count and identify a process.
    int iterations;                                                             // Random number in [1,total_iterations].
    int peer_type;                                                              // Reader or writer functionality.
    int status = 0;                                                             // Used in waitpid.
    int entry_num;                                                              // No. of entry that each process will occupy.
    int pid;

    /* Will be used when all the child processes are done,
       and store the total reads and writes on the entries. */
    int total_read_count = 0;
    int total_write_count = 0;

    /* Semaphores */
    int mutex_sem;
    int write_sem;

    FILE *f = fopen("test.txt", "w");
    if (argc < 0) {
        printf("Please provide more arguments. Correct usage: run <peers> <ratio> <entries> <iterations>\n");
        exit(-1);
    }
    if (argc < 5) {
        printf("Please provide more arguments. Correct usage: run <peers> <ratio> <entries> <iterations>\n");
        exit(-1);
    }
    if (argc > 5) {
        printf("Please provide less arguments. Correct usage: run <peers> <ratio> <entries> <iterations>\n");
        exit(-1);
    }
    else {
        total_peers = atoi(argv[1]);
        rw_ratio = atoi(argv[2]);
        total_entries = atoi(argv[3]);
        total_iterations = atoi(argv[4]);
    }
    if (rw_ratio <= 0 || rw_ratio >= 100) {
        printf("Please provide a ratio amount in [1,99]!\n");
        exit(-1);
    }
    /* Produce a unique key for shmget */
    if ((s_key = ftok("src/coordinator.c", 'R')) == -1) {
        printf("ftok() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    /* Get an id for the wanted shared memory segment */
    int shared_mem_id = shmget(s_key,total_entries * sizeof(Entry), IPC_CREAT | 0666);
    if (shared_mem_id < 0) {
        printf("shmget() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    /* Attach the shared memory to a local array in the program. */
    shared_data = shmat(shared_mem_id, (void *)0, 0);
    if (shared_data == (Entry *)(-1)){
        printf("shmat() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    for(int i = 0; i < total_entries; ++i) {
        shared_data[i].readers_count = 0;
        shared_data[i].writers_count = 0;
    }

    /* Get a shared variable for the readers/writers algorithm. */
    int shared_readers_count_id = shmget(s_key,sizeof(int), IPC_CREAT | 0666);
    if (shared_readers_count_id < 0) {
        printf("shmget() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    int shared_readers_count = shmat(shared_readers_count_id, (void *)0, 0);
    if (shared_readers_count == (int *)(-1)){
        printf("shmat() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    /* Initialize the semaphores that will be used later. */
    write_sem = SEM_INIT((key_t)1234,1,1);
    if (write_sem < 0){
        printf("SEM_INIT() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    mutex_sem = SEM_INIT((key_t)2345,1,1);
    if (mutex_sem < 0){
        printf("SEM_INIT() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    /* Create <total_peers> child processes. */
    for(int i = 0; i < total_peers; i++){
        pid = fork();
        if(pid == 0){                                                           // If a child process is running, don't fork.
            current_peer += i;                                                  // Hold a number for each child process.
            break;
        }
        if (pid == -1) {
            printf("fork() returned error %d. Exiting...\n", errno);
            exit(-1);
        }
    }
    /* Seed must change for each process. */
    srand(time(NULL) ^ (getpid() << 16));

    if (pid == 0) {
        iterations = rand() % total_iterations + 1;
        int nreads = 0;                                                         // Total read operations for this process.
        int nwrites = 0;                                                        // Total write operations for this process.
        double peer_time = 0;                                                   // Total time for each process.

        for(int n = 0; n < iterations; n++) {
            clock_t start_t = clock();
            sleep(expo(0.9));
            peer_type = rand() % total_peers + 1;                               // Choose between reader and writer.

            entry_num = rand() % total_entries;                                 // Choose an entry from the shared memory segment.
            if (peer_type <= total_peers * rw_ratio / 100) {
                nreads++;
                SEM_DOWN(mutex_sem, 0);
                shared_readers_count++;
                /* Block writers from writing if
                   there's one process trying to read. */
                if (shared_readers_count == 1)
                    SEM_DOWN(write_sem, 0);
                SEM_UP(mutex_sem, 0);
                shared_data[entry_num].readers_count++;                         // Read from the shared memory.

                SEM_DOWN(mutex_sem, 0);
                shared_readers_count--;
                if (shared_readers_count == 0)                                  // Unblock a writer.
                    SEM_UP(write_sem, 0);
                SEM_UP(mutex_sem, 0);
            }
            else {
                nwrites++;
                SEM_DOWN(write_sem, 0);
                shared_data[entry_num].writers_count++;
                SEM_UP(write_sem, 0);
            }
            clock_t end_t = clock();
            /* Compute the time for the current iteration and add it to the other iterations' times. */
            double time_taken = (double)(end_t - start_t) / CLOCKS_PER_SEC + expo(0.9);
            peer_time += time_taken;
        }

        /* Print the statistics for the process before it ends. */
        double average_peer_time = peer_time / (double)iterations;
        fprintf(f, "Peer %3ld iterated over the memory %3ld times, "
                   "with average waiting time %f. Number of reads: %3ld, Number of writes: %3ld\n",
                    average_peer_time, nreads, nwrites);
        exit(0);
    }


    while ((pid = waitpid(-1,&status,0)) != -1);                                // Wait for child processes.

    /* Read and store the results from the entries. */
    for(int i = 0; i < total_entries; ++i) {
        total_read_count += shared_data[i].readers_count;
        total_write_count += shared_data[i].writers_count;
    }

    /* Print the final results. */
    fprintf(f, "\nCoordinator Test Results\n");
    fprintf(f, "------------------------\n");
    fprintf(f, "peers: %d\n",total_peers);
    fprintf(f, "entries: %d\n",total_entries);
    fprintf(f, "total iterations: %d\n",total_iterations);
    fprintf(f, "reads: %d\n",total_read_count);
    fprintf(f, "writes: %d\n",total_write_count);

    /* Detach and delete the shared memory segment. */
    if (shmdt(shared_data) < 0) {
        printf("shmdt() returned error %d. Exiting...\n", errno);
        exit(-1);
    }
    if (shmctl(shared_mem_id, IPC_RMID, 0) <0) {
        printf("shmctl() returned error %d. Exiting...\n", errno);
        exit(-1);
    }

    /* Delete the semaphores. */
    SEM_DEL(mutex_sem, 0);
    SEM_DEL(write_sem, 0);
    return 0;
}
