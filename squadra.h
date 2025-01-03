#ifndef SQUADRA_H
#define SQUADRA_H

#define SIZE_NAME_TEAM 128
#define SIZE_ARRAY_TEAMS 50
#define SIZE_ARRAY_PLAYER_PARTECIPANTI 4
#define SIZE_ARRAY_SQUADRE_IN_ATTESA_DI_MATCH 10

#include "player.h"
#include <pthread.h>

typedef struct{

    char nome_squadra[SIZE_NAME_TEAM];
    player *capitano;
    player *players[SIZE_ARRAY_PLAYER_PARTECIPANTI];
    player *richiestePartecipazione[SIZE_ARRAY_PLAYERS];
    int isPronto;
    int numeroPlayers;

}squadra;

int aggiungi_nuova_squadra(char *messaggio, int client_socket);

void send_lista_squadre_clients();

void send_lista_squadre_client(int client);

void aggiungi_richiesta_partecipazione_squadra(char *messaggio, int client_socket);

void aggiornamento_composizione_squadra(char *messaggio);

void cerca_squadra_match(char *messaggio,int sockCapitano);

#endif
