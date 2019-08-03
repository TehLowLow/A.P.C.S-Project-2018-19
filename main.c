#include <stdio.h>


struct Relation {

    char *relName;                 //Nome della relazione
    struct Entity *sourcePtr;      //Sorgente relazione
    struct Entity *destPtr;        //Destinazione relazione

};


struct Entity {

    char *entName;

};


/*Primo passo: Definire le strutture dati.
 *
 * Sicuramente entità devono essere lightweight, meno dati contengono più l hashing sarà compatto in termini di spazio.
 * Conviene inserire nelle relazioni tutti i dati corposi. Per ora le entità sono una struct, ma se riesco a trattenere
 * tutti i dati nelle relazioni le entità saranno solamente stringhe.
 *
 * Il problema da risolvere è come salvare le associazioni, una volta che è stata creata la relazione e le entità.
 * Sicuramente va salvata ogni cosa che viene immessa in input dal server. COnsiderando che non devo generare nulla
 * derivato da calcoli la massima dimensione di memoria che mi aspetto è la somma di addrel e addent di un file, che
 * essendo testuale non può generare molti piu dati di quanto sia grande il file (escludendo l iniziale overhead e gli
 * integer che contengono le informazioni sul numero di relazioni.)
 *
 * Considerando che il numero di entità e decisamente superiore al numero di relazioni, posso trattenere il peso nelle
 * suddette.
 *
 * Le relazioni le carico sicuramente in un array, garantendo accesso diretto, ideale per rescheduling, e per algoritmi
 * di ricerca efficenti (binary search ad es.).
 *
 * Le relazioni devono restituire sempre il destinatario con più sorgenti, o in caso di parità tutti i destinatari
 * equivalenti.
 *
 * Il problema è che devo salvare ogni cosa, perchè le specifiche permettono cancellazioni, percui non basta salvare
 * solo il più grande di ogni relazione.
 *
 * Idea di Relation:
 *
 * Relation piu che una struct potrebbe essere una matrice di struct
 *
 * Nel primo array carico il nome della relazione (riga)
 * Sotto tale nome saranno presenti tutte le lightweight struct che dovranno contenere
 *
 * Sorgente
 * Destinazione
 *
 * Come puntatori
 *
 * Resta il problema del numerare i destinatari.
 *
 *
 * Sicuramente numerare andando a ricalcolare i valori solo nel report potrebbe essere oneroso necessito di aggiornare
 * questi valori a ogni nuova operazione che interviene su una data relazione (ovvero se agisco sulla relazione X
 * non vado ad aggiornare V.W,Y e Z).
 *
 *
 * TODO studiare un metodo di aggiornamento del numero.
 */





int main() {


}
