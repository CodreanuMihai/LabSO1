#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libgen.h> // pt basename


/*1.Aplicatie care creeaza un proiect ce monitorizeaza modificarile intr-un director:
a. Verificati daca nr de arg > 0
b. verificati daca e director 
c. parcurgem directorul cu un while
d. afisam metadate(= denumire, marime, daca are link-uri, data ultimei modificari) despre fisierele din toate directoarele recursiv*/

void info_fis(const char *numeFis) {
    struct stat st;

    if (lstat(numeFis, &st) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    printf("Nume: %s\n", basename((char *)numeFis));
    printf("Marime: %ld bytes\n", st.st_size);

    if (S_ISLNK(st.st_mode)) {
        printf("Are link-uri: Da\n");
    } else {
        printf("Are link-uri: Nu\n");
    }

    printf("Data ultimei modificari: %s", ctime(&st.st_mtime));
}

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

void parcurgereDir(char *numeDir)
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului!\ns");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char path[1024]; // cale pentru fisiere
            sprintf(path, "%s/%s", numeDir, entry->d_name);

            if (entry->d_type == DT_REG) { // daca e fisier afiseaza-l!
                info_fis(path);
            }
            else if (entry->d_type == DT_DIR) { // daca e director reapelam functia!
                parcurgereDir(path);
            }
        }
    }
    closedir(dir);
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Nu sunt destule argumente!\n");
        exit(-1);
    }
    int err = verificareDir(argv[1]);
    if(err != 0)
    {
        printf("Argumentul dat nu e director!\n");
        exit(-1);
    }

    parcurgereDir(argv[1]);
    return 0;
}
