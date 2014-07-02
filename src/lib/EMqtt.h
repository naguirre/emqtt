#ifndef __EMQTT_H__
#define __EMQTT_H__

#include <sys/socket.h>
#include <netinet/in.h>

#include <Ecore.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#define EMQTT_MAX_RETRY 6
#define EMQTT_TIMEOUT_CONNECT 5
enum _EMqtt_Sn_CONNECTION_STATE
{
  CONNECTION_ACCEPTED,
  CONNECTION_IN_PROGRESS,
  CONNECTION_ERROR
};
typedef enum _EMqtt_Sn_CONNECTION_STATE EMqtt_Sn_CONNECTION_STATE;

enum _EMqtt_Sn_ERROR_TYPE
{
  ACCEPTED,
  ERROR
};
typedef enum _EMqtt_Sn_ERROR_TYPE EMqtt_Sn_ERROR_TYPE;
typedef struct _EMqtt_Sn_Server EMqtt_Sn_Server;
typedef struct _EMqtt_Sn_Client EMqtt_Sn_Client;

typedef void (*EMqtt_Sn_Client_Connect_Cb) (EMqtt_Sn_Client *client, EMqtt_Sn_CONNECTION_STATE connection_state);
typedef void (*EMqtt_Sn_Client_Topic_Received_Cb) (void *data, EMqtt_Sn_Client *client, const char *topic, const char *value);
typedef void (*EMqtt_Sn_Client_Subscribe_Error_Cb) (EMqtt_Sn_ERROR_TYPE state);

int emqtt_init(void);
int emqtt_shutdown(void);

EMqtt_Sn_Server *emqtt_sn_server_add(char *addr, unsigned short port);

void emqtt_sn_server_del(EMqtt_Sn_Server *srv);

EMqtt_Sn_Client *emqtt_sn_client_add(char *addr, unsigned short port, char *client_name);

void emqtt_sn_client_connect_send(EMqtt_Sn_Client *client, EMqtt_Sn_Client_Connect_Cb connected_cb, void *data, double keepalive);

void emqtt_sn_client_subscribe(EMqtt_Sn_Client *client, const char *topic_name, EMqtt_Sn_Client_Topic_Received_Cb topic_received_cb, EMqtt_Sn_Client_Subscribe_Error_Cb subscribe_error_cb, void *data);

void emqtt_sn_client_del(EMqtt_Sn_Client *srv);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif
