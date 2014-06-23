#ifndef __ECORE_MQTT_H__
#define __ECORE_MQTT_H__

#include <Ecore.h>
#include <Ecore_Con.h>

typedef struct _Ecore_Mqtt_Sn_Server Ecore_Mqtt_Sn_Server;

int ecore_mqtt_init(void);
int ecore_mqtt_shutdown(void);

Ecore_Mqtt_Sn_Server *ecore_mqtt_sn_server_add(char *addr, unsigned short port);
void ecore_mqtt_sn_server_del(Ecore_Mqtt_Sn_Server *srv);

#endif
