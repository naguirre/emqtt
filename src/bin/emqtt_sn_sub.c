
#include <Ecore.h>
#include <Ecore_Con.h>

#include "EMqtt.h"



void _topic_received_cb(void *data, EMqtt_Sn_Client *client, const char *topic, const char *value)
{

  printf("TOPIC : %s\n", topic);
  printf("MSG : %s\n", value);

}


void _suback_received_cb(EMqtt_Sn_SUBACK_TYPE suback_state)
{
  printf("SUBACK : %d\n", suback_state);
}


void _connect_received_cb(EMqtt_Sn_Client *client, EMqtt_Sn_CONNECTION_TYPE connection_state)
{
 
  printf("State: %d\n",connection_state);
  if(connection_state == CONNECTION_ACCEPTED){
    emqtt_sn_client_subscribe(client,"temp", _topic_received_cb, _suback_received_cb, NULL);
  }

}


int
main(int argc, char **argv)
{
    EMqtt_Sn_Client *client;

    emqtt_init();

    client = emqtt_sn_client_add(argv[1], atoi(argv[2]), "EMqtt Client");
    if (!client)
    {
        printf("Erreur creating client! Exiting\n");
        return EXIT_FAILURE;
    }
    emqtt_sn_client_connect_send(client, _connect_received_cb, NULL, 10.0);

    ecore_main_loop_begin();

    emqtt_shutdown();

    return 0;
}
