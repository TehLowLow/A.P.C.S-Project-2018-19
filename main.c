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

    int *relKeys; // Array per il backtracking, contiene tutte le chiavi di relazioni in cui la PlainEnt è coinvolta TODO 2
};


struct RelTable {

    unsigned int relNumber; //Numero di entità hashate sotto questo indice

    struct PlainRel *relEntries; //Array che contiene tutte le relazioni hashate sotto un indice
};


struct PlainRel {

    char *relName; //Nome della relazione hashata sotto questo indice

    unsigned int cplNumber; //Numero di elementi(a coppie) presenti nell' array binded

    struct Couples *binded;  //Array di coppie di entità collegate dalla relazione
};


struct Couples {  //coppia sorgente destinazione collegata da una relazione

    struct PlainEnt *source;

    struct PlainEnt *destination;
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
    int tableIndex;
    bool found = false;

    scanf("%ms", &inputEnt);      //Leggo l' entità

    if (strlen(inputEnt) > 3) {

        tableIndex = hash64(inputEnt[1]) * 64 + hash64(inputEnt[2]);   //Se ha più di 1 carattere, hashing

    } else {

        tableIndex = 4096;  //Altrimenti array di singletons

    }


    for (unsigned int a = 0; a < hashTable[tableIndex].entNumber; a++) {

        if (strcmp(hashTable[tableIndex].entEntries[a].entName, inputEnt) == 0) {

            found = true;  //Se l' entità è gia monitorata, non la aggiungo
            break;

        }
    }

    if (!found) {

        if (hashTable[tableIndex].entEntries == NULL) {

            hashTable[tableIndex].entEntries = calloc(1, sizeof(struct PlainEnt));

            //Se la tabella hash non ha ancora entità hashate con quella chiave, alloco
        }

        hashTable[tableIndex].entNumber++;

        unsigned int temp = hashTable[tableIndex].entNumber;

        //Aggiungo una casella in cui salvare l' entità e la aggiungo

        hashTable[tableIndex].entEntries = realloc(hashTable[tableIndex].entEntries,
                                                   hashTable[tableIndex].entNumber * sizeof(struct PlainEnt));

        hashTable[tableIndex].entEntries[temp - 1].entName = malloc(1); //cerco un indirizzo per la stringa.
        strcpy(hashTable[tableIndex].entEntries[temp - 1].entName, inputEnt);

    }
}

//----------------------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addrel, ne calcolo l hash, e aggiungo la realzione
//passata dal comando, sse questa non è gia presente nella tabella.

