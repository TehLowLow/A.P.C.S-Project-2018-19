#include <stdio.h>
#include <string.h>
#include <malloc.h>


typedef enum Bool {
    false, true
} bool;

struct EntTable {

    unsigned int entNumber;  //Number of entities under a key
    struct PlainEnt *entEntries; //Array containing all the entities under a key
};

struct PlainEnt {

    char *entName; //Entity name
    struct Track *backTrack; //Array of backtrack
    unsigned int backtrackIndex; //Number of backtracks
};

struct RelTable {

    unsigned int relNumber; //Number of relations under a key
    struct PlainRel *relEntries; //Array of relations
};

struct PlainRel {

    char *relName; //Name of the relation in the array
    unsigned int cplNumber; //Number of recivers for the relation
    struct Couples *binded;  //Array of entities binded by the  relation
};

struct Couples { //Data type that binds a destination entity with all the sources

    char **source; //Array of sources
    char *destination; //Name of the reciver in the relations
    unsigned int srcNumber; //Number of sources for a reciver
};

struct Track {

    char *relName;        //Name of the relation for backtracking
    unsigned int counter;  //Number of times this entity shows up in the relation
};


//-----PROTOTYPES-------------------------------------------------------------------------------------------------------

static inline int hash64(char input);

static inline struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash);

static inline struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash);

static inline void FixBacktrack(char *relName, char *entName, struct EntTable *entHash);

static inline void bindRemover(char *relName, char *entName, struct RelTable *relHash, struct EntTable *entHash);

static inline void AddBacktrack(struct PlainEnt *toAdd, char *relName);

//-----MEMORY INIT------------------------------------------------------------------------------------------------------

//This function is responsible of the initialization of the hashtable used to store the entities
struct EntTable *initEntHash() {


    struct EntTable *hash = NULL;

    hash = calloc(4097, sizeof(struct EntTable));
    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

//This function initializes the hashtable for the relations
struct RelTable *initRelHash() {

    struct RelTable *hash = NULL;

