#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXSIZE 1024
#define CMD_SIZE 300
#define CHANGE 'c'
#define DELETE 'd'
#define PRINT 'p'
#define UNDO 'u'
#define REDO 'r'
#define QUIT 'q'

typedef struct state_s{
    struct state_s *next;
    struct state_s *prev;
    struct indexstring_s *savedState;
    int uneffectiveUndoLoop;
    int uneffectiveRedoLoop;
    int lastIndex; //indice per ogni stato del loro ultimo elemento di saved state
}state_t;

typedef struct indexstring_s{
    char *currString;
}indexstring_t;

typedef struct stringspool_s{
    char *s;
    struct stringspool_s *next;
}stringspool_t;

/*FUNZIONI*/
void change(int, int);
void printRange(int, int);
void delete(int, int);
void undoState(int);
void redoState(int);
void repairStateList();

stringspool_t* allocNewString(stringspool_t*, char*);
bool isEffective(int, int, char);
state_t* appendInExistingState(int, int, int, state_t*);

state_t *currentState=NULL;
state_t *lastState=NULL;
state_t *hState=NULL;
stringspool_t *allStringsh = NULL;
stringspool_t *lastString = NULL;


int main(int argc, char *argv[]){
    char cmd[CMD_SIZE+1];
    int ind1, ind2, j;
    bool effective = false;
    bool hadUndoRedo = false;

    if(hState=(state_t*)malloc(sizeof(state_t))){
        //stato di documento vuoto
        hState->prev=NULL;
        hState->next=NULL;
        hState->lastIndex = -1;
        hState->uneffectiveUndoLoop = 0;
        hState->uneffectiveRedoLoop = 0;
        hState->savedState = NULL;
        currentState = hState;
        lastState = hState;
    }else
        printf("inizializzazione lista stati andata male\n");

    fgets(cmd, CMD_SIZE, stdin);
    while(cmd[0]!= QUIT){
        ind1 = atoi(cmd);
        ind2=-1; //lo zero è ancora un numero di input possibile, anche se è un caso particolare da gestire
        j=0;
        while(cmd[j]!= '\0'){
            if(cmd[j]==',')
                ind2 = atoi(cmd+j+1);
            j++;
        }

        if((cmd[j-2]==CHANGE || cmd[j-2]==DELETE) && hadUndoRedo){
            effective = isEffective(ind1, ind2, cmd[j-2]);
            if(effective) {
                hadUndoRedo = false;
            }
            currentState->uneffectiveRedoLoop = 0;
            repairStateList();
        }
        switch(cmd[j-2]){
            case CHANGE: change(ind1, ind2);
                        break;
            case DELETE: delete(ind1, ind2);
                        break;
            case PRINT: printRange(ind1, ind2);
                        break;
            case UNDO: undoState(ind1);
                        break;
            case REDO: redoState(ind1);
            default: break;
        }
        if(cmd[j-2]==UNDO || cmd[j-2]==REDO)
            hadUndoRedo = true;
        fgets(cmd, CMD_SIZE, stdin);
    }

    return 0;
}

void delete(int ind1, int ind2){
    state_t *newState;
    indexstring_t *indexp1, *indexp2;
    int j, toDel, tot, i;

    if(ind2==0){
        currentState->uneffectiveUndoLoop = currentState->uneffectiveUndoLoop + 1;
        return;
    }

    if(ind1==0)
        ind1++;
    /*caso 1: eliminazione di righe non presenti*/
    if(ind1>(currentState->lastIndex+1)){
        currentState->uneffectiveUndoLoop = currentState->uneffectiveUndoLoop +1;
        return;
    }
    /*la cancellazione è in parte di righe non esistenti*/
    if(ind2>(currentState->lastIndex+1))
        ind2=currentState->lastIndex+1;
    toDel = ind2 - ind1 +1;
    tot = (currentState->lastIndex +1) - toDel;


    if(newState=(state_t*)malloc(sizeof(state_t))){
        /*elimino tutte le stringhe*/
        currentState->next = newState;
        newState->prev = currentState;
        newState->next = NULL;
        if(tot==0){
            newState->lastIndex = -1;//savedstate è vuoto, nè allocato
            newState->savedState = NULL;
            newState->uneffectiveUndoLoop = 0;
            newState->uneffectiveRedoLoop = 0;
            currentState = newState;
            lastState = newState;
            return;
        }
        if(indexp1=(indexstring_t*)malloc(sizeof(indexstring_t)*tot)) {
            newState->savedState = indexp1;
            newState->uneffectiveUndoLoop = 0;
            newState->uneffectiveRedoLoop = 0;
            j=0;
            indexp2 = currentState->savedState;
            /*caso 2: la cancellazione è in mezzo*/
            while(j<ind1-1){
                (indexp1+j)->currString = (indexp2+j)->currString;
                j++;
            }
            /*caso 3: la cancellazione è in testa*/
            /*caso 4: la cancellazione è una parte finale (e al limite anche stringhe inesistenti che abbiamo già gestito modificando ind2)*/

            i = ind2;//elemento in indexp2 successivo a ind2, dopo la cancellazione in mezzo o in testa
            while(i<(currentState->lastIndex+1)){
                (indexp1+j)->currString = (indexp2+i)->currString;
                j++;
                i++;
            }
        }else
                printf("DELETE: errore malloc indexstring\n");
        newState->lastIndex = tot-1;
        currentState = newState;
        lastState = newState;
    }else
        printf("DELETE: errore malloc nuovo stato\n");
}


