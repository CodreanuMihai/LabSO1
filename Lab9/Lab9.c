#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h> // pt waitpid()
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  // pt write(), open() si close()
#include <libgen.h> // pt basename()

#define LUNGIME_CMD 4096
#define LUNGIME_PATH 1024


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

void cautaSir(const char *numeDir, const char *sirCautat)
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului de lucru!\n");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        //daca nu e directorul curent sau cel in care se afla deja atunci mergi mai departe!
        {
            char path[LUNGIME_PATH]; 
            snprintf(path, sizeof(path), "%s/%s", numeDir, entry->d_name); // calea catre directorul/ fisierul curent

            if (entry->d_type == DT_REG)  // daca e fisier afiseaza-l!
            {
                int status, pid = fork(); //creeam un proces
                if (pid == -1) 
                {
                    printf("Eroare la crearea unui proces!\n");
                } else if (pid == 0)  // Proces fiu
                {
                    char comanda[LUNGIME_CMD];
                    snprintf(comanda, sizeof(comanda), "grep -H \"%s\" \"%s\" ", sirCautat, path);
                    // includem comanda de executat intr-un string pentru a o putea reprezenta cum trebuie(ca o expresie regulata)
                    // -H: aceasta optiune indica comenzii grep sa afiseze numele fisierului din care citeste
                    if(execlp("sh", "sh", "-c", comanda, NULL) == -1) // execlp va executa shel-ul sh cu comanda data in 'comanda' 
                    // primul sh -> numele interpretorului de comenzi
                    // al doilea sh -> programul pe care il vede sistemul de operare
                    // -c -> arata ca urmeaza o comanda
                    // comanda -> comanda care se va executa
                    {
                        printf("Eroare la executia sed!\n");
                        exit(-1);
                    }
                    exit(0); // inchidem firul de executie al procesului la incheierea tuturor instructiunilor acestuia
                }

                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                {
                    printf("Procesul pentru fisierul %s e invalid!\n", path);
                }
            }
            else if (entry->d_type == DT_DIR)  // daca e director reapelam functia!
            {
                int status, pid = fork(); //creeam un proces
                if (pid == -1) 
                {
                    printf("Eroare la crearea unui proces!\n");
                } else if (pid == 0)  // Proces fiu
                {
                    cautaSir(path, sirCautat);
                    exit(0); // inchidem firul de executie al procesului la incheierea tuturor instructiunilor acestuia
                }

                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                {
                    printf("Procesul pentru directorul %s e invalid!\n", path);
                }
            }
        }
    }
    closedir(dir);
}

void inlocuiesteSir(const char *numeDir, const char *textVechi, const char *textNou)
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului de lucru!\n");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        //daca nu e directorul curent sau cel in care se afla deja atunci mergi mai departe!
        {
            char path[LUNGIME_PATH]; 
            snprintf(path, sizeof(path), "%s/%s", numeDir, entry->d_name); // calea catre directorul/ fisierul curent

            if (entry->d_type == DT_REG)  // daca e fisier afiseaza-l!
            {
                int status, pid = fork(); //creeam un proces
                if (pid == -1) 
                {
                    printf("Eroare la crearea unui proces!\n");
                } else if (pid == 0)  // Proces fiu
                {
                    char comanda[LUNGIME_CMD];
                    snprintf(comanda, sizeof(comanda), "sed -i 's/%s/%s/g' %s", textVechi, textNou, path);
                    // includem comanda de executat intr-un string pentru a o putea reprezenta cum trebuie(ca o expresie regulata)
                    // -i: aceasta comanda inseamna ca inlocuirea se face direct in fisier
                    if(execlp("sh", "sh", "-c", comanda, NULL) == -1) // execlp va executa shel-ul sh cu comanda data in 'comanda' 
                    // primul sh -> numele interpretorului de comenzi
                    // al doilea sh -> programul pe care il vede sistemul de operare
                    // -c -> arata ca urmeaza o comanda
                    // comanda -> comanda care se va executa
                    {
                        printf("Eroare la executia sed!\n");
                        exit(-1);
                    }
                    exit(0); // inchidem firul de executie al procesului la incheierea tuturor instructiunilor acestuia
                }

                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                {
                    printf("Procesul pentru fisierul %s e invalid!\n", path);
                }
                else
                {
                    printf("Inlocuire realizata in fisierul %s!\n", path);
                }
            }
            else if (entry->d_type == DT_DIR)  // daca e director reapelam functia!
            {
                int status, pid = fork(); //creeam un proces
                if (pid == -1) 
                {
                    printf("Eroare la crearea unui proces!\n");
                } else if (pid == 0)  // Proces fiu
                {
                    inlocuiesteSir(path, textVechi, textNou);
                    exit(0); // inchidem firul de executie al procesului la incheierea tuturor instructiunilor acestuia
                }

                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) // verificam modul in care s-a terminat procesul
                {
                    printf("Procesul pentru directorul %s e invalid!\n", path);
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc < 4)
    {
        printf("Nu ati dat destule argumente!\n");
        exit(-1);
    }

    char dirIesire[256];
    int err = verificareDir(argv[1]);
    if(err != 0)
    {
        printf("Nu avem fisier de iesire! => Il cream noi!\n");
        // Il cream noi:
        if(mkdir(basename((char *)argv[1]), 0777) == -1)
        //mkdir() din unistd.h este o functie care creeaz un director nou in directorul curent!
        {
            printf("Nu s-a putut crea directorul de iesire!\n");
            exit(-1);
        }
    }        
    snprintf(dirIesire, sizeof(dirIesire), "%s", argv[1]); // directorul de iesire

    if(strcmp(argv[2], "grep") == 0)
    {
        //partea cu grep
        cautaSir(dirIesire, argv[3]);
    }
    else if(strcmp(argv[2], "sed") == 0)
    {
        //partea cu sed
        if(argc < 5)
        {
            printf("Nu ati dat destule argumente pt sed!\n");
            exit(-1);
        }
        inlocuiesteSir(dirIesire, argv[3], argv[4]);
    }
    else
    {
        printf("Al doilea argument dat nu este o comanda!\n");
        exit(-1);
    }
    return 0;
}