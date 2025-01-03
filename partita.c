#include "partita.h"
#include "variabiliGlobali.h"
#include "player.h"
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

static int numeroInfortuni = 0;

////////////////////////////////////Funzioni interne////////////////////////////////////////////////////////

int annulla_match(int indexPartita){

printf("Sono nella funzione\n");

    int annullaPartita = 0;

    if( (indexPartita>=0) && (indexPartita<SIZE_ARRAY_PARTITE) ){

        if(partite[indexPartita] != NULL){

            partita *match = partite[indexPartita];

            if(match->squadra_A == NULL || match->squadra_B == NULL){


                annullaPartita = 1;
            }

            if(match->squadra_A->capitano == NULL || match->squadra_B->capitano == NULL){

                annullaPartita = 1;
            }

            for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                if(match->squadra_A->players[i] == NULL || match->squadra_B->players[i] == NULL){

                    annullaPartita = 1;
                }
            }
        }
    }

    if(annullaPartita == 1){
printf("Sono 1\n");
        //Costruisci messaggio
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("annullamentoMatch"));


        const char *json_str = json_object_to_json_string(jobj);
        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

        printf("Annullamento match per disconnessione client\n");

    }

    return annullaPartita;
}

void send_evento_partecipanti_match(char *messaggio, int indexPartita){

    if( (indexPartita >= 0) && (indexPartita < SIZE_ARRAY_PARTITE) ){

        if( partite[indexPartita] != NULL ){

            partita *match = partite[indexPartita];

            int sockCapitanoA = -1;
            int sockCapitanoB = -1;

            if(match->squadra_A->capitano != NULL)
                sockCapitanoA = match->squadra_A->capitano->socket;
            else
                sockCapitanoA = -1;

            if(match->squadra_B->capitano != NULL)
                sockCapitanoB = match->squadra_B->capitano->socket;
            else
                sockCapitanoB = -1;

            if(sockCapitanoA >= 0)
                send(sockCapitanoA,messaggio,strlen(messaggio),0);
            if(sockCapitanoB >= 0)
                send(sockCapitanoB,messaggio,strlen(messaggio),0);

            int socketPlayerA = -1;
            int socketPlayerB = -1;

            for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                if(match->squadra_A->players[i] != NULL)
                    socketPlayerA = match->squadra_A->players[i]->socket;
                else
                    socketPlayerA = -1;

                if(match->squadra_B->players[i] != NULL)
                    socketPlayerB = match->squadra_B->players[i]->socket;
                else
                    socketPlayerB = -1;

                if(socketPlayerA >= 0)
                    send(socketPlayerA,messaggio,strlen(messaggio),0);

                if(socketPlayerB >= 0)
                    send(socketPlayerB,messaggio,strlen(messaggio),0);

            }
        }
    }
}

int get_squadra_from_player(char *player, int indexPartita){

    if(player != NULL){

        if( (indexPartita >= 0) && (indexPartita < SIZE_ARRAY_PARTITE) ){

            if(partite[indexPartita] != NULL){

                partita *match = partite[indexPartita];

                if(strcmp(player,match->squadra_A->capitano->nome_player) == 0) return 0;

                if(strcmp(player,match->squadra_B->capitano->nome_player) == 0) return 1;

                for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                    if(strcmp(player,match->squadra_A->players[i]->nome_player) == 0) return 0;

                    if(strcmp(player,match->squadra_B->players[i]->nome_player) == 0) return 1;
                }
            }
        }
    }

    return -1;
}

int get_index_player(char *player, int indexPartita){

    if(player != NULL){

        if( (indexPartita >= 0)  && (indexPartita < SIZE_ARRAY_PARTITE) ){

            if(partite[indexPartita] != NULL){

                partita *match = partite[indexPartita];

                if(strcmp(player,match->squadra_A->capitano->nome_player) == 0) return 4;

                if(strcmp(player,match->squadra_B->capitano->nome_player) == 0) return 4;

                for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                    if(strcmp(player,match->squadra_A->players[i]->nome_player) == 0) return i;

                    if(strcmp(player,match->squadra_B->players[i]->nome_player) == 0) return i;
                }
            }
        }
    }

    return -1;
}

