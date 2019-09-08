#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


typedef enum Bool {
    false, true
} bool;

struct EntTable {

    unsigned int entNumber;  //Numero di entità hashate sotto questo indice

    struct PlainEnt *entEntries; //Array che contiene tutte le entità hashate sotto un indice
};


struct PlainEnt {

    char *entName; //Nome dell' entità

    unsigned int destCounter; //Int per il conteggio di sorgenti nel report. TODO Potrebbe essere inutile

    char **relKeys; // Array per il backtracking, contiene tutte le chiavi di relazioni in cui la PlainEnt è coinvolta

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

    char *source;

    char *destination;
};

struct ReportEnt {  //Struct per la creazione dell array di destinatari nel report.

    char *entName;

    unsigned int destCounter;


};

char *CONST_TERM = "@@@@";
//-----PROTOTYPES-------------------------------------------------------------------------------------------------------

static inline int hash64(char input);

static inline struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash);

static inline struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash);

static inline struct ReportEnt *
sortCouples(struct RelTable *relHash, int tableIndex, unsigned int relIndex, unsigned int coupleNum);

static inline void FixBacktrack(char *entName, struct EntTable *entHash);



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


static inline void HashInputEnt(struct EntTable *hashTable) {

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

            hashTable[tableIndex].entEntries->relKeys = malloc(1);


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
            hashTable[tableIndex].entEntries[temp - 1].relKeys = malloc(1);

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
    struct PlainEnt *relFound = NULL;
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

                    unsigned int cplIndex = relHashTable[tableIndex].relEntries[a].cplNumber;

                    relHashTable[tableIndex].relEntries[a].binded = calloc(1, sizeof(struct Couples));
                    relHashTable[tableIndex].relEntries[a].binded[0].source = srcFound->entName;
                    relHashTable[tableIndex].relEntries[a].binded[0].destination = destFound->entName;

                    //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
                    srcFound->backtrackIndex++;
                    srcFound->relKeys = realloc(srcFound->relKeys,
                                                srcFound->backtrackIndex * sizeof(char *));
                    strcpy(srcFound->relKeys[srcFound->backtrackIndex - 1],
                           relHashTable[tableIndex].relEntries[a].relName); //COpia il nome della stringa per backtrack

                    destFound->backtrackIndex++;
                    destFound->relKeys = realloc(destFound->relKeys,
                                                 destFound->backtrackIndex * sizeof(char *));
                    strcpy(destFound->relKeys[destFound->backtrackIndex - 1],
                           relHashTable[tableIndex].relEntries[a].relName);

                    return;

                } else {

                    //C'è gia gente allocata, devo verificare che questa coppia non esista ancora per quella relazione,
                    // altrimenti non faccio nulla.

                    for (unsigned int b = 0; b < relHashTable[tableIndex].relEntries[a].cplNumber; b++) {
                        // b è l' indice di coppia per verificare se la relazione gia esiste

                        if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].source, src) == 0 &&
                            strcmp(relHashTable[tableIndex].relEntries[a].binded[b].destination, dest) == 0) {

                            //Esiste gia, ritorno
                            return;

                        }
                    }

                    //Non esiste, devo aggiungere in coda la coppia di entità.
                    //Incremento il numero di couples, rialloco l' array e assegno i nomi di source e dest alla nuova couple.

                    relHashTable[tableIndex].relEntries[a].cplNumber++;

                    unsigned int cplNumbTemp = relHashTable[tableIndex].relEntries[a].cplNumber;

                    struct Couples *tempBind = realloc(relHashTable[tableIndex].relEntries[a].binded,
                                                       relHashTable[tableIndex].relEntries[a].cplNumber *
                                                       sizeof(struct PlainRel));

                    relHashTable[tableIndex].relEntries[a].binded = tempBind;

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].source = malloc(
                            strlen(srcFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].source, srcFound->entName);

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].destination = malloc(
                            strlen(destFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].destination,
                           destFound->entName);

                    //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
                    srcFound->backtrackIndex++;
                    srcFound->relKeys = realloc(srcFound->relKeys,
                                                srcFound->backtrackIndex * sizeof(char *));

                    srcFound->relKeys[srcFound->backtrackIndex - 1] = malloc(
                            strlen(relHashTable[tableIndex].relEntries[a].relName) + 1);
                    strcpy(srcFound->relKeys[srcFound->backtrackIndex - 1],
                           relHashTable[tableIndex].relEntries[a].relName);

                    destFound->backtrackIndex++;
                    destFound->relKeys = realloc(destFound->relKeys,
                                                 destFound->backtrackIndex * sizeof(char *));

                    destFound->relKeys[destFound->backtrackIndex - 1] = malloc(
                            strlen(relHashTable[tableIndex].relEntries[a].relName) + 1);
                    strcpy(destFound->relKeys[destFound->backtrackIndex - 1],
                           relHashTable[tableIndex].relEntries[a].relName);

                    return;

                }


            }
        }

        //Qua devo invece aggiungere la nuova relazione che non ho mai incontrato, e la aggiungo gia in ordine



        if (relHashTable[tableIndex].relNumber == 0) {

            // Se la table è vuota inserisco subito in testa, e aggiungo le due entità

            relHashTable[tableIndex].relNumber++;

            relHashTable[tableIndex].relEntries = malloc(1);

            relHashTable[tableIndex].relEntries = calloc(relHashTable[tableIndex].relNumber,
                                                         sizeof(struct PlainRel));  //Alloco la prima cella di entries e la nomino

            relHashTable[tableIndex].relEntries[0].relName = malloc(strlen(inputRel) + 1);

            strcpy(relHashTable[tableIndex].relEntries[0].relName, inputRel);

            relHashTable[tableIndex].relEntries[0].cplNumber++;

            relHashTable[tableIndex].relEntries[0].binded = calloc(1,
                                                                   sizeof(struct Couples)); // Alloco la prima cella di Coppie e le assegno

            relHashTable[tableIndex].relEntries[0].binded[0].source = malloc(strlen(srcFound->entName) + 1);

            relHashTable[tableIndex].relEntries[0].binded[0].destination = malloc(strlen(destFound->entName) + 1);

            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].source, srcFound->entName);

            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].destination, destFound->entName);

            //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
            srcFound->backtrackIndex++;

            srcFound->relKeys = realloc(srcFound->relKeys,
                                        srcFound->backtrackIndex * sizeof(char *));

            srcFound->relKeys[srcFound->backtrackIndex - 1] = malloc(
                    strlen(relHashTable[tableIndex].relEntries[0].relName) + 1);

            strcpy(srcFound->relKeys[srcFound->backtrackIndex - 1], relHashTable[tableIndex].relEntries[0].relName);

            destFound->backtrackIndex++;
            destFound->relKeys = realloc(destFound->relKeys,
                                         destFound->backtrackIndex * sizeof(char *));

            destFound->relKeys[destFound->backtrackIndex - 1] = malloc(
                    strlen(relHashTable[tableIndex].relEntries[0].relName) + 1);

            strcpy(destFound->relKeys[destFound->backtrackIndex - 1], relHashTable[tableIndex].relEntries[0].relName);

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

            strcpy(buffer[bufferCounter].relName,
                   relHashTable[tableIndex].relEntries[bufferCounter].relName);  //Copio il nome
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter].cplNumber; //Copio il numero di coppie
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter].binded; //Copio il ptr all' array di coppie

            bufferCounter++; //Incremento e ripeto

        }

        bufferCounter = ordered; //Inserisco la nuova struct. La copia è identica a quella scritta sopra e a quella successiva.

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

        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].source, srcFound->entName);
        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].destination, destFound->entName);

        //Aggiungo il nome della relazione al backtracking delle entità  //TODO testare
        srcFound->backtrackIndex++;
        srcFound->relKeys = realloc(srcFound->relKeys,
                                    srcFound->backtrackIndex * sizeof(char *));
        strcpy(srcFound->relKeys[srcFound->backtrackIndex - 1], relHashTable[tableIndex].relEntries[ordered].relName);

        destFound->backtrackIndex++;
        destFound->relKeys = realloc(destFound->relKeys,
                                     destFound->backtrackIndex * sizeof(char *));
        strcpy(destFound->relKeys[destFound->backtrackIndex - 1], relHashTable[tableIndex].relEntries[ordered].relName);

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

