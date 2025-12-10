#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define ZOMBIE_INTERVAL 3  // Intervalle en secondes entre chaque zombie
#define MAX_ZOMBIES 50     // Nombre maximum de zombies avant nettoyage auto

// Variables globales
volatile sig_atomic_t cleanup_flag = 0;
volatile sig_atomic_t stop_creation = 0;
int zombie_count = 0;

/**
 * Handler pour SIGUSR1 - Nettoie tous les zombies
 */
void cleanup_zombies_handler(int signum) {
    cleanup_flag = 1;
}

/**
 * Handler pour SIGUSR2 - Arrête la création de nouveaux zombies
 */
void stop_creation_handler(int signum) {
    stop_creation = 1;
    printf("\n[MANAGER] Arrêt de la création de zombies demandé (SIGUSR2)\n");
}

/**
 * Handler pour SIGINT (Ctrl+C) - Arrêt propre
 */
void sigint_handler(int signum) {
    printf("\n[MANAGER] Arrêt demandé (SIGINT). Nettoyage en cours...\n");
    cleanup_flag = 1;
    stop_creation = 1;
}

/**
 * Crée un processus zombie
 */
void create_zombie() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    
    if (pid == 0) {
        // Processus enfant - meurt immédiatement
        printf("[ZOMBIE %d] Créé et terminé\n", getpid());
        exit(0);
    } else {
        // Processus parent - n'attend pas l'enfant (crée un zombie)
        zombie_count++;
        printf("[MANAGER] Zombie #%d créé (PID: %d). Total zombies: %d\n", 
               zombie_count, pid, zombie_count);
    }
}

/**
 * Nettoie tous les processus zombies
 */
void cleanup_zombies() {
    int status;
    pid_t pid;
    int cleaned = 0;
    
    // Récupère tous les zombies en attente
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        cleaned++;
    }
    
    if (cleaned > 0) {
        printf("[MANAGER] ✓ %d zombie(s) nettoyé(s)\n", cleaned);
        zombie_count = 0;
    }
}

/**
 * Affiche les statistiques
 */
void print_stats() {
    printf("\n========== STATISTIQUES ==========\n");
    printf("Zombies actuels:        %d\n", zombie_count);
    printf("PID du manager:         %d\n", getpid());
    printf("==================================\n\n");
}

int main() {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║     GESTIONNAIRE DE PROCESSUS ZOMBIES        ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");
    printf("[MANAGER] Démarrage avec PID: %d\n\n", getpid());
    
    // Configuration des signaux
    struct sigaction sa_usr1, sa_usr2, sa_int;
    
    // SIGUSR1 - Nettoyage des zombies
    sa_usr1.sa_handler = cleanup_zombies_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, NULL);
    
    // SIGUSR2 - Arrêt de la création
    sa_usr2.sa_handler = stop_creation_handler;
    sigemptyset(&sa_usr2.sa_mask);
    sa_usr2.sa_flags = 0;
    sigaction(SIGUSR2, &sa_usr2, NULL);
    
    // SIGINT (Ctrl+C) - Arrêt propre
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    printf("Signaux configurés:\n");
    printf("  - SIGUSR1: Nettoyer les zombies    (kill -SIGUSR1 %d)\n", getpid());
    printf("  - SIGUSR2: Arrêter création        (kill -SIGUSR2 %d)\n", getpid());
    printf("  - SIGINT:  Quitter proprement      (Ctrl+C)\n\n");
    
    // Boucle principale
    time_t last_zombie_time = time(NULL);
    
    while (1) {
        // Vérifie si nettoyage demandé
        if (cleanup_flag) {
            cleanup_zombies();
            cleanup_flag = 0;
            
            // Si arrêt demandé, quitter après nettoyage
            if (stop_creation) {
                break;
            }
        }
        
        // Crée un nouveau zombie si pas d'arrêt demandé
        time_t current_time = time(NULL);
        if (!stop_creation && 
            (current_time - last_zombie_time) >= ZOMBIE_INTERVAL) {
            
            create_zombie();
            last_zombie_time = current_time;
            
            // Auto-nettoyage si trop de zombies
            if (zombie_count >= MAX_ZOMBIES) {
                printf("[MANAGER] ⚠ Limite atteinte (%d zombies). Auto-nettoyage...\n", 
                       MAX_ZOMBIES);
                cleanup_zombies();
            }
        }
        
        sleep(1);
    }
    
    // Nettoyage final
    printf("\n[MANAGER] Arrêt du programme. Nettoyage final...\n");
    cleanup_zombies();
    
    print_stats();
    printf("[MANAGER] Programme terminé proprement.\n");
    
    return 0;
}