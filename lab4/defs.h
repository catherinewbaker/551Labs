//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN
//BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine W Baker

// general structures
// message struct
struct message {
    long type;
    int data;
};

// Structure for process statistics
struct process {
    int pid;
    int found;
    int tested;
    int skipped;
};

// Key definitions
#define MSG_KEY 34522     // Message Queue Key
#define SEM_KEY 34522     // Semaphore Key
#define SHM_KEY 34522     // Shared Memory Key

#define BM_SIZE (1 << 22)
#define PROCESS_SIZE 20
#define THREAD_SIZE 5
#define STATS_SIZE (20 * sizeof(struct process))
#define PERFECT_SIZE (20 * sizeof(int))
#define SHM_SIZE (size_t)BM_SIZE + PERFECT_SIZE + STATS_SIZE + sizeof(pid_t) + sizeof(int)

#define PID_TYPE 2
#define PERFECT_TYPE 3