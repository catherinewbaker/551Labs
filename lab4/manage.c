//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN
//BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine W Baker

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include "defs.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

// Global variables
int shmid = -1, msgid = -1, semid = -1; // ids
void * bitmap; // 2^22 bytes = 2^25 / 8
struct process * pStats; // Array of process statistics

// handles signal from report
void handler(int sig) {
    // kill each compute process
    for (int i = 0; i < PROCESS_SIZE; i++) {
        if (pStats[i].pid != -1){
            kill(pStats[i].pid, SIGINT);
        }
    }
    sleep(5);
    // delete semaphore
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        perror("semctl");
        exit(1);
    }
    // delete message q
    if (msgctl(msgid, IPC_RMID, 0) == -1) {
        perror("msgctl");
        exit(1);
    }
    // delete shared memory with bitmap
    if (shmdt(bitmap) == -1) {
        perror("shmdt");
        exit(1);
    }
    // delete final shared memory
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl");
        exit(1);
    }
    exit(0);
}

int main() {
    int * perfectNums; // Array to store perfect numbers
    struct sembuf semBuf; // semaphore buffer
    struct message msg; // message structure
    pid_t * manPid; // manage.c pid
    int * tracker; // makes sure that compute.c only signals report.c to print once

    // reset signals to handler
    signal(SIGINT, handler);
    signal(SIGHUP, handler);
    signal(SIGQUIT, handler);

    // Create IPC elements
    // create shared memory
    if ((shmid = shmget(SHM_KEY, (size_t)SHM_SIZE, IPC_CREAT | 0666))== -1) {
        perror("shmget");
        exit(1);
    }
    // create message q
    if ((msgid = msgget(MSG_KEY, IPC_CREAT | 0666))== -1) {
        perror("msgget");
        exit(1);
    }
    // create semaphore
    if ((semid = semget(SEM_KEY, 1 ,IPC_CREAT | 0666))== -1) {
        perror("semget");
        exit(1);
    }
    
    // cast bitmap
    if ((bitmap = (shmat(shmid, 0, 0))) == (void *) -1) {
        perror("shmat");
        exit(1);
    }
    // cast the rest of the data
    perfectNums = bitmap + BM_SIZE;
    pStats = (struct process*) (perfectNums + PROCESS_SIZE);
    manPid = (pid_t*) (pStats + PROCESS_SIZE);
    tracker = (int *) (manPid + 1);
    *manPid = getpid(); // get current pid
    *tracker = 0;
    
    // initialize pStats and perfectNums
    for (int y = 0; y < PROCESS_SIZE; y++) {
        perfectNums[y] = 0; // 0 will never be perfect so it's a good placeholder
    }
    for (int j = 0; j < PROCESS_SIZE; j++) {
        pStats[j].pid = -1; // in case 0 can be a pid
        pStats[j].tested = 0;
        pStats[j].skipped = 0;
        pStats[j].found = 0;
    }

    // semaphore that unlocks things
    semBuf.sem_num = 0;
    semBuf.sem_op = 1;
    semBuf.sem_flg = 0;  
    
    // continue receiving and organizing messages til death
    while (1) {
        msgrcv(msgid, &msg, sizeof(msg.data), 0, 0);
        if (msg.type == PID_TYPE) {
            // find pStats row
            for (int i = 0; i < PROCESS_SIZE; i++) {
                if (pStats[i].pid == -1) {
                    pStats[i].pid = msg.data;
                    break;
                }
                // if we don't have enough space for the new compute process then kill it
                if(i == PROCESS_SIZE - 1){
                    kill(msg.data, SIGINT);
                }
            }
            // unlock semaphore
            if (semop(semid, &semBuf, 1) == -1) {
                perror("semop");
                exit(1);
            }
        } else if (msg.type == PERFECT_TYPE) { // otherwise if we were sent a perfect number
            int curr = msg.data;
            for (int i = 0; i < PROCESS_SIZE; i++) {
                // if there's free space then add it to the array
                if (perfectNums[i] == 0) {
                    perfectNums[i] = curr;
                    break;
                } else {
                    // if the number is already listed then skip the addition
                    if (perfectNums[i] == curr) {
                        break;
                    } else if(perfectNums[i] > curr){
                        int holder = perfectNums[i];
                        perfectNums[i] = curr;
                        curr = holder;
                    }
                }
            }
        }
    }

    handler(SIGINT);
    return 0;
}
