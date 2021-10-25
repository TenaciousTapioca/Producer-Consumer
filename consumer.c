/** consumer.c
 *  Nghia Dao
 *  Producer-Consumer Problem
 *  CS33211
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
 
#define bufferSize 2
 
// shared buffer between producer and consumer
struct sharedBuffer {
    char array[bufferSize];
};
 
int main() {
    // wait until producer has created shared memory
    int shm_fd;
    int shmCreated = 0;
    while (shmCreated == 0) {
        shm_fd = shm_open("/buffer", O_RDWR, 0666);
        if (shm_fd >= 0) { shmCreated = 1; }
    }
    
    // resize the shared memory 
    ftruncate(shm_fd, sizeof(struct sharedBuffer));
    
    // create a mapping between process's address space and shared memory
    struct sharedBuffer *bufferPtr = mmap(0, sizeof(struct sharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // semaphores for synchronization between the two processes
    sem_t *full = sem_open("full", O_CREAT, 0666, 0);
    sem_t *empty = sem_open("empty", O_CREAT, 0666, 2);
    sem_t *mutex = sem_open("mutex", O_CREAT, 0666, 1);
    
    // initialization before consumption
    int out = 0;
    int numOfLoops = 5;
    while (numOfLoops > 0) {
        sem_wait(full);
        sleep(rand() % 2 + 1);
        sem_wait(mutex);

        // Critical Section: accessing shared buffer
        bufferPtr->array[out] = ' ';
        if (out == (bufferSize - 1)) {
            out = 0;
        } else {
            ++out;
        }
        
        sem_post(mutex);
        printf("Consumer:\t[%c, %c]\n", bufferPtr->array[0], bufferPtr->array[1]);
        sem_post(empty);
        
        --numOfLoops;
    }
    
    printf("**Consumer has ended**\n");
    
    // close and unlink semaphores
    sem_close(full);
    sem_close(empty);
    sem_close(mutex);
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");
    
    // unmap shared memory
    munmap(bufferPtr, sizeof(struct sharedBuffer));
    close(shm_fd);
    shm_unlink("/buffer");
    
    printf("Press 'Enter' to end...\n");
    
    return 0;
}
