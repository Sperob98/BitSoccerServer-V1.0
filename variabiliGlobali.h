#ifndef VARIABILI_GLOBALI_H
#define VARIABILI_GLOBALI_H

#include "squadra.h"
#include "player.h"
#include "partita.h"
#include <pthread.h>

extern pthread_mutex_t mutexListaSquadre;
extern pthread_cond_t condListaSquadre;
extern squadra *squadreInCostruzione[SIZE_ARRAY_TEAMS];

extern player *playersConnessi[SIZE_ARRAY_PLAYERS];
extern pthread_mutex_t mutexPlayers;
extern pthread_cond_t condPlayers;

extern partita *partite[SIZE_ARRAY_PARTITE];
extern pthread_mutex_t mutexPartite;
extern pthread_cond_t condPartite;

#endif
