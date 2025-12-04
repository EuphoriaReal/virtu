/*
 * writer.c - Processus qui écrit dans la mémoire partagée
 * Compile: gcc -o writer writer.c
 * Usage: ./writer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SHM_SIZE 256  // Taille VOLONTAIREMENT petite pour la démo

typedef struct {
    int count;
    char buffer[100];  // Buffer limité à 100 caractères
    char overflow_detector[20]; // Zone pour détecter l'overflow
} shared_data;

int main() {
    int shmid;
    shared_data *data;
    
    printf("=== WRITER - Démonstration Buffer Overflow ===\n\n");
    
    // Créer ou obtenir le segment de mémoire partagée
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    
    // Attacher le segment à notre espace d'adressage
    data = (shared_data *)shmat(shmid, NULL, 0);
    if (data == (shared_data *)(-1)) {
        perror("shmat");
        exit(1);
    }
    
    // Initialiser la zone de détection d'overflow
    strcpy(data->overflow_detector, "CANARY_INTACT");
    data->count = 0;
    
    printf("Mémoire partagée créée (ID: %d)\n", shmid);
    printf("Taille du buffer: %lu octets\n", sizeof(data->buffer));
    printf("Canary initial: %s\n\n", data->overflow_detector);
    
    // Menu interactif
    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Écriture normale (sécurisée)\n");
        printf("2. Écriture avec overflow\n");
        printf("3. Réinitialiser\n");
        printf("0. Quitter\n");
        printf("Choix: ");
        
        int choice;
        scanf("%d", &choice);
        getchar(); // Consommer le '\n'
        
        switch(choice) {
            case 1: {
                // Écriture sécurisée
                printf("Entrez un texte (max 99 chars): ");
                char input[100];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;
                
                strncpy(data->buffer, input, sizeof(data->buffer) - 1);
                data->buffer[sizeof(data->buffer) - 1] = '\0';
                data->count++;
                
                printf("Écriture sécurisée effectuée\n");
                break;
            }
            
            case 2: {
                // Overflow important - écrase tout
                printf("Génération d'un overflow...\n");
                char very_long[200];
                memset(very_long, 'X', 199);
                very_long[199] = '\0';
                
                strcpy(data->buffer, very_long);
                data->count++;
                
                printf("Overflow important généré (199 chars)\n");
                break;
            }
            
            case 3: {
                // Réinitialiser
                memset(data->buffer, 0, sizeof(data->buffer));
                strcpy(data->overflow_detector, "CANARY_INTACT");
                data->count = 0;
                printf("Mémoire réinitialisée\n");
                break;
            }
            
            case 0: {
                printf("Nettoyage...\n");
                shmdt(data);
                shmctl(shmid, IPC_RMID, NULL);
                printf("Mémoire partagée supprimée\n");
                exit(0);
            }
            
            default:
                printf("Choix invalide\n");
        }
    }
    
    return 0;
}