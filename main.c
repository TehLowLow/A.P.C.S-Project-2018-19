#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdatomic.h>


typedef enum Bool {
    false, true
} bool;

struct EntTable {

    unsigned int entNumber;  //Numero di entità hashate sotto questo indice

    struct PlainEnt *entEntries; //Array che contiene tutte le entità hashate sotto un indice
};


struct PlainEnt {

    char *entName; //Nome dell' entità

    struct Track *backTrack;

    unsigned int backtrackIndex; //Conteggio dim array
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

    char **source; //Array di sorgenti

    char *destination; //nome della destinazione a cui tutte e sorgenti son legate

    unsigned int srcNumber;
};


struct Track {

    char *relName;
    unsigned int counter;


};

char *CONST_TERM = "@@@@";
//-----PROTOTYPES-------------------------------------------------------------------------------------------------------

static inline int hash64(char input);

static inline struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash);

static inline struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash);

static inline struct ReportEnt *
sortCouples(struct RelTable *relHash, int tableIndex, unsigned int relIndex, unsigned int coupleNum);

static inline void FixBacktrack(char *relName, char *entName, struct EntTable *entHash);

static inline void bindRemover(char *relName, char *entName, struct RelTable *relHash, struct EntTable *entHash);

static inline void AddBacktrack(struct PlainEnt *toAdd, char *relName);

//-----MEMORY INIT------------------------------------------------------------------------------------------------------

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

//-----COMMANDS---------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addent, ne calcolo l hash, e aggiungo in coda
//l' entità passata dal comando, sse questa non è gia presente nella tabella.


