
#include <Ecore.h>
#include <Ecore_Con.h>

typedef struct _MQTTSN_Msg_Desc MQTTSN_Msg_Desc;
typedef enum _MQTTSN_MSG_TYPE MQTTSN_MSG_TYPE;

enum _MQTTSN_MSG_TYPE
{
	MQTTSN_ADVERTISE, 
	MQTTSN_SEARCHGW,
	MQTTSN_GWINFO,
	MQTTSN_RESERVED1,
	MQTTSN_CONNECT,
	MQTTSN_CONNACK,
	MQTTSN_WILLTOPICREQ,
	MQTTSN_WILLTOPIC, 
	MQTTSN_WILLMSGREQ,
	MQTTSN_WILLMSG,
	MQTTSN_REGISTER,
	MQTTSN_REGACK,
	MQTTSN_PUBLISH,
	MQTTSN_PUBACK,
	MQTTSN_PUBCOMP,
	MQTTSN_PUBREC,
	MQTTSN_PUBREL,
	MQTTSN_RESERVED2,
	MQTTSN_SUBSCRIBE,
	MQTTSN_SUBACK,
	MQTTSN_UNSUBSCRIBE,
	MQTTSN_UNSUBACK,
	MQTTSN_PINGREQ,
	MQTTSN_PINGRESP,
	MQTTSN_DISCONNECT,
	MQTTSN_RESERVED3,
	MQTTSN_WILLTOPICUPD,
	MQTTSN_WILLTOPICRESP, 
	MQTTSN_WILLMSGUPD,
	MQTTSN_WILLMSGRESP
};

struct _MQTTSN_Msg_Desc
{
  MQTTSN_MSG_TYPE val;
  char *name;
};


const MQTTSN_Msg_Desc mqttsn_msg_desc[] = 
  {
    {MQTTSN_ADVERTISE,"ADVERTISE" },
    {MQTTSN_SEARCHGW,"SEARCHGW"},
    {MQTTSN_GWINFO,"GWINFO"},
    {MQTTSN_RESERVED1,"RESERVED1"},
    {MQTTSN_CONNECT,"CONNECT"},
    {MQTTSN_CONNACK,"CONNACK"},
    {MQTTSN_WILLTOPICREQ,"WILLTOPICREQ"},
    {MQTTSN_WILLTOPIC, "WILLTOPIC"},
    {MQTTSN_WILLMSGREQ,"WILLMSGREQ"},
    {MQTTSN_WILLMSG,"WILLMSG"},
    {MQTTSN_REGISTER,"REGISTER"},
    {MQTTSN_REGACK,"REGACK"},
    {MQTTSN_PUBLISH,"PUBLISH"},
    {MQTTSN_PUBACK,"PUBACK"},
    {MQTTSN_PUBCOMP,"PUBCOMP"},
    {MQTTSN_PUBREC,"PUBREC"},
    {MQTTSN_PUBREL,"PUBREL"},
    {MQTTSN_RESERVED2,"RESERVED2"},
    {MQTTSN_SUBSCRIBE,"SUBSCRIBE"},
    {MQTTSN_SUBACK,"SUBACK"},
    {MQTTSN_UNSUBSCRIBE,"UNSUBSCRIBE"},
    {MQTTSN_UNSUBACK,"UNSUBACK"},
    {MQTTSN_PINGREQ,"PINGREQ"},
    {MQTTSN_PINGRESP,"PINGRESP"},
    {MQTTSN_DISCONNECT,"DISCONNECT"},
    {MQTTSN_RESERVED3,"RESERVED3"},
    {MQTTSN_WILLTOPICUPD,"WILLTOPICUPD"},
    {MQTTSN_WILLTOPICRESP,"WILLTOPICRESP"},
    {MQTTSN_WILLMSGUPD,"WILLMSGUPD"},
    {MQTTSN_WILLMSGRESP, "WILLMSGRESP"},
  };


