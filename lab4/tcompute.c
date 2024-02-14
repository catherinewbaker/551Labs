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
#include <pthread.h>

// Shared memory variables
struct process *pStats; // Array of process statistics
void *bitmap; // 2^22 bytes = 2^25 / 8
int start = -1, shmid = -1, semid = -1; // Shared memory ID and start number
int *perfectNums; // Array to store perfect numbers
struct message msg; // message setup
struct sembuf semOp; // semaphore buffer
int msgid = -1;
int statIndex;

// Mutex and condition variables
int N; // Shared N representing testable number
int processFlag = 0; // flag variable to account for multiple threads waking up
pthread_mutex_t Nmutex; // mutex hold on N
pthread_mutex_t flagMutex; // mutex hold on N
pthread_mutex_t shmMutex; // mutex hold on shared memory
pthread_cond_t freeN; // Signal that N is available
pthread_cond_t takenN; // Signal that N has been taken
volatile sig_atomic_t terminate;

// TIDs
pthread_t searchTid; // Search thread ID
pthread_t computeTids[THREAD_SIZE]; // Compute threads IDs
int threads[THREAD_SIZE]; // store thread ID (0, 1, 2,...) for printing

// handles sig INTR from manage
void handler(int sig) {
    terminate = 1; // Set termination flag

    // Notify all waiting threads
    pthread_cond_broadcast(&freeN);
    pthread_cond_broadcast(&takenN);

    // Join threads
    pthread_join(searchTid, NULL);
    for (int i = 0; i < THREAD_SIZE; i++) {
        pthread_join(computeTids[i], NULL);
    }

    // Cleanup resources
    pthread_mutex_destroy(&Nmutex);
    pthread_mutex_destroy(&shmMutex);
    pthread_mutex_destroy(&flagMutex);
    pthread_cond_destroy(&freeN);
    pthread_cond_destroy(&takenN);

    // Detach shared memory
    if (shmdt(bitmap) == -1) {
        perror("shmdt");
    }

    _exit(0); // Terminate the program
}

void setMap(int* bitmap, int num) {
    int byteCount = num / (8 * sizeof(int));
    int bitCount = num % (8 * sizeof(int));
    bitmap[byteCount] |= (1 << bitCount); // sets bit in byte position
}

