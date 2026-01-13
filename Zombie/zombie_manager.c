#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define INTERVAL 2

volatile sig_atomic_t cleanup = 0;

/* Signal personnalisé : nettoyage des zombies */
void sigusr1_handler(int sig)
{
    (void)sig;
    cleanup = 1;
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    printf("PID du parent: %d\n", getpid());
    printf("Envoyer SIGUSR1 pour nettoyer les zombies\n");

    while (1) {
        pid_t pid = fork();

        if (pid == 0) {
            /* Enfant : termine immédiatement */
            _exit(0);
        }

        /* Parent : ne fait pas wait() => zombie */
        sleep(INTERVAL);

        if (cleanup) {
            while (waitpid(-1, NULL, WNOHANG) > 0);
            cleanup = 0;
            printf("Zombies nettoyés\n");
        }
    }
}
