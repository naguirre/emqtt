#include "EMqtt.h"
#include "emqtt_private.h"

static Eina_Bool
_mqtt_keepalive_timer_cb(void *data)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Pingreq_Msg msg;

    msg.header.len = 2;
    msg.header.msg_type = EMqtt_Sn_PINGREQ;

    send(client->fd, &msg, msg.header.len, 0);
    /* TODO return value */
}


static Eina_Bool
_mqtt_timeout_connect_cb(void *data)
{
  EMqtt_Sn_Client *client = data;
  EMqtt_Sn_Connect_Msg *msg; 
  char d[256];
  EMqtt_Sn_CONNECTION_TYPE connection_state;
 
  msg = (EMqtt_Sn_Connect_Msg *)d;
  msg->header.msg_type = EMqtt_Sn_CONNECT;
  msg->flags = 0;
  msg->protocol_id = 1;
  msg->duration = client->keepalive;
 
  snprintf(d + sizeof(msg), sizeof(d) - sizeof(msg), "%s", client->name);
  msg->header.len = sizeof(msg) + strlen(client->name);
  
  send(client->fd, msg, msg->header.len, 0);
  printf("Send %d bytes to %s\n", msg->header.len, client->server_addr.sa_data);

  connection_state = CONNECTION_WIP;
  connected_received_cb(client,connection_state);
      
}



static void
_mqtt_sn_connack_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Connack_Msg *msg;
    EMqtt_Sn_CONNECTION_TYPE connection_state;

    msg = (EMqtt_Sn_Connack_Msg *)cdata->data;
    /* TODO : Check if connack is a valid request */
    if (msg->ret_code != EMqtt_Sn_RETURN_CODE_ACCEPTED)
    {
      /* TODO : call connected_cb with error code */
        printf("Error : connection not accepted by server\n");
	connection_state = CONNECTION_ERROR;
    }else{
	connection_state = CONNECTION_ACCEPTED;
	client->timeout = ecore_timer_del(client->timeout);
	/* TODO : check keppalive_timer doesn't exist */
	/* Client now accepted, create a timer to launch Ping request each keepalive seconds */
	client->keepalive_timer = ecore_timer_add(client->keepalive, _mqtt_keepalive_timer_cb, client);
    }

    connected_received_cb(client,connection_state);

}

static Eina_Bool
_timer_cb(void *data)
{
    EMqtt_Sn_Server *srv = data;
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;


    printf("timer\n");
    EINA_LIST_FOREACH(srv->subscribers, l, subscriber)
    {
        EMqtt_Sn_Pingreq_Msg msg;
        msg.header.len = 2;
        msg.header.msg_type = EMqtt_Sn_PINGREQ;

        sendto(srv->fd, &msg, msg.header.len, 0, (struct sockaddr *)&subscriber->client_addr, sizeof(subscriber->client_addr));

    }
    return EINA_TRUE;
}

static void
_mqtt_sn_suback_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Suback_Msg *msg;
    EMqtt_Sn_Subscriber *subscriber;
    Eina_List *l;


    /* TODO : check if suback is a valid request */
    msg = (EMqtt_Sn_Suback_Msg *)cdata->data;

    if (msg->ret_code != EMqtt_Sn_RETURN_CODE_ACCEPTED)
    {
        /* TODO : send subscribe_cb with error code */
        printf("Error : publish not accepted by server\n");
        return;
    }

    EINA_LIST_FOREACH(client->subscribers, l, subscriber)
    {
        if (subscriber->msg_id == htons(msg->msg_id))
        {
            subscriber->topic->id = htons(msg->topic_id);
        }
    }
}

static void
_mqtt_sn_client_publish_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Publish_Msg *msg;
    EMqtt_Sn_Puback_Msg resp;
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;
    char *data;
    size_t s;

    msg = (EMqtt_Sn_Publish_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqtt_Sn_Publish_Msg));
    data = calloc(1, s + 1);
    memcpy(data, cdata->data + sizeof(EMqtt_Sn_Publish_Msg) , s);

    resp.header.len = sizeof(EMqtt_Sn_Puback_Msg);
    resp.header.msg_type = EMqtt_Sn_PUBACK;
    resp.topic_id = msg->topic_id;
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;


    send(client->fd, &resp, resp.header.len, 0);

    EINA_LIST_FOREACH(client->subscribers, l, subscriber)
    {
        if (subscriber->topic->id == htons(msg->topic_id))
        {
            if (subscriber->topic_received_cb)
                subscriber->topic_received_cb(subscriber->data, client, subscriber->topic->name, data);
        }
    }
}

static Eina_Bool _mqtt_client_data_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Small_Header *header;
    int i;
    char fmt[32];
    int fd;
    socklen_t len;
    Mqtt_Client_Data *cdata;


    char* d;
    struct sockaddr_in6 cliaddr;

    cdata = calloc(1, sizeof(Mqtt_Client_Data));

    len = sizeof(cdata->client_addr);
    cdata->len = recvfrom(client->fd, cdata->data,READBUFSIZ, 0, (struct sockaddr *)&cdata->client_addr, &len);

    header = (EMqtt_Sn_Small_Header *)cdata->data;

    d = cdata->data;

    printf("Receive Message : %s[%d]\n", mqttsn_msg_desc[header->msg_type].name, header->msg_type);

    // Header
    if (header->len == 0x01)
    {
        printf("Error long header not handle yet !\n");
        free(cdata);
        return ECORE_CALLBACK_RENEW;
    }

    switch(header->msg_type)
    {
    case EMqtt_Sn_CONNACK:
        _mqtt_sn_connack_msg(client, cdata);
        break;
    case EMqtt_Sn_SUBACK:
        _mqtt_sn_suback_msg(client, cdata);
        break;
    case EMqtt_Sn_PUBLISH:
        _mqtt_sn_client_publish_msg(client, cdata);
        break;
	/* TODO : REGISTER */
    /* TODO : PINRESP & PINGREQ */
    default:
        printf("Unknown message\n");
        break;
    }

    free(cdata);

    return ECORE_CALLBACK_RENEW;
}

