In directorul dirIzolare se afla fisierele corupte, dar ele au drept de citire pentru user
si group caci altfel nu le puteam adauga pe git. Daca se vrea folosirea lor, trebuie mutate
in alta parte si apoi in directorul in care a fost mutat/mutate sa fie rulata comanda "chmod
000 nume_fisier" in terminal cu numele fiecarui fisier! De asemenea, in directorul dirTest2
se afla si un fisier cauia ii vor trebui luate drepturile pentru a se verifica ca e safe (el 
a fost conceput ca un fisier safe, dar care nu are drepturi!). Numele lui este fakeCorr.txt!