    hash = calloc(4097, sizeof(struct RelTable));
    return hash;
}

//-----COMMANDS---------------------------------------------------------------------------------------------------------

//This function is invoked when the parser encounters the "addent" command. The entity is then read from stdin,
//hashed with the main rule of this software and then placed last under the key it belongs. There are no rules
//of sorting colliding entities under a certain key.
//The command is valid only if the ent is not already in the hash.

static inline void HashInputEnt(struct EntTable *hashTable) {

    char *inputEnt = NULL;
    int tableIndex;
    struct PlainEnt *result = NULL;

    scanf("%ms", &inputEnt);    //read the entity from stdin

    if (strlen(inputEnt) > 3) {
        tableIndex = hash64(inputEnt[1]) * 64 + hash64(inputEnt[2]);
    } else {
        tableIndex = 4096;
    }

    //After hashing the software looks for the entity's place in the table
    result = EntityLookup(inputEnt, tableIndex, hashTable);

    if (result == NULL) { //If the key is empty, hash first, saving all the data in the table

        if (hashTable[tableIndex].entEntries == NULL) {  //TODO 1

            hashTable[tableIndex].entNumber = 1;
            hashTable[tableIndex].entEntries = calloc(1, sizeof(struct PlainEnt));
            hashTable[tableIndex].entEntries[0].entName = malloc(strlen(inputEnt) + 1);
            strcpy(hashTable[tableIndex].entEntries[0].entName, inputEnt);
            hashTable[tableIndex].entEntries->backtrackIndex = 0;
            hashTable[tableIndex].entEntries->backTrack = NULL;


        } else {
            //The key is not empty, we have a collision. Realloc the array with one more space and save the entity last

            hashTable[tableIndex].entNumber++;

            unsigned int temp = hashTable[tableIndex].entNumber;

            hashTable[tableIndex].entEntries = realloc(hashTable[tableIndex].entEntries,
                                                       temp * sizeof(struct PlainEnt));

            //This malloc is needed to have a valid address where to store the string
            hashTable[tableIndex].entEntries[temp - 1].entName = malloc(
                    strlen(inputEnt) + 1);

            strcpy(hashTable[tableIndex].entEntries[temp - 1].entName, inputEnt);
            hashTable[tableIndex].entEntries[temp - 1].backtrackIndex = 0;
            hashTable[tableIndex].entEntries[temp - 1].backTrack = NULL;

        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

//This function is invoked when the parser encounters an "addrel" command from the stdin.
//Then proceeds to hash the relation and store it under its key.
//The command addrel is valid only if the 2 ents are in the Entities hash, and then either stores the newly encountered
//relation or if the rel is already in memory stores the new bind, again only if the bind is not already there.

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

    //Read the params from stdin

    scanf("%ms", &src);
    scanf("%ms", &dest);
    scanf("%ms", &inputRel);

    //Verification of the entities

    if (strlen(src) > 3) {
        hashedSrc = hash64(src[1]) * 64 + hash64(src[2]);
    } else {
        hashedSrc = 4096;
    }

    srcFound = EntityLookup(src, hashedSrc, entHashTable);

    //After finding if there is the source, the function look fo the destination

    if (srcFound != NULL) {

        if (strlen(dest) > 3) {
            hashedDest = hash64(dest[1]) * 64 + hash64(dest[2]);
        } else {
            hashedDest = 4096;
        }

        destFound = EntityLookup(dest, hashedDest, entHashTable);

    }


    //At this point if both the ents are valid, the function proceeds to hash the relation

    if (srcFound != NULL && destFound != NULL) {

        if (strlen(inputRel) > 3) {

            tableIndex = hash64(inputRel[1]) * 64 + hash64(inputRel[2]);

        } else {

            tableIndex = 4096;
        }

        //With the hash key, we look for the input relation, as no duplicates are allowed

        for (unsigned int a = 0;
             a < relHashTable[tableIndex].relNumber; a++) {

            if (strcmp(relHashTable[tableIndex].relEntries[a].relName, inputRel) == 0) {

                //Relation found in memory, the function looks for duplicates.

                if (relHashTable[tableIndex].relEntries[a].binded ==
                    NULL) {

                    //There are no other couples (due to deletions). Save this new rel first.

                    relHashTable[tableIndex].relEntries[a].cplNumber = 1;
                    relHashTable[tableIndex].relEntries[a].binded = calloc(1, sizeof(struct Couples));
                    relHashTable[tableIndex].relEntries[a].binded[0].destination = malloc(
                            strlen(destFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[0].destination, destFound->entName);
                    relHashTable[tableIndex].relEntries[a].binded[0].srcNumber = 1;
                    relHashTable[tableIndex].relEntries[a].binded[0].source = malloc(sizeof(char *));
                    relHashTable[tableIndex].relEntries[a].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);
                    strcpy(relHashTable[tableIndex].relEntries[a].binded[0].source[0], srcFound->entName);

                    //Add the backtrack for this relation to the entities.
                    AddBacktrack(srcFound, inputRel);
                    AddBacktrack(destFound, inputRel);
                    return;

                } else {

                    //This relation is not the first one.
                    //First the function looks for duplications

                    for (unsigned int b = 0; b < relHashTable[tableIndex].relEntries[a].cplNumber; b++) {

                        if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].destination, dest) == 0) {

                            //Found one ent, the destination, now evaluates the sources.

                            for (unsigned int i = 0;
                                 i < relHashTable[tableIndex].relEntries[a].binded[b].srcNumber; i++) {

                                if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].source[i],
                                           srcFound->entName) == 0) {

                                    //Source found, this command is a duplicate, return
                                    return;

                                }

                            }

                            //The source is not in the array, valid command and add source last inside the array.
                            relHashTable[tableIndex].relEntries[a].binded[b].srcNumber++;

                            unsigned int temp = relHashTable[tableIndex].relEntries[a].binded[b].srcNumber;

                            relHashTable[tableIndex].relEntries[a].binded[b].source = realloc(
                                    relHashTable[tableIndex].relEntries[a].binded[b].source, temp * sizeof(char *));

                            relHashTable[tableIndex].relEntries[a].binded[b].source[temp - 1] = malloc(
                                    strlen(srcFound->entName) + 1);

                            strcpy(relHashTable[tableIndex].relEntries[a].binded[b].source[temp - 1],
                                   srcFound->entName);

                            //Adding backtrack for this relation
                            AddBacktrack(srcFound, inputRel);
                            AddBacktrack(destFound, inputRel);
                            return;
                        }
                    }

                    //The function hasn't found the destination, now it scans the array and finds the lexicographical order
                    //where to place this destination.

                    unsigned int cplNumbTemp = relHashTable[tableIndex].relEntries[a].cplNumber;
                    unsigned int counter = 0;

                    for (unsigned int j = 0; j < cplNumbTemp; j++) {

                        if (strcmp(destFound->entName, relHashTable[tableIndex].relEntries[a].binded[j].destination) <
                            0) {

                            break;

                        }

                        counter++;
                    }

                    //The value of counter is the cell in which the new dest should go.
                    //Realloc, places the new dest and then fixes the array.

                    relHashTable[tableIndex].relEntries[a].cplNumber++;

                    struct Couples *tempCouples = calloc(relHashTable[tableIndex].relEntries[a].cplNumber,
                                                         sizeof(struct Couples));

                    unsigned int temp = 0;

                    while (temp < counter) {

                        //The first counter-1 dest are the same, simple copy.

                        tempCouples[temp].destination = malloc(
                                strlen(relHashTable[tableIndex].relEntries[a].binded[temp].destination) + 1);

                        strcpy(tempCouples[temp].destination,
                               relHashTable[tableIndex].relEntries[a].binded[temp].destination);

                        tempCouples[temp].srcNumber = relHashTable[tableIndex].relEntries[a].binded[temp].srcNumber;
                        tempCouples[temp].source = relHashTable[tableIndex].relEntries[a].binded[temp].source;

                        temp++;
                    }

                    //Insertion of the new dest

                    temp = counter;

                    tempCouples[temp].destination = malloc(strlen(destFound->entName) + 1);
                    strcpy(tempCouples[temp].destination, destFound->entName);
                    tempCouples[temp].srcNumber = 1;
                    tempCouples[temp].source = malloc(sizeof(char *));
                    tempCouples[temp].source[0] = malloc(strlen(srcFound->entName) + 1);
                    strcpy(tempCouples[temp].source[0], srcFound->entName);

                    temp++;

                    while (temp < relHashTable[tableIndex].relEntries[a].cplNumber) {

                        //Copy of the other dests.

                        tempCouples[temp].destination = malloc(
                                strlen(relHashTable[tableIndex].relEntries[a].binded[temp - 1].destination) + 1);

                        strcpy(tempCouples[temp].destination,
                               relHashTable[tableIndex].relEntries[a].binded[temp - 1].destination);

                        tempCouples[temp].srcNumber = relHashTable[tableIndex].relEntries[a].binded[temp - 1].srcNumber;
                        tempCouples[temp].source = relHashTable[tableIndex].relEntries[a].binded[temp - 1].source;

                        temp++;
                    }

                    relHashTable[tableIndex].relEntries[a].binded = tempCouples;

                    //Backtracking for this relation
                    AddBacktrack(srcFound, inputRel);
                    AddBacktrack(destFound, inputRel);

                    return;

                }
            }
        }

        //At this point the function hasn't found the input relation, and needs to sort it and place it into the hash.

        if (relHashTable[tableIndex].relNumber == 0) {

            //If the hash is empty this relation is saved in first place.

            relHashTable[tableIndex].relNumber++;
            relHashTable[tableIndex].relEntries = calloc(relHashTable[tableIndex].relNumber,
                                                         sizeof(struct PlainRel));

            relHashTable[tableIndex].relEntries[0].relName = malloc(strlen(inputRel) + 1);
            strcpy(relHashTable[tableIndex].relEntries[0].relName, inputRel);
            relHashTable[tableIndex].relEntries[0].cplNumber++;

            //Then we save the new couple

            relHashTable[tableIndex].relEntries[0].binded = calloc(1,
                                                                   sizeof(struct Couples));

            relHashTable[tableIndex].relEntries[0].binded[0].destination = malloc(strlen(destFound->entName) + 1);
            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].destination, destFound->entName);
            relHashTable[tableIndex].relEntries[0].binded[0].srcNumber = 1;
            relHashTable[tableIndex].relEntries[0].binded[0].source = malloc(sizeof(char *));
            relHashTable[tableIndex].relEntries[0].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);
            strcpy(relHashTable[tableIndex].relEntries[0].binded[0].source[0], srcFound->entName);

            //Backtracking for the relation
            AddBacktrack(srcFound, inputRel);
            AddBacktrack(destFound, inputRel);
            return;
        }

        //If this relation is not the first, the function sorts it and places it in order.

        ordered = 0;

        for (unsigned int a = 0; a < relHashTable[tableIndex].relNumber; a++) {

            if (strcmp(inputRel, relHashTable[tableIndex].relEntries[a].relName) < 0) {

                break;

            }

            ordered++;
        }


        //Order is the index where the relation will be placed

        relHashTable[tableIndex].relNumber++;

        struct PlainRel *buffer = calloc(relHashTable[tableIndex].relNumber, sizeof(struct PlainRel));

        bufferCounter = 0;

        //The first ordered-1 relations are copied over

        while (bufferCounter < ordered) {

            buffer[bufferCounter].relName = malloc(
                    strlen(relHashTable[tableIndex].relEntries[bufferCounter].relName) + 1);
            strcpy(buffer[bufferCounter].relName,
                   relHashTable[tableIndex].relEntries[bufferCounter].relName);
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter].cplNumber;
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter].binded;

            bufferCounter++;

        }

        //The new relation is saved
        bufferCounter = ordered;

        buffer[bufferCounter].relName = malloc(strlen(inputRel) + 1);
        strcpy(buffer[bufferCounter].relName, inputRel);
        buffer[bufferCounter].cplNumber = 1;
        buffer[bufferCounter].binded = calloc(1, sizeof(struct Couples));

        bufferCounter++;

        //The others relations are copied over.

        while (bufferCounter < relHashTable[tableIndex].relNumber) {

            strcpy(buffer[bufferCounter].relName, relHashTable[tableIndex].relEntries[bufferCounter - 1].relName);
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter - 1].cplNumber;
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter - 1].binded;

            bufferCounter++;


        }

        //Free old memory
        free(relHashTable[tableIndex].relEntries);

        //Save the new array
        relHashTable[tableIndex].relEntries = buffer;

        //Adding the sub data for the relation
        relHashTable[tableIndex].relEntries[ordered].binded[0].srcNumber = 1;
        relHashTable[tableIndex].relEntries[ordered].binded[0].destination = malloc(strlen(destFound->entName) + 1);
        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].destination, destFound->entName);
        relHashTable[tableIndex].relEntries[ordered].binded[0].source = malloc(sizeof(char *));
        relHashTable[tableIndex].relEntries[ordered].binded[0].source[0] = malloc(strlen(srcFound->entName) + 1);
        strcpy(relHashTable[tableIndex].relEntries[ordered].binded[0].source[0], srcFound->entName);

        //Adding backtrack
        AddBacktrack(srcFound, inputRel);
        AddBacktrack(destFound, inputRel);

        return;
    }
}