int checkMap(int* bitmap, int num) {
    int byteIndex = num / (8 * sizeof(int));
    int bitIndex = num % (8 * sizeof(int));
    return (bitmap[byteIndex] & (1 << bitIndex)); // returns 1 if bit is set and 0 if not
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

// searching thread function
void* search(void* arg) {
    int num = start - 1;
    while (!terminate) {
        // Find a new number
        num++;
        if(num > (8 * BM_SIZE)){ // size of bitmap = (1 << 25) = 8 * (1 << 22)
            num = 1;
        }
        while (checkMap((int*)bitmap, num)) {
            if(num == start){
                terminate = 1;
                execl("report", "report", "-k", NULL);
            }
            num = (num + 1);
            if(num > 8 * BM_SIZE){ // also check for size while we cycle through numbers
                num = 1;
            }
            pthread_mutex_lock(&shmMutex); // Lock the mutex
            pStats[statIndex].skipped++;
            pthread_mutex_unlock(&shmMutex); // unlock the mutex
        }
        setMap((int *) bitmap, num);

        pthread_mutex_lock(&Nmutex); // Lock the mutex
        N = num; // Update N

        pthread_mutex_lock(&flagMutex); // Lock the flag mutex
        processFlag = 1; // Reset the flag after processing
        pthread_mutex_unlock(&flagMutex); // Unlock the flag mutex

        pthread_cond_signal(&freeN); // Signal compute to process the number
        //printf("SEARCH: Number %d ready for processing\n", N);
        //pthread_mutex_unlock(&Nmutex); // Unlock the mutex
        
        //pthread_mutex_lock(&Nmutex); // lock the mutex
        pthread_cond_wait(&takenN, &Nmutex); // Wait for compute to finish processing
        pthread_mutex_unlock(&Nmutex); // Unlock the mutex
    }
    return NULL;
}

void* compute(void* arg) {
    int tidIndex = *((int*)arg);
    while (!terminate) {
        pthread_mutex_lock(&Nmutex); // Lock the mutex

        // Wait for a number to be ready for processing
        while (processFlag == 0) { 
            pthread_cond_wait(&freeN, &Nmutex);
        }

        int num = N; // Get the number to process
        processFlag = 0; // Reset the flag after getting the number
        pthread_cond_signal(&takenN); // Signal search that the number has been taken
        //printf("COMPUTE: Thread %d processing num: %i\n", tidIndex, num);
        pthread_mutex_unlock(&Nmutex); // Unlock the mutex
        //printf("COMPUTE: testing num: %i\n", num);

        // Process the number
        if (isPerfect(num)) {
            //printf("COMPUTE: Thread %d found perfect number: %d\n", tidIndex, num);
            msg.type = PERFECT_TYPE; // set type
            msg.data = num; // and data
            msgsnd(msgid, &msg, sizeof(msg.data), 0); // send number to manage.c
            //printf("MESSAGE: message sent to manage\n");
            pthread_mutex_lock(&shmMutex); // Lock the mutex
            pStats[statIndex].found++;
            pthread_mutex_unlock(&shmMutex); // Lock the mutex
            //printf("SHMEM: updated shared memory\n");
        }
        pthread_mutex_lock(&shmMutex); // Lock the mutex
        pStats[statIndex].tested++;
        pthread_mutex_unlock(&shmMutex); // Lock the mutex 
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Reset signals to handler
    signal(SIGINT, handler);
    signal(SIGHUP, handler);
    signal(SIGQUIT, handler);

    // Pull initial values
    if (argc != 2) {
        fprintf(stderr, "USAGE: compute [startingNumber]\n");
        return 1;
    }
    start = atoi(argv[1]);
    terminate = 0;
    pid_t comPid = getpid();

    printf("1");
    // Shared memory access
    if ((shmid = shmget(SHM_KEY, (size_t) SHM_SIZE, IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        return 1;
    }
    // message q creation
    if ((msgid = msgget(MSG_KEY, IPC_CREAT | 0666))== -1) {
        perror("msgget");
        exit(1);
    }
    // semaphore creation
    if ((semid = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1) {
        perror("semget");
        exit(1);
    }
    // Cast data
    if ((bitmap = (shmat(shmid, 0, 0))) == (void *)-1) {
        perror("shmat");
        return 1;
    }
    perfectNums = bitmap + BM_SIZE;
    pStats = (struct process*)(perfectNums + PROCESS_SIZE);
    printf("2");

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

    // Initialize mutexes
    if (pthread_mutex_init(&Nmutex, NULL) != 0 || pthread_mutex_init(&flagMutex, NULL) != 0 || pthread_mutex_init(&shmMutex, NULL) != 0) {
        perror("Mutex init failed");
        exit(1);
    }
    //Initialize condition variables
    if (pthread_cond_init(&freeN, NULL) != 0 || pthread_cond_init(&takenN, NULL) != 0) {
        perror("Condition var init failed");
        exit(1);
    }
    printf("3");
    // Create and start threads
    pthread_create(&searchTid, NULL, search, NULL); // search thread
    // cycle through compute thread creation
    for (int i = 0; i < THREAD_SIZE; i++) {
        threads[i] = i;
        pthread_create(&computeTids[i], NULL, compute, &threads[i]); // setup to run compute() function
    }
    printf("4");
    // Wait for threads to finish
    if (pthread_join(searchTid, NULL) != 0) {
        perror("Failed to join search thread");
        exit(1);
    }
    for (int i = 0; i < THREAD_SIZE; i++) {
        if (pthread_join(computeTids[i], NULL) != 0) {
            perror("Failed to join compute thread");
            exit(1);
        }
    }
    printf("5");
    // Cleanup
    pthread_mutex_destroy(&Nmutex);
    pthread_mutex_destroy(&shmMutex);
    pthread_cond_destroy(&freeN);
    pthread_cond_destroy(&takenN);

    // Detach shared memory
    if (shmdt(bitmap) == -1) {
        perror("shmdt");
    }
    return 0;
}
