#include "squadra.h"
#include "variabiliGlobali.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <json-c/json.h>

////////////////////Funzioni interne per ausilio alle funzione chiamate dal main//////////////////////////
char* serializza_array_squadre(){

    json_object *jarray = json_object_new_array();

    if(jarray != NULL){

        for (int i = 0; i<SIZE_ARRAY_TEAMS; i++) {

            if(squadreInCostruzione[i] != NULL){

                if(squadreInCostruzione[i]->isPronto == 0){

                    json_object *jobj = json_object_new_object();
                    json_object_object_add(jobj, "nomeSquadra", json_object_new_string(squadreInCostruzione[i]->nome_squadra));
                    json_object_object_add(jobj, "capitano", json_object_new_string(squadreInCostruzione[i]->capitano->nome_player));
                    json_object_object_add(jobj, "numeroPlayers", json_object_new_int(squadreInCostruzione[i]->numeroPlayers));
                    json_object_array_add(jarray, jobj);
                }
            }
        }

        const char *json_str = json_object_to_json_string(jarray);
        char *json_copy = strdup(json_str); // Copia la stringa JSON per restituirla
        json_object_put(jarray); // Dealloca l'oggetto JSON

        return json_copy;
    }

    return NULL;
}

char *serializza_oggetto_composizione_squadre(int indexSquadra){

    // Creazione dei due array JSON di stringhe
    struct json_object *json_array_richieste = json_object_new_array();
    struct json_object *json_array_accettati = json_object_new_array();

    if( (json_array_richieste != NULL) && (json_array_accettati != NULL)){

        if(squadreInCostruzione[indexSquadra]!= NULL){

            //Costruisce l'array dei player che hanno fatto richiesta di partecipazione
             for(int j=0; j<SIZE_ARRAY_TEAMS; j++){

                if(squadreInCostruzione[indexSquadra]->richiestePartecipazione[j] != NULL){

                    char *richiesta = squadreInCostruzione[indexSquadra]->richiestePartecipazione[j]->nome_player;
                    json_object_array_add(json_array_richieste, json_object_new_string(richiesta));
                }
            }

            //Costruisce l'array dei player accettati nella squadra
            char playerCapitano[SIZE_NAME_PLAYER];
            strcpy(playerCapitano,squadreInCostruzione[indexSquadra]->capitano->nome_player);
            strcat(playerCapitano," (capitano)\0");
            json_object_array_add(json_array_accettati, json_object_new_string(playerCapitano));
            for(int j=0; j<4; j++){

                if(squadreInCostruzione[indexSquadra]->players[j] != NULL){
                    char *playerAccettato = squadreInCostruzione[indexSquadra]->players[j]->nome_player;
                    json_object_array_add(json_array_accettati, json_object_new_string(playerAccettato));
                }
            }
        }

        //Creazione dell'oggetto principale che contiene i due array
        struct json_object *root = json_object_new_object();
        json_object_object_add(root, "richieste", json_array_richieste);
        json_object_object_add(root, "accettati", json_array_accettati);

        // Serializza l'oggetto JSON in una stringa
        char *json_string = json_object_to_json_string(root);
        strcat(json_string,"\n");

        return json_string;
    }

    return NULL;
}

