#ifndef PARTITA_H
#define PARTITA_H

#define SIZE_ARRAY_PARTITE 5

#include "squadra.h"

typedef struct{

    squadra *squadra_A;
    squadra *squadra_B;
    char inizioTurno[SIZE_NAME_PLAYER];
    int finePartita;
    int inizioPartita;

}partita;

#endif