void change(int ind1, int ind2){
    state_t *newState;
    stringspool_t *stringp;
    indexstring_t *indexp1, *indexp2;
    char str[MAXSIZE+2];
    int addStr, addInd, first, j;

    if(ind2==0){
        currentState->uneffectiveUndoLoop = currentState->uneffectiveUndoLoop +1;
        return;
    }

    if(newState=(state_t*)malloc(sizeof(state_t))){
        currentState->next = newState;
        newState->prev = currentState;
        newState->next = NULL;
        newState->uneffectiveUndoLoop = 0;
        newState->uneffectiveRedoLoop = 0;
        lastState = newState;

        if(ind1==0)
            ind1++;
        addStr = ind2-ind1+1;
        //Le stringhe sono tutte nuove, le devo appendere in coda
        if(ind1>(currentState->lastIndex+1)){
            addInd = (currentState->lastIndex+1) + addStr;
            if(currentState!=hState && currentState->savedState!=NULL) {
                newState = appendInExistingState(addInd, ind1, ind2, newState);
                return;
            }
        }
        //le stringhe sono tutte già presenti devo modificare il contenuto
        else if(ind2<=(currentState->lastIndex+1)){
            addInd = currentState->lastIndex+1;
        }
        //parte delle stringhe esiste già ma almeno l'ultima è nuova, ind2 è il mio nuovo totale di stringhe
        else{
            addInd = ind2;
        }
        if(indexp1=(indexstring_t*)malloc(sizeof(indexstring_t)*addInd)){
            newState->savedState = indexp1;
            j=0;
            /*copia degli indirizzi di tutte le stringhe precedenti immodificate, se lo stato precedente non ha stringhe (ad esempio hState),
             non entra nell'if, se le modifiche da effettuare partono da ind1 allora non fa il for*/
            if(currentState!=hState && currentState->savedState!=NULL){
                indexp2 = currentState->savedState;
                //ricopio le prime stringhe dallo stato precedente
                for(first = 1; first < ind1 && j<addInd; first++, j++){
                    (indexp1+j)->currString = (indexp2+j)->currString;
                }

            }
            /*ha una modifica in mezzo; newState è il primo stato; le modifiche sono parzialmente o totalmente in fondo*/
            while(ind1<=ind2){
                fgets(str, MAXSIZE, stdin);
                if(stringp=(stringspool_t*)malloc(sizeof(stringspool_t))){
                    if(allStringsh==NULL){
                        allStringsh = stringp;
                    }else{
                        lastString->next = stringp;
                    }
                    lastString = stringp;
                    stringp=allocNewString(stringp, str);
                    (indexp1+j)->currString = stringp->s;
                }else
                    printf("CHANGE: errore malloc nuovo nodo stringa\n");
                ind1++;
                j++;
            }
            /*ci sono ancora delle stringhe da copiare perché la modifica era interna a first-last di currString*/
            while(currentState->savedState!=NULL && ind1<=(currentState->lastIndex+1)){
                (indexp1+j)->currString = (indexp2+j)->currString;
                ind1++;
                j++;
            }
            fgets(str, MAXSIZE, stdin);
            if(*str!='.')
                printf("non ho letto l'ultima stringa punto\n");
            newState->lastIndex = addInd-1;
            currentState = newState;
        }else
            printf("CHANGE: errore malloc nuova cronologia stringhe stato\n");
    }else
        printf("CHANGE: errore malloc nuovo stato\n");

    return;
}

stringspool_t* allocNewString(stringspool_t *dest, char *copyS){

    if(dest->s=(char*)malloc(sizeof(char)*(strlen(copyS)+1))){
        dest->s=strcpy(dest->s, copyS);
        dest->next = NULL;
    }else
        printf("ALLOC NEW STRING: errore malloc nuova stringa\n");

    return dest;
}

