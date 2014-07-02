#include "EMqtt.h"
#include "emqtt_private.h"

static Eina_Bool
_mqtt_subscriber_name_exists(Mqtt_Client_Data *cdata, const char *name, Eina_List *subscribers)
{
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;

    EINA_LIST_FOREACH(subscribers, l, subscriber)
    {
        if (!memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
                !strcmp(name, subscriber->topic->name))
            return EINA_TRUE;
    }
    return EINA_FALSE;
}

static Eina_Bool
_mqtt_subscriber_id_exists(Mqtt_Client_Data *cdata, uint16_t id, Eina_List *subscribers)
{
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;

    EINA_LIST_FOREACH(subscribers, l, subscriber)
    {
        if (!memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
                id == subscriber->topic->id)
            return EINA_TRUE;
    }
    return EINA_FALSE;
}



static void
_mqtt_sn_connect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Connect_Msg *msg;
    EMqtt_Sn_Connack_Msg resp;
    char *client_id;
    size_t s;

    msg = (EMqtt_Sn_Connect_Msg *)cdata->data;
    s = msg->header.len - (sizeof(EMqtt_Sn_Connect_Msg));
    client_id = calloc(1, s + 1);
    memcpy(client_id, cdata->data + sizeof(EMqtt_Sn_Connect_Msg) , s);

    resp.header.len = 0x03;
    resp.header.msg_type = EMqtt_Sn_CONNACK;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;

    sendto(srv->fd, &resp, sizeof(resp), 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_register_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Register_Msg *msg;
    EMqtt_Sn_Regack_Msg resp;
    EMqtt_Sn_Topic *topic;
    Eina_List *l;
    char *topic_name;
    size_t s;
    Eina_Bool found = EINA_FALSE;

    msg = (EMqtt_Sn_Register_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqtt_Sn_Register_Msg));
    topic_name = calloc(1, s + 1);
    memcpy(topic_name, cdata->data + sizeof(EMqtt_Sn_Register_Msg) , s);

    topic = emqtt_topic_name_get(topic_name, srv->topics);
    if (!topic)
        /* Create the new topic */
    {
        topic = emqtt_topic_new(topic_name, &srv->last_topic);
        srv->topics = eina_list_append(srv->topics, topic);
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;
    }
    else
    {
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;
    }

    resp.header.len = sizeof(EMqtt_Sn_Regack_Msg);
    resp.header.msg_type = EMqtt_Sn_REGACK;
    resp.msg_id = msg->msg_id;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_publish_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
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


    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));

    EINA_LIST_FOREACH(srv->subscribers, l, subscriber)
    {
        if (subscriber->topic->id == htons(msg->topic_id))
        {
            printf("Header len : %d\n", msg->header.len);
            sendto(srv->fd, msg, msg->header.len, 0, (struct sockaddr *)&subscriber->client_addr, sizeof(subscriber->client_addr));
        }
    }

}