EMqtt_Sn_Client *emqtt_sn_client_add(char *addr, unsigned short port, char *client_name)
{
    EMqtt_Sn_Client *client;
    int optval;
    int flags;
    struct addrinfo hints;
    struct addrinfo *res, *it;
    int ret;
    int fd;
    struct timeval tv;
    char port_s[16];

    if (!addr || !port)
        return NULL;

    client = calloc(1, sizeof(EMqtt_Sn_Client));
    client->addr = eina_stringshare_add(addr);
    client->port = port;
    client->name = eina_stringshare_add(client_name);
    client->fd = socket(PF_INET6, SOCK_DGRAM, 0);
    client->connection_state = CONNECTION_WIP;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    snprintf(port_s, sizeof(port_s), "%d", port);

    ret =  getaddrinfo(client->addr, port_s, &hints, &res);
    if (ret != 0)
    {
       printf("udpclient error for %s, %s: %s", client->addr, client->port, gai_strerror(ret));
    }
    else
    {
        printf("res : %s %s\n", res->ai_addr, res->ai_canonname);
    }

    for (it = res; it != NULL; it = it->ai_next)
    {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd == -1)
            continue;

        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0)
            break;

        close(fd);
    }

    if (it == NULL)
    {
        perror("Could not connect to remote host.\n");
        return NULL;
    }

    client->fd = fd;
    memcpy(&client->server_addr, it->ai_addr, sizeof(client->server_addr));
    freeaddrinfo(res);

    tv.tv_sec = 10;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("Error setting timeout on socket");
    }

    if (fcntl(client->fd, F_SETFL, O_NONBLOCK) < 0)
      perror("setsockoption\n");
    if (fcntl(client->fd, F_SETFD, FD_CLOEXEC) < 0)
      perror("setsockoption\n");

    if (setsockopt(client->fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(optval)) < 0)
      perror("setsockoption\n");

    ecore_main_fd_handler_add(client->fd, ECORE_FD_READ, _mqtt_client_data_cb, client, NULL, NULL);

    return client;

}

void emqtt_sn_client_connect_send(EMqtt_Sn_Client *client, EMqtt_Sn_Client_Connect_Cb connected_cb, void *data, double keepalive)
{
    char d[256];
    EMqtt_Sn_Connect_Msg *msg;

    if (!client)
        return;

    /* TODO : check connected state */
    /* TODO : check connection in progress state */

    if(client->connection_state == CONNECTION_WIP){

      msg = (EMqtt_Sn_Connect_Msg *)d;
      msg->header.msg_type = EMqtt_Sn_CONNECT;
      msg->flags = 0;
      msg->protocol_id = 1;
      msg->duration = htons((uint16_t)keepalive);
          
      connected_received_cb = connected_cb;
      client->keepalive = keepalive;
      snprintf(d + sizeof(msg), sizeof(d) - sizeof(msg), "%s", client->name);
      msg->header.len = sizeof(msg) + strlen(client->name);

      client->timeout = ecore_timer_add(TIMEOUT_CONNECT, _mqtt_timeout_connect_cb, client);

      send(client->fd, msg, msg->header.len, 0);
      printf("Send %d bytes to %s\n", msg->header.len, client->server_addr.sa_data);

    }

}

void emqtt_sn_client_subscribe(EMqtt_Sn_Client *client, const char *topic_name, EMqtt_Sn_Client_Topic_Received_Cb topic_received_cb, void *data)
{
    char d[256];
    EMqtt_Sn_Subscribe_Msg *msg;
    EMqtt_Sn_Topic *topic;
    EMqtt_Sn_Subscriber *subscriber;

    if (!topic)
        return;

    msg = (EMqtt_Sn_Subscribe_Msg *)d;

    msg->header.len;
    msg->header.msg_type = EMqtt_Sn_SUBSCRIBE;
    msg->flags = 0;
    msg->msg_id = client->last_msg_id++;
    snprintf(d + sizeof(msg) - 2, sizeof(d) - sizeof(msg) - 2, "%s", topic);
    msg->header.len = sizeof(msg) - 2 + strlen(client->name);

    topic = emqtt_topic_name_get(topic_name, client->topics);
    if (!topic)
    {
        topic = emqtt_topic_new(topic_name, NULL);
        client->topics = eina_list_append(client->topics, topic);
    }

    subscriber = calloc(1, sizeof(EMqtt_Sn_Subscriber));
    subscriber->topic = topic;
    subscriber->topic_received_cb = topic_received_cb;
    subscriber->data = data;
    subscriber->msg_id = msg->msg_id;
    client->subscribers = eina_list_append(client->subscribers, subscriber);

    send(client->fd, msg, msg->header.len, 0);
    /* return emqtt_sn_subscriber */
}

/* TODO : Emqtt_Sn_Subscriber delete */
