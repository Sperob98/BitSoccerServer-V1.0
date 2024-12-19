#include "player.h"
#include "variabiliGlobali.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <json-c/json.h>

void aggiungi_utente_connesso(char *messaggio, int client_socket){

    //Deserializza messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *playerJson;
    if( (json_object_object_get_ex(parsed_json, "utente", &playerJson))>0 ){

        if( (json_object_get_string(playerJson))!= NULL ){

            //Controlla se l'username non esiste già
            for(int i=0; i<SIZE_ARRAY_PLAYERS; i++){

                if(playersConnessi[i] != NULL){

                    if(strcmp(playersConnessi[i]->nome_player,json_object_get_string(playerJson)) == 0){

                        send(client_socket, "busy\n", strlen("busy\n"), 0);
                        printf("Impossibile aggiungere l'utente %s, già esiste\n",json_object_get_string(playerJson));
                        return;
                    }
                }
            }

            //Aggiungi nuovo utente se c'è spazio nell'array dei player connessi
            player *newPlayer = malloc(sizeof(player));
            if(newPlayer != NULL){

                strcpy(newPlayer->nome_player,json_object_get_string(playerJson));
                newPlayer->socket = client_socket;
                newPlayer->infortunato = 0;
                newPlayer->penalizzato = 0;

                for(int i=0; i<SIZE_ARRAY_PLAYERS; i++){

                    if(playersConnessi[i] == NULL){

                        playersConnessi[i] = newPlayer;
                        printf("Player %s aggiunto ai player connessi\n",newPlayer->nome_player);
                        send(client_socket, "ok\n", strlen("ok\n"), 0);
                        return;
                    }
                }

                //Se arriva qui significa che non ci sono posti NULL nell'array dei player connessi, server sovraffollato
                send(client_socket, "pieno\n", strlen("pieno\n"), 0);
                printf("Impossibile aggiungere l'utente %s, l'array dei player connessi è pieno",newPlayer->nome_player);
                free(newPlayer);
                return;
            }
        }
    }

    printf("Errore generico nella richiesta di aggiunta utente\n");
    send(client_socket, "ko\n", strlen("ko\n"),0);
}
