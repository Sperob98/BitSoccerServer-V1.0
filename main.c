#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "squadra.h"
#include "player.h"
#include "partita.h"
#include "variabiliGlobali.h"
#include "gestioneConnessioni.h"

//Inizializzazione variabili globali
pthread_mutex_t mutexListaSquadre = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condListaSquadre = PTHREAD_COND_INITIALIZER;
squadra *squadreInCostruzione[SIZE_ARRAY_TEAMS]; //Array di tutte le squadre fondate in attesa di essere completate e partipare a un match

player *playersConnessi[SIZE_ARRAY_PLAYERS];//Array di tutti gli utenti connnessi
pthread_mutex_t mutexPlayers = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPlayers = PTHREAD_COND_INITIALIZER;

partita *partite[SIZE_ARRAY_PARTITE];//Array delle partite in corso
pthread_mutex_t mutexPartite = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPartite = PTHREAD_COND_INITIALIZER;


void *gestione_richieste_client(void *arg){

    //Recupero del client_sock passato come argomento alla creazione del thread
    int client_sock = *(int *)arg;

    //Buff per i messaggi recevuti dal client
    char client_message[8192];

    //pid del processo padre, ce lo salviamo perché il processo padre deve essere l'unico a gestire le richieste
    pid_t processoPadre = getpid();

    while(1){

        //Punto di inizio dove vengono ricevuti i messaggi dal client
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) { //Gestione disconessione client

                if(getpid()==processoPadre){

                    pthread_mutex_lock(&mutexPlayers);
                    pthread_mutex_lock(&mutexListaSquadre);

                    printf("Ricenzione di un messaggio di un client fallito, inizio operazione disconessione player relativo al client socket\n");

                    gestione_disconessione_client(client_sock);

                    pthread_mutex_unlock(&mutexPlayers);
                    pthread_mutex_unlock(&mutexListaSquadre);

                    pthread_cond_broadcast(&condListaSquadre);

                    pthread_exit(NULL);
                }
            }

        //Decodifica del tipo di richiesta del client
        char *tipoRIchiesta = get_tipo_richiesta(client_message);

        if(tipoRIchiesta != NULL && getpid()==processoPadre){

            if(strcmp(tipoRIchiesta,"nuovoUtente")==0){

                printf("Richiesta di aggiunta di nuovo player\n");

                pthread_mutex_lock(&mutexPlayers);

                aggiungi_utente_connesso(client_message,client_sock);

                pthread_mutex_unlock(&mutexPlayers);

            }else if(strcmp(tipoRIchiesta,"newSquadra")==0){

                printf("Richiesta creazione nuova squadra\n");

                //Mutua escusione
                pthread_mutex_lock(&mutexPlayers);
                pthread_mutex_lock(&mutexListaSquadre);

                int stato = aggiungi_nuova_squadra(client_message, client_sock);

                //Rilascia mutex dei players e delle squadre
                pthread_mutex_unlock(&mutexPlayers);
                pthread_mutex_unlock(&mutexListaSquadre);

                //Sveglia il thread per l'aggiornamento delle nuove squadre
                if(stato == 1){

                    pthread_cond_broadcast(&condListaSquadre);
                    printf("Avvertiti i client della nuova squadra fondata\n");

                }

            }else if(strcmp(tipoRIchiesta,"getSquadreInCostruzione")==0){

                printf("Richiesta dell' array delle squadre fondate\n");

                //Parametro per la creazione del nuvo thread
                int *sock_client_arg = malloc(sizeof(int));
                if(sock_client_arg != NULL){

                    *sock_client_arg = client_sock;
                    pthread_t newThread;
                    if(pthread_create(&newThread,NULL,thread_send_lista_squadre_client,(void*)sock_client_arg) < 0){

                        printf("Errore creazione thread_send_lista_squadre_client\n");
                    }
                }
            }else if(strcmp(tipoRIchiesta,"partecipazioneSquadra")==0){

                printf("Richiesta di parcetipazione a una squadra\n");

                //Acquisizione mutex
                pthread_mutex_lock(&mutexListaSquadre);
                pthread_mutex_lock(&mutexPlayers);

                aggiungi_richiesta_partecipazione_squadra(client_message,client_sock);

                //Rilascio mutex
                pthread_mutex_unlock(&mutexListaSquadre);
                pthread_mutex_unlock(&mutexPlayers);

            }else if(strcmp(tipoRIchiesta,"decisioneCapitano")==0){

                printf("Richiesta decisione capitano\n");

                //Acquisizione mutex
                pthread_mutex_lock(&mutexListaSquadre);
                pthread_mutex_lock(&mutexPlayers);

                aggiornamento_composizione_squadra(client_message);

                //Rilascio mutex
                pthread_mutex_unlock(&mutexListaSquadre);
                pthread_mutex_unlock(&mutexPlayers);

                //Sveglia il thread aggiornamenti squadre per aggiornare il numero di partecipanti
                pthread_cond_broadcast(&condListaSquadre);
                printf("Avvertiti i client di eventuali aggiornamento del numero di partecianti\n");

            }else if(strcmp(tipoRIchiesta,"cercaMatch")==0){

                printf("Richiesta cerca match\n");

                //Acquisizione mutex
                pthread_mutex_lock(&mutexListaSquadre);
                pthread_mutex_lock(&mutexPlayers);
                pthread_mutex_lock(&mutexPartite);

                //Cerca squadra avversaria e avvisa l client della ricerca match
                cerca_squadra_match(client_message,client_sock);

                //Rilascio mutex
                pthread_mutex_unlock(&mutexListaSquadre);
                pthread_mutex_unlock(&mutexPlayers);
                pthread_mutex_unlock(&mutexPartite);

                pthread_cond_broadcast(&condListaSquadre);

            }else if(strcmp(tipoRIchiesta,"inizioTurno")==0){

                printf("Richiesta di inizio turno di un match\n");

                int indexPartita = get_index_partita(client_message);

                if(indexPartita >= 0 && indexPartita < SIZE_ARRAY_PARTITE){

                    if( partite[indexPartita] != NULL){

                        pthread_mutex_lock(&mutexPartite);

                        partita *match = partite[indexPartita];

                        //Avvia il processo di nuova partita una sola volta
                        if(match->inizioPartita != 1){

                            match->inizioPartita = 1;

                            pthread_mutex_unlock(&mutexPartite);

                            pid_t pid = fork();

                            if(pid < 0){

                                printf("Errore creazione processo partita\n");

                            }else if(pid == 0){

                                printf("Creato un nuovo processo per una nuova partita\n");

                                //Inizializza il seed con l'ora corrente
                                srand(time(NULL));

                                assegna_turno_iniziale_e_avvia_match(client_message);

                                exit(0);

                            }else{

                                /*int status;
                                waitpid(pid,&status,0);

                                if(WIFEXITED(status)){

                                    int exit_status = WEXITSTATUS(status);
                                    printf("Terminano con %d\n",exit_status);

                                }else if(WIFSIGNALED(status)) {

                                    int term = WTERMSIG(status);
                                    printf("Terminato a causa del segnale: %d\n",term);

                                }*/

                                free(match);
                                partite[indexPartita] = NULL;
                            }

                        }else{

                            pthread_mutex_unlock(&mutexPartite);
                        }
                    }
                }
            }
        }
    }
}

