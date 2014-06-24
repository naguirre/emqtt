#include "Ecore_Mqtt.h"

static int _ecore_mqtt_init_count = 0;

int ecore_mqtt_init(void)
{
    if (++_ecore_mqtt_count != 1)
        return _ecore_mqtt_count;

    if (!ecore_con_init ())
        return --_ecore_mqtt_count;

    return _ecore_mqtt_count;

}

int ecore_mqtt_shutdown(void)
{
    if (--_ecore_mqtt_init_count != 0)
        return _ecore_mqtt_init_count;

    ecore_con_shutdown();

    return _ecore_mqtt_init_count;
}

Ecore_Mqtt *ecore_mqtt_server_add(char *addr, unsigned short port)
{
  
}