void *avvia_timer(void *arg){

    int indexPartita = *(int*) arg;

    if( (indexPartita >= 0) && (indexPartita < SIZE_ARRAY_PARTITE) ){

        if(partite[indexPartita] != NULL){

            partita *match = partite[indexPartita];

            json_object *jobj;

            for(int i=0; i<5; i++){

                sleep(60);

                switch(i){

                    case 0:

                        //Costruisci messaggio e invia messaggio ai partecipanti
                        jobj = json_object_new_object();
                        json_object_object_add(jobj, "tipoEvento", json_object_new_string("refreshTime"));
                        json_object_object_add(jobj, "countDown", json_object_new_string("4m"));
                        const char *json_str = json_object_to_json_string(jobj);

                        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                        int sizeMSG = strlen(tmp) + 2;
                        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                        strcpy(messaggioJSON,tmp);
                        strcat(messaggioJSON,"\n");

                        send_evento_partecipanti_match(messaggioJSON,indexPartita);

                        break;

                    case 1:

                        //Costruisci messaggio e invia messaggio ai partecipanti
                        jobj = json_object_new_object();
                        json_object_object_add(jobj, "tipoEvento", json_object_new_string("refreshTime"));
                        json_object_object_add(jobj, "countDown", json_object_new_string("3m"));
                        const char *json_str1 = json_object_to_json_string(jobj);

                        char *tmp1 = strdup(json_str1); // Copia la stringa JSON per restituirla
                        int sizeMSG1 = strlen(tmp1) + 2;
                        char *messaggioJSON1 = malloc(sizeof(char)*sizeMSG1);
                        strcpy(messaggioJSON1,tmp1);
                        strcat(messaggioJSON1,"\n");

                        send_evento_partecipanti_match(messaggioJSON1,indexPartita);

                        break;

                    case 2:

                        //Costruisci messaggio e invia messaggio ai partecipanti
                        jobj = json_object_new_object();
                        json_object_object_add(jobj, "tipoEvento", json_object_new_string("refreshTime"));
                        json_object_object_add(jobj, "countDown", json_object_new_string("2m"));
                        const char *json_str2 = json_object_to_json_string(jobj);

                        char *tmp2 = strdup(json_str2); // Copia la stringa JSON per restituirla
                        int sizeMSG2 = strlen(tmp2) + 2;
                        char *messaggioJSON2 = malloc(sizeof(char)*sizeMSG2);
                        strcpy(messaggioJSON2,tmp2);
                        strcat(messaggioJSON2,"\n");

                        send_evento_partecipanti_match(messaggioJSON2,indexPartita);

                        break;

                    case 3:

                        //Costruisci messaggio e invia messaggio ai partecipanti
                        jobj = json_object_new_object();
                        json_object_object_add(jobj, "tipoEvento", json_object_new_string("refreshTime"));
                        json_object_object_add(jobj, "countDown", json_object_new_string("1m"));
                        const char *json_str3 = json_object_to_json_string(jobj);

                        char *tmp3 = strdup(json_str3); // Copia la stringa JSON per restituirla
                        int sizeMSG3 = strlen(tmp3) + 2;
                        char *messaggioJSON3 = malloc(sizeof(char)*sizeMSG3);
                        strcpy(messaggioJSON3,tmp3);
                        strcat(messaggioJSON3,"\n");

                        send_evento_partecipanti_match(messaggioJSON3,indexPartita);

                        break;
                }
            }

            match->finePartita = 1;
        }
    }
}

int get_evento(){

    int evento = -1;

    while(evento == -1){

        //Numero tra 0 e 99
        int random_number = rand() % 100; // Numero tra 0 e 99

        //Simula una probabilità del 40% per l'evento tiro
        if(random_number < 40) evento = 0;

        //Simula una probabilità del 50% per l'evento dribbling
        else if(random_number < 90) evento = 1;

        //Simula una probabilità del 10% per l'evento infortunio
        else if (numeroInfortuni <= 3 ) evento = 2;
    }

    return evento;

}

int esito_tiro(){

    int esito;

    //Numero tra 0 e 99
    int random_number = rand() % 100; // Numero tra 0 e 99

    //Simula una probabilità del 40% di fare goal
    if(random_number < 40) esito = 1;

    //Simula una probabilità del 60% per tiro fallito
    else esito = 0;

    return esito;
}

void tira(char *player, int indexPartita, int *scoreA, int *scoreB){

    printf("%s tenta il tiro\n", player);

    partita *match = partite[indexPartita];
    int turnoSquadra = get_squadra_from_player(player,indexPartita);

    if(esito_tiro() == 0){

        printf("Tiro di %s fallito\n",player);

        //Costruisci messaggio
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("tiro"));
        json_object_object_add(jobj, "esitoTiro", json_object_new_string("fallito"));
        json_object_object_add(jobj, "turnoPlayer", json_object_new_string(player));
        json_object_object_add(jobj, "turnoSquadra", json_object_new_int(turnoSquadra));


        const char *json_str = json_object_to_json_string(jobj);
        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

    }else{

        printf("Goal di %s!!\n",player);

        //Costruisci messaggio
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("tiro"));
        json_object_object_add(jobj, "esitoTiro", json_object_new_string("goal"));
        json_object_object_add(jobj, "turnoPlayer", json_object_new_string(player));
        json_object_object_add(jobj, "turnoSquadra", json_object_new_int(turnoSquadra));


        const char *json_str = json_object_to_json_string(jobj);
        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

        if(turnoSquadra == 0){

            printf("punto per la squadra %s\n",match->squadra_A->nome_squadra);
            (*scoreA)++;

        }else{

            printf("punto per la squadra %s\n",match->squadra_B->nome_squadra);
            (*scoreB)++;
        }

        printf("nuovo risultato: %d-%d\n",*scoreA,*scoreB);
        printf("\n");
    }
}

