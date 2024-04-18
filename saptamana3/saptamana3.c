#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h> // pt waitpid()
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  // pt write(), open() si close()
#include <time.h>   // pt ctime()
#include <libgen.h> // pt basename()


/*
Saptamana 3 (15.4.2024 - 21.4.2024)
*/

#define ARGS_MAX 10 // numarul de argumente maxime

void parcurgereDir(const char *numeDir, const int f)
{
    // Deschide fișierul pentru scriere
    int fd = dup2(f, STDOUT_FILENO); // redirectionam STDOUT pentru execlp(), asta face dup2()
    if (fd == -1) 
    {
        printf("Eroare la redirecționarea ieșirii standard!\n");
        exit(-1);
    }

    if(execlp("ls", "ls", "-lsihAR", "-I", "^\\.\\.$", numeDir, NULL) == -1)
    // Execută comanda ls -lsihAR pentru a obține informațiile despre fișier
    // s -> dimensiunea fisierului in octeti
    // i -> daca are link-uri
    // h -> afiseaza o mica legenda deasupra pentru a sti ce reprezinta fiecare coloana 
    // A -> afiseaza toate fisierele, chiar si cele ascunse, dar ignora tot ce incepe cu '.'
    // R -> afiseaza recursiv pentru cazul in acre sunt alte directoare in cel dat
    // I -> permite folosirea de expresii regulate pentru a reglementa ce sa nu afiseze. In cazul nostru, nu va afisa pe cele care au unul sau 2 puncte una dupa alta: '.' si '..'
    {
        
        printf("Eroare la executarea comenzii ls\n!");
        exit(-1);
    }
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

int existaSnapshot(const char *numeDir, const int t)
//functia cauta daca exista un fisier snapshot cu cifra t
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului de snapshots!\n");
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

// verificam daca acelasi director e dat de 2 ori ca argument!
int verifApar(const int i, char **argv, int argc) 
{
    for(int j = 2; j < argc; j++) 
    {
        if(strcmp(argv[i], argv[j]) == 0 && i != j) {
            return 1;  
        }
    }
    return 0;
}

void stergeSnapshot(const char *path)
{
    if(unlink(path) != 0)
    // functia unlink(), definita in unistd.h sterge fisierul dat ca parametru 
    {
        printf("Eroare la stergerea unui fisier snapshot!\n");
        exit(-1);
    }
}

int comparaSnapshoturi(char *path1, char *path2)
{
    int fd1 = open(path1, O_RDONLY);
    if (fd1 == -1) 
    {
        printf("Eroare la deschiderea primului snapshot in comparare!\n");
        exit(-1);
    }

    int fd2 = open(path2, O_RDONLY);
    if (fd2 == -1) 
    {
        printf("Eroare la deschiderea celui de-al doilea snapshot in comparare!\n");
        exit(-1);
    }

    char buffer1[4096], buffer2[4096];
    int dim1, dim2;

    //comparam snapshot-urile:
    do 
    {
        dim1 = read(fd1, buffer1, 4096);
        dim2 = read(fd2, buffer2, 4096);

        //daca au dimensiuni diferite sau bufferele sunt diferite atunci snapshot-urile sunt diferite
        if (dim1 != dim2 || memcmp(buffer1, buffer2, dim1) != 0) 
        //memcmp() compara 2 zone de memorie de dimensiune dim1
        {
            close(fd1);
            close(fd2);
            return 0;
        }
    } while (dim1 > 0 && dim2 > 0);

    if (dim1 != dim2) 
    {
        close(fd1);
        close(fd2);
        return 0;
    }

    close(fd1);
    close(fd2);
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

    char dirIesire[256];
    int err = verificareDir(argv[2]);
    if(err != 0)
    {
        printf("Nu avem fisier de iesire! => Il cream noi!\n");
        // Il cream noi:
        if(mkdir(basename((char *)argv[2]), 0777) == -1)
        //mkdir() din unistd.h este o functie care creeaz un director nou in directorul curent!
        {
            printf("Nu s-a putut crea directorul de iesire!\n");
            exit(-1);
        }
    }        
    snprintf(dirIesire, sizeof(dirIesire), "%s", argv[2]); // directorul de iesire

    int t[10] = {0};
    int f; // variabila de cale
    for(int i = 3; i <= 12; i++)
    {
        int err = verificareDir(argv[i]);
        if(err == 0)
        {
            if(verifApar(i, argv, argc) == 0)
            {
                char path[270];
                snprintf(path, sizeof(path), "%s/snapshot%d.txt", dirIesire, i - 3);
                t[i - 3] = 1;
                f = open(path, O_WRONLY | O_CREAT, 0744);
                close(f);
                // deschidem si inchidem fisierul aici pentru a-i putea verifica existenta! 
                struct stat fis_stat;
                if(stat(path, &fis_stat) == -1)
                {
                    printf("Nu putem accesa fisierul de scriere!\n");
                    exit(-1);
                }
                if(fis_stat.st_size == 0) //verificam daca fisierul e gol: daca are size 0 inseamna ca e gol si se poate scrie in el
                { 
                    int status, pid = fork(); //creeam un proces
                    if (pid == -1) 
                    {
                        printf("Eroare la creearea unui proces!\n");
                    } else if (pid == 0)  // Proces fiu
                    {
                        f = open(path, O_WRONLY, 0744);
                        parcurgereDir(argv[i], f);
                        close(f);
                    }

                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                    {
                        printf("Procesul cu  PID %d s-a terminat cu codul %d!\n", pid, WEXITSTATUS(status));
                        //WEXITSTATUS ne da codul de iesire al procesului
                    }
                    else // asta inseamna ca nu includem procesul parinte in verificare
                    {
                        printf("Procesul pentru directorul %s e invalid!\n", argv[i]);
                    }
                }
                else //daca exista deja:
                {
                    char path1[273];
                    snprintf(path1, sizeof(path1), "%s/snapshot%d_v1.txt", dirIesire, i - 3);
                    f = open(path1, O_WRONLY | O_CREAT, 0744); 
                    int status, pid = fork(); //creeam un proces
                    if (pid == -1) 
                    {
                        printf("Eroare la creearea unui proces!\n");
                    } else if (pid == 0)  // Proces fiu
                    {
                        f = open(path1, O_WRONLY, 0744);
                        parcurgereDir(argv[i], f);
                        close(f);
                        
                    }

                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                    {
                        printf("Procesul cu  PID %d s-a terminat cu codul %d!\n", pid, WEXITSTATUS(status));
                        //WEXITSTATUS ne da codul de iesire al procesului
                    }
                    else // asta inseamna ca nu includem procesul parinte in verificare
                    {
                        printf("Procesul pentru directorul %s e invalid!\n", argv[i]);
                    }

                    if(comparaSnapshoturi(path, path1) == 0) // comparam pentru a sterge fisierul inutil
                    {
                        stergeSnapshot(path);
                        if (rename(path1, path) != 0) 
                        // functia rename() din stdio.h este folosita pentru a redenumi fisierele 
                        {
                            printf("Fisierul nu a putut fi inlocuit!\n");
                            exit(-1);
                        }
                    }
                    else
                    {
                        stergeSnapshot(path1);
                    }
                }
            }
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
            stergeSnapshot(path);
        }
    }

    return 0;
}