static inline void HashInputEnt(struct EntTable *hashTable) {  //TODO Funzione OK, non necessita cambi.

    char *inputEnt = NULL;
    int tableIndex;
    struct PlainEnt *result = NULL;


    scanf("%ms", &inputEnt);      //Leggo l' entità

    if (strlen(inputEnt) > 3) {
        tableIndex = hash64(inputEnt[1]) * 64 + hash64(inputEnt[2]);   //Se ha più di 1 carattere, hashing
    } else {
        tableIndex = 4096;  //Altrimenti array di singletons
    }

    result = EntityLookup(inputEnt, tableIndex, hashTable);

    if (result == NULL) { //Se la tabella hash non ha ancora entità hashate con quella chiave, alloco

        if (hashTable[tableIndex].entEntries == NULL) {  //TODO 1

            hashTable[tableIndex].entNumber = 1;

            hashTable[tableIndex].entEntries = calloc(1, sizeof(struct PlainEnt));

            hashTable[tableIndex].entEntries[0].entName = malloc(strlen(inputEnt) + 1);

            strcpy(hashTable[tableIndex].entEntries[0].entName, inputEnt);

            hashTable[tableIndex].entEntries->backtrackIndex = 0;

            hashTable[tableIndex].entEntries->backTrack = NULL;


        } else {

            hashTable[tableIndex].entNumber++;

            unsigned int temp = hashTable[tableIndex].entNumber;

            //Aggiungo una casella in cui salvare l' entità e la aggiungo

            hashTable[tableIndex].entEntries = realloc(hashTable[tableIndex].entEntries,
                                                       temp * sizeof(struct PlainEnt));


            hashTable[tableIndex].entEntries[temp - 1].entName = malloc(
                    strlen(inputEnt) + 1); //cerco un indirizzo per la stringa. Segfaulta altrimenti

            strcpy(hashTable[tableIndex].entEntries[temp - 1].entName, inputEnt);

            hashTable[tableIndex].entEntries[temp - 1].backtrackIndex = 0;
            hashTable[tableIndex].entEntries[temp - 1].backTrack = NULL;

        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addrel, ne calcolo l hash, e aggiungo la realzione
//passata dal comando, sse questa non è gia presente nella tabella.

static inline void HashInputRel(struct RelTable *relHashTable, struct EntTable *entHashTable) {

    char *src;
    char *dest;
    char *inputRel;
    int hashedSrc = 0;
    int hashedDest = 0;
    int tableIndex = 0;
    struct PlainEnt *srcFound = NULL;
    struct PlainEnt *destFound = NULL;

    unsigned int ordered;
    unsigned int bufferCounter;

    //Leggo i parametri del comando

    scanf("%ms", &src);
    scanf("%ms", &dest);
    scanf("%ms", &inputRel);

    //Per prima cosa verifico che le due entità esistano nella EntTable

    if (strlen(src) > 3) {  //Calcolo l' hash
        hashedSrc = hash64(src[1]) * 64 + hash64(src[2]);
    } else {
        hashedSrc = 4096;
    }

    srcFound = EntityLookup(src, hashedSrc, entHashTable); //Cerco la sorgente

    if (srcFound != NULL) { //Se non trovo la sorgente è inutile cercare la dest, tanto poi scarterei il comando

        if (strlen(dest) > 3) {  //Cerco la destinazione
            hashedDest = hash64(dest[1]) * 64 + hash64(dest[2]);
        } else {
            hashedDest = 4096;
        }

        destFound = EntityLookup(dest, hashedDest, entHashTable);

    }


    //Ho trovato entrambe, proseguo. Se non trovo entrambe non faccio nulla.

    if (srcFound != NULL && destFound != NULL) {

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

                    relHashTable[tableIndex].relEntries[a].cplNumber = 1;
                    relHashTable[tableIndex].relEntries[a].binded = calloc(1, sizeof(struct Couples));
                    relHashTable[tableIndex].relEntries[a].binded[0].destination = malloc(
                            strlen(destFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[0].destination, destFound->entName);
                    relHashTable[tableIndex].relEntries[a].binded[0].srcNumber = 1;
                    relHashTable[tableIndex].relEntries[a].binded[0].source = malloc(sizeof(char *));
                    relHashTable[tableIndex].relEntries[a].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[0].source[0], srcFound->entName);


                    //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare

                    AddBacktrack(srcFound, inputRel);
                    AddBacktrack(destFound, inputRel);


                    return;

                } else {

                    //C'è gia gente allocata, devo verificare che questa coppia non esista ancora per quella relazione,
                    // altrimenti non faccio nulla.

                    for (unsigned int b = 0; b < relHashTable[tableIndex].relEntries[a].cplNumber; b++) {
                        // b è l' indice di coppia per verificare se la relazione gia esiste

                        if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].destination, dest) == 0) {

                            //trovo la dest, vedo se c è anche la src

                            for (unsigned int i = 0;
                                 i < relHashTable[tableIndex].relEntries[a].binded[b].srcNumber; i++) {

                                if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].source[i],
                                           srcFound->entName) == 0) {

                                    //Ho trovato anche la src, ritorno senza fare nulla
                                    return;

                                }

                            }

                            //Se dopo questo for la src non esiste, devo aggiungerla in coda.
                            relHashTable[tableIndex].relEntries[a].binded[b].srcNumber++;

                            unsigned int temp = relHashTable[tableIndex].relEntries[a].binded[b].srcNumber;

                            relHashTable[tableIndex].relEntries[a].binded[b].source = realloc(
                                    relHashTable[tableIndex].relEntries[a].binded[b].source, temp * sizeof(char *));

                            relHashTable[tableIndex].relEntries[a].binded[b].source[temp - 1] = malloc(
                                    strlen(srcFound->entName) + 1);

                            strcpy(relHashTable[tableIndex].relEntries[a].binded[b].source[temp - 1],
                                   srcFound->entName);

                            //Aggiungo bt

                            AddBacktrack(srcFound, inputRel);
                            AddBacktrack(destFound, inputRel);

                            return;

                        }
                    }

                    //Non esiste, devo aggiungere la coppia di entità.

                    //Primo major fix: L' aggiunta della couple deve avvenire in ordine


                    unsigned int cplNumbTemp = relHashTable[tableIndex].relEntries[a].cplNumber;
                    unsigned int counter = 0;

                    for (unsigned int j = 0; j < cplNumbTemp; j++) {

                        if (strcmp(destFound->entName, relHashTable[tableIndex].relEntries[a].binded[j].destination) <
                            0) {

                            break;

                        }

                        counter++;


                    }

                    //Counter contiene la posizione in cui va piazzata la nuova coppia

                    relHashTable[tableIndex].relEntries[a].cplNumber++;


                    struct Couples *tempCouples = calloc(relHashTable[tableIndex].relEntries[a].cplNumber,
                                                         sizeof(struct Couples));

                    unsigned int temp = 0;

                    while (temp < counter) {

                        //Copia in ordine

                        tempCouples[temp].destination = malloc(
                                strlen(relHashTable[tableIndex].relEntries[a].binded[temp].destination) + 1);

                        strcpy(tempCouples[temp].destination,
                               relHashTable[tableIndex].relEntries[a].binded[temp].destination);

                        tempCouples[temp].srcNumber = relHashTable[tableIndex].relEntries[a].binded[temp].srcNumber;
                        tempCouples[temp].source = relHashTable[tableIndex].relEntries[a].binded[temp].source;

                        temp++;


                    }

                    temp = counter;

                    tempCouples[temp].destination = malloc(strlen(destFound->entName) + 1);
                    strcpy(tempCouples[temp].destination, destFound->entName);
                    tempCouples[temp].srcNumber = 1;
                    tempCouples[temp].source = malloc(sizeof(char *));
                    tempCouples[temp].source[0] = malloc(strlen(srcFound->entName) + 1);
                    strcpy(tempCouples[temp].source[0], srcFound->entName);

                    temp++;

                    while (temp < relHashTable[tableIndex].relEntries[a].cplNumber) {

                        //Copia le vecchie sfasate di uno

                        tempCouples[temp].destination = malloc(
                                strlen(relHashTable[tableIndex].relEntries[a].binded[temp - 1].destination) + 1);

                        strcpy(tempCouples[temp].destination,
                               relHashTable[tableIndex].relEntries[a].binded[temp - 1].destination);

                        tempCouples[temp].srcNumber = relHashTable[tableIndex].relEntries[a].binded[temp - 1].srcNumber;
                        tempCouples[temp].source = relHashTable[tableIndex].relEntries[a].binded[temp - 1].source;

                        temp++;


                    }

                    relHashTable[tableIndex].relEntries[a].binded = tempCouples;

                    //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
                    AddBacktrack(srcFound, inputRel);
                    AddBacktrack(destFound, inputRel);

                    return;

                }


            }
        }

        //Qua devo invece aggiungere la nuova relazione che non ho mai incontrato, e la aggiungo gia in ordine



        if (relHashTable[tableIndex].relNumber == 0) {

            // Se la table è vuota inserisco subito in testa, e aggiungo le due entità

            relHashTable[tableIndex].relNumber++;

            relHashTable[tableIndex].relEntries = calloc(relHashTable[tableIndex].relNumber,
                                                         sizeof(struct PlainRel));  //Alloco la prima cella di entries e la nomino

            relHashTable[tableIndex].relEntries[0].relName = malloc(strlen(inputRel) + 1);

            strcpy(relHashTable[tableIndex].relEntries[0].relName, inputRel);

            relHashTable[tableIndex].relEntries[0].cplNumber++;

            relHashTable[tableIndex].relEntries[0].binded = calloc(1,
                                                                   sizeof(struct Couples)); // Alloco la prima cella di Coppie e le assegno

            relHashTable[tableIndex].relEntries[0].binded[0].destination = malloc(strlen(destFound->entName) + 1);

            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].destination, destFound->entName);

            relHashTable[tableIndex].relEntries[0].binded[0].srcNumber = 1;

            relHashTable[tableIndex].relEntries[0].binded[0].source = malloc(sizeof(char *));

            relHashTable[tableIndex].relEntries[0].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);

            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].source[0], srcFound->entName);

            //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare


            AddBacktrack(srcFound, inputRel);
            AddBacktrack(destFound, inputRel);
            return;

        }

        //Se la table non è vuota, devo cercare l' indice

        ordered = 0;

        for (unsigned int a = 0; a < relHashTable[tableIndex].relNumber; a++) {

            if (strcmp(inputRel, relHashTable[tableIndex].relEntries[a].relName) < 0) {

                break;

            }

            ordered++;

        }


        //ordered ora contiene l' indice di dove devo piazzare la nuova relazione, devo riallocare

        relHashTable[tableIndex].relNumber++;

        struct PlainRel *buffer = calloc(relHashTable[tableIndex].relNumber, sizeof(struct PlainRel));

        bufferCounter = 0;

        while (bufferCounter < ordered) { //Copio tutti quelli prima del posto in cui inserire la nuova struct

            buffer[bufferCounter].relName = malloc(
                    strlen(relHashTable[tableIndex].relEntries[bufferCounter].relName) + 1);
            strcpy(buffer[bufferCounter].relName,
                   relHashTable[tableIndex].relEntries[bufferCounter].relName);  //Copio il nome
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter].cplNumber; //Copio il numero di coppie
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter].binded; //Copio il ptr all' array di coppie

            bufferCounter++; //Incremento e ripeto

        }

        bufferCounter = ordered; //Inserisco la nuova struct. La copia è identica a quella scritta sopra e a quella successiva.

        buffer[bufferCounter].relName = malloc(strlen(inputRel) + 1);

        strcpy(buffer[bufferCounter].relName, inputRel);
        buffer[bufferCounter].cplNumber = 1;
        buffer[bufferCounter].binded = calloc(1, sizeof(struct Couples));


        bufferCounter++;

        while (bufferCounter < relHashTable[tableIndex].relNumber) { //Copio tutti i successivi

            strcpy(buffer[bufferCounter].relName, relHashTable[tableIndex].relEntries[bufferCounter - 1].relName);
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter - 1].cplNumber;
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter - 1].binded;

            bufferCounter++;


        }

        free(relHashTable[tableIndex].relEntries); //Libero il vecchio array precedente all' inserimento

        relHashTable[tableIndex].relEntries = buffer;  //Gli assegno il ptr del nuovo array ordinato

        //Ci aggiungo la nuova relazione
        relHashTable[tableIndex].relEntries[ordered].binded[0].srcNumber = 1;
        relHashTable[tableIndex].relEntries[ordered].binded[0].destination = malloc(strlen(destFound->entName) + 1);
        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].destination, destFound->entName);
        relHashTable[tableIndex].relEntries[ordered].binded[0].source = malloc(sizeof(char *));
        relHashTable[tableIndex].relEntries[ordered].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);
        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].source[0], srcFound->entName);
        //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
        AddBacktrack(srcFound, inputRel);
        AddBacktrack(destFound, inputRel);

        return;
    }
}

