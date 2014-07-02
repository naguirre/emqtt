
#include <Ecore.h>
#include <Ecore_Con.h>

#include "EMqtt.h"

void _topic_received_cb(void *data, EMqtt_Sn_Client *client, const char *topic, const char *value)
{
    printf("TOPIC : %s\n", topic);
    printf("MSG : %s\n", value);

}

void _suback_received_cb(EMqtt_Sn_Client_Subscribe_Error_Cb subscribe_error_cb)
{
    printf("SUBACK : %d\n", subscribe_error_cb);
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
    emqtt_sn_client_connect_send(client, NULL, NULL, 10.0);

    emqtt_sn_client_subscribe(client,"temp", _topic_received_cb, _suback_received_cb, NULL);

    ecore_main_loop_begin();

    emqtt_shutdown();

    return 0;
}