int esito_dribbling(){

    int esito;

    //Numero tra 0 e 99
    int random_number = rand() % 100; // Numero tra 0 e 99

    //Simula una probabilità del 40% di successo
    if(random_number < 40) esito = 1;

    //Simula una probabilità del 60% per dribbling fallito
    else esito = 0;

    return esito;
}

void dribbling(char *player,int indexPartita, int *scoreA, int *scoreB){

    printf("%s tenta il dribbling\n", player);

    int esito = esito_dribbling();

    if(esito == 0){

        printf("dribbling fallito, il possesso palla passa all'avversario");
        printf("\n");

        //Costruisci messaggio
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("dribbling"));
        json_object_object_add(jobj, "esitoDribbling", json_object_new_string("fallito"));
        json_object_object_add(jobj, "turnoPlayer", json_object_new_string(player));

        const char *json_str = json_object_to_json_string(jobj);
        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

    }else{

        printf("dribbling fantastisco,ha spazio per un tiro\n");

        //Costruisci messaggio
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("dribbling"));
        json_object_object_add(jobj, "esitoDribbling", json_object_new_string("ok"));
        json_object_object_add(jobj, "turnoPlayer", json_object_new_string(player));

        const char *json_str = json_object_to_json_string(jobj);
        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

        tira(player,indexPartita, scoreA, scoreB);
    }
}

void *penalizzazione(void *infoThread){

    //Estrazione parametri thread
    argomentiThreadPenalizzazione *arg = (argomentiThreadPenalizzazione*) infoThread;
    int indexPartita = arg->indexPartita;
    int time = arg->timeP;
    char *playerString = arg->player;
    partita *match = partite[indexPartita];
    player *playerP = NULL;

    sleep(time * 60);

    //Cerca player
    if(strcmp(playerString,match->squadra_A->capitano->nome_player) == 0){

        playerP = match->squadra_A->capitano;

        //Rendi player disponibile
        pthread_mutex_lock(&mutexPartite);
        playerP->penalizzato = 0;
        printf("Il player %s ha scontato la penalizzazione\n",playerP->nome_player);
        pthread_mutex_unlock(&mutexPartite);

        //Costruisci messaggio e invia messaggio ai partecipanti
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("ritornoPenalizzazione"));
        json_object_object_add(jobj, "playerRitornato", json_object_new_string(playerString));
        const char *json_str = json_object_to_json_string(jobj);

        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

        pthread_exit((void*)1);

    }

     if(strcmp(playerString,match->squadra_B->capitano->nome_player) == 0){

        playerP = match->squadra_B->capitano;

        //Rendi player disponibile
        pthread_mutex_lock(&mutexPartite);
        playerP->penalizzato = 0;
        pthread_mutex_unlock(&mutexPartite);
        printf("Il player %s ha scontato la penalizzazione\n",playerP->nome_player);

        //Invia messaggio ritorno player dalla penalizzazione
        //Costruisci messaggio e invia messaggio ai partecipanti
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "tipoEvento", json_object_new_string("ritornoPenalizzazione"));
        json_object_object_add(jobj, "playerRitornato", json_object_new_string(playerString));
        const char *json_str = json_object_to_json_string(jobj);

        char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
        int sizeMSG = strlen(tmp) + 2;
        char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
        strcpy(messaggioJSON,tmp);
        strcat(messaggioJSON,"\n");

        send_evento_partecipanti_match(messaggioJSON,indexPartita);

        pthread_exit((void*)2);

    }

    for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

        if(strcmp(playerString,match->squadra_A->players[i]->nome_player) == 0){

            playerP = match->squadra_A->players[i];

            //Rendi player disponibile
            pthread_mutex_lock(&mutexPartite);
            playerP->penalizzato = 0;
            pthread_mutex_unlock(&mutexPartite);
            printf("Il player %s ha scontato la penalizzazione\n",playerP->nome_player);

            //Invia messaggio ritorno player dalla penalizzazione

            //Costruisci messaggio e invia messaggio ai partecipanti
            json_object *jobj = json_object_new_object();
            json_object_object_add(jobj, "tipoEvento", json_object_new_string("ritornoPenalizzazione"));
            json_object_object_add(jobj, "playerRitornato", json_object_new_string(playerString));
            const char *json_str = json_object_to_json_string(jobj);

            char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
            int sizeMSG = strlen(tmp) + 2;
            char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
            strcpy(messaggioJSON,tmp);
            strcat(messaggioJSON,"\n");

            send_evento_partecipanti_match(messaggioJSON,indexPartita);

            pthread_exit((void*)3);

        }

         if(strcmp(playerString,match->squadra_B->players[i]->nome_player) == 0){

            playerP = match->squadra_B->players[i];

            //Rendi player disponibile
            pthread_mutex_lock(&mutexPartite);
            playerP->penalizzato = 0;
            pthread_mutex_unlock(&mutexPartite);
            printf("Il player %s ha scontato la penalizzazione\n",playerP->nome_player);

            //Invia messaggio ritorno player dalla penalizzazione
            //Costruisci messaggio e invia messaggio ai partecipanti
            json_object *jobj = json_object_new_object();
            json_object_object_add(jobj, "tipoEvento", json_object_new_string("ritornoPenalizzazione"));
            json_object_object_add(jobj, "playerRitornato", json_object_new_string(playerString));
            const char *json_str = json_object_to_json_string(jobj);

            char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
            int sizeMSG = strlen(tmp) + 2;
            char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
            strcpy(messaggioJSON,tmp);
            strcat(messaggioJSON,"\n");

            send_evento_partecipanti_match(messaggioJSON,indexPartita);

            pthread_exit((void*)4);

        }
    }

}

