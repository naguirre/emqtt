#ifndef __EMQTT_H__
#define __EMQTT_H__

#include <sys/socket.h>
#include <netinet/in.h>

#include <Ecore.h>

typedef struct _EMqttSn_Server EMqttSn_Server;

int emqtt_init(void);
int emqtt_shutdown(void);

EMqttSn_Server *emqtt_sn_server_add(char *addr, unsigned short port);
void emqtt_sn_server_del(EMqttSn_Server *srv);

#endif
