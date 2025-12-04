/*
 * reader.c - Processus qui lit la mémoire partagée
 * Compile: gcc -o reader reader.c
 * Usage: ./reader
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SHM_SIZE 256

typedef struct {
    int count;
    char buffer[100];
    char overflow_detector[20];
} shared_data;

void print_memory_hex(void *ptr, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) printf("\n%04zx: ", i);
        printf("%02x ", p[i]);
        if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

int main() {
    int shmid;
    shared_data *data;
    
    printf("=== READER - Surveillance de la mémoire ===\n\n");
    
    // Obtenir le segment de mémoire partagée
    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget - La mémoire partagée n'existe pas encore");
        printf("Lancez d'abord ./writer\n");
        exit(1);
    }
    
    // Attacher le segment
    data = (shared_data *)shmat(shmid, NULL, 0);
    if (data == (shared_data *)(-1)) {
        perror("shmat");
        exit(1);
    }
    
    printf("Connecté à la mémoire partagée (ID: %d)\n", shmid);
    printf("Adresse: %p\n\n", (void *)data);
    
    // Menu interactif
    while (1) {
        printf("\n--- MENU READER ---\n");
        printf("1. Lecture avec vérification d'intégrité\n");
        printf("2. Dump hexadécimal de la mémoire\n");
        printf("3. Surveillance continue (Ctrl+C pour arrêter)\n");
        printf("0. Quitter\n");
        printf("Choix: ");
        
        int choice;
        scanf("%d", &choice);
        
        switch(choice) {

            case 1: {
                printf("\n--- VÉRIFICATION D'INTÉGRITÉ ---\n");
                printf("Count: %d\n", data->count);
                printf("Buffer (longueur: %zu): %s\n", 
                       strlen(data->buffer), data->buffer);
                
                // Vérifier le canary
                if (strcmp(data->overflow_detector, "CANARY_INTACT") == 0) {
                    printf("Canary: INTACT\n");
                    printf("Pas d'overflow détecté\n");
                } else {
                    printf("Canary: CORROMPU! (%s)\n", data->overflow_detector);
                    printf("OVERFLOW DÉTECTÉ!\n");
                    
                    // Analyser la corruption
                    printf("\nAnalyse de la corruption:\n");
                    printf("- Valeur attendue: 'CANARY_INTACT'\n");
                    printf("- Valeur actuelle: '%s'\n", data->overflow_detector);
                }
                
                // Vérifier si le buffer a dépassé sa taille
                size_t buf_len = strlen(data->buffer);
                if (buf_len > sizeof(data->buffer) - 1) {
                    printf(" Le buffer dépasse sa taille allouée!\n");
                    printf(" Taille max: %zu, Taille actuelle: %zu\n",
                           sizeof(data->buffer) - 1, buf_len);
                }
                break;
            }
            
            case 2: {
                printf("\n--- DUMP HEXADÉCIMAL ---\n");
                printf("Structure complète (%zu octets):", sizeof(shared_data));
                print_memory_hex(data, sizeof(shared_data));
                
                printf("\n\nZones mémoire:\n");
                printf("- count (4 bytes à offset 0)\n");
                printf("- buffer (100 bytes à offset 4)\n");
                printf("- overflow_detector (20 bytes à offset 104)\n");
                break;
            }
            
            case 3: {
                printf("\n--- SURVEILLANCE CONTINUE ---\n");
                printf("Mise à jour toutes les 2 secondes...\n");
                printf("Appuyez sur Ctrl+C pour arrêter\n\n");
                
                int last_count = -1;
                while (1) {
                    if (data->count != last_count) {
                        printf("\n[CHANGEMENT DÉTECTÉ - %d]\n", data->count);
                        printf("Buffer: %.50s%s\n", 
                               data->buffer,
                               strlen(data->buffer) > 50 ? "..." : "");
                        
                        if (strcmp(data->overflow_detector, "CANARY_INTACT") != 0) {
                            printf("OVERFLOW! Canary = '%s'\n", 
                                   data->overflow_detector);
                        } else {
                            printf("Canary intact\n");
                        }
                        
                        last_count = data->count;
                    }
                    sleep(2);
                }
                break;
            }
            
            case 0: {
                printf("Détachement de la mémoire partagée...\n");
                shmdt(data);
                exit(0);
            }
            
            default:
                printf("Choix invalide\n");
        }
    }
    
    return 0;
}