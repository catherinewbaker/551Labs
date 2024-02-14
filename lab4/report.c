//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN
//BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine W Baker
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include "defs.h"
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int shmid = -1;
    void * bitmap;
    int * perfectNums;
    struct process * pStats;
    pid_t * manPid;
    int * tracker;
    long tested = 0, skipped = 0, found = 0;

    // access shared memory
    if ((shmid = shmget(SHM_KEY, (size_t) SHM_SIZE, IPC_CREAT | 0666))== -1) {
        perror("shmget");
        exit(1);
    }
    // cast bitmap
    if ((bitmap = (shmat(shmid, 0, 0))) == (void *) -1) {
        perror("shmat");
        exit(1);
    }
    // cast rest of data
    perfectNums = bitmap + BM_SIZE;
    pStats = (struct process*) (perfectNums + PROCESS_SIZE);
    manPid = (pid_t*) (pStats + PROCESS_SIZE);
    tracker = (int *) (manPid + 1);

    // prevents printing multiple reports
    if(*tracker == 1){
        // if we've already called report then just wait for the compute who called this to die
        sleep(10);
        return 0;
    } else {
        *tracker = 1;  
    }
    
    printf("\n\n\nPerfect Number Found: ");
    // cycle through perfectNums array and print the perfect numbers
    for (int i = 0; i < PROCESS_SIZE; i++) {
        if (perfectNums[i] == 0){
            break;
        }
        printf("%d ", perfectNums[i]);
    }
    printf("\n");
    // compute processes
    for (int g = 0; g < PROCESS_SIZE; g++) {
        if (pStats[g].pid == -1){
            continue;
        }
        printf("pid(%i): found: %i, tested: %i, skipped: %i\n", pStats[g].pid, pStats[g].found, pStats[g].tested, pStats[g].skipped);
        found += pStats[g].found;
        tested += pStats[g].tested;
        skipped += pStats[g].skipped;
    }

    // Stats
    printf("Statistics:\n");
    printf("Total found:   %-15ld\n", found);
    printf("Total tested:  %-15ld\n", tested);
    printf("Total skipped: %-15ld\n\n\n", skipped);

    // kill manage
    if(argc > 1){
        if (strcmp(argv[1], "-k") == 0) {
            kill(*manPid, SIGINT);
        }
    }
    return 0;
}