/** consumer.c
 *  Nghia Dao
 *  Producer-Consumer Problem
 *  CS33211
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // gets rid of "implicit declaration of function 'truncate' warning
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
    char *in;
    char *out;
};
 
int main() {
    int shm_fd = shm_open("buffer", O_RDWR, 0666);
    
    ftruncate(shm_fd, sizeof(struct sharedBuffer));
    
    struct sharedBuffer *bufferPtr = mmap(0, sizeof(struct sharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *full = sem_open("full", O_CREAT, 0666, 0);
    sem_t *empty = sem_open("empty", O_CREAT, 0666, 2);
    sem_t *mutex = sem_open("mutex", O_CREAT, 0666, 1);

    
    // initialization before consumption
    bufferPtr->out = bufferPtr->array;
    
    int numOfLoops = 5;
    while (numOfLoops > 0) {
        sem_wait(full);
        sleep(rand() % 2 + 1);
        sem_wait(mutex);

        // accessing shared buffer
        *(bufferPtr -> out) = ' ';
        if (bufferPtr->out == &(bufferPtr->array[bufferSize - 1])) {
            bufferPtr->out = bufferPtr->array;
        } else {
            ++(bufferPtr->out);
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
    shm_unlink("buffer");
    
    return 0;
}