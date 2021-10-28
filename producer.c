/** producer.c
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

// max size for shared buffer; must be the same for both processes
#define bufferSize 2
 
// shared buffer between producer and consumer
struct sharedBuffer {
    char array[bufferSize];
};
 
int main() {
    // allocate shared memory
    int shm_fd = shm_open("/buffer", O_CREAT | O_RDWR, 0666);
    
    // resize the shared memory
    ftruncate(shm_fd, sizeof(struct sharedBuffer));
    
    // create a mapping between process's address space and shared memory
    struct sharedBuffer *bufferPtr = mmap(0, sizeof(struct sharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    // semaphores for synchronization between the two processes
    sem_t *full = sem_open("full", O_CREAT, 0666, 0);
    sem_t *empty = sem_open("empty", O_CREAT, 0666, bufferSize);
    sem_t *mutex = sem_open("mutex", O_CREAT, 0666, 1);
    
    // initialization before production
    for (int i = 0; i < bufferSize; ++i) {
        bufferPtr->array[i] = ' ';
    }
    
    int in = 0;
    int numOfLoops = 5; 
    printf("\nProducer can produce %d items.\n", numOfLoops);  
    while (numOfLoops > 0) {
        sem_wait(empty);
        sleep(rand() % 2 + 1);
        sem_wait(mutex);
        
        // Critical Section: accessing shared buffer
        bufferPtr->array[in] = 'x';
        if (in == (bufferSize - 1)) {
            in = 0;
        } else {
            ++in;
        }

        sem_post(mutex);
        printf("Producer:\t[");
        for (int i = 0; i < bufferSize; ++i) {
            printf("%c", bufferPtr->array[i]);
            if (i != (bufferSize - 1)) { printf(", "); }
        }
        printf("]\n");
        sem_post(full);
        
        --numOfLoops;
    }
    
    printf("**Producer has ended**\n");
    
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
    
    return 0;
}