void *infortunio(void *infoThread){

    numeroInfortuni++;

    //Esrazione parametri thread
    argomentiThreadInfortunio *arg = (argomentiThreadInfortunio*) infoThread;
    int indexPartita = arg->indexPartita;
    char *playerInfortunatoString = arg->player_name;

    partita *match = partite[indexPartita];
    player *playerInfortunato = NULL;

    printf("Thread infotunio creato: player infortunato %s\n",playerInfortunatoString);

    //Cerca player infortunato
    char *capitanoA = match->squadra_A->capitano->nome_player;
    char *capitanoB = match->squadra_B->capitano->nome_player;

    if(strcmp(capitanoA,playerInfortunatoString) == 0)
        playerInfortunato = match->squadra_A->capitano;

    if(playerInfortunato == NULL){

        if(strcmp(capitanoB,playerInfortunatoString) == 0)
            playerInfortunato = match->squadra_B->capitano;
    }

    if(playerInfortunato == NULL){

        for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

            char *playerA = match->squadra_A->players[i]->nome_player;
            char *playerB = match->squadra_B->players[i]->nome_player;

            if(strcmp(playerA,playerInfortunatoString) == 0){

                playerInfortunato = match->squadra_A->players[i];
                break;
            }

            if(strcmp(playerB,playerInfortunatoString) == 0){

                playerInfortunato = match->squadra_B->players[i];
                break;
            }
        }
    }

    int random_number = rand() % 4; // Numero tra 0 e 4 che rappresenzano i minuti di infortunio
    random_number++;
    int secondiInfortunio = random_number*60; //converti in secondi

    printf("Minuti infortunio di %s: %d minuti\n",playerInfortunatoString,random_number);

    //Ottiene la squadra del giocatore infortunato per penalizzare la squadra avversaria
    int squadra = get_squadra_from_player(playerInfortunatoString,indexPartita);
    //Variabili per penalizzazione
    int random_player;
    int random_time_p = (rand() % 4) + 1;
    player *playerPenalizzato = NULL;
    int trovato_player_da_penalizzare = 0;
    pthread_t thread;

    pthread_mutex_lock(&mutexPartite);

    while(trovato_player_da_penalizzare != 1){

        random_player = rand() % 5;

        if(squadra == 0){ //Penalizza squadra B

            if(random_player == 4){ //penalizza il capitano della squadra 1 (B)

                playerPenalizzato = match->squadra_B->capitano;

                if(playerPenalizzato->infortunato != 1 && playerPenalizzato->penalizzato != 1){

                    playerPenalizzato->penalizzato = 1;

                    printf("Fallo commesso da %s penalizzato per %d min\n",playerPenalizzato->nome_player,random_time_p);

                    //Crea thread penalizzazione
                    argomentiThreadPenalizzazione *infoThreadPenalizzazione = malloc(sizeof(argomentiThreadPenalizzazione));
                    strcpy(infoThreadPenalizzazione->player,playerPenalizzato->nome_player);
                    infoThreadPenalizzazione->indexPartita = indexPartita;
                    infoThreadPenalizzazione->timeP = random_time_p;
                    if (pthread_create(&thread, NULL, penalizzazione, (void*)infoThreadPenalizzazione) != 0) {

                        printf("Errore nella creazione del thread infortunio\n");
                    }

                    trovato_player_da_penalizzare = 1;
                }

            }else{ //penalizza un player della squadra B

                playerPenalizzato = match->squadra_B->players[random_player];

                if(playerPenalizzato->infortunato != 1 && playerPenalizzato->penalizzato != 1){

                    playerPenalizzato->penalizzato = 1;
                    printf("Fallo commesso da %s penalizzato per %d min\n",playerPenalizzato->nome_player,random_time_p);

                    //Crea thread penalizzazione
                    argomentiThreadPenalizzazione *infoThreadPenalizzazione = malloc(sizeof(argomentiThreadPenalizzazione));
                    strcpy(infoThreadPenalizzazione->player,playerPenalizzato->nome_player);
                    infoThreadPenalizzazione->indexPartita = indexPartita;
                    infoThreadPenalizzazione->timeP = random_time_p;
                    if (pthread_create(&thread, NULL, penalizzazione, (void*)infoThreadPenalizzazione) != 0) {

                        printf(stderr, "Errore nella creazione del thread infortunio\n");
                    }

                    trovato_player_da_penalizzare = 1;
                }
            }

        }else{ //Penaliza squadra A

            if(random_player == 4){ //Penalliza il capitano della squadra 1 (A)

                playerPenalizzato = match->squadra_A->capitano;

                if(playerPenalizzato->infortunato != 1 && playerPenalizzato->penalizzato != 1){

                    playerPenalizzato->penalizzato = 1;
                    printf("Fallo commesso da %s penalizzato per %d min\n",playerPenalizzato->nome_player,random_time_p);

                    //Crea thread penalizzazione
                    argomentiThreadPenalizzazione *infoThreadPenalizzazione = malloc(sizeof(argomentiThreadPenalizzazione));
                    strcpy(infoThreadPenalizzazione->player,playerPenalizzato->nome_player);
                    infoThreadPenalizzazione->indexPartita = indexPartita;
                    infoThreadPenalizzazione->timeP = random_time_p;
                    if (pthread_create(&thread, NULL, penalizzazione, (void*)infoThreadPenalizzazione) != 0) {

                        printf(stderr, "Errore nella creazione del thread infortunio\n");
                    }

                    trovato_player_da_penalizzare = 1;
                }

            }else{ //penalizza un player della squadra A

                playerPenalizzato = match->squadra_A->players[random_player];

                if(playerPenalizzato->infortunato != 1 && playerPenalizzato->penalizzato != 1){

                    playerPenalizzato->penalizzato = 1;
                    printf("Fallo commesso da %s penalizzato per %d min\n",playerPenalizzato->nome_player,random_time_p);

                    //Crea thread penalizzazione
                    argomentiThreadPenalizzazione *infoThreadPenalizzazione = malloc(sizeof(argomentiThreadPenalizzazione));
                    strcpy(infoThreadPenalizzazione->player,playerPenalizzato->nome_player);
                    infoThreadPenalizzazione->indexPartita = indexPartita;
                    infoThreadPenalizzazione->timeP = random_time_p;
                    if (pthread_create(&thread, NULL, penalizzazione, (void*)infoThreadPenalizzazione) != 0) {

                        printf(stderr, "Errore nella creazione del thread infortunio\n");
                    }

                    trovato_player_da_penalizzare = 1;
                }
            }
        }
    }

    //Rendi indisponibile il player
    playerInfortunato->infortunato = 1;

    pthread_mutex_unlock(&mutexPartite);

    //Costruisci messaggio
    json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "tipoEvento", json_object_new_string("infortunio"));
    json_object_object_add(jobj, "turnoPlayer", json_object_new_string(playerInfortunatoString));
    json_object_object_add(jobj, "minuti", json_object_new_int(random_number));
    json_object_object_add(jobj, "playerPenalizzato", json_object_new_string(playerPenalizzato->nome_player));
    json_object_object_add(jobj, "minutiP", json_object_new_int(random_time_p));

    const char *json_str = json_object_to_json_string(jobj);
    char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
    int sizeMSG = strlen(tmp) + 2;
    char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
    strcpy(messaggioJSON,tmp);
    strcat(messaggioJSON,"\n");

    send_evento_partecipanti_match(messaggioJSON,indexPartita);

    sleep(secondiInfortunio);

    //Ritorno del player
    pthread_mutex_lock(&mutexPartite);
    playerInfortunato->infortunato = 0;
    pthread_mutex_unlock(&mutexPartite);

    printf("%s è ritornato dall'infortunio\n",playerInfortunatoString);

    //Costruisci messaggio
    json_object *jobj2 = json_object_new_object();
    json_object_object_add(jobj2, "tipoEvento", json_object_new_string("ritornoInfortunio"));
    json_object_object_add(jobj2, "turnoPlayer", json_object_new_string(playerInfortunatoString));

    const char *json_str2 = json_object_to_json_string(jobj2);
    char *tmp2 = strdup(json_str2); // Copia la stringa JSON per restituirla
    int sizeMSG2 = strlen(tmp2) + 2;
    char *messaggioJSON2 = malloc(sizeof(char)*sizeMSG2);
    strcpy(messaggioJSON2,tmp2);
    strcat(messaggioJSON2,"\n");
    json_object_put(jobj2); // Dealloca l'oggetto JSON

    send_evento_partecipanti_match(messaggioJSON2,indexPartita);

    void *status;
    pthread_join(thread,&status);

    printf("Thread penalizzazione terminato con stato: %ld\n",(long)status);

    numeroInfortuni--;
}

