// Author: Jason Khuu
// OU 4X4: Khuu0000
// OU ID: 113527144
// ID: cs054.cs.ourcloud.ou.edu

// Semaphores will be used in this program to synchronize the processes.

#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>

// Define a shared memory key so that UNIX can identify memory segments
#define SHM_KEY ((key_t) 2000)

// Define a semaphore key 
#define SEM_KEY ((key_t) 4000)

// Define number of semaphores
#define NUM_SEMS 1

// Global variables for semaphore
int semid; // semaphore id
static struct sembuf OP = {0,-1,0}; // semaphore operation
static struct sembuf OV = {0,1,0}; // semaphore operation
struct sembuf *P =&OP; // pointer to semaphore operation
struct sembuf *V =&OV; // pointer to semaphore operation

// Define a shared memory
typedef struct {
    int value;
} shared_data;

shared_data *total; // shared memory

// Semaphore union
typedef union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} semunion;

// POP function to protect the critical section
int POP() {
    int status;
    status = semop(semid, P, 1);
    return status;
}

// VOP function to release the critical section
int VOP() {
    int status;
    status = semop(semid, V, 1);
    return status;
}

// First Process which increments up to 100000
void process1() {
    int i;
    for (i = 0; i < 100000; i++) {
       POP();
       total->value++;
       VOP();
    }
    // Print the parent 
    printf("From Process 1: Counter = %d\n", total->value);
    exit(0);
}

// Second Process increments up to 200000
void process2() {
    int i;
    for (i = 0; i < 200000; i++) {
       POP();
       total->value++;
       VOP();
    }
    // Print the parent
    printf("From Process 2: Counter = %d\n", total->value);
    exit(0);
}

// Third Process increments up to 300000
void process3() {
    int i;
    for (i = 0; i < 300000; i++) {
       POP();
       total->value++;
       VOP();
    }
    // Print Parent
    printf("From Process 3: Counter = %d\n", total->value);
    exit(0);
}

// Foruth Process increments up to 500000
// This process is also used for the total sum of the counter
void process4() {
    int i;
    for (i = 0; i < 500000; i++) {
       POP();
       total->value++;
       VOP();
    }
    // Print the parent
    printf("From Process 4: Counter = %d\n", total->value);
    exit(0);
}

// Main function
int main() {
    // Declare variables
    int shmid, pid1=0, pid2=0, pid3=0, pid4=0, ID=0, status, value, value1; 
    semunion semctl_arg;
    semctl_arg.val = 1;
    char *shmadd; 
    shmadd = (char *) 0;
    
    // Create semaphores 
    semid = semget(SEM_KEY, NUM_SEMS, IPC_CREAT | 0666);
    if(semid < 0) printf("Error in creating the semaphore./n");

    // Initialize semaphore 
    value1 = semctl(semid, 0, SETVAL, semctl_arg);
    value = semctl(semid, 0, GETVAL, semctl_arg);
    if (value < 0) printf("Error detected in SETVAL.\n");

    // Create and connect to a shared memory segment
    if ((shmid = shmget (SHM_KEY, sizeof(int), IPC_CREAT | 0666)) < 0){
        perror ("shmget");
        exit (1);
    }

    if ((total = (shared_data *) shmat (shmid, shmadd, 0)) == (shared_data *) -1) {
        perror ("shmat");
        exit (0);
    }
    
    // Initialize the total value to 0
    total->value = 0;

    // Create and call process1()
    if ((pid1 = fork()) == 0) {
        process1();
    }

    // Create and call process2()
    else if ((pid2 = fork()) == 0) {
        process2(); 
    }

    // Create and call process3() 
    else if ((pid3 = fork()) == 0) {
        process3();
    }

    // Create and call process4()
    else if ((pid4 = fork()) == 0) {
        process4();
    }
    
    // Wait for children to finish and print child results 
    for (int i = 0; i < 4; i++) {
        ID = wait(&status); 
        printf("Child with ID: %d has just exited.\n", ID);
    }
    
    printf("\n");

    // Print the final value of the counter
    printf("Final Counter Value: %d\n", total->value);

    // Disconnect the shared memory and remove it
    shmdt(total);
    shmctl(shmid,IPC_RMID,NULL);
    
    printf("\n");

    // De-allocate semaphore
    semctl_arg.val = 0;
    status = semctl(semid, 0, IPC_RMID, semctl_arg);
    if(status < 0) printf("Error in removing the semaphore.\n");
    
    printf("End of Simulation\n");
    return 0;
}
