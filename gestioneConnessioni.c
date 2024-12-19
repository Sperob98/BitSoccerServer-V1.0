#include "gestioneConnessioni.h"
#include "variabiliGlobali.h"
#include "squadra.h"

char *get_tipo_richiesta(char *messaggio){

    //parse JSON del messaggio
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(messaggio);

    if(parsed_json != NULL){

        //Estrazione tipo di richiesta
        struct json_object *tipoRichiesta;
        json_object_object_get_ex(parsed_json, "tipoRichiesta", &tipoRichiesta);

        if( (json_object_get_string(tipoRichiesta)) != NULL){

        //Return stringa della richiesta
        return json_object_get_string(tipoRichiesta);

        }

    }

    return "Parsing fallito";

}
