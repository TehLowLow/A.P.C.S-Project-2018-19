#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define DEBUG 0


typedef enum Bool {
    false, true
} bool;

struct EntTable {

    unsigned int entNumber;  //Numero di entità hashate sotto questo indice

    struct PlainEnt *entEntries; //Array che contiene tutte le entità hashate sotto un indice
};


struct PlainEnt {

    char *entName; //Nome dell' entità

    int *relKeys; // Array per il backtracking, contiene tutte le chiavi di relazioni in cui la PlainEnt è coinvolta
};


struct RelTable {

    unsigned int relNumber; //Numero di entità hashate sotto questo indice

    struct PlainRel *relEntries; //Array che contiene tutte le relazioni hashate sotto un indice
};


struct PlainRel {

    char *relName; //Nome della relazione hashata sotto questo indice

    struct Couples *binded;  //Array di coppie di entità collegate dalla relazione
};


struct Couples {  //coppia sorgente destinazione collegata da una relazione

    char **source;

    char **destination;
};

//----------------------------------------------------------------------------------------------------------------------

int hash64(char input);


//----------------------------------------------------------------------------------------------------------------------

//Questa funzione crea la tabella di hashing per le entità. Gestisce 64*64 indici (hashing numerico per i primi due
// caratteri) più una colonna speciale in caso di entità con nome composto da 1 carattere.


struct EntTable *initEntHash() {


    struct EntTable *hash = NULL;

    hash = calloc(4097, sizeof(struct EntTable));

    return hash;

}


//----------------------------------------------------------------------------------------------------------------------

//Questa funzione crea la tabella di hashing per le relazioni. Gestisce 64*64 indici (hashing numerico per i primi due
// caratteri) più una colonna speciale in caso di relazioni con nome composto da 1 carattere.

struct RelTable *initRelHash() {

    struct RelTable *hash = NULL;

    hash = calloc(4097, sizeof(struct RelTable));

    return hash;

}

//----------------------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addent, ne calcolo l hash, e aggiungo in coda
//l' entità passata dal comando, sse questa non è gia presente nella tabella.


void HashInputEnt(struct EntTable *hashTable) {

    char *inputEnt = NULL;
    unsigned int length;
    int tableIndex;
    bool found = false;

    scanf("%ms", &inputEnt);

    if (strlen(inputEnt) > 3) {

        tableIndex = hash64(inputEnt[1]) * 64 + hash64(inputEnt[2]);

    } else {

        tableIndex = 4096;

    }


    for (unsigned int a = 0; a < hashTable[tableIndex].entNumber; a++) {

        if (strcmp(hashTable[tableIndex].entEntries[a].entName, inputEnt) == 0) {

            found = true;

        }


    }

    if (!found) {

        if(hashTable[tableIndex].entEntries == NULL){

            hashTable[tableIndex].entEntries = calloc(1, sizeof(struct PlainEnt));

        }

        hashTable[tableIndex].entNumber++;

        int temp = hashTable[tableIndex].entNumber;

        hashTable[tableIndex].entEntries = realloc(hashTable[tableIndex].entEntries,
                                                   hashTable->entNumber * sizeof(struct PlainEnt));

        strcpy(hashTable[tableIndex].entEntries[temp - 1].entName, inputEnt);

    }
}








//----------------------------------------------------------------------------------------------------------------------

//Questa funzione calcola l indice dell' array in cui inserire l' entità o la relazione. La chiave ritornata è calcolata
//in base a una regola di riduzione dei caratteri validi in input da un indice base 64 a un indice decimale.

//La compressione dei valori ascii di un carattere a valori compresi fra 0 e 63 avviene come:

/* Se incontro il carattere "-", gli assegno il valore 0;
 * Se incontro una cifra sottraggo al suo valore ascii 47;
 * Se incontro una lettera maiuscola sottraggo 54
 * Se incontro il carattere "_" gli assegno il valore 37
 * Se incontro una lettera minuscola sottraggo 59
 */

//Una volta compressa la lettera, calcolo l hash come:

//Val_prima * 64 + val_seconda.

//Questo mi ritorna un indice compreso fra 0 e 4095, e lascia il valore indice 4096 per le entità che son più corte di
//2 caratteri.


int hash64(char input) {


    int hashed = 0;

    hashed = input;

    if (input == 45) {

        hashed = 0;


    } else if (48 <= input && input <= 57) {

        hashed = input - 47;

    } else if (65 <= input && input <= 90) {

        hashed = input - 54;


    } else if (input == 95) {

        hashed = 37;

    } else if (97 <= input && input <= 122) {

        hashed = input - 59;

    }

    return hashed;

}


//----------------------------------------------------------------------------------------------------------------------
//Questa funzione legge il main txt e si occupa di richiamare le diverse funzioni di parsing nel caso di comando su entità, comando su
//relazione o comando di flusso.

bool ParseTxt() {

    char *inCommand = NULL;

    scanf("%ms", &inCommand);

    if (inCommand[0] == 'a') {

        if (strcmp(inCommand, "addent") == 0) {//Chiama la funzione che aggiunge un elemento all hash



            if (DEBUG) { printf("%s", inCommand); }
            return true;

        } else {//Chiama la funzione che aggiunge una relazione alla hash


            if (DEBUG) { printf("%s", inCommand); }
            return true;

        }

    } else if (inCommand[0] == 'd') {

        if (strcmp(inCommand, "delent") == 0) { //chiama la funzione di elimina elemento


            if (DEBUG) { printf("%s", inCommand); }
            return true;


        } else {  //chiama la funzione di elimina relazione


            if (DEBUG) { printf("%s", inCommand); }
            return true;

        }


    } else if (inCommand[0] == 'r') {/*chiama il report*/

        if (DEBUG) { printf("%s", inCommand); }
        return true;

    } else if (inCommand[0] == 'e') {/*termino*/ return false; }


}


int main() {

    struct EntTable *test = initEntHash();

    HashInputEnt(test);

}
