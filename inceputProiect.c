#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*1.Aplicatie care creeaza un proiect ce monitorizeaza modificarile intr-un director:
a. Verificati daca nr de arg > 0
b. verificati daca e director 
c. parcurgem directorul cu un while
d. afisam metadate(= denumire, marime, daca are link-uri, data ultimei modificari) despre fisierele din toate directoarele recursiv*/

int verificareDir(char *numeDir)
{
    struct stat st;
    if (stat(numeDir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) 
            return 0;  // Este un director
        else
            return 1;  // Nu este un director
    } 
    return 1; // nu-l putem accesa
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Nu sunt destule argumente!\n");
        exit(-1);
    }
    printf("%s\n", argv[1]);
    /*int err = verificareDir(argv[1]);
    if(err != 0)
    {
        printf("Argumentul dat nu e director!\n");
        exit(-1);
    }
    printf("Salut!\n");*/
    return 0;
}