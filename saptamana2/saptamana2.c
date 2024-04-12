#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  // pt write(), open() si close()
#include <time.h>   // pt ctime()
#include <libgen.h> // pt basename()


/*
Saptamana 2 (5.4.2023 - 11.4.2023) 
1. Se va actualiza functionalitatea programului în așa fel încât acesta să primească un număr nespecificat de argumente 
în linia de comandă, dar nu mai mult de 10, cu mențiunea ca niciun argument nu se va repeta. Programul va procesa numai 
directoarele, alte tipuri de argumente vor fi ignorate. Logica de captură a metadatelor se va aplica acum tuturor 
argumentelor primite valide, ceea ce înseamnă că programul va actualiza snapshoturile pentru toate directoarele specificate 
de utilizator. 
2. În cazul în care se vor înregistra modificări la nivelul directoarelor, utilizatorul va putea să compare snapshot-ul 
anterior al directorului specificat cu cel curent. În cazul în care există diferențe între cele două snapshot-uri, snapshot-ul 
vechi va fi actualizat cu noile informații din snapshot-ul curent. 
3. Funcționalitatea codului va fi extinsă astfel încât programul să primească un argument suplimentar, care va reprezenta 
directorul de ieșire în care vor fi stocate toate snapshot-urile intrărilor din directoarele specificate în linia de comandă. 
Acest director de ieșire va fi specificat folosind opțiunea `-o`. De exemplu, comanda de rulare a programului va fi: 
`./program_exe -o output input1 input2 ...`. 

*/

#define ARGS_MAX 10 // numarul de argumente maxime

void drepturiFisier(struct stat st, const int f)
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

void info_fis(const char *numeFis, const int f) {
    struct stat st;
    char buffer[8192];
    int dimensiune = 0;

    if (lstat(numeFis, &st) == -1) { //verificam daca putem obtine informatii despre fisier!
        dimensiune += snprintf(buffer, sizeof(buffer), "Nu putem obtine informatii despre fisier!\n");
        exit(-1);
    }

    //printam detalii despre fisier:
    dimensiune += snprintf(buffer + dimensiune, sizeof(buffer) - dimensiune, "Nume: %s\n", basename((char *)numeFis)); //-> pentru nume restrans!
    // printeaza sizeof(buffer) - dimensiune elemente in buffer, incepand de la pozitia dimensiune din buffer!
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

int verificareDir(const char *numeDir)
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

void parcurgereDir(const char *numeDir, const int f)
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului!\ns");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        //daca nu e directorul curent sau cel in care se afla deja atunci mergi mai departe!
        {
            char path[1024]; 
            snprintf(path, sizeof(path), "%s/%s", numeDir, entry->d_name); // calea catre directorul/ fisierul curent

            if (entry->d_type == DT_REG)  // daca e fisier afiseaza-l!
            {
                info_fis(path, f);
            }
            else if (entry->d_type == DT_DIR)  // daca e director reapelam functia!
            {
                parcurgereDir(path, f);
            }
        }
    }
    closedir(dir);
}

int existaSnapshot(const char *numeDir, const int t)
//functia cauta daca exista un fisier snapshot cu cifra t
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului!\ns");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if(strstr(entry->d_name, ".txt") != NULL) //daca e un fisier.txt
        {
            char *gasit = entry->d_name;
            for(int i = 0; i < strlen(gasit); i++)
            {
                if((gasit[i] >= '0' && gasit[i] <= '9') && gasit[i] == t + '0')
                {
                    closedir(dir);
                    return 0;
                }
            }
        }
    }
    closedir(dir);
    return 1;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Nu sunt destule argumente!\n");
        exit(-1);
    }
    else
    {
        if (argc > ARGS_MAX + 2)
        {
            printf("Ati dat prea multe argumente!\n");
            exit(-1);
        }
    }

    if(strcmp(argv[1], "-o") != 0)
    {
        printf("Sintaxa nu a fost respectata!\n");
        exit(-1);
    }

    int err = verificareDir(argv[2]);
    if(err != 0)
    {
        printf("Nu avem fisier de iesire!\n");
        exit(-1);
    }

    char dirIesire[256];
    snprintf(dirIesire, sizeof(dirIesire), "%s", argv[2]); // directorul de iesire
    int t[10] = {0};
    int f; // variabila de cale
    for(int i = 3; i <= 12; i++)
    {
        int err = verificareDir(argv[i]);
        if(err == 0)
        {
            char path[270];
            snprintf(path, sizeof(path), "%s/snapshot%d.txt", dirIesire, i - 3);
            t[i - 3] = 1;
            f = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0744); 
            // folosirea lui trunc ne asigura o actualizare constanta a snapshot-urilor la fiecare rulare!
            parcurgereDir(argv[i], f);
            close(f);
        }
    }

    //daca schimbam ordinea argumentelor raman fisierele vechi -> acestea trebuie sterse
    for(int i = 0; i < 10; i++)
    {
        if(t[i] == 0 && existaSnapshot(dirIesire, i) == 0) 
        // daca exista fisierul, dar nu mai e nevoie de el pentru ca s-a schimbat ordinea argumentelor sau argumentele in sine
        {
            char path[270];
            snprintf(path, sizeof(path), "%s/snapshot%d.txt", dirIesire, i);
            if(unlink(path) != 0)
            // functia unlink(), definita in unistd.h sterge fisierul dat ca parametru 
            {
                printf("Eroare la stergerea unui fisier snapshot!\n");
                exit(-1);
            }
        }
    }

    return 0;
}