/*Ordinamento Lessicografico
 *
 * Uso strcmp
 *
 * Entro nell' array e faccio strcmp della stringa in input con tutte le stringhe in memoria,al primo indice in cui
 * ho minore scorrendo dall' inizio esco dal ciclo e so che devo infilarla li.
 *
 *
 * Funzionamento generale:
 *
 *
 * leggo e salvo;
 * entro nella tabella sotto l' hash che calcolo;
 * scorro l' array dall' inizio facendo
 *
 *
 * (for every_relation){
 * if(strcmp(input, hashtable)<0){la parola va in quell' indice, return indice}}
 *
 *
 * Non uso realloc, perchè mi copia anche i dati, ma devo pensare a una mia realloc, che copi i dati prima dell' indice
 * uguali, all' indice inserisca la nuova relazione, e poi copi i dati successivi sfasati di un indice.
 *
 */


//TODO 3
//----------------------------------------------------------------------------------------------------------------------

//La delent deve entrare in un entità, leggere tutto il backtrack e eliminare le relazioni con lei come sorgente o destinazione

//Per prima cosa hash e ricerca dell' entità


static inline void DeleteEnt(struct EntTable *entHash, struct RelTable *relHash) {

    char *toDelete;
    unsigned int hashEnt;
    struct PlainEnt *deleteEnt;


    scanf("%ms", &toDelete);

    if (strlen(toDelete) > 3) {

        hashEnt = hash64(toDelete[1]) * 64 + hash64(toDelete[2]);

    } else {

        hashEnt = 4096;

    }


    deleteEnt = EntityLookup(toDelete, hashEnt, entHash);

    //Una volta che ho l' entità da cancellare entro nel suo backtrack e inizio a rimuovere tutte le couples per ogni relazione.

    if (deleteEnt != NULL) {


        //Per tutti i backtrack che ha salvato in memoria elimino ogni traccia dell' entità

        for (unsigned int i = 0; i < deleteEnt->backtrackIndex; i++) {

            bindRemover(deleteEnt->backTrack[i].relName, toDelete, relHash, entHash);

        }

        //Eliminata ogni traccia di deleteEnt dalle relazioni, devo eliminarla dalla hashtable

        //Cerco l' indice a cui compare l' entità, alloco un altro array, e poi ricopio nel nuovo array tutte le entità
        // esclusa quella da eliminare

        unsigned int order = 0;

        for (unsigned int j = 0; j < entHash[hashEnt].entNumber; j++) {

            if (strcmp(entHash[hashEnt].entEntries[j].entName, deleteEnt->entName) == 0) {


                break;


            }

            order++;  //Order salva l' indice dell' entità da eliminare



        }


        unsigned int i = 0;

        struct PlainEnt *newEntities = calloc(entHash[hashEnt].entNumber - 1, sizeof(struct PlainEnt));

        while (i < order) {

            //Ricopio le entità paro paro

            newEntities[i].backtrackIndex = entHash[hashEnt].entEntries[i].backtrackIndex;
            newEntities[i].backTrack = entHash[hashEnt].entEntries[i].backTrack;
            newEntities[i].entName = malloc(strlen(entHash[hashEnt].entEntries[i].entName) + 1);
            strcpy(newEntities[i].entName, entHash[hashEnt].entEntries[i].entName);

            i++;


        }

        i = order + 1;

        while (i < entHash[hashEnt].entNumber) {

            newEntities[i - 1].backtrackIndex = entHash[hashEnt].entEntries[i].backtrackIndex;
            newEntities[i - 1].backTrack = entHash[hashEnt].entEntries[i].backTrack;
            newEntities[i - 1].entName = malloc(strlen(entHash[hashEnt].entEntries[i].entName) + 1);
            strcpy(newEntities[i - 1].entName, entHash[hashEnt].entEntries[i].entName);

            i++;

        }

        entHash[hashEnt].entNumber--;

        free(entHash[hashEnt].entEntries);

        entHash[hashEnt].entEntries = newEntities;


    }
}

