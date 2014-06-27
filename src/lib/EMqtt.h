#ifndef __EMQTT_H__
#define __EMQTT_H__

#include <sys/socket.h>
#include <netinet/in.h>

#include <Ecore.h>

typedef struct _EMqtt_Sn_Server EMqtt_Sn_Server;
typedef struct _EMqtt_Sn_Client EMqtt_Sn_Client;

typedef void (*EMqtt_Sn_Client_Connect_Cb) (EMqtt_Sn_Client *client);

int emqtt_init(void);
int emqtt_shutdown(void);

EMqtt_Sn_Server *emqtt_sn_server_add(char *addr, unsigned short port);

void emqtt_sn_server_del(EMqtt_Sn_Server *srv);

EMqtt_Sn_Client *emqtt_sn_client_add(char *addr, unsigned short port, char *client_name);

void emqtt_sn_client_connect_send(EMqtt_Sn_Client *client, EMqtt_Sn_Client_Connect_Cb connected_cb, void *data, double keepalive);


void emqtt_sn_client_del(EMqtt_Sn_Client *srv);



#endif
