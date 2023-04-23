#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Incorrect input, please enter N and M.");
        exit(1);
    }
    int N = atoi(argv[1]); // Amount of savages.
    int M = atoi(argv[2]); // Volume of the pot.
    int fd = shm_open("secondTask", O_CREAT | O_RDWR, 0666); // Creating unnamed memory.
    if (fd == -1) {
        perror("shm_open error");
        exit(1);
    }
    if (ftruncate(fd, sizeof(int)) == -1) {
        perror("ftruncate error");
        exit(1);
    }
    int *shared_mem = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap error");
        exit(1);
    }
    sem_t mutex, empty, full; // Creating unnamed semaphores.
    if (sem_init(&mutex, 1, 1) == -1 || sem_init(&empty, 1, M) == -1 || sem_init(&full, 1, 0) == -1) {
        perror("creation error");
        exit(1);
    }
    *shared_mem = M; // Initializing shared memory.
    for (int i = 0; i < N; i++) { // Savage processes.
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork error");
            exit(1);
        } else if (pid == 0) {
            while (1) {
                sem_wait(&full);
                sem_wait(&mutex);
                (*shared_mem)--; // Taking meat.
                printf("Savage #%d eating; %d pieces of meat left.\n", i, *shared_mem);
                sem_post(&mutex);
                sem_post(&empty);
                sleep(1);
                if (*shared_mem == 0) {
                    sem_wait(&mutex);
                    printf("Savage #%d wakes up the cook.\n", i);
                    *shared_mem = M; // Filling the pot.
                    sem_post(&mutex);
                }
            }
            exit(0);
        }
    }
    for (int i = 0; i < N; i++) { // Waiting for all processes to stop.
        wait(0);
    }
    if (sem_destroy(&mutex) == -1 || sem_destroy(&empty) == -1 || sem_destroy(&full) == -1 || shm_unlink("secondTask") == -1) { // Clearing the memory.
        perror("error occured during memory clearing");
        exit(1);
    }
    return 0;
}
