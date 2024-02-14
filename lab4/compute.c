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
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/types.h>

struct process * pStats; // Array of process statistics
void * bitmap; // 2^22 bytes = 2^25 / 8
int statIndex; // keeps track of our stats pid index
int start = -1, shmid = -1, msgid = -1, semid = -1; // ids
int * perfectNums; // Array to store perfect numbers

// handles signal from report
void handler(int sig) {
    // note pid terminated
    pStats[statIndex].pid = -1;
    // delete semaphore
    if (semctl(semid, 0, IPC_RMID, 0)) {
        perror("semctl");
        exit(1);
    }
    // delete message q
    if (msgctl(msgid, IPC_RMID, 0) == -1) {
        perror("msgctl");
        exit(1);
    }
    // delete bitmap cast on shmem
    if (shmdt(bitmap) == -1) {
        perror("shmdt");
        exit(1);
    }
    // Terminate the program
    _exit(0);
}

void setMap(int* bitmap, int num) {
    int byteCount = num / (8 * sizeof(int));
    int bitCount = num % (8 * sizeof(int));
    bitmap[byteCount] |= (1 << bitCount); // sets bit in byte position
}

int checkMap(int* bitmap, int num) {
    int byteIndex = num / (8 * sizeof(int));
    int bitIndex = num % (8 * sizeof(int));
    return !(bitmap[byteIndex] & (1 << bitIndex)); // returns 0 if bit is set and 1 if not
}

int isPerfect(int num) {
    int sum = 0;
    for (int i = 1; i < num; i++){
        if ((num % i) == 0){
            sum += i;
        }
    }
    if (sum == num && num > 1) {
        return num;
    } else {
        return 0;
    }
}

int main(int argc, char *argv[]) {
    // reset signals to handler
    signal(SIGINT, handler);
    signal(SIGHUP, handler);
    signal(SIGQUIT, handler);

    struct sembuf semOp; // semaphore buffer
    struct message msg; // message setup

    // pull initial values
    if (argc != 2) {
        fprintf(stderr, "USAGE: compute [startingNumber]\n");
    }
    start = atoi(argv[1]);
    pid_t comPid = getpid();

    // ID Access
    // shared memory access
    if ((shmid = shmget(SHM_KEY, (size_t) SHM_SIZE, IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        exit(1);
    }
    // message q creation
    if ((msgid = msgget(MSG_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }
    // semaphore creation
    if ((semid = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1) {
        perror("semget");
        exit(1);
    }

    // Cast data
    // starting with bitmap
    if ((bitmap = (shmat(shmid, 0, 0))) == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    // cast rest of data to bitmap
    perfectNums = bitmap + BM_SIZE;
    pStats = (struct process*)(perfectNums + PROCESS_SIZE);
    
    // send PID to manage
    msg.type = PID_TYPE;
    msg.data = comPid;
    msgsnd(msgid, &msg, sizeof(msg.data), 0);

    // Semaphore lock
    semOp.sem_num = 0;  
    semOp.sem_op = -1;
    semOp.sem_flg = 0;  
    if (semop(semid, &semOp, 1) == -1) { // locks til manage releases it
        perror("semop");
        exit(1);
    }

    // find index of current pid and store in statIndex
    for(int y = 0; y < 20; y++){
        if(pStats[y].pid == comPid){
            statIndex = y;
            break;
        }
    }

    // setup vars
    int num = start;
    // find perfect numbers
    while(1) {
        if (num >= 8 * BM_SIZE){ // change to big num later (8* BM_SIZE)
            num = 0;
        } 
        // testing and checking numbers
        if (checkMap(bitmap, num)) { // true if bit is not set, false if it is
            pStats[statIndex].tested += 1; // add one because the number is testable
            if(isPerfect(num)){ // check if it's perfect
                pStats[statIndex].found += 1; // if yes then add 1 to perfect numbers found
                msg.type = PERFECT_TYPE; // set type
                msg.data = num; // and data
                msgsnd(msgid, &msg, sizeof(msg.data), 0); // send number to manage.c
            }
            setMap(bitmap, num); // set the number as checked in the bitmap
        } else { // if the number has already been checked
            pStats[statIndex].skipped += 1; // skip it
        }
        num++; // move to the next number
        if(num == start){ // break if we reach the start again
            break;
        }
    }
    // when done call report.c
    if(num == start){
        execl("report", "report", "-k", NULL);
    }
    return 0;
}