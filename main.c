#include <stdio.h>
#include <string.h>

#define DEBUG 0


typedef enum Bool {
    false, true
} bool;


struct Relation {

    char *relName;                 //Nome della relazione
    struct Entity *sourcePtr;      //Sorgente relazione
    struct Entity *destPtr;        //Destinazione relazione

};

/*
struct Entity {

    char *entName;                //Nome dell' entità
    char **involvedRelation;     //Elenco di relazioni in cui è coinvolta

};

 Il primo test lo eseguo senza backtracking delle relazioni, provo a ricercare la singola entità in tutto l' elenco di relazioni

 */



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

    bool flow = true;

    while (flow) {

        flow = ParseTxt();

    }



    //return flow;


}
