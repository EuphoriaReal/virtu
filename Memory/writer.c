/*
 * writer.c
 * Démonstration simple de buffer overflow en mémoire partagée
 * Usage: ./writer safe|overflow|reset
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

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s safe|overflow|reset\n", argv[0]);
        return 1;
    }

    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    if (strcmp(argv[1], "reset") == 0) {
        memset(data, 0, sizeof(*data));
        strcpy(data->canary, "CANARY_OK");
        printf("Mémoire réinitialisée\n");
    }
    else if (strcmp(argv[1], "safe") == 0) {
        strncpy(data->buffer, "Écriture sécurisée", sizeof(data->buffer) - 1);
        data->counter++;
        printf("Écriture sécurisée effectuée\n");
    }
    else if (strcmp(argv[1], "overflow") == 0) {
        char big[200];
        memset(big, 'A', sizeof(big) - 1);
        big[199] = '\0';

        strcpy(data->buffer, big);   // overflow volontaire
        data->counter++;
        printf("Overflow généré\n");
    }
    else {
        fprintf(stderr, "Argument invalide\n");
    }

    shmdt(data);
    return 0;
}
