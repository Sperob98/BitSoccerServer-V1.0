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

typedef struct{

    int indexPartita;
    char player_name[SIZE_NAME_PLAYER];

}argomentiThreadInfortunio;

typedef struct{

    int indexPartita;
    int timeP;
    char player[SIZE_NAME_PLAYER];

}argomentiThreadPenalizzazione;

int get_index_partita(char *messaggio);

void assegna_turno_iniziale_e_avvia_match(char *messaggio);

#endif