static void
_mqtt_sn_pingreq_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Pingresp_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMqtt_Sn_PINGRESP;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_disconnect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Disconnect_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMqtt_Sn_DISCONNECT;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_subscribe_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Subscribe_Msg *msg;
    EMqtt_Sn_Suback_Msg resp;
    uint8_t topic_id_type;
    char *topic_name = NULL;
    uint16_t topic_id;
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;
    EMqtt_Sn_Topic *topic;
    Eina_Bool found = EINA_FALSE;

    printf("Subscribe\n");

    msg = (EMqtt_Sn_Subscribe_Msg *)cdata->data;
    topic_id_type = msg->flags & 0x03;
    switch(topic_id_type)
    {
    case 0:
    {
        /* Topic name */
        size_t s;

        s = msg->header.len - ((sizeof(EMqtt_Sn_Subscribe_Msg) - sizeof(uint16_t)));
        topic_name = calloc(1, s + 1);
        memcpy(topic_name, cdata->data + sizeof(EMqtt_Sn_Subscribe_Msg) - sizeof(uint16_t) , s);
    }
        break;
    case 1:
        /* pre-define Topic id */
        break;
    case 2:
        /* Short Topic name */
        break;
    default:
        break;
    }

    topic = emqtt_topic_name_get(topic_name, srv->topics);
    if (!topic)
    {
        topic = emqtt_topic_new(topic_name, &srv->last_topic);
        srv->topics = eina_list_append(srv->topics, topic);
    }

    if (!_mqtt_subscriber_name_exists(cdata, topic_name, srv->subscribers))
    {
        subscriber = calloc(1, sizeof(EMqtt_Sn_Subscriber));
        subscriber->topic = topic;
        subscriber->client_addr = cdata->client_addr;
        srv->subscribers = eina_list_append(srv->subscribers, subscriber);
    }

    printf("%s subscrive to topic %s[%d]\n", cdata->client_addr, topic->name, topic->id);

    resp.header.len = sizeof(EMqtt_Sn_Suback_Msg);
    resp.header.msg_type = EMqtt_Sn_SUBACK;
    resp.flags = 0x00;
    resp.topic_id = htons(topic->id);
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;
    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static Eina_Bool _mqtt_server_data_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
    EMqtt_Sn_Server *srv = data;
    EMqtt_Sn_Small_Header *header;
    int i;
    char fmt[32];
    int fd;
    socklen_t len;
    Mqtt_Client_Data *cdata;

#define READBUFSIZ 65536
    char* d;
    struct sockaddr_in6 cliaddr;

    cdata = calloc(1, sizeof(Mqtt_Client_Data));

    len = sizeof(cdata->client_addr);
    cdata->len = recvfrom(srv->fd, cdata->data,READBUFSIZ, 0, (struct sockaddr *)&cdata->client_addr, &len);

    printf("Receive %d bytes from %s\n", cdata->len, cdata->client_addr.sin6_addr);

    //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));


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
    case EMqtt_Sn_CONNECT:
        _mqtt_sn_connect_msg(srv, cdata);
        break;
    case EMqtt_Sn_REGISTER:
        _mqtt_sn_register_msg(srv, cdata);
        break;
    case EMqtt_Sn_PUBLISH:
        _mqtt_sn_publish_msg(srv, cdata);
        break;
    case EMqtt_Sn_PINGREQ:
        _mqtt_sn_pingreq_msg(srv, cdata);
        break;
    case EMqtt_Sn_SUBSCRIBE:
        _mqtt_sn_subscribe_msg(srv, cdata);
        break;
    case EMqtt_Sn_DISCONNECT:
        _mqtt_sn_disconnect_msg(srv, cdata);
        break;
    default:
        printf("Unknown message\n");
        break;
    }

    free(cdata);

    return ECORE_CALLBACK_RENEW;
}

EMqtt_Sn_Server *emqtt_sn_server_add(char *addr, unsigned short port)
{
    EMqtt_Sn_Server *srv;
    struct sockaddr_in6 servaddr;
    int optval;
    int flags;

    if (!addr || !port)
        return NULL;

    srv = calloc(1, sizeof(EMqtt_Sn_Server));
    srv->addr = eina_stringshare_add(addr);
    srv->port = port;
    srv->topics = NULL;
    srv->subscribers = NULL;

    srv->fd = socket(PF_INET6, SOCK_DGRAM, 0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(port);

    optval = 1;
    setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (fcntl(srv->fd, F_SETFL, O_NONBLOCK) < 0)
        printf("Error\n");
    if (fcntl(srv->fd, F_SETFD, FD_CLOEXEC) < 0)
        printf("Error\n");

    if (setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(optval)) < 0)
      printf("Error\n");


    if (bind(srv->fd,  (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
      printf("Error");

    ecore_main_fd_handler_add(srv->fd, ECORE_FD_READ, _mqtt_server_data_cb, srv, NULL, NULL);

    return srv;

}

void emqtt_sn_server_del(EMqtt_Sn_Server *srv)
{
    if (!srv)
        return;

    if (srv->addr)
        eina_stringshare_del(srv->addr);

    free(srv);
}