//----------------------------------------------------------------------------------------------------------------------

/*DelRel deve cancellare il binded nella relazione che compare nel comando, quindi:
 *
 * delrel a b relname
 *
 * è come dire
 *
 * "Entra in relname, ed elimina la struct Couple che contiene a e b
 *
 *
 * Quindi la prima cosa da fare è verificare:
 *
 * Che la relazione esista;
 * Che esista la coppia a & b.
 *
 * è inutile verificare che esistano a e b, perchè se esiste la coppia a & b, allora è garantito che esistano sia a che b
 * se non esiste la coppia, allora delrel non deve fare nulla, sia che a o b esistano o meno.
 *
 * Quando cancello una relazione devo cancellare il backtracking? Si
 *
 *
 * Scorro tutte le coppie, cancello quella interessata e poi faccio il fix del backtrack.
 * Il fix consiste nello scorrere tutto l' array di binded della relazione.
 *
 * Se sorgente e destinazione compaiono in altre relazioni tutto ok, se non compaiono devo cancellare l' id dal backtracking
 *
 */


static inline void DeleteRel(struct RelTable *relHash, struct EntTable *entHash) {

    char *src;
    char *dest;
    char *rel;
    struct PlainRel *relFound;
    bool nullDest = false;


    int tableIndex;

    //Leggo il comando
    scanf("%ms", &src);
    scanf("%ms", &dest);
    scanf("%ms", &rel);


    if (strlen(rel) > 3) {

        tableIndex = hash64(rel[1]) * 64 + hash64(rel[2]);
    } else {

        tableIndex = 4096;

    }


    relFound = RelationLookup(rel, tableIndex, relHash); //Cerco la relazione nella hash

    if (relFound != NULL && relFound->binded != NULL) {//Se ho delle coppie sotto la relazione


        for (unsigned int i = 0; i < relFound->cplNumber; i++) {

            if (strcmp(relFound->binded[i].destination, dest) == 0) {

                //Ho trovato la dest, ora vedo se la dest ha una src uguale a quella del comando

                for (unsigned int j = 0; j < relFound->binded[i].srcNumber; j++) {

                    if (strcmp(relFound->binded[i].source[j], src) == 0) {

                        //ho trovato anche la src, percui devo levarla, riallocare e poi fixare il bt di src e dest.
                        //i casi son tre, o la src è l' unica entità legata a dest e allora devo far sparire entrambi
                        //o la src è una src fra tante MA non è l'ultima
                        //la src è l ultima dell array


                        if (relFound->binded[i].srcNumber == 0) { //TODO fix barbaro

                            //ho finito le src percui poi devo liberare anche la dest

                            nullDest = true;
                            break;

                        } else if (j == relFound->binded[i].srcNumber - 1) {

                            //La src sta in fondo, devo semplicemente deallocare e riallocare

                            free(relFound->binded[i].source[j]);
                            relFound->binded[i].srcNumber--;
                            relFound->binded[i].source = realloc(relFound->binded[i].source,
                                                                 relFound->binded[i].srcNumber * sizeof(char *));

                        } else {

                            //la src ha piu stringhe src, quindi sposto in fondo e rialloco

                            relFound->binded[i].source[j] = relFound->binded[i].source[relFound->binded[i].srcNumber -
                                                                                       1];
                            //Ora rialloco

                            relFound->binded[i].srcNumber--;

                            relFound->binded[i].source = realloc(relFound->binded[i].source,
                                                                 relFound->binded[i].srcNumber *
                                                                 sizeof(char *));


                            FixBacktrack(rel, src, entHash);
                            FixBacktrack(rel, dest, entHash);

                            break;


                        }
                    }


                }

                if (nullDest) {

                    //Devo verificare il caso in cui oltre a essere l' ultima source di dest, questa dest sia anche l' ultima
                    //dest di rel

                    if (relFound->cplNumber == 1) {

                        //libero tutto e me ne fotto, ma la relazione resta in memoria

                        relFound->cplNumber = 0;
                        free(relFound->binded[0].source);
                        free(relFound->binded);
                        relFound->binded = NULL;

                        FixBacktrack(rel, src, entHash);
                        FixBacktrack(rel, dest, entHash);

                        return;

                    } else {

                        //Libero la source sotto la dest vuota
                        free(relFound->binded[i].source); //TODO devo liberare tutte le stringhe

                        //devo riallocare l' array di dest

                        //Tutti quelli prima di i vengono ricopiati uguale

                        struct Couples *newBind = calloc(relFound->cplNumber - 1, sizeof(struct Couples));
                        unsigned int temp = 0;


                        while (temp < i) {

                            newBind[temp].destination = malloc(strlen(relFound->binded[temp].destination) + 1);
                            strcpy(newBind[temp].destination, relFound->binded[temp].destination);
                            newBind[temp].srcNumber = relFound->binded[temp].srcNumber;
                            newBind[temp].source = relFound->binded[temp].source;

                            temp++;

                        }

                        temp = i + 1;

                        while (temp < relFound->cplNumber) {

                            newBind[temp - 1].destination = malloc(strlen(relFound->binded[temp].destination));
                            strcpy(newBind[temp - 1].destination, relFound->binded[temp].destination);
                            newBind[temp - 1].srcNumber = relFound->binded[temp].srcNumber;
                            newBind[temp - 1].source = relFound->binded[temp].source;

                            temp++;

                        }


                        relFound->binded = newBind;  //Todo mancan delle free importanti i think, nel caso fixo
                        relFound->cplNumber--;

                        //Fixo i bt di source e dest

                        FixBacktrack(rel, src, entHash);
                        FixBacktrack(rel, dest, entHash);

                        return;
                    }
                }


            }


        }

    }
}