void printRange(int ind1, int ind2) {
    int i;
    indexstring_t *indexp;

    if (ind2 == 0) {
        printf(".\n");
        return;
    }
    if (ind1 == 0) {
        printf(".\n");
        ind1++;
    }

    i = ind1 - 1;
    if (currentState != hState) {
        indexp = currentState->savedState;
        while (ind1 <= (currentState->lastIndex+1) && ind1 <= ind2) {
            fputs((indexp + i)->currString, stdout);
            ind1++;
            i++;
        }
    }
    while(ind1<=ind2 && (ind1>(currentState->lastIndex+1) || currentState==hState)){
        printf(".\n");
        ind1++;
    }

    return;
}

stringspool_t *searchString(char *toSearch){
    stringspool_t *stringp;
    int found = 0;
    int i;

    for(stringp = allStringsh; stringp != NULL && found == 0; stringp = stringp->next){
        for(i=0; *(toSearch+i)!='\0' && *((stringp->s)+i)!='\0' && *(toSearch+i)==*((stringp->s)+i); i++)
            ;
        if(*(toSearch+i)=='\0' && *((stringp->s)+i)=='\0') {
            found = 1;
            break;
        }
    }

    return stringp;
}

void undoState(int count){
    int doneCount;

    if(currentState==hState && currentState->uneffectiveUndoLoop==0){
        return;
    }
    doneCount = 1;
    while(doneCount<=count) {
        while (currentState->uneffectiveUndoLoop > 0 && doneCount <= count) {
            currentState->uneffectiveUndoLoop = currentState->uneffectiveUndoLoop - 1;
            currentState->uneffectiveRedoLoop = currentState->uneffectiveRedoLoop + 1;
            doneCount++;
        }
        if(currentState==hState)
            return;
        /*se currentstate->uneffectiveUndoLoop = 3 e ho 3 undo, per spostarmi di stato dovrei avere 4u ma non ce li ho*/
        if (doneCount <= count) {
            currentState = currentState->prev;
            doneCount++;
        }
    }

    return;
}

void redoState(int count){
    int doneCount;

    if(currentState==lastState && currentState->uneffectiveRedoLoop==0){
        return;
    }
    doneCount = 1;
    while(doneCount<=count){
        while(currentState->uneffectiveRedoLoop>0 && doneCount<=count){
            currentState->uneffectiveUndoLoop = currentState->uneffectiveUndoLoop +1;
            currentState->uneffectiveRedoLoop = currentState->uneffectiveRedoLoop -1;
            doneCount++;
        }
        if(currentState==lastState)
            return;
        if(doneCount<=count) {
            currentState = currentState->next;
            doneCount++;
        }
    }
    return;
}

void repairStateList(){
    state_t *toDel, *statep;

    /*già non ci sono stati dopo*/
    if(currentState==lastState)
        return;
    toDel = currentState->next;
    currentState->next = NULL;
    lastState = currentState;
    while(toDel->next!=NULL){
        statep = toDel->next;
        free(toDel);
        toDel = statep;
    }
    /*cancellazione dell'ultimo stato*/
    if(toDel->next==NULL){
        free(toDel);
    }
    return;
}

bool isEffective(int ind1, int ind2, char cmd){
    if(ind2==0)
        return false;
    if(ind1==0)
        ind1++;
    if(cmd==DELETE){
        if(ind1>(currentState->lastIndex+1))
            return false;
        return true;
    }else
        return true;
}


state_t *appendInExistingState(int totNewAlloc, int ind1, int ind2, state_t *newState){
    int j;
    char str[MAXSIZE+2];
    stringspool_t *stringp;
    indexstring_t *indexp1;

    /*espandiamo la memoria dello stato precedente, appendendo le nuove stringhe in coda
     * lo stato precedente mantiene il suo lastIndex corretto mentre newState ne avrà uno più alto*/
    if(currentState->savedState=(indexstring_t*)realloc(currentState->savedState, totNewAlloc*(sizeof(indexstring_t)))){
        newState->savedState = currentState->savedState;
        newState->lastIndex = totNewAlloc-1;
        indexp1 = currentState->savedState;
        j=ind1-1;
        while(j<ind2){
            fgets(str, MAXSIZE, stdin);
            if(stringp=(stringspool_t*)malloc(sizeof(stringspool_t))){
                if(allStringsh==NULL){
                    allStringsh = stringp;
                }else{
                    lastString->next = stringp;
                }
                lastString = stringp;
                stringp=allocNewString(stringp, str);
                (indexp1+j)->currString = stringp->s;
            }else
                printf("CHANGE: errore malloc nuovo nodo stringa\n");
            j++;
        }
        lastState = newState;
        currentState = newState;
    }else
        printf("errore realloc\n");
    /*per la lettura del punto finale*/
    fgets(str, MAXSIZE, stdin);

    return newState;
}
