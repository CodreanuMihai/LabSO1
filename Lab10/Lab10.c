#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  // pt write(), open() si close()


int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Ati dat un numar gresit de argumente!\n");
        exit(-1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) 
    {
        printf("Eroare la deschiderea fisierului din care citim!\n");
        exit(-1);
    }
    close(fd);

    // Cream un pipe
    int fisDes[2];
    if (pipe(fisDes) == -1) 
    {
        printf("Eroare la crearea pipe-ului!\n");
        exit(-1);
    }

    int fisDes2[2];
    if (pipe(fisDes2) == -1) 
    {
        printf("Eroare la crearea celui de-al doile pipe!\n");
        exit(-1);
    }

    int pid = fork();
    if (pid == -1) 
    {
        printf("Eroare la creearea unui proces!\n");
        exit(-1);
    }

    if(pid == 0) // proces fiu pentru cat!
    {
        close(fisDes[0]);
        dup2(fisDes[1], 1);
        close(fisDes[1]);
        execlp("cat", "cat", argv[1], NULL);
        exit(-1);
    }

    int pid2 = fork();
    if (pid2 == -1) 
    {
        printf("Eroare la creearea unui proces!\n");
        exit(-1);
    } 
    if (pid2 == 0) // Proces fiu pentru grep
    {
        close(fisDes[1]);
        dup2(fisDes[1], 0);
        close(fisDes[1]);

        close(fisDes2[0]);
        dup2(fisDes2[1], 1);
        close(fisDes2[1]);
        execlp("grep", "grep", argv[2], argv[1], NULL);
        exit(-1);
    }

    close(fisDes[0]);
    close(fisDes[1]);
    close(fisDes2[1]);
    wait(NULL);
    // Citim rezultatul din pipe-ul al doilea si il afisam
    char buffer[512];
    int biti_cititi;
    while ((biti_cititi = read(fisDes2[0], buffer, 512)) > 0) 
    {
        write(1, buffer, biti_cititi);
    }

    if (biti_cititi == -1) 
    {
        printf("Eroare la citire!\n");
        exit(-1);
    }

    close(fisDes2[0]);

    waitpid(pid, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}