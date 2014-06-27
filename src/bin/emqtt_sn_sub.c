
#include <Ecore.h>
#include <Ecore_Con.h>

#include "EMqtt.h"

void _topic_received_cb(void *data, const char *topic, const char *value)
{
    printf("%s\n", value);
}

int
main(int argc, char **argv)
{
    EMqtt_Sn_Client *client;

    emqtt_init();

    client = emqtt_sn_client_add(argv[1], atoi(argv[2]), "EMqtt Client");
    emqtt_sn_client_connect_send(client, NULL, NULL, 10.0);

    //emqtt_sn_subscribe(client, argv[3], _topic_received_cb, NULL);

    ecore_main_loop_begin();

    emqtt_shutdown();

    return 0;
}
