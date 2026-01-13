/*
 * reader.c
 * Lecture et détection simple de corruption mémoire
 * Usage: ./reader
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY  0x1234
#define SHM_SIZE 256

typedef struct {
    int  counter;
    char buffer[100];
    char canary[20];
} shared_data;

int main(void)
{
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        fprintf(stderr, "Lancez d'abord writer\n");
        return 1;
    }

    shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Counter : %d\n", data->counter);
    printf("Buffer  : %.50s\n", data->buffer);
    printf("Canary  : %s\n", data->canary);

    if (strcmp(data->canary, "CANARY_OK") != 0) {
        printf(">>> OVERFLOW DÉTECTÉ <<<\n");
    } else {
        printf("Mémoire intacte\n");
    }

    shmdt(data);
    return 0;
}
