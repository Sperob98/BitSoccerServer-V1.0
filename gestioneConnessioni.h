#ifndef GESTIONE_CONNESSSIONI_H
#define GESTIONE_CONNESSSIONI_H

#include <json-c/json.h>

char *get_tipo_richiesta(char *messaggio);

void gestione_disconessione_client(int socket_disconessa);

#endif