//----------------------------------------------------------------------------------------------------------------------

//Il report deve scorrere tutta la tabella hash, e stampare tutte le relazioni che hanno almeno una coppia con il loro max ricevente
//Il conteggio dei max riceventi lo faccio a tempo di report.
//L' idea è, scorro per ogni relazione l' array di coppie, entro negli indirizzi e nella tabella di entità incremento
//un contatore. Una volta incrementato tutti i contatori, rileggo l' array e trovo l' int più grande. Scorro una terza volta
// e salvo tutti coloro che come indice han quel valore e nel mentre resetto tutti gli indici a zero

//Complessità spaziale: un unsigned int per ogni entità
//Complessità temporale = Theta(3n)

//Si potrebbe fare meglio in tempo, ma in spazio è praticamente minimo, perche si tratta di un byte per ogni diverso destinatario, e un array con
// k elementi = numero di destinatari con valore max di sorgenti.




static inline void Report(struct RelTable *relHash) {


    struct ReportEnt *entReport;

    bool isEmpty = true;
    bool isFirst = true;


    for (int index = 0; index < 4097; index++) { //Per ogni chiave della hash

        if (relHash[index].relNumber != 0) { //Se la chiave corrisponde a delle relazioni

            for (unsigned int a = 0; a < relHash[index].relNumber; a++) {//Per ogni relazione di quella chiave

                //Se l' array di binded è diverso da null, entra e cerca il massimo ricevente con un primo giro,
                //Successivamente stampa tutti i massimi riceventi


                if (relHash[index].relEntries[a].binded != NULL && relHash[index].relEntries[a].cplNumber > 0) {


                    isEmpty = false;

                    unsigned int MAX_REC = 0;

                    for (unsigned int i = 0; i < relHash[index].relEntries[a].cplNumber; i++) {

                        if (relHash[index].relEntries[a].binded[i].srcNumber > MAX_REC) {


                            MAX_REC = relHash[index].relEntries[a].binded[i].srcNumber;

                        }
                    }

                    //Ora devo stampare il nome relazione nei due formati, tutti quelli che sono a MAX_REC e poi il valore di MAX_REC

                    if (MAX_REC > 0) {

                        if (isFirst) {


                            printf("%s", relHash[index].relEntries[a].relName);

                            isFirst = false;

                        } else {
                            printf(" %s", relHash[index].relEntries[a].relName);

                        }

                        for (unsigned int j = 0; j < relHash[index].relEntries[a].cplNumber; j++) {

                            if (relHash[index].relEntries[a].binded[j].srcNumber == MAX_REC) {

                                printf(" %s", relHash[index].relEntries[a].binded[j].destination);

                            }

                        }

                        printf(" %d", MAX_REC);
                        printf(";");

                    }
                }
            }
        }
    }


    if (isEmpty) {

        printf("none");


    }

    printf("\n");

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


            return true;

        } else {//Chiama la funzione che aggiunge una relazione alla hash

            HashInputRel(relTable, entTable);


            return true;

        }

    } else if (inCommand[0] == 'd') {

        if (strcmp(inCommand, "delent") == 0) { //chiama la funzione di elimina elemento

            DeleteEnt(entTable, relTable);

            return true;


        } else {  //chiama la funzione di elimina relazione


            DeleteRel(relTable, entTable);
            return true;

        }

    } else if (inCommand[0] == 'r') {/*chiama il report*/

        Report(relTable);


        return true;

    } else if (inCommand[0] == 'e') {/*termino*/ return false; }

}

