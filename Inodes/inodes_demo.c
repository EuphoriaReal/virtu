#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define WORKDIR "inodes_demo"
#define BIGFILE WORKDIR "/fake_image.jpg"
#define SMALLDIR WORKDIR "/many_small_files"

#define BIGFILE_SIZE_MB 50
#define NUM_SMALL_FILES 50000
#define INODE_SIZE 256 

// Fonction utilitaire : créer un répertoire
void create_dir(const char *dir) {
    mkdir(dir, 0755);
}

// -----------------------------------------------------
// Cas 1 : gros fichier simulé
// -----------------------------------------------------
void create_big_file() {
    printf("=== Cas 1 : Gros fichier simulé ===\n");
    create_dir(WORKDIR);

    printf("[1] Création d'un fichier de %d Mo...\n", BIGFILE_SIZE_MB);
    int fd = open(BIGFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) { perror("open"); return; }

    char buffer[1024*1024]; // 1 Mo
    memset(buffer, 0, sizeof(buffer));
    for(int i=0; i<BIGFILE_SIZE_MB; i++) {
        if(write(fd, buffer, sizeof(buffer)) < 0) { perror("write"); close(fd); return; }
    }

    // écrire magic number JPG au début
    lseek(fd, 0, SEEK_SET);
    char jpg_magic[4] = {0xFF, 0xD8, 0xFF, 0xE0};
    write(fd, jpg_magic, sizeof(jpg_magic));
    close(fd);

    // récupérer infos fichier
    struct stat st;
    if(stat(BIGFILE, &st) != 0) { perror("stat"); return; }

    printf(" Calcul de l'espace théorique et réel :\n");
    printf("→ Taille théorique (octets)      : %lld\n", (long long)st.st_size);
    printf("→ Nombre de blocs utilisés       : %lld\n", (long long)st.st_blocks);
    printf("→ Taille d'un bloc (octets)      : %lld\n", (long long)st.st_blksize);
    printf("→ Espace réel sur disque (octets): %lld\n", (long long)(st.st_blocks*512));
    printf("\n");
}

// -----------------------------------------------------
// Cas 2 : saturer les inodes
// -----------------------------------------------------
void saturate_inodes() {
    printf("=== Cas 2 : Saturation des inodes ===\n");
    create_dir(SMALLDIR);

    printf("[1] Création de %d petits fichiers...\n", NUM_SMALL_FILES);
    char filename[256];
    for(int i=1; i<=NUM_SMALL_FILES; i++) {
        snprintf(filename, sizeof(filename), "%s/file_%d.txt", SMALLDIR, i);
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0) break; // stop si erreur (inode saturé)
        write(fd, "x", 1);
        close(fd);
    }

    // Calcul des tailles
    DIR *dir = opendir(SMALLDIR);
    if(!dir) { perror("opendir"); return; }

    struct dirent *entry;
    long long theoretical_size = 0;
    long long num_files = 0;
    while((entry = readdir(dir)) != NULL) {
        // Construire le chemin complet
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", SMALLDIR, entry->d_name);

        // Récupérer les infos du fichier
        struct stat st;
        if(stat(filepath, &st) == 0 && S_ISREG(st.st_mode)) {
            theoretical_size += st.st_size;
            num_files++;
        }
    }
    closedir(dir);

    // Calcul espace réel (du -sb approximation)
    char command[512];
    snprintf(command, sizeof(command), "du -sb %s | cut -f1", SMALLDIR);
    FILE *fp = popen(command, "r");
    long long real_disk_size = 0;
    if(fp) {
        fscanf(fp, "%lld", &real_disk_size);
        pclose(fp);
    }

    printf("Résultats :\n");
    printf("→ Nombre de fichiers créés       : %lld\n", num_files);
    printf("→ Taille théorique totale (octets): %lld\n", theoretical_size);
    printf("→ Espace pris par les inodes (~octets): %lld\n", num_files*INODE_SIZE);
    printf("→ Espace réel sur disque (octets): %lld\n", real_disk_size);
    printf("\n");
}

// -----------------------------------------------------
// Réinitialisation
// -----------------------------------------------------
void reset_demo() {
    printf("=== Réinitialisation ===\n");
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", WORKDIR);
    system(command);
    printf("Tout a été supprimé.\n");
}

// -----------------------------------------------------
// Menu principal
// -----------------------------------------------------
int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s {big|inode|reset}\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "big") == 0) {
        create_big_file();
    } else if(strcmp(argv[1], "inode") == 0) {
        saturate_inodes();
    } else if(strcmp(argv[1], "reset") == 0) {
        reset_demo();
    } else {
        printf("Usage: %s {big|inode|reset}\n", argv[0]);
        return 1;
    }

    return 0;
}
