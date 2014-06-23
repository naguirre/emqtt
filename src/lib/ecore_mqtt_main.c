#include "Ecore_Mqtt.h"

struct _Ecore_Mqtt_Sn_Server
{
  const char *addr;
  unsigned short port;
  Ecore_Con_Server *econ_srv;
};

static int _ecore_mqtt_init_count = 0;

static Eina_Bool
_mqtt_ecore_conn_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
  printf("New connection : ip %s, port %d, connected = %d\n",
         ecore_con_client_ip_get(ev->client),
	 ecore_con_client_port_get(ev->client),
	 ecore_con_client_connected_get(ev->client));
  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_mqtt_ecore_conn_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
  printf("Connection close\n");
  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_mqtt_ecore_conn_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Data *ev)
{
  printf("Connection data\n");
  return ECORE_CALLBACK_RENEW;
}

int ecore_mqtt_init(void)
{
    if (++_ecore_mqtt_init_count != 1)
        return _ecore_mqtt_init_count;


    if (!eina_init ())
        return --_ecore_mqtt_init_count;

    if (!ecore_init ())
        return --_ecore_mqtt_init_count;

    if (!ecore_con_init ())
        return --_ecore_mqtt_init_count;

    return _ecore_mqtt_init_count;

}

int ecore_mqtt_shutdown(void)
{
    if (--_ecore_mqtt_init_count != 0)
        return _ecore_mqtt_init_count;

    ecore_con_shutdown();
    ecore_shutdown();
    eina_shutdown();

    return _ecore_mqtt_init_count;
}

Ecore_Mqtt_Sn_Server *ecore_mqtt_sn_server_add(char *addr, unsigned short port)
{
  Ecore_Mqtt_Sn_Server *srv;
   
   if (!addr || !port)
     return NULL;

   srv = calloc(1, sizeof(Ecore_Mqtt_Sn_Server));
   srv->addr = eina_stringshare_add(addr);
   srv->port = port;

   if (!(srv->econ_srv = ecore_con_server_add(ECORE_CON_REMOTE_UDP, addr, port, NULL)))
     {
       ecore_mqtt_sn_server_del(srv);
       return NULL;
     }
   
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                           (Ecore_Event_Handler_Cb)_mqtt_ecore_conn_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                           (Ecore_Event_Handler_Cb)_mqtt_ecore_conn_del, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                           (Ecore_Event_Handler_Cb)_mqtt_ecore_conn_data, NULL);


   return srv;

}

void ecore_mqtt_sn_server_del(Ecore_Mqtt_Sn_Server *srv)
{
  if (!srv)
    return;

  if (srv->addr)
      eina_stringshare_del(srv->addr);
  if (srv->econ_srv)
    ecore_con_server_del(srv->econ_srv);
  
  free(srv);
}