//-----HELPERS----------------------------------------------------------------------------------------------------------

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


static inline int hash64(char input) {    //Gioele     71  105


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

//Questa funzione scansiona la tabella di entità per verificare se un' entità esiste già in memoria.

static inline struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash) {

    if (entHash[tableHash].entNumber > 0 && entHash[tableHash].entEntries != NULL) {

        for (unsigned int i = 0; i < entHash[tableHash].entNumber; i++) {

            if (strcmp(entHash[tableHash].entEntries[i].entName, inputName) == 0) {

                return &(entHash[tableHash].entEntries[i]);

            }
        }
    }
    return NULL;
}



//----------------------------------------------------------------------------------------------------------------------

static inline struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash) {

    for (unsigned int i = 0; i < relHash[tableHash].relNumber; i++) {

        if (strcmp(relHash[tableHash].relEntries[i].relName, inputName) == 0) {

            return &(relHash[tableHash].relEntries[i]);

        }
    }

    return NULL;
}
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
//Entro nella hash a tableindex, accedo alla relazione a relIndex e scorro tutte le coupleNum coppie.
//Mentre scorro mi creo un array di destinatari in ordine alfabetico e intanto conteggio
//Ogni volta che aggiungo un dest, lo creo nell array con counter = 1, e ogni volta che lo reincontro incremento.
//Alla fine ripasso e cerco il max, e con sole due scansioni, una di N binnded e una di k riceventi univoci, ho l array da stampare e ordinato.



