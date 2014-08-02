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
#include <Ecore_Getopt.h>

static Ecore_Timer *publish_timer = NULL;
static char *topic = NULL;
static char *value = NULL;
static char *host = NULL;
static int port = 1883;

static const Ecore_Getopt optdesc = {
    "emqtt_sn_pub",
    NULL,
    "1.0",
    "(C) 2014 Nicolas Aguirre & Julien Masson",
    "GPLv2",
    "emqtt_sn_pub is a test program for emqtt, and ca be used to publish values over an SN network.\n",
    0,
    {
        ECORE_GETOPT_STORE_STR('h', "host", "Host to connect to."),
        ECORE_GETOPT_STORE_INT('p', "port", "Host port."),
        ECORE_GETOPT_STORE_STR('t', "topic", "Topic to publish."),
        ECORE_GETOPT_STORE_STR('v', "value", "Value to publish."),
        ECORE_GETOPT_LICENSE('L', "license"),
        ECORE_GETOPT_COPYRIGHT('C', "copyright"),
        ECORE_GETOPT_VERSION('V', "version"),
        ECORE_GETOPT_HELP('h', "help"),
        ECORE_GETOPT_SENTINEL
    }
};

void _puback_received_test_cb(void *data, EMqtt_Sn_Client *client)
{
    printf("Publish sent\n");
}


static Eina_Bool
_mqtt_publish_timer_cb(void *data)
{
    EMqtt_Sn_Client *client = (EMqtt_Sn_Client *)data;
    printf("Publish %s on topic %s\n", value, topic);
    emqtt_sn_client_publish(client, topic, value, _puback_received_test_cb, NULL);
    return ECORE_CALLBACK_RENEW;
}

void _connect_received_cb(void *data, EMqtt_Sn_Client *client, EMQTT_SN_CONNECTION_STATE connection_state)
{
    if (connection_state == EMQTT_SN_CONNECTION_ACCEPTED)
    {
        printf("Connected\n");
        if (!publish_timer)
            publish_timer = ecore_timer_add(4.0, _mqtt_publish_timer_cb, client);
    }
    else
    {
        fprintf(stderr, "Error connecting to host %s:%d\n", host, port);
        ecore_main_loop_quit();
    }
}

int
main(int argc, char **argv)
{
    Eina_Bool quit = EINA_FALSE;
    EMqtt_Sn_Client *client;
    Ecore_Getopt_Value values[] = {
        ECORE_GETOPT_VALUE_STR(host),
        ECORE_GETOPT_VALUE_INT(port),
        ECORE_GETOPT_VALUE_STR(topic),
        ECORE_GETOPT_VALUE_STR(value),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_NONE
    };

    eina_init();
    ecore_init();
    emqtt_init();

    if (ecore_getopt_parse(&optdesc, values, argc, argv) < 0)
    {
        fprintf(stderr, "Failed to parse args\n");
        return EXIT_FAILURE;
    }

    if (quit)
        return EXIT_SUCCESS;


    printf("Connecting to host %s:%d\n", host, port);

    client = emqtt_sn_client_add(host, port, "EMqtt Client");
    if (!client)
    {
        fprintf(stderr, "Erreur creating client! Exiting\n");
        return EXIT_FAILURE;
    }

    emqtt_sn_client_connect(client, _connect_received_cb, NULL, 10.0);

    ecore_main_loop_begin();

    emqtt_shutdown();

    return 0;
}