void send_aggiornamento_composizione_squadra(char *nomeSquadra){

    //indice for
    int indexSquadra;

    //Cerca la squdra nell'array
    for(indexSquadra=0;indexSquadra<SIZE_ARRAY_TEAMS;indexSquadra++){

        if(squadreInCostruzione[indexSquadra] != NULL){

            if(strcmp(squadreInCostruzione[indexSquadra]->nome_squadra,nomeSquadra) == 0) break;
        }
    }

    if(indexSquadra < SIZE_ARRAY_TEAMS){

        //Recupero degli oggetti che contengono array richieste e l'array accettati da mandare ai clients dello spogliatolio della squadra
        char *oggettoDaInviare = serializza_oggetto_composizione_squadre(indexSquadra);

        if(oggettoDaInviare != NULL){

            //Aggiorna tutti i player client (e il capitano) della lista richieste

            //Aggiornamento al capitano
            int socket_player = squadreInCostruzione[indexSquadra]->capitano->socket;
            send(socket_player,"AggiornamentoComposizioneSquadra\n",strlen("AggiornamentoComposizioneSquadra\n"),0);
            send(socket_player,oggettoDaInviare,strlen(oggettoDaInviare),0);
            printf("Inviato l'oggetto: %s al capitano %s\n",oggettoDaInviare,squadreInCostruzione[indexSquadra]->capitano->nome_player);

            //Aggiornamento ai player che sono in lista di richiesta
            for(int k=0; k<SIZE_ARRAY_PLAYERS; k++){

                if(squadreInCostruzione[indexSquadra]->richiestePartecipazione[k] != NULL){

                    int socket_player = squadreInCostruzione[indexSquadra]->richiestePartecipazione[k]->socket;
                    send(socket_player,"AggiornamentoComposizioneSquadra\n",strlen("AggiornamentoComposizioneSquadra\n"),0);
                    send(socket_player,oggettoDaInviare,strlen(oggettoDaInviare),0);
                    printf("Inviato l'oggetto: %s al player %s\n",oggettoDaInviare,squadreInCostruzione[indexSquadra]->richiestePartecipazione[k]->nome_player);
                }
            }

            //Aggiornamento dei player accettati
            for(int k=0; k<SIZE_ARRAY_PLAYER_PARTECIPANTI; k++){

                if(squadreInCostruzione[indexSquadra]->players[k] != NULL){

                    int socket_player = squadreInCostruzione[indexSquadra]->players[k]->socket;
                    send(socket_player,"AggiornamentoComposizioneSquadra\n",strlen("AggiornamentoComposizioneSquadra\n"),0);
                    send(socket_player,oggettoDaInviare,strlen(oggettoDaInviare),0);
                    printf("Inviato l'oggetto: %s al player %s\n",oggettoDaInviare,squadreInCostruzione[indexSquadra]->players[k]->nome_player);
                }
            }
        }
    }
}

char *serializza_oggetto_info_match(int indexPartita){

    // Creazione array JSON
    struct json_object *json_array_playersA = json_object_new_array();
    struct json_object *json_array_playersB = json_object_new_array();


    //Inizializzi array JSON
    for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

        char *playerA = partite[indexPartita]->squadra_A->players[i]->nome_player;
        char *playerB = partite[indexPartita]->squadra_B->players[i]->nome_player;
        json_object_array_add(json_array_playersA,json_object_new_string(playerA));
        json_object_array_add(json_array_playersB,json_object_new_string(playerB));

    }

    //Creazione stignhe JSON
    char *capitanoA = partite[indexPartita]->squadra_A->capitano->nome_player;
    char *capitanoB = partite[indexPartita]->squadra_B->capitano->nome_player;
    char *squadraA = partite[indexPartita]->squadra_A->nome_squadra;
    char *squadraB = partite[indexPartita]->squadra_B->nome_squadra;

    struct json_object *json_capitanoA = json_object_new_string(capitanoA);
    struct json_object *json_capitanoB = json_object_new_string(capitanoB);
    struct json_object *json_squadraA = json_object_new_string(squadraA);
    struct json_object *json_squadraB = json_object_new_string(squadraB);
    struct json_object *json_indexPartita = json_object_new_int(indexPartita);

    //Creazione dell'oggetto principale che contiene i due array e le stringhe
    struct json_object *root = json_object_new_object();
    json_object_object_add(root, "playersA", json_array_playersA);
    json_object_object_add(root, "playersB", json_array_playersB);
    json_object_object_add(root, "capitanoA", json_capitanoA);
    json_object_object_add(root, "capitanoB", json_capitanoB);
    json_object_object_add(root, "squadraA", json_squadraA);
    json_object_object_add(root, "squadraB", json_squadraB);
    json_object_object_add(root, "indexPartita", json_indexPartita);

    // Serializza l'oggetto JSON in una stringa
    char *json_string = json_object_to_json_string(root);
    char delimitatore[] = "\n";
    strcat(json_string,delimitatore);

    return json_string;
}