void avvia_server_socket(int porta){

    //Dichiarazione variabili socket
    int server_sock, client_sock; //descrittori socket
    struct sockaddr_in infoServer, infoClient; //Informazioni server e client

    //Creazione socket del server
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Impossibile creare socket");
        exit(EXIT_FAILURE);
    }

    //Inizializzazione info server
    infoServer.sin_family = AF_INET;
    infoServer.sin_addr.s_addr = INADDR_ANY;
    infoServer.sin_port = htons(porta);

    //Bind del server alla porta
    if (bind(server_sock, (struct sockaddr*)&infoServer, sizeof(infoServer)) < 0) {
        perror("Bind fallito");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server BitSoccerSimulator avviato..\n");

    //Ascolto delle richieste
    if(listen(server_sock,10) < 0){

        perror("listen fallito");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    int sizeStrutturaSockaddr_in = sizeof(struct sockaddr_in);
    while(1) {

        //Attesa di nuovi client
        client_sock = accept(server_sock, (struct sockaddr*)&infoClient, (socklen_t*)&sizeStrutturaSockaddr_in); //Richiesta dal client ricevuta
        if (client_sock < 0) {
            perror("Accettazione fallita");
            close(server_sock);
            exit(EXIT_FAILURE);
        }

        printf("Nuova connessione accettata\n");

        //Avvia un nuovo thread, per il nuovo client connesso, che resta in ascolto di nuovo richieste
        pthread_t newThread;
        int *arg = (int *)malloc(sizeof(int));
        *arg = client_sock;
        pthread_create(&newThread,NULL,gestione_richieste_client,(void*)arg);
    }

    close(server_sock);
}

int main()
{
    //Inizializzazione a NULL degli array globali
    for(int i = 0; i<SIZE_ARRAY_TEAMS; i++) //Inizializzaione arraySuqadre a NULL
        squadreInCostruzione[i] = NULL;

    for(int i = 0; i<SIZE_ARRAY_PLAYERS; i++) //Inizializzaione arrayPlayerConnessi a NULL
        playersConnessi[i] = NULL;

    for(int i=0; i<SIZE_ARRAY_PARTITE;i++)
        partite[i] = NULL;

    avvia_server_socket(8080);
}
