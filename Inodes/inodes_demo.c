#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define WORKDIR "fs_demo"
#define BIGFILE WORKDIR "/image.jpg"
#define SMALLDIR WORKDIR "/small"

#define BIG_MB 50
#define NB_FILES 10000
#define INODE_SIZE 256

/* ---------- Réinitialisation de la démo ---------- */
void reset_demo(void)
{
    // Supprime le dossier de travail et tout son contenu
    system("rm -rf " WORKDIR);
}

/* ---------- Cas 1 : gros fichier ---------- */
void demo_big_file(void)
{
    mkdir(WORKDIR, 0755);

    int fd = open(BIGFILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return; }

    char buf[1024 * 1024] = {0};
    for (int i = 0; i < BIG_MB; i++)
        write(fd, buf, sizeof(buf));

    /* Signature JPG */
    unsigned char jpg[4] = {0xFF, 0xD8, 0xFF, 0xE0};
    lseek(fd, 0, SEEK_SET);
    write(fd, jpg, sizeof(jpg));

    close(fd);

    struct stat st;
    stat(BIGFILE, &st);

    printf("=== Gros fichier ===\n");
    printf("Taille logique        : %lld octets\n", (long long)st.st_size);
    printf("Nombre de blocs       : %lld\n", (long long)st.st_blocks);
    printf("Espace disque réel    : %lld octets\n",
           (long long)st.st_blocks * 512);
}

/* ---------- Cas 2 : saturation des inodes ---------- */
void demo_inodes(void)
{
    mkdir(WORKDIR, 0755);
    mkdir(SMALLDIR, 0755);

    char name[256];
    struct stat st;

    long long size = 0;
    long long blocks = 0;

    for (int i = 0; i < NB_FILES; i++) {
        snprintf(name, sizeof(name), "%s/f%d", SMALLDIR, i);

        int fd = open(name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0) break;

        write(fd, "x", 1);
        close(fd);

        stat(name, &st);
        size += st.st_size;
        blocks += st.st_blocks;
    }

    printf("\n=== Saturation des inodes ===\n");
    printf("Nombre de fichiers      : %d\n", NB_FILES);
    printf("Taille logique totale   : %lld octets\n", size);
    printf("Espace théorique (inodes inclus) : %lld octets\n",
           size + NB_FILES * INODE_SIZE);
    printf("Espace disque réel      : %lld octets\n",
           blocks * 512);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s {big|inode|reset}\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "big"))
        demo_big_file();
    else if (!strcmp(argv[1], "inode"))
        demo_inodes();
    else if (!strcmp(argv[1], "reset"))
        reset_demo();
    else
        printf("Usage: %s {big|inode|reset}\n", argv[0]);

    return 0;
}