void avvisa_players_stato_match(int indexPartita, char *messaggio){

    //CASO: AVVIO PARTITA
    if(indexPartita > -1){

        //serializza info partita
        partita *partitaInAvvio = partite[indexPartita];
        char *jsonInfoMatch = serializza_oggetto_info_match(indexPartita);

        if(jsonInfoMatch != NULL){

            //Avverti capitano squadra A
            int clientCapitanoA = partitaInAvvio->squadra_A->capitano->socket;
            send(clientCapitanoA,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
            send(clientCapitanoA, "avvioMatch\n", strlen("avvioMatch\n"),0);
            send(clientCapitanoA,jsonInfoMatch,strlen(jsonInfoMatch),0);

            printf("Avvisato il capitano %s della squadra %s dell'avvio del match\n",partitaInAvvio->squadra_A->capitano->nome_player,partitaInAvvio->squadra_A->nome_squadra);

            //Avverti capitano squadra B
            int clientCapitanoB = partitaInAvvio->squadra_B->capitano->socket;
            send(clientCapitanoB,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
            send(clientCapitanoB, "avvioMatch\n", strlen("avvioMatch\n"),0);
            send(clientCapitanoB,jsonInfoMatch,strlen(jsonInfoMatch),0);

            printf("Avvisato il capitano %s della squadra %s dell'avvio del match\n",partitaInAvvio->squadra_B->capitano->nome_player,partitaInAvvio->squadra_B->nome_squadra);

            //Avverti i player accettati
            for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                int playerA = partitaInAvvio->squadra_A->players[i]->socket;
                int playerB = partitaInAvvio->squadra_B->players[i]->socket;


                send(playerA,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
                send(playerA, "avvioMatch\n", strlen("avvioMatch\n"),0);
                send(playerA,jsonInfoMatch,strlen(jsonInfoMatch),0);

                printf("Avvisato il player %s della squadra %s dell'avvio del match\n",partitaInAvvio->squadra_A->players[i]->nome_player,partitaInAvvio->squadra_A->nome_squadra);

                send(playerB,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
                send(playerB, "avvioMatch\n", strlen("avvioMatch\n"),0);
                send(playerB,jsonInfoMatch,strlen(jsonInfoMatch),0);

                printf("Avvisato il player %s della squadra %s dell'avvio del match\n",partitaInAvvio->squadra_B->players[i]->nome_player,partitaInAvvio->squadra_B->nome_squadra);

            }
        }

    //CASO: ATTESA SQUADRA AVVERSARIA
    }else if(indexPartita == -1){

        //Deserializzazione del messaggio per estrarre la squdra che ha chiesto il match
        struct json_object *parsed_json;
        parsed_json = json_tokener_parse(messaggio);

        json_object *nomeSquadraJSON;

        json_object_object_get_ex(parsed_json, "squadra", &nomeSquadraJSON);
        char *nomeSquadra = json_object_get_string(nomeSquadraJSON);

        //Cerca squadra nell'array
        int i;
        for(i=0; i<SIZE_ARRAY_TEAMS;i++){

            if(squadreInCostruzione[i] != NULL){

                if(strcmp(squadreInCostruzione[i]->nome_squadra,nomeSquadra) == 0){

                    //Avverti capitano
                    int clientCapitano = squadreInCostruzione[i]->capitano->socket;
                    send(clientCapitano,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
                    send(clientCapitano, "attesaMatch\n", strlen("attesaMatch\n"),0);

                    printf("Avvisato il capitano %s della squadra %s di in attesa di squadra avversaria\n",squadreInCostruzione[i]->capitano->nome_player,squadreInCostruzione[i]->nome_squadra);

                    //Avverti i player
                    for(int j=0; j<SIZE_ARRAY_PLAYER_PARTECIPANTI; j++){

                        int clientPlayer = squadreInCostruzione[i]->players[j]->socket;
                        send(clientPlayer,"rispostaMatch\n", strlen("rispostaMatch\n"),0);
                        send(clientPlayer, "attesaMatch\n", strlen("attesaMatch\n"),0);

                        printf("Avvisato il player %s della squadra %s di in attesa di squadra avversaria\n",squadreInCostruzione[i]->players[j]->nome_player,squadreInCostruzione[i]->nome_squadra);

                    }
                }
            }
        }
    }
}

////////////////////////////////Funzione del header////////////////////////////////////////////////////////

int aggiungi_nuova_squadra(char *messaggio, int client_socket){

    int statoDiCreazione = 0;

    //Deserializzazione messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *nomeSquadraJSON;
    json_object *capitanoJSON;

    json_object_object_get_ex(parsed_json, "squadra", &nomeSquadraJSON);
    json_object_object_get_ex(parsed_json, "capitano", &capitanoJSON);

    if( (json_object_get_string(nomeSquadraJSON) == NULL) || (json_object_get_string(capitanoJSON) == NULL) ){

        printf("Errore parsing JSON durante una richiesta di aggiunta di una squadra\n");
        return statoDiCreazione;
    }

    //Controlla se il nome squadra è unico
    for(int i=0; i<SIZE_ARRAY_TEAMS; i++){

        if(squadreInCostruzione[i] != NULL){

            if(strcmp(squadreInCostruzione[i]->nome_squadra,json_object_get_string(nomeSquadraJSON)) == 0){

                printf("Errore creazione dquadra, nome squadra %s già esistente\n",squadreInCostruzione[i]->nome_squadra);

                //Invio stato di fallimento creazione, nome non unico
                send(client_socket, "statoCreazione\n", strlen("statoCreazione\n"), 0);
                send(client_socket, "busy\n", strlen("busy\n"), 0);

                return statoDiCreazione;
            }
        }
    }

    //Creazione della struttura della nuova squadra
    squadra *newSquadra = malloc(sizeof(squadra));
    if(newSquadra != NULL){

        //assegnazione nome della squadra
        strcpy(newSquadra->nome_squadra,json_object_get_string(nomeSquadraJSON));

        //Settaggio a false della variabile isPronto che indica quando la squadra è completa e pront per avviare un match
        newSquadra->isPronto = 0;

        //Setting numeroPlayers a 1 (alla creazione della squadra c'è solo il capitano)
        newSquadra->numeroPlayers = 1;

        //Cerca il capitano nell'array globale e lo assegna alla nuova squdra creata
        int i;
        for(i=0; i<SIZE_ARRAY_PLAYERS; i++){

            if(playersConnessi[i] != NULL){

                if(strcmp(playersConnessi[i]->nome_player,json_object_get_string(capitanoJSON)) == 0){

                    //Assegnazione capitano
                    newSquadra->capitano = playersConnessi[i];

                    break;
                }
            }
        }

        //Inizializza array players
        for(int k=0; k<SIZE_ARRAY_PLAYER_PARTECIPANTI; k++){

            newSquadra->players[k] = NULL;
        }

        //Inizialliza array richieste
         for(int k=0; k<SIZE_ARRAY_PLAYERS; k++){

            newSquadra->richiestePartecipazione[k] = NULL;
        }

        //Aggiunta della nuova squadra nell'array globale delle squadre
        for(i=0;i<SIZE_ARRAY_TEAMS;i++){

            if(squadreInCostruzione[i] == NULL) break;
        }

        //Se i è 50 non sono stati trovati posti liberi nell'array per aggiugnere la squadra
        if(i>49){

            printf("Impossibile aggiungere la squadra %s nell'array delle squadre. Array pieno\n",newSquadra->nome_squadra);

            send(client_socket, "statoCreazione\n", strlen("statoCreazione\n"), 0);
            send(client_socket, "max\n", strlen("max\n"), 0);

            free(newSquadra);

            return statoDiCreazione;
        }

        //Aggiunta squadra nell'array
        squadreInCostruzione[i] = newSquadra;
        printf("Squadra %s fondata dal capitano %s\n", newSquadra->nome_squadra,newSquadra->capitano->nome_player);
        statoDiCreazione = 1;

        send(client_socket, "statoCreazione\n", strlen("statoCreazione\n"), 0);
        send(client_socket, "ok\n", strlen("ok\n"), 0);

        return statoDiCreazione;
    }

    return statoDiCreazione;
}

void *thread_send_lista_squadre_client(void* client_socket){

    printf("Nuovo thread(invio lista squadre) creato\n");

    int client = *(int*)client_socket;
    char msg_daInviare[1024];
    int bytes_read;
    int aggiornamentoLista = 1;

    //Ricerca del player a cui è associato la socket
    int indexPlayer = 0;
    for(indexPlayer=0;indexPlayer<SIZE_ARRAY_PLAYERS;indexPlayer++){

        if(playersConnessi[indexPlayer]->socket==client) break;
    }

    while(1){

        pthread_mutex_lock(&mutexListaSquadre);
        //Mette in pausa il thread finché non è richiesto di aggiornare la lista squadre
        while(aggiornamentoLista == 0){

            pthread_cond_wait(&condListaSquadre,&mutexListaSquadre);
            aggiornamentoLista = 1;
        }

        //Costrunzione array json dellle squadre fondate
        char *json__arraySquadre_str = serializza_array_squadre();

        if(json__arraySquadre_str != NULL){

            //Invio lista squadre
            char buffer[8192];
            snprintf(buffer, sizeof(buffer), "%s\n", json__arraySquadre_str);
            send(client, "AggiornamentoSquadra\n", strlen("AggiornamentoSquadra\n"), 0);
            send(client, buffer, strlen(buffer), 0);

            if(indexPlayer < 50)
                printf("Lista squadre: %s inviato al player: %s\n",json__arraySquadre_str,playersConnessi[indexPlayer]->nome_player);
        }

        aggiornamentoLista = 0;

        pthread_mutex_unlock(&mutexListaSquadre);
    }
}

void aggiungi_richiesta_partecipazione_squadra(char *messaggio, int client_socket){

    //Deserializzazione del messaggio JSON
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    //Estrazione squadra a cui partecipare
    struct json_object *squadraJSON;
    json_object_object_get_ex(parsed_json, "squadra", &squadraJSON);

    //Estrazione nomePlayer
    struct json_object *playerJSON;
    json_object_object_get_ex(parsed_json, "player", &playerJSON);

    if( (json_object_get_string(squadraJSON) != NULL) && (json_object_get_string(playerJSON) != NULL) ){

        //Dichiarazioni indici for
        int indexSquadra;
        int indexPlayer;

        //Cerca indice squadra a cui vuole partecipare il player nell'array globale
        for(indexSquadra=0; indexSquadra<SIZE_ARRAY_TEAMS; indexSquadra++){

            if(squadreInCostruzione[indexSquadra] != NULL){

                if(strcmp(squadreInCostruzione[indexSquadra]->nome_squadra,json_object_get_string(squadraJSON)) == 0) break;
            }
        }

        //Cerca indice player che ha fatto la richiesta nell'array globale
        for(indexPlayer=0; indexPlayer<SIZE_ARRAY_PLAYERS; indexPlayer++){

            if(playersConnessi[indexPlayer] != NULL){

                if(strcmp(playersConnessi[indexPlayer]->nome_player,json_object_get_string(playerJSON)) == 0) break;
            }
        }

        if( (indexSquadra < SIZE_ARRAY_TEAMS) && (indexPlayer < SIZE_ARRAY_PLAYERS) ){

            //Cerca il primo slot di richieste libero per aggiungere la richiesta di partecipazione
            int k;
            for(k=0; k<50; k++){

                if(squadreInCostruzione[indexSquadra]->richiestePartecipazione[k] == NULL){

                    squadreInCostruzione[indexSquadra]->richiestePartecipazione[k] = playersConnessi[indexPlayer];
                    printf("Aggiunta richiesta di partecipazione del player %s alla squadra %s\n",json_object_get_string(playerJSON),json_object_get_string(squadraJSON));

                    //Risposta richiesta
                    send(client_socket, "statoRichiesta\n", strlen("statoRichiesta\n"), 0);
                    send(client_socket, "ok\n", strlen("ok\n"), 0);

                    //Aggiorna la nuova composizione della squadra a tutto lo spogliatoio
                    send_aggiornamento_composizione_squadra(json_object_get_string(squadraJSON));

                    return;
                }
            }
        }
    }

    printf("Errore generico durante l'elaborazione della richiesta di aggiunta di una partecipazione a una squadra\n");
    send(client_socket, "statoRichiesta\n", strlen("statoRichiesta\n"), 0);
    send(client_socket, "ko\n", strlen("ko\n"), 0);
}

void aggiornamento_composizione_squadra(char *messaggio){

    //Deserializzazione del messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *nomeSquadraJSON;
    json_object *nomePlayerJSON;
    json_object *decisioneCapitanoJSON;

    json_object_object_get_ex(parsed_json, "squadra", &nomeSquadraJSON);
    json_object_object_get_ex(parsed_json, "nomePlayer", &nomePlayerJSON);
    json_object_object_get_ex(parsed_json, "decisione", &decisioneCapitanoJSON);

    if( (json_object_get_string(nomeSquadraJSON) != NULL) && (json_object_get_string(nomePlayerJSON) != NULL) && (json_object_get_string(decisioneCapitanoJSON) != NULL) ){

        //indice for che individua la squadra nell'array
        int indexSquadra;
        //indice for che individua il player nell'array
        int indexPlayer;

        //Ricerca della squadra con cui il capitano ha preso un decisione
        for(indexSquadra=0; indexSquadra<SIZE_ARRAY_TEAMS; indexSquadra++){

            if(squadreInCostruzione[indexSquadra] != NULL){

                if(strcmp(squadreInCostruzione[indexSquadra]->nome_squadra,json_object_get_string(nomeSquadraJSON)) == 0) break;
            }

        }

        //Ricerca del player della squadra il cui capitano ha preso una decisione
        for(indexPlayer=0; indexPlayer<SIZE_ARRAY_PLAYERS; indexPlayer++){

            if(playersConnessi[indexPlayer] != NULL){

                if(strcmp(playersConnessi[indexPlayer]->nome_player,json_object_get_string(nomePlayerJSON)) == 0) break;
            }
        }

        if(indexSquadra < SIZE_ARRAY_PLAYERS && indexPlayer < SIZE_ARRAY_TEAMS){

            //Rimozione del player tra le richieste a prescindere se accettato o rifiutato
            for(int k=0; k<SIZE_ARRAY_PLAYERS; k++){

                if(squadreInCostruzione[indexSquadra]->richiestePartecipazione[k] != NULL){

                    if(strcmp(squadreInCostruzione[indexSquadra]->richiestePartecipazione[k]->nome_player,json_object_get_string(nomePlayerJSON)) == 0){

                        squadreInCostruzione[indexSquadra]->richiestePartecipazione[k] = NULL;
                        printf("Rimosso il player %s tra le richieste della squadra %s\n",json_object_get_string(nomePlayerJSON),json_object_get_string(nomeSquadraJSON));

                        break;
                    }
                }
            }

            //Se accettato, inserisce nella lista accettati
            if(strcmp(json_object_get_string(decisioneCapitanoJSON),"accettato") == 0){

                int k;

                for(k=0; k<SIZE_ARRAY_PLAYER_PARTECIPANTI; k++){

                    if(squadreInCostruzione[indexSquadra]->players[k] == NULL){

                        squadreInCostruzione[indexSquadra]->players[k] = playersConnessi[indexPlayer];
                        (squadreInCostruzione[indexSquadra]->numeroPlayers)++;
                        printf("Aggiunto il player %s nella squadra %s \n",playersConnessi[indexPlayer]->nome_player,squadreInCostruzione[indexSquadra]->nome_squadra);

                        int socketCapitano = squadreInCostruzione[indexSquadra]->capitano->socket;
                        send(socketCapitano, "rispostaDecisione\n", strlen("rispostaDecisione\n"),0);
                        send(socketCapitano, "ok\n", strlen("ok\n"),0);

                        break;
                    }
                }

                //Avviso squadra al completo
                if(k >= SIZE_ARRAY_PLAYER_PARTECIPANTI){

                    int socketCapitano = squadreInCostruzione[indexSquadra]->capitano->socket;
                    send(socketCapitano, "rispostaDecisione\n", strlen("rispostaDecisione\n"),0);
                    send(socketCapitano, "full\n", strlen("full\n"),0);

                    //Avverti il client di essere stato rimosso
                    int socketPlayer = playersConnessi[indexPlayer]->socket;
                    send(socketPlayer, "rimozione\n", strlen("rimozione\n"),0);
                }

            }else{

                //Avverti il client di essere stato rimosso
                int socketPlayer = playersConnessi[indexPlayer]->socket;
                send(socketPlayer, "rimozione\n", strlen("rimozione\n"),0);
            }
        }

        //Avverti i client della squadra dell'aggiornamento
        send_aggiornamento_composizione_squadra(json_object_get_string(nomeSquadraJSON));
    }
}

void cerca_squadra_match(char *messaggio,int sockCapitano){

    //Deserializzazione del messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *nomeSquadraJSON;

    json_object_object_get_ex(parsed_json, "squadra", &nomeSquadraJSON);

    if(json_object_get_string(nomeSquadraJSON) != NULL){

        char *nomesquadra = json_object_get_string(nomeSquadraJSON);

        //Cerca l'indice della squadra che ha chiesto il match nell'array delle squadreInCostruzione
        int indexSquadraCheHaChiestoIlMatch;
        for(indexSquadraCheHaChiestoIlMatch=0; indexSquadraCheHaChiestoIlMatch<SIZE_ARRAY_TEAMS; indexSquadraCheHaChiestoIlMatch++){

            if(squadreInCostruzione[indexSquadraCheHaChiestoIlMatch] != NULL){

                int risultatoCmp = strcmp(squadreInCostruzione[indexSquadraCheHaChiestoIlMatch]->nome_squadra,nomesquadra);
                if(risultatoCmp == 0) break;
            }
        }

        //Caso fallimento nella ricerca della squadra
        if(indexSquadraCheHaChiestoIlMatch >= SIZE_ARRAY_TEAMS){

            printf("Stato richiesta match fallito\n");

            //Avverti capitano del fallimento della richiesta
            send(sockCapitano,"rispostaAlCapitano\n", strlen("rispostaAlCapitano\n"),0);
            send(sockCapitano,"ko\n", strlen("ko\n"),0);

            return;
        }

        //Cerca nell'array delle squadre una squadra pronta (diversa dalla squadra che ha chiesto il match)
        int matchTrovato = 0;
        int indexSquadrePronte;
        for(indexSquadrePronte=0; indexSquadrePronte<SIZE_ARRAY_TEAMS; indexSquadrePronte++){

            if(squadreInCostruzione[indexSquadrePronte] != NULL){

                if( squadreInCostruzione[indexSquadrePronte]->isPronto != 0){

                    matchTrovato = 1;
                    break;
                }
            }
        }

        //Se match trovato aggiungi partita e libera l'array squadreIncostruzione
        if(matchTrovato == 1){

            //Aggiunge partita nell'array delle partite
            int i;
            for(i=0; i<SIZE_ARRAY_PARTITE; i++){

                if(partite[i] == NULL){

                    partite[i] = malloc(sizeof(partita));
                    if(partite[i] != NULL){

                        partite[i]->squadra_A = squadreInCostruzione[indexSquadraCheHaChiestoIlMatch];
                        partite[i]->squadra_B = squadreInCostruzione[indexSquadrePronte];
                        strcpy(partite[i]->inizioTurno,"null");
                        partite[i]->finePartita = 0;
                        partite[i]->inizioPartita = 0;
                        printf("Aggiunta partita: %s vs %s\n",nomesquadra,squadreInCostruzione[indexSquadraCheHaChiestoIlMatch]->nome_squadra);

                        //Libera gli array
                        squadreInCostruzione[indexSquadraCheHaChiestoIlMatch] = NULL;
                        squadreInCostruzione[indexSquadrePronte] = NULL;

                        //Avverti il capitano che la richiesta ha avuto successo
                        send(sockCapitano,"rispostaAlCapitano\n", strlen("rispostaAlCapitano\n"),0);
                        send(sockCapitano,"ok\n", strlen("ok\n"),0);

                        //Avvisa stato match
                        avvisa_players_stato_match(i,messaggio);
                        return;
                    }
                }else{ //malloc fallita

                    //Avverti il capitano che la richiesta è fallita
                    send(sockCapitano,"rispostaAlCapitano\n", strlen("rispostaAlCapitano\n"),0);
                    send(sockCapitano,"ko\n", strlen("ko\n"),0);

                    printf("Stato richiesta match fallito\n");

                    return;
                }
            }

            if(i>SIZE_ARRAY_PARTITE){ //Fallimento caso array partite pieno

                //Avverti il capitano che la richiesta è fallita
                send(sockCapitano,"rispostaAlCapitano\n", strlen("rispostaAlCapitano\n"),0);
                send(sockCapitano,"ko\n", strlen("ko\n"),0);

                printf("Stato richiesta match fallito\n");

                return;
            }

        }else{ //Setta squadra pronta per avviare un match non appena è pronta un altra squadra

            squadreInCostruzione[indexSquadraCheHaChiestoIlMatch]->isPronto = 1;

            printf("Aggiunta la squadra %s in attesa di match\n",nomesquadra);

            //Avverti il client che la richiesta ha avuto successo
            send(sockCapitano,"rispostaAlCapitano\n", strlen("rispostaAlCapitano\n"),0);
            send(sockCapitano,"ok\n", strlen("ok\n"),0);

            avvisa_players_stato_match(-1,messaggio);
            return;
        }
    }
}
