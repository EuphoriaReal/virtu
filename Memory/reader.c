/*
 * reader.c - Monitoring continu de la mémoire partagée
 * Usage: ./reader [once]
 *   sans argument : monitoring continu
 *   avec "once"   : lecture unique
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY  0x1234
#define SHM_SIZE 256

typedef struct {
    int  counter;
    char buffer[100];
    char canary[20];
} shared_data;

void check_memory(shared_data *data)
{
    printf("Counter: %d | ", data->counter);
    
    if (strcmp(data->canary, "CANARY_OK") != 0) {
        printf(">>> OVERFLOW <<<\n");
    } else {
        printf("OK\n");
    }
}

int main(int argc, char *argv[])
{
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        printf("Lancez: ./writer reset\n");
        return 1;
    }

    shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    /* Mode unique ou continu */
    if (argc > 1 && strcmp(argv[1], "once") == 0) {
        check_memory(data);
    } else {
        printf("Monitoring (Ctrl+C pour arrêter)\n");
        while (1) {
            check_memory(data);
            sleep(1);
        }
    }

    shmdt(data);
    return 0;
}