/*
 * Lexicographical sorting is achieved by using strcmp.
 *
 * If strcmp(input, hashed relation) ==0 then this means that the strings are equals.
 * If strcmp(input, hashed relation) <0 it means that input has a value less than hashed, thus making it lexicographically
 * before the hashed one.
 */


//----------------------------------------------------------------------------------------------------------------------
//This function is responsible of deleting an entity from the hash. This deletion means that also all the occurrences
//of Ent inside relations need to be erased.
//The command is valid only if the entity is still in memory at the time of invocation.

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

    //Hash Lookup for the entity to be deleted

    if (deleteEnt != NULL) {


        //If the entity exists, the function starts removing all its occurrencies from the relation hash

        for (unsigned int i = 0; i < deleteEnt->backtrackIndex; i++) {

            bindRemover(deleteEnt->backTrack[i].relName, toDelete, relHash, entHash);

        }

        //After all the relations have been deleted, the function removes the entity from the table.

        unsigned int order = 0;

        for (unsigned int j = 0; j < entHash[hashEnt].entNumber; j++) {

            if (strcmp(entHash[hashEnt].entEntries[j].entName, deleteEnt->entName) == 0) {

                break;
            }

            order++;
        }

        //In a slightly inefficient way (my bad, i forgot to fix this monstruosity with the new deletion method,
        // more of that at the end of the code) the function copies over all the valid entities, leaving behind the deleted one.

        unsigned int i = 0;

        struct PlainEnt *newEntities = calloc(entHash[hashEnt].entNumber - 1, sizeof(struct PlainEnt));

        while (i < order) {


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

/*This function deletes a relation between two entities.
 * The command is valid only if there is a relation between the two ents.
 * Looking up the entities is not necessary, as if those are linked in memory, then they are by specification
 * in the entities hash (adding a relation is done only if the ents are present in memory).
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

    relFound = RelationLookup(rel, tableIndex, relHash);

    //The function looks for the relation in the table

    if (relFound != NULL && relFound->binded != NULL) {

        //If the relation has some couples the function looks for the desired one, or else it stops immediately

        for (unsigned int i = 0; i < relFound->cplNumber; i++) {

            if (strcmp(relFound->binded[i].destination, dest) == 0) {

                //Found the destination, it looks for the source

                for (unsigned int j = 0; j < relFound->binded[i].srcNumber; j++) {

                    if (strcmp(relFound->binded[i].source[j], src) == 0) {

                        //Found the source. The function evaluates its position and then proceeds to the deletion


                        if (relFound->binded[i].srcNumber == 0) {

                            //There are no other sources, the function needs to delete also the destination.

                            nullDest = true; //Flag for the deletion
                            break;

                        } else if (j == relFound->binded[i].srcNumber - 1) {

                            //The source is the last of the array, simple realloc.

                            free(relFound->binded[i].source[j]);
                            relFound->binded[i].srcNumber--;
                            relFound->binded[i].source = realloc(relFound->binded[i].source,
                                                                 relFound->binded[i].srcNumber * sizeof(char *));

                        } else {

                            //The source is in the middle of the array.
                            //Overwrite it with the last string and then simple realloc.

                            relFound->binded[i].source[j] = relFound->binded[i].source[relFound->binded[i].srcNumber -
                                                                                       1];
                            relFound->binded[i].srcNumber--;

                            relFound->binded[i].source = realloc(relFound->binded[i].source,
                                                                 relFound->binded[i].srcNumber *
                                                                 sizeof(char *));

                            //Fix the backtrack array of source and destination
                            FixBacktrack(rel, src, entHash);
                            FixBacktrack(rel, dest, entHash);
                            break;
                        }
                    }
                }

                if (nullDest) {

                    //The function if flagged needs to delete the destination

                    if (relFound->cplNumber == 1) {

                        //If the destination is the only one in the relation, free everything and return

                        relFound->cplNumber = 0;
                        free(relFound->binded[0].source);
                        free(relFound->binded);
                        relFound->binded = NULL;

                        FixBacktrack(rel, src, entHash);
                        FixBacktrack(rel, dest, entHash);

                        return;

                    } else {

                        //The function frees all the data under the dest (that is invalid data, only to avoid mem leak.
                        free(relFound->binded[i].source);

                        //Then the function proceeds to realloc the array mantaining lexicographical order

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

                        relFound->binded = newBind;
                        relFound->cplNumber--;

                        //At last, backtrack fix for both source and dest
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

//Report has to scan all the relations hash and print all the destinations that are associated with the most sources.
//There could be more than one, so each parity has to be printed in lexicographical order, thus the sorting in memory.

static inline void Report(struct RelTable *relHash) {

    bool isEmpty = true;
    bool isFirst = true;

    for (int index = 0; index < 4097; index++) {

        if (relHash[index].relNumber != 0) {

            for (unsigned int a = 0; a < relHash[index].relNumber; a++) {

                //For each relation saved under the hash, if the relation has some entities binded, looks for the
                //Max receiver and save the number of sources it has.


                if (relHash[index].relEntries[a].binded != NULL && relHash[index].relEntries[a].cplNumber > 0) {

                    isEmpty = false;

                    unsigned int MAX_REC = 0;

                    for (unsigned int i = 0; i < relHash[index].relEntries[a].cplNumber; i++) {

                        if (relHash[index].relEntries[a].binded[i].srcNumber > MAX_REC) {

                            MAX_REC = relHash[index].relEntries[a].binded[i].srcNumber;

                        }
                    }

                    //Then prints the name of the relation, all the max receivers and the number of sources they have
                    //In a particular format

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

    //If the table is empty prints none

    if (isEmpty) {

        printf("none");
    }

    printf("\n");

}

//----------------------------------------------------------------------------------------------------------------------
//This is the parser, responsible of reading from stdin and invoking the functions to manage the data.

static inline bool ParseTxt(struct EntTable *entTable, struct RelTable *relTable) {

    char *inCommand = NULL;

    scanf("%ms", &inCommand);

    if (inCommand[0] == 'a') {

        if (strcmp(inCommand, "addent") == 0) {

            HashInputEnt(entTable);
            return true;

        } else {

            HashInputRel(relTable, entTable);
            return true;

        }

    } else if (inCommand[0] == 'd') {

        if (strcmp(inCommand, "delent") == 0) {

            DeleteEnt(entTable, relTable);
            return true;

        } else {

            DeleteRel(relTable, entTable);
            return true;

        }

    } else if (inCommand[0] == 'r') {

        Report(relTable);
        return true;

    } else  { return false; }

}

//-----HELPERS----------------------------------------------------------------------------------------------------------

//This function is the core of the software. The specs restricted the usable characters from the ascii table from 127 to 64
//This is the conversion table used in the software as a keygen fro the hash.


/*  "-" = 0;
 * 0...9 = x - 47;
 * A...Z = x - 54
 *  "_" = 37
 * a...z = x - 59
 */

//After the compression, the algorythm generates the key as:

//If the word is longer than one char, take the first letter compressed int value, multiply it by 64 and then adds
//to it the second letter compressed int value.

//Example:


// "Test" ---> T = Ascii 84 ; e ---> Ascii 101

// Key = (84 - 54) * 64 + (101 - 59) = 1962

//Hence the tables have to have at least 4097 indexes, 0-4095 are the numbers of possible keys, and 4096 is the special
//key for the words composed of 1 single char

static inline int hash64(char input) {    //Gioele     71  105


    int hashed = 0;

    hashed = input;

    if (input == 45) {  //'-'

        hashed = 0;

    } else if (48 <= input && input <= 57) {  // 0...9

        hashed = input - 47;

    } else if (65 <= input && input <= 90) { // A..Z

        hashed = input - 54;

    } else if (input == 95) { //'_'

        hashed = 37;

    } else if (97 <= input && input <= 122) {  // a...z

        hashed = input - 59;

    }

    return hashed;
}
//----------------------------------------------------------------------------------------------------------------------

//This function scans the ent hash looking for a particular entity.

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

//This function scans the rel hash looking for a particular relation.

static inline struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash) {

    for (unsigned int i = 0; i < relHash[tableHash].relNumber; i++) {

        if (strcmp(relHash[tableHash].relEntries[i].relName, inputName) == 0) {

            return &(relHash[tableHash].relEntries[i]);

        }
    }
    return NULL;
}

//----------------------------------------------------------------------------------------------------------------------

//This function fixes the backtrack of a particular entity

static inline void FixBacktrack(char *relName, char *entName, struct EntTable *entHash) {

    int hashResult;
    struct PlainEnt *foundEnt = NULL;


    if (strlen(entName) > 3) {

        hashResult = hash64(entName[1]) * 64 + hash64(entName[2]);

    } else {

        hashResult = 4096;

    }

    foundEnt = EntityLookup(entName, hashResult, entHash);

    //If there is the entity, then decrease the counter for a particular relation in which the entity is involved

    if (foundEnt != NULL) {

        for (unsigned int j = 0; j < foundEnt->backtrackIndex; j++) {

            if (strcmp(foundEnt->backTrack[j].relName, relName) ==
                0) {

                foundEnt->backTrack[j].counter--;
                break;


            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

//This function scans the relation hash looking for occurrencies of a particular entity.
//The relations are those that shows up inside of the bhacktrack of an entity being deleted.

static inline void
bindRemover(char *relName, char *entName, struct RelTable *relHash, struct EntTable *entHash) {

    int result = 0;
    struct PlainRel *relFound = NULL;

    if (strlen(relName) > 3) {

        result = hash64(relName[1]) * 64 + hash64(relName[2]);

    } else {

        result = 4096;

    }

    relFound = RelationLookup(relName, result, relHash);

    if (relFound != NULL) {

        if (relFound->binded != NULL) {

            //The functions first looks if the entity is a destination

            for (unsigned int i = 0; i < relFound->cplNumber; i++) {

                if (strcmp(relFound->binded[i].destination, entName) == 0) {

                    //It is a destination, frees all the sources binded and fixes all their backtracks

                    for (unsigned int j = 0; j < relFound->binded[i].srcNumber; j++) {

                        FixBacktrack(relFound->relName, relFound->binded[i].source[j], entHash);
                        free(relFound->binded[i].source[j]);

                    }

                    struct Couples *newBind = calloc(relFound->cplNumber - 1, sizeof(struct Couples));
                    unsigned int temp = 0;

                    //Deletes the destinations, and fixes the destinations array

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

                }

                //After deleting a possible destination, the function starts looking for sources that matches the
                //Name of the entity to delete.
                for (unsigned int k = 0; k < relFound->cplNumber; k++) {

                    for (unsigned int j = 0; j < relFound->binded[k].srcNumber; j++) {

                        if (strcmp(relFound->binded[k].source[j], entName) == 0) {

                            //If the function finds a source, fixes the backtrack and goes to the next dest
                            //as per requirements finding another would mean duplicates, which is impossible

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

//This function creates a backtrack for a particular relation inside an entity

static inline void AddBacktrack(struct PlainEnt *toAdd, char *relName) {

    bool trackFound = false;
    unsigned int index = 0;

    //If the backtrack array is empty, add the relation first
    if (toAdd->backTrack == NULL) {

        toAdd->backtrackIndex = 1;
        toAdd->backTrack = malloc(sizeof(struct Track));
        toAdd->backTrack[0].relName = malloc(strlen(relName) + 1);
        strcpy(toAdd->backTrack[0].relName, relName);

    } else {

        //Looks for the relation, if it finds an existing backtrack, increment the counter

        for (unsigned int i = 0; i < toAdd->backtrackIndex; i++) {

            if (strcmp(toAdd->backTrack[i].relName, relName) ==
                0) {

                trackFound = true;
                break;
            }

            index++;

        }

        if (trackFound == true) {

            toAdd->backTrack[index].counter++;


        } else { //If it doesn't exist in memory, adds it last

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

    while (ParseTxt(entitiesHash, relationHash)) {}

}

//The inefficient deletion (626)  reallocs all the entities as they were placed in order, but it was enough to overwrite
//the entity with the last one under the same key and realloc the array with one less cell. Forgot to fix.