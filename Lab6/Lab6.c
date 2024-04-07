#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  // pt write(), open() si close()
#include <time.h>   // pt ctime()
#include <libgen.h> // pt basename()


/*1.Aplicatie care creeaza un proiect ce monitorizeaza modificarile intr-un director:
a. Verificati daca nr de arg > 0
b. verificati daca e director 
c. parcurgem directorul cu un while
d. afisam metadate(= denumire, marime, daca are link-uri, data ultimei modificari, ...) despre fisierele din toate directoarele recursiv*/

void drepturiFisier(struct stat st, int f)
{
    char buffer[40];
    int dimensiune = snprintf(buffer, sizeof(buffer), "Drepturi asupra fisierului: ");
    if (st.st_mode & S_IRUSR)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "r");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IWUSR)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "w");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IXUSR)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "x");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IRGRP)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "r");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IWGRP)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "w");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IXGRP)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "x");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IROTH)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "r");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IWOTH)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "w");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-");

    if (st.st_mode & S_IXOTH)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "x\n");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "-\n");

    write(f, buffer, dimensiune);
}

void info_fis(const char *numeFis, int f) {
    struct stat st;
    char buffer[8192];
    int dimensiune = 0;

    if (lstat(numeFis, &st) == -1) { //verificam daca putem obtine informatii despre fisier!
        dimensiune += snprintf(buffer, sizeof(buffer), "Nu putem obtine informatii despre fisier!\n");
        exit(-1);
    }

    //printam detalii despre fisier:
    dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Nume: %s\n", basename((char *)numeFis)); //-> pentru nume restrans!
    dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Nume extins: %s\n", numeFis); //-> pentru nume complet cu tot cu directoarele precedente!
    dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Marime: %ld bytes\n", st.st_size);

    if(st.st_nlink > 1) // verifica daca are vreun fel de legaturi grele(hardlink)
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Are legaturi grele(hardlink): Da\n");
    else
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Are legaturi grele(hardlink): Nu\n");

    if (S_ISLNK(st.st_mode))  // verifica daca are vreun fel de legaturi simbolice
         dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Are legaturi simbolice: Da\n");
    else 
        dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Are legaturi simbolice: Nu\n");

    dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Data ultimei modificari: %s", ctime(&st.st_mtime));

    write(f, buffer, dimensiune);
    drepturiFisier(st, f);
    write(f, "\n\n", 2);
}

int verificareDir(char *numeDir)
{
    struct stat st;
    if (stat(numeDir, &st) == 0) { // verificam daca putem obtine informatii despre director!
        if (S_ISDIR(st.st_mode)) 
            return 0;  // Este un director
        else
            return 1;  // Nu este un director
    } 
    return 1; 
}

void parcurgereDir(char *numeDir, int f)
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
        //daca nu e directorul curent sau cel in care se afla deja atunci mergi mai departe!
        {
            char path[1024]; 
            snprintf(path, sizeof(path), "%s/%s", numeDir, entry->d_name); // calea catre directorul/ fisierul curent

            if (entry->d_type == DT_REG) { // daca e fisier afiseaza-l!
                info_fis(path, f);
            }
            else if (entry->d_type == DT_DIR) { // daca e director reapelam functia!
                parcurgereDir(path, f);
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

    int f = open("file.txt", O_WRONLY | O_CREAT, 0744);
    parcurgereDir(argv[1], f);
    close(f);
    return 0;
}
