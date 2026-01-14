#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <string.h>

#define WORKDIR "fs_demo"
#define BIGFILE WORKDIR "/image.jpg"
#define SMALLDIR WORKDIR "/small"

#define BIG_MB 50
#define NB_FILES 10000

/* ---------- Réinitialisation ---------- */
void reset_demo(void)
{
    system("rm -rf " WORKDIR);
    printf("Demo réinitialisée\n");
}

/* ---------- Cas 1 : gros fichier ---------- */
void demo_big_file(void)
{
    mkdir(WORKDIR, 0755);

    /* Récupère la taille de bloc du système de fichiers */
    struct statvfs vfs;
    statvfs(WORKDIR, &vfs);
    unsigned long block_size = vfs.f_bsize;

    int fd = open(BIGFILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return; }

    /* Écriture de BIG_MB Mo */
    char buf[1024 * 1024] = {0};
    for (int i = 0; i < BIG_MB; i++)
        write(fd, buf, sizeof(buf));

    /* Signature JPG en début de fichier */
    unsigned char jpg[4] = {0xFF, 0xD8, 0xFF, 0xE0};
    lseek(fd, 0, SEEK_SET);
    write(fd, jpg, sizeof(jpg));
    close(fd);

    struct stat st;
    stat(BIGFILE, &st);

    /* Calculs */
    long long data_size = st.st_size;
    long long disk_size = (long long)st.st_blocks * 512;
    long long expected_blocks = (data_size + block_size - 1) / block_size;
    long long expected_disk = expected_blocks * block_size;

    printf("=== Gros fichier (%d Mo) ===\n", BIG_MB);
    printf("Taille bloc FS          : %lu octets\n", block_size);
    printf("Espace données          : %lld octets\n", data_size);
    printf("Blocs théoriques        : %lld\n", expected_blocks);
    printf("Espace théorique        : %lld octets\n", expected_disk);
    printf("Espace réel disque      : %lld octets\n", disk_size);

    if (disk_size != expected_disk)
        printf("Écart: %+lld octets (métadonnées/fragmentation)\n",
               disk_size - expected_disk);
}

/* ---------- Cas 2 : saturation inodes ---------- */
void demo_inodes(void)
{
    mkdir(WORKDIR, 0755);
    mkdir(SMALLDIR, 0755);

    struct statvfs vfs;
    statvfs(WORKDIR, &vfs);
    unsigned long block_size = vfs.f_bsize;

    char name[256];
    struct stat st;
    int created = 0;

    long long total_data = 0;
    long long total_blocks = 0;

    for (int i = 0; i < NB_FILES; i++) {
        snprintf(name, sizeof(name), "%s/f%d", SMALLDIR, i);

        int fd = open(name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0) break;

        write(fd, "x", 1);
        close(fd);

        stat(name, &st);
        total_data += st.st_size;
        total_blocks += st.st_blocks;
        created++;
    }

    /* Espace réel sur disque */
    long long disk_size = total_blocks * 512;

    /* Espace théorique : 1 bloc minimum par fichier */
    long long expected_disk = (long long)created * block_size;

    printf("\n=== Saturation inodes (%d fichiers) ===\n", created);
    printf("Taille bloc FS          : %lu octets\n", block_size);
    printf("Espace données          : %lld octets (%d x 1 octet)\n",
           total_data, created);
    printf("Espace théorique        : %lld octets (%d blocs)\n",
           expected_disk, created);
    printf("Espace réel disque      : %lld octets\n", disk_size);
    printf("Overhead                : x%.1f\n",
           (double)disk_size / total_data);
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