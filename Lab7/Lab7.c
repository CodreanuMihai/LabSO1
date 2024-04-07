#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> // pt functiile sistem open, close, read
#include <time.h>   // pt ctime()
#include <libgen.h> // pt basename()

typedef struct
{
    int lmica[25];
    int lmare[25];
}Hist;

void getLetters(char *fisier, Hist *litere) 
{
    int fd = open(fisier, O_RDONLY, 0);
    if (fd == -1) //verificare deschidere fisier
    {
        printf("Eroare la fisier-ul %s!", fisier);
        exit(-1);
    }

    char buffer;
    int citit;
    while ((citit = read(fd, &buffer, sizeof(char))) > 0) {
        if (buffer >= 'A' && buffer <= 'Z') {
            litere->lmare[buffer - 'A']++;
        } else if (buffer >= 'a' && buffer <= 'z') {
            litere->lmica[buffer - 'a']++;
        }
    }

    close(fd);
}

void scrieHistograma(Hist litere, char *fisHist) 
{
    int his = open(fisHist, O_WRONLY | O_CREAT, 0744);
    // 0744 inseamna:
    // 0 -> fisier obisnuit
    // 744 -> permisiunile
    if (his == -1) //verificare deschidere fisier 
    {
        printf("Eroare la fisier-ul %s!\n", fisHist);
        exit(-1);
    }

    char buffer[2048];
    int dim = 0;
    dim += snprintf(buffer, sizeof(buffer), "Histograma literelor este:\n"); // printeaza sizeof(buffer) elemente in buffer
    
    for (int i = 0; i < 26; i++) 
        dim += snprintf(buffer + dim, sizeof(buffer) - dim, "%c : %d\n", 'a' + i, litere.lmica[i]); 
    // printeaza sizeof(buffer) - dim elemente in buffer, incepand de la pozitia dim!
    for (int i = 0; i < 26; i++) 
        dim += snprintf(buffer + dim, sizeof(buffer) - dim, "%c : %d\n", 'A' + i, litere.lmare[i]);

    dim += snprintf(buffer + dim, sizeof(buffer) - dim, "\n");

    //printf("Dimensiune buffer: %d\n", dim);
    //printf("Buffer: %s\n", buffer);

    if (write(his, buffer, dim) == -1) 
    {
        printf("Eroare la scrierea in fisierul %s!\n", fisHist);
        exit(-1);
    }
    
    close(his);
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

//hardlink = pointer direct spre fisier
//symlink = arata calea (path - ul: exemplu: /home/student/...)

void scrieStat(int statGen, char *path, struct dirent *entry)
{
    char buffer[2048];
    int dim = 0;
    struct stat st;

    if (lstat(path, &st) == -1)  //verificam daca putem obtine informatii despre fisier!
    {
        dim += snprintf(buffer, sizeof(buffer), "Nu putem obtine informatii despre fisier!\n");
        exit(-1);
    }
    
    if (entry->d_type == DT_REG)  // daca e fisier!
    {
        dim += snprintf(buffer, sizeof(buffer), "%s regular_file\n", path);
    }
    else if (S_ISLNK(st.st_mode)) // altfel daca are symlink
    {
        dim += snprintf(buffer, sizeof(buffer), "%s symlink\n", path);
    }

    if (write(statGen, buffer, dim) == -1) 
    {
        printf("Eroare la scrierea in fisierul de statistica generala!\n");
        exit(-1);
    }
}

void parcurgereDir(char *numeDir, Hist *litere, int statGen)
{
    DIR *dir = opendir(numeDir);
    struct dirent *entry;

    if(dir == NULL)
    {
        printf("Eroare la deschiderea directorului!\n");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        //daca nu e directorul curent sau cel in care se afla deja atunci mergi mai departe!
        {
            char path[1024]; 
            sprintf(path, "%s/%s", numeDir, entry->d_name); // calea catre directorul/ fisierul curent

            if (entry->d_type == DT_REG && strstr(path, ".txt")!= NULL)  // daca e fisier afiseaza-l!
            {
                getLetters(path, litere);
                scrieStat(statGen, path, entry);
            }
            else if (entry->d_type == DT_DIR)  // daca e director reapelam functia!
            {
                char buffer[1050];
                int dim = snprintf(buffer, sizeof(buffer), "%s directory\n", path);
                if (write(statGen, buffer, dim) == -1) 
                {
                    printf("Eroare la scrierea in fisierul de statistica generala!\n");
                    exit(-1);
                }
                parcurgereDir(path, litere, statGen);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc < 4)
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

    int statGen = open(argv[3], O_WRONLY | O_CREAT, 0744);
    if (statGen == -1) //verificare deschidere fisier
    {
        printf("Eroare la fisierului de statistica generala!\n");
        exit(-1);
    }

    Hist litere = {0};
    parcurgereDir(argv[1], &litere, statGen);
    close(statGen);

    scrieHistograma(litere, argv[2]);
    return 0;
}