struct _Client
{
  int sdata;
};
Eina_Bool
_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
  char welcome[] = "hello! - sent from the server";
  Ecore_Con_Server *srv;
  Ecore_Con_Client *cl;
  const Eina_List *clients, *l;
  struct _Client *client = malloc(sizeof(*client));
  client->sdata = 0;
  printf("Client with ip %s, port %d, connected = %d!\n",
	 ecore_con_client_ip_get(ev->client),
	 ecore_con_client_port_get(ev->client),
	 ecore_con_client_connected_get(ev->client));

  //  ecore_con_client_timeout_set(ev->client, 6);
  ecore_con_client_data_set(ev->client, client);
  srv = ecore_con_client_server_get(ev->client);
  printf("Clients connected to this server:\n");
  clients = ecore_con_server_clients_get(srv);
  EINA_LIST_FOREACH(clients, l, cl)
    printf("%s\n", ecore_con_client_ip_get(cl));
  return ECORE_CALLBACK_RENEW;
}
Eina_Bool
_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
  struct _Client *client;
  if (!ev->client)
    return ECORE_CALLBACK_RENEW;
  client = ecore_con_client_data_get(ev->client);
  printf("Lost client with ip %s!\n", ecore_con_client_ip_get(ev->client));
  printf("Total data received from this client: %d\n", client->sdata);
  printf("Client was connected for %0.3f seconds.\n",
	 ecore_con_client_uptime_get(ev->client));
  if (client)
    free(client);
  ecore_con_client_del(ev->client);
  return ECORE_CALLBACK_RENEW;
}
Eina_Bool
_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Data *ev)
{
  int i;
  char fmt[128];
  char *d;
  size_t header_size = 0;
  size_t data_size = 0;
  struct _Client *client = ecore_con_client_data_get(ev->client);
  snprintf(fmt, sizeof(fmt),
	   "Received %i bytes from client %s port %d:\n"
	   ">>>>>\n"
	   "%%.%is\n"
	   ">>>>>\n",
	   ev->size, ecore_con_client_ip_get(ev->client),
	   ecore_con_client_port_get(ev->client), ev->size);

  d = ev->data;
  printf("Data :\n");
  for (i = 0; i < ev->size; i++)
    printf("0x%02X ", d[i]);
  printf("\n");

  // Header
  if (d[0] == 0x01)
    {
      header_size = 4;
      data_size = d[1] << 8 | d[2];
      printf("Header size : 4, data_size = %d, msg_type : %s(%d)\n", data_size, mqttsn_msg_desc[d[3]].name, d[3]);
    }
  else
    {
      header_size = 2;
      data_size = d[0];
      printf("Header size : 2, data_size = %d, msg_type : %s(%d)\n", data_size,  mqttsn_msg_desc[d[1]].name, d[1]);

    }

  switch(d[header_size - 1])
    {
    case MQTTSN_CONNECT:
      {
	char response[3];
	memset(&fmt, 0, sizeof(fmt));
	memcpy(&fmt, &d[header_size+4], data_size - 4 - header_size); 
	printf("flags: %d, ProtocolId: %d, Duration: %d, ClientId : %s\n", d[header_size], d[header_size+1], d[header_size + 2] << 8 | d[header_size+3], fmt);
	response[0] = 0x03;
	response[1] = MQTTSN_CONNACK;
	response[2] = 0x00;
	ecore_con_client_send(ev->client, response, sizeof(response));
	ecore_con_client_flush(ev->client);
      }
      break;
    case MQTTSN_REGISTER:
      {
	char response[7];
	memset(&fmt, 0, sizeof(fmt));
	memcpy(&fmt, &d[header_size+4], data_size - 4 - header_size); 
	printf("TopicId: %d, MsgId: %d, TopicName : %s\n", d[header_size] << 8 | d[header_size+1], d[header_size + 2] << 8 | d[header_size+3], fmt);
	response[0] = 0x07;
	response[1] = MQTTSN_REGACK;
	response[2] = d[header_size];
	response[3] = d[header_size+1];
	response[4] = d[header_size+2];
	response[5] = d[header_size+3];
	response[6] = 0;

	ecore_con_client_send(ev->client, response, sizeof(response));
	ecore_con_client_flush(ev->client);
      }
      break;
    case MQTTSN_PUBLISH:
      {
	char response[7];
	memset(&fmt, 0, sizeof(fmt));
	memcpy(&fmt, &d[header_size+5], data_size - 5 - header_size); 
	printf("flags: %d, TopicId: %d, MsgId : %d, Data : %s\n", d[header_size], d[header_size + 1] << 8 | d[header_size+2], d[header_size + 3] << 8 | d[header_size+4], fmt);
	response[0] = 0x07;
	response[1] = MQTTSN_PUBACK;
	response[2] = d[header_size+1];
	response[3] = d[header_size+2];
	response[4] = d[header_size+3];
	response[5] = d[header_size+4];
	response[6] = 0;
	
	ecore_con_client_send(ev->client, response, sizeof(response));
	ecore_con_client_flush(ev->client);
      }
      break;
    case MQTTSN_PINGREQ:
      {
	char response[2];
	memset(&fmt, 0, sizeof(fmt));
	memcpy(&fmt, &d[header_size+4], data_size - header_size); 
	printf("ClientId : %s\n", fmt);
	response[0] = 0x02;
	response[1] = MQTTSN_PINGRESP;
	
	ecore_con_client_send(ev->client, response, sizeof(response));
	ecore_con_client_flush(ev->client);
      }
      break;
  case MQTTSN_DISCONNECT:
      {
	char response[2];
	memset(&fmt, 0, sizeof(fmt));
	memcpy(&fmt, &d[header_size+4], data_size - header_size); 
	printf("Disconnect\n");
	response[0] = 0x02;
	response[1] = MQTTSN_DISCONNECT;
	
	ecore_con_client_send(ev->client, response, 2);
	ecore_con_client_flush(ev->client);
      }
      break;
    default:
      printf("Unknown message type\n");
      break;
      }
  


  client->sdata += ev->size;
  return ECORE_CALLBACK_RENEW;
}
int
main(void)
{
  Ecore_Con_Server *svr;
  Ecore_Con_Client *cl;
  const Eina_List *clients, *l;
  eina_init();
  ecore_init();
  ecore_con_init();
  if (!(svr = ecore_con_server_add(ECORE_CON_REMOTE_UDP, "::", 1883, NULL)))
    //  if (!(svr = ecore_con_server_add(ECORE_CON_REMOTE_UDP, "2a01:6600:8081:2c00:d267:e5ff:fe43:4c95", 1883, NULL)))
    exit(1);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_data, NULL);
  ecore_con_server_timeout_set(svr, 10);
  ecore_con_server_client_limit_set(svr, 3, 0);
  ecore_main_loop_begin();
  clients = ecore_con_server_clients_get(svr);
  printf("Clients connected to this server when exiting: %d\n",
	 eina_list_count(clients));
  EINA_LIST_FOREACH(clients, l, cl)
    {
      printf("%s\n", ecore_con_client_ip_get(cl));
      free(ecore_con_client_data_get(cl));
    }
  printf("Server was up for %0.3f seconds\n",
	 ecore_con_server_uptime_get(svr));
  ecore_con_shutdown();
  ecore_shutdown();
  eina_shutdown();
  return 0;
}
