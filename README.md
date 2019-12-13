# Readers-Writers Problem

For more information about the problem, please visit: https://en.wikipedia.org/wiki/Readers-writers_problem

## About this project
This project is a simulation of the classic readers-writers problem, where a number of processes are created and executed a number of times. In each iteration, a role is chosen for a process at random, either reader or writer, and an entry in the shared memory segment is occupied for some random given time, according to the exponential distribution.

## Installing and running

To compile the program, type

```
make
```
at the project's main directory.

To run the program, type

```
./run <processes> <ratio> <entries> <iterations>
```

where:

- *processes* is the number of processes that will be created for the simulation,

- *ratio* is a number between 1 and 99, representing the analogy between the readers and writers,

- *entries* is the number of entries in the shared memory, and

- *iterations* is an upper bound for the times a process will be executed.
