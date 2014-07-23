/******************************************************************************
** Copyright (c) 2014, emqtt. All Rights Reserved.
**
** Authors: Nicolas Aguirre, Julien Masson
**
** This file is part of emqtt.
**
** emqtt is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** emqtt is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Foobar; if not, write to the Free Software
** Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
**
******************************************************************************/
#include <EMqtt.h>

void _topic_received_temp_cb(void *data, EMqtt_Sn_Client *client, const char *topic, const char *value)
{
    printf("TEMP TOPIC : %s\n", topic);
    printf("TEMP MSG : %s\n", value);
}

void _suback_received_temp_cb(void *data, EMQTT_SN_ERROR_TYPE state)
{
    printf("TEMP SUBACK : %d\n", state);
}

void _topic_received_bumper_cb(void *data, EMqtt_Sn_Client *client, const char *topic, const char *value)
{
    printf("BUMPER TOPIC : %s\n", topic);
    printf("BUMPER MSG : %s\n", value);
}

void _suback_received_bumper_cb(void *data, EMQTT_SN_ERROR_TYPE state)
{
    printf("BUMPER SUBACK : %d\n", state);
}


void _suback_received_cb(void *data, EMQTT_SN_ERROR_TYPE state)
{
    printf("SUBACK : %d\n", state);
}

void _topic_received_cb(void *data, EMqtt_Sn_Client *client, const char *topic, const char *value)
{
    printf("TOPIC : %s\n", topic);
    printf("MSG : %s\n", value);
}


void _connect_received_cb(void *data, EMqtt_Sn_Client *client, EMqtt_Sn_CONNECTION_STATE connection_state)
{
    printf("State: %d\n",connection_state);
    if(connection_state == CONNECTION_ACCEPTED){
        emqtt_sn_client_subscribe(client,"#", _topic_received_cb, _suback_received_cb, NULL);
        /* emqtt_sn_client_subscribe(client,"temp", _topic_received_temp_cb, _suback_received_temp_cb, NULL); */
        /* emqtt_sn_client_subscribe(client,"bumper", _topic_received_bumper_cb, _suback_received_bumper_cb, NULL); */
    }
}


void _puback_received_test_cb(void *data, EMqtt_Sn_Client *client)
{
    printf("PUBACK RECEIVED from test\n\n");
}


void _puback_received_state_cb(void *data, EMqtt_Sn_Client *client)
{
    printf("PUBACK RECEIVED from state\n\n");
}


static Eina_Bool
_mqtt_publish_timer_cb(void *data)
{
    EMqtt_Sn_Client *client = (EMqtt_Sn_Client *)data;
    emqtt_sn_client_send_publish(client, "test", "2738", _puback_received_test_cb, NULL);
    emqtt_sn_client_send_publish(client, "state", "2739", _puback_received_state_cb, NULL);
    return ECORE_CALLBACK_RENEW;
}

int
main(int argc, char **argv)
{
    EMqtt_Sn_Client *client;
    Ecore_Timer *publish_timer;

    emqtt_init();

    client = emqtt_sn_client_add(argv[1], atoi(argv[2]), "EMqtt Client");
    if (!client)
    {
        printf("Erreur creating client! Exiting\n");
        return EXIT_FAILURE;
    }
    emqtt_sn_client_connect_send(client, _connect_received_cb, NULL, 10.0);

    publish_timer = ecore_timer_add(4.0, _mqtt_publish_timer_cb, client);

    ecore_main_loop_begin();

    emqtt_shutdown();

    return 0;
}