char *assegna_turno(int turnoSquadraAttuale, int indexPartita, int *indiceTurnoA,int *indiceTurnoB){

    partita *match = partite[indexPartita];

    //-----------Assegnare il turno alla squadra B poiché il turno attuale è della squadra A---------------------

    if(turnoSquadraAttuale == 0){

        (*indiceTurnoB)++;
        int indiceTurnoModulato = (*indiceTurnoB)%5;
        char *turnoPlayer;
        player *playerInfo;

        while(1){

            //Assegna turno al capitano se non è infortunato
            if(indiceTurnoModulato == 4){

                playerInfo = match->squadra_B->capitano;
                if(playerInfo->infortunato != 1 && playerInfo->penalizzato != 1){

                    turnoPlayer = playerInfo->nome_player;

                    //Costruisci messaggio e invia messaggio ai partecipanti
                    json_object *jobj = json_object_new_object();
                    json_object_object_add(jobj, "tipoEvento", json_object_new_string("assegnazioneTurno"));
                    json_object_object_add(jobj, "turnoPlayer", json_object_new_string(turnoPlayer));
                    const char *json_str = json_object_to_json_string(jobj);
                    char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                    int sizeMSG = strlen(tmp) + 2;
                    char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                    strcpy(messaggioJSON,tmp);
                    strcat(messaggioJSON,"\n");

                    send_evento_partecipanti_match(messaggioJSON,indexPartita);

                    return turnoPlayer;
                }

            }else {

                playerInfo = match->squadra_B->players[indiceTurnoModulato];

                if(playerInfo->infortunato != 1 && playerInfo->penalizzato != 1){

                    turnoPlayer = playerInfo->nome_player;

                    //Costruisci messaggio e invia messaggio ai partecipanti
                    json_object *jobj = json_object_new_object();
                    json_object_object_add(jobj, "tipoEvento", json_object_new_string("assegnazioneTurno"));
                    json_object_object_add(jobj, "turnoPlayer", json_object_new_string(turnoPlayer));
                    const char *json_str = json_object_to_json_string(jobj);
                    char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                    int sizeMSG = strlen(tmp) + 2;
                    char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                    strcpy(messaggioJSON,tmp);
                    strcat(messaggioJSON,"\n");

                    send_evento_partecipanti_match(messaggioJSON,indexPartita);

                    return turnoPlayer;
                }
            }

            //Se arriva qui non è stato assegnato il turno quindi fa una nuova iterazione con il turno successivo
            (*indiceTurnoB)++;
            indiceTurnoModulato = (*indiceTurnoB)%5;
        }

    }else{ //ASSENAZIONE TURNO PLYER SQUADRA A

        (*indiceTurnoA)++;
        int indiceTurnoModulato = (*indiceTurnoA)%5;
        char *turnoPlayer;
        player *playerInfo;

        while(1){

            //Assegna turno al capitano se non è infortunato
            if(indiceTurnoModulato == 4){

                playerInfo = match->squadra_A->capitano;

                if(playerInfo->infortunato != 1 && playerInfo->penalizzato != 1){

                    turnoPlayer = playerInfo->nome_player;

                    //Costruisci messaggio e invia messaggio ai partecipanti
                    json_object *jobj = json_object_new_object();
                    json_object_object_add(jobj, "tipoEvento", json_object_new_string("assegnazioneTurno"));
                    json_object_object_add(jobj, "turnoPlayer", json_object_new_string(turnoPlayer));
                    const char *json_str = json_object_to_json_string(jobj);
                    char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                    int sizeMSG = strlen(tmp) + 2;
                    char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                    strcpy(messaggioJSON,tmp);
                    strcat(messaggioJSON,"\n");

                    send_evento_partecipanti_match(messaggioJSON,indexPartita);

                    return turnoPlayer;
                }

            }else{

                playerInfo = match->squadra_A->players[indiceTurnoModulato];

                if(playerInfo->infortunato != 1 && playerInfo->penalizzato != 1){

                    turnoPlayer = playerInfo->nome_player;

                    //Costruisci messaggio e invia messaggio ai partecipanti
                    json_object *jobj = json_object_new_object();
                    json_object_object_add(jobj, "tipoEvento", json_object_new_string("assegnazioneTurno"));
                    json_object_object_add(jobj, "turnoPlayer", json_object_new_string(turnoPlayer));
                    const char *json_str = json_object_to_json_string(jobj);

                    char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                    int sizeMSG = strlen(tmp) + 2;
                    char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                    strcpy(messaggioJSON,tmp);
                    strcat(messaggioJSON,"\n");

                    send_evento_partecipanti_match(messaggioJSON,indexPartita);

                    return turnoPlayer;
                }
            }

             //Se arriva qui non è stato assegnato il turno quindi fa una nuova iterazione con il turno successivo
            (*indiceTurnoA)++;
            indiceTurnoModulato = (*indiceTurnoA)%5;
        }
    }
}

