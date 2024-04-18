#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> // pt waitpid()
#include <fcntl.h>  // pt write(), open() si close()
#include <time.h>   // pt ctime()
#include <libgen.h> // pt basename()

int verificareDir(const char *numeDir)
{
    struct stat st;
    if (stat(numeDir, &st) == 0) // verificam daca putem obtine informatii despre director!
    {
        if (S_ISDIR(st.st_mode)) 
            return 0;  // Este un director
        else
            return 1;  // Nu este un director
    } 
    return 1; 
}

void apeleazaPeDir(const char *numeDir)
{
    printf("Continutul directorului %s: \n", numeDir);
    if(execlp("/bin/ls", "ls", "-l", numeDir, NULL) == -1)
    {
        printf("Eroare la procesul din directorul %s!\n", numeDir);
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    if(argc < 2 || argc > 6)
    {
        printf("Nu este un numar acceptat de argumente!\n");
        exit(-1);
    }

    for(int i = 1; i < argc; i++)
    {
        int status;
        if(verificareDir(argv[i]) != 0)
        {
            printf("Ati dat un argument care nu este director!\n");
            exit(-1);
        }

        int pid = fork(); //creeam un proces
        if (pid == -1) 
        {
            printf("Eroare la creearea unui proces!\n");
        } else if (pid == 0)  // Proces fiu
        {
            apeleazaPeDir(argv[i]);
        }

        waitpid(pid, &status, 0);
        if (!WIFEXITED(status))
        {
            printf("Procesul pentru directorul %s e invalid!\n", argv[i]);
        }
    }
    return 0;
}