//----------------------------------------------------------------------------------------------------------------------

//Data una relazione che ha subito cancellazioni, entro nella tabella hash e fixo il backtrack dell' entità ricercata.

static inline void FixBacktrack(char *relName, char *entName, struct EntTable *entHash) {

    int hashResult;
    struct PlainEnt *foundEnt = NULL;


//Cerco l hash dell' entità interessata.

    if (strlen(entName) > 3) {

        hashResult = hash64(entName[1]) * 64 + hash64(entName[2]);

    } else {

        hashResult = 4096;

    }

    foundEnt = EntityLookup(entName, hashResult, entHash);

    if (foundEnt != NULL) {

        for (unsigned int j = 0; j < foundEnt->backtrackIndex; j++) { //Cerco fra tutti i backtrack

            if (strcmp(foundEnt->backTrack[j].relName, relName) ==
                0) { //Decremento il counter ogni volta che fixo il bt.

                foundEnt->backTrack[j].counter--;
                break;


            }
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------

/*Deve entrare dentro ogni songola relazione puntata dal backtrack e cancellare i riferimenti all' entità che possiede il backtrack, inoltre
 * deve sistemare poi tutte le relazioni che modifica.
 *
 * La funzione viene invocata con il nome dell' entità da eliminare e della relazione da eliminare come parametro
 *
 */

static inline void
bindRemover(char *relName, char *entName, struct RelTable *relHash, struct EntTable *entHash) {  //TODO Possibile sbocco

    int result = 0;
    struct PlainRel *relFound = NULL;
    struct PlainEnt *entFound = NULL;


    if (strlen(relName) > 3) {

        result = hash64(relName[1]) * 64 + hash64(relName[2]);

    } else {

        result = 4096;

    }

    relFound = RelationLookup(relName, result, relHash);

    if (relFound != NULL) { //Se ho trovato la relazione coinvolta, devo eliminare entName dall' array di binded.

        if (relFound->binded != NULL) { //Se questa relazione ha dei binded, cerco nei binded il nome che mi interessa

            //I casi sono due, o il nome è una dest o il nome è una rel.
            //prima verifico se è una dest, e scorro tutte le dest. Se la trovo come dest la elimino
            //Per ogni relazione può essere dest una volta sola, percui appena trovo una dest break, e si passa a verifica delle sources

            for (unsigned int i = 0; i < relFound->cplNumber; i++) {

                if (strcmp(relFound->binded[i].destination, entName) == 0) {

                    //è una dest, percui dealloco tutto quello che ha sotto e rialloco l' array di dest.

                    for (unsigned int j = 0; j < relFound->binded[i].srcNumber; j++) {

                        FixBacktrack(relFound->relName, relFound->binded[i].source[j], entHash);
                        free(relFound->binded[i].source[j]);

                    }

                    struct Couples *newBind = calloc(relFound->cplNumber - 1, sizeof(struct Couples));
                    unsigned int temp = 0;
                    //ricopio tutte le altre dest con la loro struttura dati uguale

                    while (temp < i) {

                        newBind[temp].destination = malloc(strlen(relFound->binded[temp].destination) + 1);
                        strcpy(newBind[temp].destination, relFound->binded[temp].destination);
                        newBind[temp].srcNumber = relFound->binded[temp].srcNumber;
                        newBind[temp].source = relFound->binded[temp].source;

                        temp++;
                    }


                    temp = i + 1;

                    while (temp < relFound->cplNumber) {

                        newBind[temp - 1].destination = malloc(strlen(relFound->binded[temp].destination) + 1);
                        strcpy(newBind[temp - 1].destination, relFound->binded[temp].destination);
                        newBind[temp - 1].srcNumber = relFound->binded[temp].srcNumber;
                        newBind[temp - 1].source = relFound->binded[temp].source;

                        temp++;

                    }


                    relFound->binded = newBind;
                    relFound->cplNumber--;

                    if (relFound->cplNumber == 0) {

                        relFound->binded = NULL;

                    }


                    //Fixo il bt, ma in un modo speciale, devo pensarci
                    //Ogni entità src perde un counter se dest è l' entità da levare


                }

                //Finito di controllare se l' ent da levare è una dest, passo alle src.
                //devo controllare in ogni dest se è presente entname, se è presente è univoca percui elimino e rialloco
                //Ogni entità dest perde un counter se ha l' entità da eliminare fra le src
                for (unsigned int k = 0; k < relFound->cplNumber; k++) {

                    for (unsigned int j = 0; j < relFound->binded[k].srcNumber; j++) {

                        if (strcmp(relFound->binded[k].source[j], entName) == 0) {



                            //trovata ent nelle src, devo levarla e so che è unica perchè le coppie src e dest sono univoche

                            FixBacktrack(relFound->relName, relFound->binded[i].destination, entHash);

                            if (j == relFound->binded[k].srcNumber - 1) {

                                relFound->binded[k].srcNumber--;

                                relFound->binded[k].source = realloc(relFound->binded[k].source,
                                                                     relFound->binded[k].srcNumber *
                                                                     sizeof(char *));


                            } else {

                                relFound->binded[k].source[j] = malloc(
                                        strlen(relFound->binded[k].source[relFound->binded[k].srcNumber - 1]) + 1);

                                strcpy(relFound->binded[k].source[j],
                                       relFound->binded[k].source[relFound->binded[k].srcNumber - 1]);

                                relFound->binded[k].srcNumber--;

                                relFound->binded[k].source = realloc(relFound->binded[k].source,
                                                                     relFound->binded[k].srcNumber *
                                                                     sizeof(char *));

                                break;

                            }

                        }
                    }


                }
            }
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------

//Deve prendere in input un ptr a entità, cercare se esiste o meno una track per una data relazione, e aggiungerla se manca o incrementare un counter se è gia presente

static inline void AddBacktrack(struct PlainEnt *toAdd, char *relName) {  //TODO Sembra corretta, ma non lo è

    bool trackFound = false;
    unsigned int index = 0;

    if (toAdd->backTrack == NULL) {

        toAdd->backtrackIndex = 1;
        toAdd->backTrack = malloc(sizeof(struct Track));
        toAdd->backTrack[0].relName = malloc(strlen(relName) + 1);
        strcpy(toAdd->backTrack[0].relName, relName);

    } else {


        for (unsigned int i = 0; i < toAdd->backtrackIndex; i++) { //Per tutte le struct track

            if (strcmp(toAdd->backTrack[i].relName, relName) ==
                0) { //Se trovo la relazione gia nella lista salvo la posizione

                trackFound = true;
                break;
            }

            index++;

        }

        if (trackFound == true) { //Esiste gia in memoria la relazione, incremento il counter

            toAdd->backTrack[index].counter++;


        } else { //Non esiste, la aggiungo in coda

            toAdd->backtrackIndex = index + 1;

            toAdd->backTrack = realloc(toAdd->backTrack, toAdd->backtrackIndex * sizeof(struct Track));

            toAdd->backTrack[toAdd->backtrackIndex - 1].relName = malloc(strlen(relName) + 1);

            strcpy(toAdd->backTrack[toAdd->backtrackIndex - 1].relName, relName);

            toAdd->backTrack[toAdd->backtrackIndex - 1].counter = 1;
        }
    }

}


//-----MAIN-------------------------------------------------------------------------------------------------------------

int main() {


    struct EntTable *entitiesHash = initEntHash();
    struct RelTable *relationHash = initRelHash();


    while (ParseTxt(entitiesHash, relationHash)) {

    }
}

/* TODO LE addent/rel sono giuste, le delete sono un po da riscrivere, perchè non sempre funzionano.
 *      Devo controllare quindi Delent, DelRel, FIxBT e BindRemover.
 *
 */