void simula_match(int indexPartita){

    if( (indexPartita >= 0) && (indexPartita < SIZE_ARRAY_PARTITE) ){

        if(partite[indexPartita] != NULL){

            partita *match = partite[indexPartita];

            //-----------------------------Cerca il player che inizia il turno-----------------------------------------
            int playerTrovato = 0;
            char *playerInizioTurnoString = match->inizioTurno;
            player *playerInizioTurno = NULL;

            if(strcmp(playerInizioTurnoString,match->squadra_A->capitano->nome_player) == 0){

                playerInizioTurno = match->squadra_A->capitano;
                playerTrovato = 1;
            }

            if(playerTrovato != 1){

                if(strcmp(playerInizioTurnoString,match->squadra_B->capitano->nome_player) == 0){

                    playerInizioTurno = match->squadra_B->capitano;
                    playerTrovato = 1;
                }

            }

            if(playerTrovato != 1){

                for(int i=0; i<SIZE_ARRAY_PLAYER_PARTECIPANTI; i++){

                    if(strcmp(playerInizioTurnoString,match->squadra_A->players[i]->nome_player) == 0){

                        playerInizioTurno = match->squadra_A->players[i];
                        playerTrovato = 1;
                        break;
                    }

                    if(strcmp(playerInizioTurnoString,match->squadra_B->players[i]->nome_player) == 0){

                        playerInizioTurno = match->squadra_B->players[i];
                        playerTrovato = 1;
                        break;
                    }
                }
            }

            if(playerInizioTurno == NULL){

                printf("Errore simulazione match: il player %s che ha iniziato il turno non trovato\n",playerInizioTurnoString);

                return;
            }

            //--------------------------------Inizia simulazione eventi---------------------------------------------

            //Variabili Match
            int fineMatch = 0;
            int scoreA = 0;
            int scoreB = 0;
            int evento;
            int turnoSquadra = get_squadra_from_player(playerInizioTurnoString,indexPartita);
            int indiceTurnoA;
            int indiceTurnoB;

            if(turnoSquadra < 0){

                printf("Errore durante la simulazione del match: turno squadra negativo\n");
                return;
            }

            if(turnoSquadra == 0){

                indiceTurnoB = 4;
                indiceTurnoA = get_index_player(playerInizioTurnoString,indexPartita);

            }else{

                indiceTurnoA = 4;
                indiceTurnoB = get_index_player(playerInizioTurnoString,indexPartita);
            }

            //Gestione timer
            pthread_t newThread;
            int *indexPartitaThreadTime = malloc(sizeof(int));
            if(indexPartitaThreadTime == NULL){

                printf("Errore durante la simulazione del match: allazione di memoria indexPartitaThread fallita\n");
                return;
            }
            *indexPartitaThreadTime = indexPartita;
            //Avvia timer
            if (pthread_create(&newThread, NULL, avvia_timer, (void*)indexPartitaThreadTime) != 0) {

                printf(stderr, "Errore durante la simulazione del match: creazione thread time fallito\n");

                return;
            }

            printf("Match iniziato, 5 minuti per la fine\n");
            printf("Possesso palla di %s\n", playerInizioTurnoString);

            evento = get_evento();

            if(evento == 0){

                tira(playerInizioTurnoString,indexPartita,&scoreA,&scoreB);

            }else if(evento == 1){

                dribbling(playerInizioTurnoString,indexPartita,&scoreA,&scoreB);

            }else{

                pthread_t newThread;
                argomentiThreadInfortunio *infoThread = malloc(sizeof(argomentiThreadInfortunio));
                if(infoThread == NULL){

                    printf("Errore durante la simulazione del match: allocazione memoria argomentithreadInfortunio fallita\n");
                    return;
                }
                strcpy(infoThread->player_name,playerInizioTurnoString);
                infoThread->indexPartita = indexPartita;

                // Crea il thread e passa il puntatore alla struct come argomento
                if (pthread_create(&newThread, NULL, infortunio, (void*)infoThread) != 0) {

                    printf(stderr, "Errore durante la simulazione match: creazione thread infortunio fallito\n");
                    return;
                }
            }

            while(match->finePartita != 1){

                pthread_mutex_lock(&mutexPartite);
                char *turnoPlayer = assegna_turno(turnoSquadra,indexPartita,&indiceTurnoA,&indiceTurnoB);
                pthread_mutex_unlock(&mutexPartite);

                if(turnoSquadra == 0) turnoSquadra = 1;
                else turnoSquadra = 0;

                printf("%s ottiene il possesso palla\n",turnoPlayer);

                evento = get_evento();

                if(evento == 0){

                    tira(turnoPlayer,indexPartita,&scoreA,&scoreB);

                }else if(evento == 1){

                    dribbling(turnoPlayer,indexPartita,&scoreA,&scoreB);

                }else{

                    pthread_t thread;
                    argomentiThreadInfortunio *infoThread = malloc(sizeof(argomentiThreadInfortunio));
                    strcpy(infoThread->player_name,turnoPlayer);
                    infoThread->indexPartita = indexPartita;

                    // Crea il thread e passa il puntatore alla struct come argomento
                    if (pthread_create(&thread, NULL, infortunio, (void*)infoThread) != 0) {

                        printf(stderr, "Errore nella creazione del thread infortunio\n");
                    }
                }

                sleep(10);
            }

             printf("\n tempo scaduto, match concluso\n");

             //Avvertiti i players della fine del match
            //Costruisci messaggio e invia messaggio ai partecipanti
            json_object *jobj = json_object_new_object();
            json_object_object_add(jobj, "tipoEvento", json_object_new_string("fineMatch"));
            const char *json_str = json_object_to_json_string(jobj);

            char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
            int sizeMSG = strlen(tmp) + 2;
            char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
            strcpy(messaggioJSON,tmp);
            strcat(messaggioJSON,"\n");

            send_evento_partecipanti_match(messaggioJSON,indexPartita);
        }
    }
}

