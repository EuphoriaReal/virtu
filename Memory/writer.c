/*
 * writer.c - Écriture en mémoire partagée
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
        printf("Usage: %s safe|overflow|reset\n", argv[0]);
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
        printf("Reset OK\n");
    }
    else if (strcmp(argv[1], "safe") == 0) {
        /* Écriture sécurisée : on respecte la taille du buffer */
        snprintf(data->buffer, sizeof(data->buffer), "Safe write #%d", data->counter);
        data->counter++;
        printf("Safe write done\n");
    }
    else if (strcmp(argv[1], "overflow") == 0) {
        /* Overflow volontaire : 150 'A' dans un buffer de 100 */
        char overflow[150];
        memset(overflow, 'A', 149);
        overflow[149] = '\0';
        strcpy(data->buffer, overflow);  /* DANGER: écrase le canary */
        data->counter++;
        printf("Overflow done\n");
    }
    else {
        printf("Argument invalide: safe|overflow|reset\n");
    }

    shmdt(data);
    return 0;
}