void HashInputRel(struct RelTable *relHashTable, struct EntTable *entHashTable) {

    char *src;
    char *dest;
    char *inputRel;
    struct PlainEnt *srcPtr = NULL;
    struct PlainEnt *destPtr = NULL;
    int hashedSrc = 0;
    int hashedDest = 0;
    int tableIndex = 0;
    bool srcFound = false;
    bool destFound = false;
    bool relFound = false;


    scanf("%ms", &src);
    scanf("%ms", &dest);
    scanf("%ms", &inputRel);

    //Per prima cosa verifico che le due entità esistano nella EntTable


    if (strlen(src) > 3) {  //Cerco la sorgente

        hashedSrc = hash64(src[1]) * 64 + hash64(src[2]);

        for (unsigned int a = 0; a < entHashTable[hashedSrc].entNumber; a++) {

            if (strcmp(entHashTable[hashedSrc].entEntries[a].entName, src) == 0) {

                srcFound = true;
                srcPtr = &(entHashTable[hashedSrc].entEntries[a]);
                break;
            }
        }

    } else {

        for (unsigned int a = 0; a < entHashTable[4096].entNumber; a++) {

            if (strcmp(entHashTable[4096].entEntries[a].entName, src) == 0) {

                srcFound = true;
                srcPtr = &(entHashTable[4096].entEntries[a]);
                break;
            }
        }
    }

    if (srcFound) { //Se non trovo la sorgente è inutile cercare la dest, tanto poi scarterei il comando

        if (strlen(dest) > 3) {  //Cerco la destinazione

            hashedDest = hash64(dest[1]) * 64 + hash64(dest[2]);

            for (unsigned int a = 0; a < entHashTable[hashedDest].entNumber; a++) {

                if (strcmp(entHashTable[hashedDest].entEntries[a].entName, src) == 0) {

                    destFound = true;
                    destPtr = &(entHashTable[hashedDest].entEntries[a]);
                    break;
                }
            }
        } else {

            for (unsigned int a = 0; a < entHashTable[4096].entNumber; a++) {

                if (strcmp(entHashTable[4096].entEntries[a].entName, src) == 0) {

                    destFound = true;
                    destPtr = &(entHashTable[4096].entEntries[a]);
                    break;
                }
            }
        }
    }


    //Ho trovato entrambe, proseguo. Se non trovo entrambe non faccio nulla.

    if (srcFound && destFound) {

        if (strlen(inputRel) > 3) {

            tableIndex = hash64(inputRel[1]) * 64 + hash64(inputRel[2]);

        } else {

            tableIndex = 4096;
        }

        //Con l' hash di dove dovrebbe collocarsi questa relazione, verifico se effettivamente è gia stata caricata.

        for (unsigned int a = 0;
             a < relHashTable[tableIndex].relNumber; a++) { // a è l' indice di relazione nell' array della hash

            if (strcmp(relHashTable[tableIndex].relEntries[a].relName, inputRel) == 0) {
                //L'ho trovata, verifico se ha gia delle coppie oppure se questa è la prima

                if (relHashTable[tableIndex].relEntries[a].binded ==
                    NULL) {//Se questa coppia è la prima, alloco e aggiungo in testa.         //TODO 1

                    relHashTable[tableIndex].relEntries[a].cplNumber++;

                    unsigned int cplIndex = relHashTable[tableIndex].relEntries[a].cplNumber;

                    relHashTable[tableIndex].relEntries[a].binded = calloc(1, sizeof(struct Couples));
                    relHashTable[tableIndex].relEntries[a].binded[cplIndex - 1].source = srcPtr;
                    relHashTable[tableIndex].relEntries[a].binded[cplIndex - 1].destination = destPtr;

                    //return

                } else {

                    //C'è gia gente allocata, devo verificare che questa coppia non esista ancora per quella relazione,
                    // altrimenti non faccio nulla.                       TODO 2

                    for (unsigned int b = 0; b < relHashTable[tableIndex].relEntries[a].cplNumber; b++) {
                        // b è l' indice di coppia per verificare se la relazione gia esiste

                        if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].source->entName, src) == 0 &&
                            strcmp(relHashTable[tableIndex].relEntries[a].binded[b].destination->entName, dest) == 0) {

                            //Esiste gia, ritorno
                            return;

                        }
                    }

                    //Non esiste, devo aggiungere in coda la coppia di entità. Incremento il numero di couples, rialloco l' array e assegno i puntatori source e dest alla nuova couple.

                    relHashTable[tableIndex].relEntries[a].cplNumber++;

                    unsigned int cplNumbTemp = relHashTable[tableIndex].relEntries[a].cplNumber;

                    struct Couples *tempBind = realloc(relHashTable[tableIndex].relEntries[a].binded, cplNumbTemp);

                    relHashTable[tableIndex].relEntries[a].binded = tempBind;

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].source = srcPtr;

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].destination = destPtr;


                }

            }
        }


    }
}
//TODO 3
//----------------------------------------------------------------------------------------------------------------------








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


inline int hash64(char input) {    //Gioele     71  105


    int hashed = 0;

    hashed = input;

    if (input == 45) {  //Carattere '-'

        hashed = 0;


    } else if (48 <= input && input <= 57) {  //Cifre 0...9

        hashed = input - 47;

    } else if (65 <= input && input <= 90) { //Lettere Maiuscole A..Z

        hashed = input - 54;


    } else if (input == 95) { //Carattere '_'

        hashed = 37;

    } else if (97 <= input && input <= 122) {  //Lettere minuscole a...z

        hashed = input - 59;

    }

    return hashed;

}


//----------------------------------------------------------------------------------------------------------------------
//Questa funzione legge il main txt e si occupa di richiamare le diverse funzioni di parsing nel caso di comando su entità, comando su
//relazione o comando di flusso.

static inline bool ParseTxt(struct EntTable *entTable, struct RelTable *relTable) {

    char *inCommand = NULL;

    scanf("%ms", &inCommand);

    if (inCommand[0] == 'a') {

        if (strcmp(inCommand, "addent") == 0) {//Chiama la funzione che aggiunge un elemento all hash

            HashInputEnt(entTable);


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

    struct EntTable *entitiesHash = initEntHash();
    struct RelTable *relationHash = initRelHash();


}

/* TODO
 *        cercare come mai usavo static inline nelle funzioni.
 *      1 ricorda nelle delete di impostare questo array a null se finiscono le entità, altrimenti si fotte
 *      2 L array di keys è inutile tenerlo di int, è più comodo risparmiare tempo a discapito dello spazio e salvare ptr alla relation in cui
 *        è contenuta l' entità monitorata. Diventa pericoloso in termini di spazio, ma sicuramente non dover fare continue scansioni della
 *        hash è conveniente in termini di tempo.
 *      3 Devo controllare di aver messo bene i return, perchè devo poter uscire dai for appena una condizione non è soddisfatta.
 *
 *
 *
 */