////////////////////////////////Funzioni header//////////////////////////////////////////////////////////

int get_index_partita(char *messaggio){

    //Deserializzazione del messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *indexPartitaJSON;

    json_object_object_get_ex(parsed_json, "indexPartita", &indexPartitaJSON);

    int indexPartita = json_object_get_int(indexPartitaJSON);

    return indexPartita;
}

void assegna_turno_iniziale_e_avvia_match(char *messaggio){

    //Deserializzazione del messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    json_object *indexPartitaJSON;
    json_object *playerInizioTurnoJSON;
    json_object *squadraInizioTurnoJSON;

    json_object_object_get_ex(parsed_json, "indexPartita", &indexPartitaJSON);
    json_object_object_get_ex(parsed_json, "player", &playerInizioTurnoJSON);
    json_object_object_get_ex(parsed_json, "squadra", &squadraInizioTurnoJSON);

    int indexPartita = json_object_get_int(indexPartitaJSON);
    char *playerInizioTurno = json_object_get_string(playerInizioTurnoJSON);
    char *squadraInizioTurno = json_object_get_string(squadraInizioTurnoJSON);

    if(annulla_match(indexPartita) == 1){

        return;
    }

    if( (indexPartita >= 0) && (indexPartita < SIZE_ARRAY_PARTITE)){

        if(partite[indexPartita] != NULL){

            if( (playerInizioTurno != NULL) && (squadraInizioTurno != NULL) ){

                //Assegna inizio turno al player e avvia la simulazione
                partita *nuovaPartita = partite[indexPartita];
                strcpy(nuovaPartita->inizioTurno,playerInizioTurno);

                //Costruisci messaggio e invia messaggio ai partecipanti
                json_object *jobj = json_object_new_object();
                json_object_object_add(jobj, "tipoEvento", json_object_new_string("inizioMatch"));
                json_object_object_add(jobj, "playerInizioTurno", json_object_new_string(playerInizioTurno));
                const char *json_str = json_object_to_json_string(jobj);

                char *tmp = strdup(json_str); // Copia la stringa JSON per restituirla
                int sizeMSG = strlen(tmp) + 2;
                char *messaggioJSON = malloc(sizeof(char)*sizeMSG);
                strcpy(messaggioJSON,tmp);
                strcat(messaggioJSON,"\n");

                send_evento_partecipanti_match(messaggioJSON,indexPartita);
                printf("Avvertiti i player delle squadre %s e %s dell'inizio del match: ",nuovaPartita->squadra_A->nome_squadra,nuovaPartita->squadra_B->nome_squadra);
                printf("il primo turno è di %s della squadra %s\n",playerInizioTurno,squadraInizioTurno);

                simula_match(indexPartita);
            }
        }
    }
}
