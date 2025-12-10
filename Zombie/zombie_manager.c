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
volatile sig_atomic_t zombie_count = 0;

/**
 * Handler pour SIGUSR1 - Déclenche le nettoyage des zombies
 * Note: On ne "tue" pas les zombies, on les récolte avec wait()
 */
void cleanup_zombies_handler(int signum) {
    (void)signum; 
    cleanup_flag = 1;
}

/**
 * Handler pour SIGUSR2 - Arrête la création de nouveaux zombies
 */
void stop_creation_handler(int signum) {
    (void)signum;
    stop_creation = 1;
}

/**
 * Handler pour SIGINT (Ctrl+C) - Arrêt propre
 */
void sigint_handler(int signum) {
    (void)signum;
    cleanup_flag = 1;
    stop_creation = 1;
}

/**
 * Crée un processus zombie
 * Le parent ne fait PAS wait(), laissant l'enfant en état zombie
 */
void create_zombie(void) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    
    if (pid == 0) {
        // Processus enfant - meurt immédiatement
        printf("[ZOMBIE %d] Processus créé et terminé immédiatement\n", getpid());
        exit(0);
    } else {
        // Processus parent - N'ATTEND PAS l'enfant
        zombie_count++;
        printf("[MANAGER] Zombie #%d créé (PID: %d). Total zombies: %d\n", 
               (int)zombie_count, pid, (int)zombie_count);
        printf("[MANAGER] → Le processus %d est maintenant un ZOMBIE (état Z)\n", pid);
    }
}

/**
 * "Récolte" les processus zombies avec waitpid()
 * Note: On ne tue PAS les zombies, on récupère leur statut de sortie
 * C'est wait() qui nettoie les zombies de la table des processus
 */
void cleanup_zombies(void) {
    int status;
    pid_t pid;
    int cleaned = 0;
    
    printf("[MANAGER] Début de la récolte des zombies...\n");
    
    // WNOHANG = non-bloquant, récupère tous les enfants terminés
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        cleaned++;
        printf("[MANAGER] Zombie récolté (PID: %d, exit status: %d)\n", 
               pid, WEXITSTATUS(status));
    }
    
    if (cleaned > 0) {
        printf("[MANAGER] Total: %d zombie(s) récolté(s) et nettoyé(s)\n", cleaned);
        zombie_count = 0;
    } else {
        printf("[MANAGER] Aucun zombie à récolter.\n");
    }
}

/**
 * Affiche les statistiques
 */
void print_stats(void) {
    printf("\n========== STATISTIQUES FINALES ==========\n");
    printf("Zombies actuels:        %d\n", (int)zombie_count);
    printf("PID du manager:         %d\n", getpid());
    printf("==========================================\n\n");
}

/**
 * Affiche les instructions
 */
void print_instructions(pid_t pid) {
    printf("\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║        GESTIONNAIRE DE PROCESSUS ZOMBIES                     ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("[MANAGER] Démarrage avec PID: %d\n\n", pid);
    
    printf("📡 SIGNAUX DISPONIBLES:\n");
    printf("  → SIGUSR1: Récolter les zombies      (kill -SIGUSR1 %d)\n", pid);
    printf("  → SIGUSR2: Arrêter création           (kill -SIGUSR2 %d)\n", pid);
    printf("  → SIGINT:  Quitter proprement         (Ctrl+C)\n\n");
    
    printf("  VÉRIFIER LES ZOMBIES:\n");
    printf("  → ps aux | grep Z\n");
    printf("  → ps --ppid %d -o pid,stat,cmd\n\n", pid);
    
    printf("  Configuration: Nouveau zombie toutes les %d secondes\n", ZOMBIE_INTERVAL);
    printf("  Auto-nettoyage après %d zombies\n\n", MAX_ZOMBIES);
    printf("═══════════════════════════════════════════════════════════════\n\n");
}

int main(void) {
    pid_t manager_pid = getpid();
    
    print_instructions(manager_pid);
    
    // Configuration des signaux avec sigaction (plus robuste que signal())
    struct sigaction sa_usr1, sa_usr2, sa_int;
    
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    memset(&sa_usr2, 0, sizeof(sa_usr2));
    memset(&sa_int, 0, sizeof(sa_int));
    
    // SIGUSR1 - Récolte des zombies
    sa_usr1.sa_handler = cleanup_zombies_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART; // Redémarre les appels système interrompus
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(EXIT_FAILURE);
    }
    
    // SIGUSR2 - Arrêt de la création
    sa_usr2.sa_handler = stop_creation_handler;
    sigemptyset(&sa_usr2.sa_mask);
    sa_usr2.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR2, &sa_usr2, NULL) == -1) {
        perror("sigaction SIGUSR2");
        exit(EXIT_FAILURE);
    }
    
    // SIGINT (Ctrl+C) - Arrêt propre
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }
    
    printf("[MANAGER] Signaux configurés avec succès\n");
    printf("[MANAGER] Début de la création de zombies...\n\n");
    
    // Boucle principale
    time_t last_zombie_time = time(NULL);
    
    while (1) {
        // Vérifie si nettoyage demandé via signal
        if (cleanup_flag) {
            printf("\n[MANAGER] Signal de nettoyage reçu!\n");
            cleanup_zombies();
            cleanup_flag = 0;
            
            // Si arrêt demandé, quitter après nettoyage
            if (stop_creation) {
                printf("\n[MANAGER] Arrêt demandé. Sortie de la boucle...\n");
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
                printf("\n[MANAGER] Limite de %d zombies atteinte!\n", MAX_ZOMBIES);
                printf("[MANAGER] Déclenchement du nettoyage automatique...\n");
                cleanup_zombies();
            }
        }
        
        sleep(1);
    }
    
    // Nettoyage final avant sortie
    printf("\n[MANAGER] ═══ ARRÊT DU PROGRAMME ═══\n");
    printf("[MANAGER] Nettoyage final de tous les zombies...\n");
    cleanup_zombies();
    
    print_stats();
    
    return EXIT_SUCCESS;
}