static inline void DeleteEnt() {};

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
    bool srcFound = false;
    bool destFound = false;

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


    relFound = RelationLookup(rel, tableIndex, relHash);

    if (relFound->binded != NULL) {//Se ho delle coppie sotto la relazione

        for (unsigned int i = 0; i < relFound->cplNumber; i++) { //Per ogni coppia che ho

            if (strcmp(relFound->binded[i].source, src) == 0 &&
                strcmp(relFound->binded[i].destination, dest) ==
                0) {//Se ho trovato la relazione la sovrascrivo con quella in coda, elimino la coda e rialloco


                if (relFound->cplNumber == 1) {//Se ho una sola coppia, metto binded a null e cplnum a 0

                    relFound->cplNumber = 0;
                    free(relFound->binded);
                    relFound->binded = NULL;
                    break;


                } else {


                    free(relFound->binded[i].source);
                    free(relFound->binded[i].destination);

                    //Alloco la dim della source e della dest in coda, sovrascrivo, libero la coda e rialloco
                    relFound->binded[i].source = malloc(
                            strlen(relFound->binded[relFound->cplNumber - 1].source) + 1); //
                    relFound->binded[i].destination = malloc(
                            strlen(relFound->binded[relFound->cplNumber - 1].destination) + 1);

                    strcpy(relFound->binded[i].source, relFound->binded[relFound->cplNumber - 1].source);
                    strcpy(relFound->binded[i].destination, relFound->binded[relFound->cplNumber - 1].destination);

                    free(relFound->binded[relFound->cplNumber - 1].source);
                    free(relFound->binded[relFound->cplNumber - 1].destination);

                    relFound->cplNumber--;

                    relFound->binded = realloc(relFound->binded, relFound->cplNumber * sizeof(struct Couples));

                    break;

                }

            }
        }

        //Successivamente cerco nell' array di binded altre occorrenze delle due entità, una per volta.
        //Se l array di binded è diventato null, devo cercare subito le due entità.

        if (relFound->binded == NULL) {

            FixBacktrack(src, entHash);
            FixBacktrack(dest, entHash);

        } else {

            //Scorri binded e chiama fixBt se non trovi o src o dest


            for (unsigned int i = 0; i < relFound->cplNumber; i++) {

                if (strcmp(relFound->binded[i].source, src) == 0) {

                    srcFound = true; //Trovata un occorrenza di sorgente, non devo fixare
                    break;
                }
            }

            for (unsigned int j = 0; j < relFound->cplNumber; j++) {

                if (strcmp(relFound->binded[j].destination, dest) == 0) {

                    destFound = true; //Trovata un occorrenza di dest, non devo fixare
                    break;
                }
            }
        }

        //Valuto i booleani, e decido quale fixare

        if (srcFound == false) {

            FixBacktrack(src, entHash);
        }

        if (destFound == false) {

            FixBacktrack(dest, entHash);
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




static inline void Report(struct RelTable *relHash, struct EntTable *entHash) {

    struct PlainEnt **entBuffer = malloc(1 * sizeof(struct PlainEnt *));
    struct ReportEnt *entReport;
    unsigned int bufferCounter = 0; //Dimensione dell'array
    bool isEmpty = true;
    bool isFirst = true;


    for (int index = 0; index < 4097; index++) { //Per ogni chiave della hash

        if (relHash[index].relNumber != 0) { //Se la chiave corrisponde a delle relazioni

            for (unsigned int a = 0; a < relHash[index].relNumber; a++) {//Per ogni relazione di quella chiave

                entReport = sortCouples(relHash, index, a,
                                        relHash[index].relEntries[a].cplNumber);  //Creo un array di destinatari da stampare


                if (entReport != NULL) {

                    isEmpty = false;

                    if (isFirst) { //Stampa nome relazione

                        printf("%s", relHash[index].relEntries[a].relName);
                        isFirst = false;


                    } else { //Stampa nome relazione (altro formato)


                        printf(" %s", relHash[index].relEntries[a].relName);

                    }

                    unsigned int n = 0;

                    while (strcmp(entReport[n].entName, CONST_TERM) !=
                           0) { //TODO Errore nel sorting, manca della gente.

                        printf(" %s", entReport[n].entName);
                        n++;

                    }

                    printf(" %d", entReport[0].destCounter);  //Stampo il max ricevente

                    printf(";");

                }

            }
        }
    }

    if (isEmpty) {
        printf("none");  //Stampa none se non ho relazioni
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



            return true;


        } else {  //chiama la funzione di elimina relazione



            return true;

        }

    } else if (inCommand[0] == 'r') {/*chiama il report*/

        Report(relTable, entTable);


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

    for (unsigned int i = 0; i < entHash[tableHash].entNumber; i++) {

        if (strcmp(entHash[tableHash].entEntries[i].entName, inputName) == 0) {

            return &(entHash[tableHash].entEntries[i]);

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
//TODO Errore qua dentro, manca della gente. Il sorting probabilmente si scassa con degli indici.

static inline struct ReportEnt *
sortCouples(struct RelTable *relHash, int tableIndex, unsigned int relIndex, unsigned int coupleNum) {

    struct ReportEnt *destArray = calloc(1, sizeof(struct ReportEnt));
    unsigned int arrayCounter = 1;
    bool found = false;

    if (relHash[tableIndex].relEntries[relIndex].binded != NULL) {//Se ho delle coppie

        destArray[0].entName = malloc(strlen(relHash[tableIndex].relEntries[relIndex].binded[0].destination) + 1);

        strcpy(destArray[0].entName,
               relHash[tableIndex].relEntries[relIndex].binded[0].destination); //Copio il primo nome di default

        destArray[0].destCounter = 1;

        for (unsigned int i = 1; i < coupleNum; i++) {//Per tutte le restanti coppie

            unsigned int sortOrder = 0;

            for (unsigned int k = 0; k < arrayCounter; k++) { //Per ogni elemento gia inserito nell array di destinatari

                if (strcmp(relHash[tableIndex].relEntries[relIndex].binded[i].destination, destArray[k].entName) ==
                    0) { //Se incontro un doppione, ne incremento il counter

                    destArray[k].destCounter++;
                    found = true;
                    break;

                    //Se trovo un doppio devo incrementare il counter

                } else if (
                        strcmp(relHash[tableIndex].relEntries[relIndex].binded[i].destination, destArray[k].entName) <
                        0) { //Se invece non lo incontro, ma trovo qualcuno lessicograficamente maggiore, allora devo inserirlo in quella posizione
                    break;
                }

                sortOrder++;

            }


            if (found == false) {//Devo aggiungere in ordine il nuovo destinatario

                arrayCounter++;

                struct ReportEnt *tempArray = malloc(
                        arrayCounter * sizeof(struct ReportEnt)); //Alloco un array temp con dim. arraycounter

                unsigned int j = 0;

                while (j < sortOrder) {

                    tempArray[j] = destArray[j];  //Copio tutti gli elementi in ordine corretto
                    j++;

                }

                tempArray[sortOrder].entName = malloc(
                        strlen(relHash[tableIndex].relEntries[relIndex].binded[i].destination) + 1);

                strcpy(tempArray[sortOrder].entName, relHash[tableIndex].relEntries[relIndex].binded[i].destination);
                tempArray[sortOrder].destCounter = 1; //Copio ed inizializzo il nuovo destinatario

                j = sortOrder + 1;

                while (j < arrayCounter) {

                    tempArray[j] = destArray[j - 1]; //Copio i successivi
                    j++;


                }

                free(destArray);
                destArray = tempArray;


            }

            found = false;

        }

        //Ora devo trovare il/i MAX

        unsigned int maxRecvr = 0; //Maggior numero riscontrato come ricevente
        unsigned int maxDim;  //dimensione dell' array di riceventi

        maxRecvr = destArray[0].destCounter;  //Parto con una base di contatore data dal primo elemento nei riceventi

        for (unsigned int l = 1; l < arrayCounter; l++) {

            //Scorro e trovo il Max integer, che deve essere strettamente maggiore (e non uguale) al max visto fin ora

            if (destArray[l].destCounter > maxRecvr) {

                maxRecvr = destArray[l].destCounter;

            }

        }

        struct ReportEnt *tempMax = malloc(sizeof(struct ReportEnt));
        maxDim = 1;

        for (unsigned int m = 0; m < arrayCounter; m++) { //Per ogni elemento nel DestArray devo salvare solo i max

            if (destArray[m].destCounter == maxRecvr) { //Se trovo un ricevente che contiene maxdim, lo salvo

                maxDim++;
                tempMax = realloc(tempMax, maxDim * sizeof(struct ReportEnt));
                tempMax[maxDim - 2] = destArray[m];
            }
        }

        tempMax[maxDim - 1].entName = malloc(5);

        strcpy(tempMax[maxDim - 1].entName, CONST_TERM);

        free(destArray);

        return tempMax;


    } else {

        return NULL;
    }

}
//----------------------------------------------------------------------------------------------------------------------

//Data una relazione che ha subito cancellazioni, entro nella tabella hash e fixo il backtrack dell' entità ricercata.


static inline void FixBacktrack(char *entName, struct EntTable *entHash) {











}




//-----MAIN-------------------------------------------------------------------------------------------------------------

int main() {

    int i = 0;
    struct EntTable *entitiesHash = initEntHash();
    struct RelTable *relationHash = initRelHash();

    //Memoria usata per inizializzare le strutture dati: 128kB

    while (ParseTxt(entitiesHash, relationHash)) {

    }
}

/* TODO
 *        Problemi con gli indci dei binded e degli array, rileggere il codice tutto in generale.
 *      1 ricorda nelle delete di impostare questo array a null se finiscono le entità, altrimenti si fotte
 *      3 Devo controllare di aver messo bene i return, perchè devo poter uscire dai for appena una condizione non è soddisfatta.
 *      IMPORTANTE: nel backtracking possono finirci anche indici uguali, percui devo pensare a come gestire un backtracking efficiente
 *
 */