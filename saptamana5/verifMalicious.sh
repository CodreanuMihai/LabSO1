#!/bin/bash
if [ $# -ne 2 ]; 
then
    echo "Scriptul nu a primit suficiente argumente! -> a primit $#"
    exit -1
fi

if [ ! -f "$1" ]; 
then
    echo "Problema la primul argument al scriptului!"
    exit -1
fi

if [ ! -d "$2" ]; 
then
    echo "Problema la al doilea argument al scriptului!"
    exit -1
fi

ok=0
# Vom folosi wc pt a numara caracterele, cuvintele si liniile (-l pt linii, -w pt covinte si -c pentru caractere)
num_linii=$(wc -l < "$1")
num_cuvinte=$(wc -w < "$1")
num_caractere=$(wc -c < "$1")
if [ $num_linii -lt 3 ] && [ $num_cuvinte -gt 1000 ] && [ $num_caractere -gt 2000 ]; 
then
    mv "$1" "$2"
    ok=1
fi

# Cautam cuvintele cheie cu grep si daca le gasim, trimitem fisierul in directorul de izolare
cuvinte_cheie=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
for cuvant in "${cuvinte_cheie[@]}"; 
do
    if [ $ok -eq 0 ]; 
    then
        if grep -q "$cuvant" "$1"; 
        # -q: il face pe grep sa nu afiseze nimic
        then
            mv "$1" "$2"
            ok=1
        fi
    fi
done

# Cautam caractere non-ASCII si daca le gasim atunci trimitem fisierul in directorul de izolare!
# Caracterele ASCII sunt reprezentate cu numere de la 0x00 la 0x7F, asa ca cele non ascii vor fi cele din afara acestui interval
# -P: il face pe grep sa lucreze cu expresii regulate
if [ $ok -eq 0 ]; 
then
    if grep -q -P "[\x80-\xFF]" "$1"; 
    then
        mv "$1" "$2"
        ok=1
    fi
fi

if [ $ok -eq 0 ]; 
then
    echo "SAFE"
else
    echo "1"
fi