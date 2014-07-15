#include "EMqtt.h"
#include "emqtt_private.h"

static Eina_Bool
_mqtt_send_data(int fd, void *data, int len)
{
    EMqtt_Sn_Small_Header *header = (EMqtt_Sn_Small_Header *)data;

    printf("[->] %s[%d]\n",
           mqttsn_msg_desc[header->msg_type].name, header->msg_type);

    send(fd, data, len, 0);

    return EINA_TRUE;
}


static Eina_Bool
_mqtt_keepalive_timer_cb(void *data)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Pingreq_Msg msg;

    msg.header.len = 2;
    msg.header.msg_type = EMQTT_SN_PINGREQ;

    _mqtt_send_data(client->fd, &msg, msg.header.len);

    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_mqtt_timeout_connect_cb(void *data)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Connect_Msg *msg;
    char d[256];

    if(client->connection_retry < EMQTT_MAX_RETRY)
    {
        msg = (EMqtt_Sn_Connect_Msg *)d;
        msg->header.msg_type = EMQTT_SN_CONNECT;
        msg->flags = 0;
        msg->protocol_id = 1;
        msg->duration = client->keepalive;

        snprintf(d + sizeof(EMqtt_Sn_Connect_Msg), sizeof(d) - sizeof(EMqtt_Sn_Connect_Msg), "%s", client->name);
        msg->header.len = sizeof(EMqtt_Sn_Connect_Msg) + strlen(client->name);

        _mqtt_send_data(client->fd, msg, msg->header.len);

        client->connection_state = CONNECTION_IN_PROGRESS;
        client->connected_received_cb(client->data, client, client->connection_state);

        (client->connection_retry)++;
    }
    else
    {
        client->connection_state = CONNECTION_ERROR;
        client->connected_received_cb(client->data, client, client->connection_state);
        client->timeout = ecore_timer_del(client->timeout);
        client->timeout = NULL;
        client->connection_retry = 0;
        return ECORE_CALLBACK_DONE;
    }
    return ECORE_CALLBACK_RENEW;
}

static void
_mqtt_sn_connack_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Connack_Msg *msg;

    msg = (EMqtt_Sn_Connack_Msg *)cdata->data;

    if (msg->ret_code != EMQTT_SN_RETURN_CODE_ACCEPTED)
    {
        printf("Error : connection not accepted by server\n");
        client->connection_state = CONNECTION_ERROR;
    }
    else
    {
        client->connection_state = CONNECTION_ACCEPTED;
        client->timeout = ecore_timer_del(client->timeout);

        /* Client now accepted, create a timer to launch Ping request each keepalive seconds */
        client->keepalive_timer = ecore_timer_add(client->keepalive, _mqtt_keepalive_timer_cb, client);
    }

    client->connected_received_cb(client->data, client, client->connection_state);

}


static void
_mqtt_sn_puback_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Puback_Msg *msg;
    Eina_List *l;
    EMqtt_Sn_Publisher *publisher;

    msg = (EMqtt_Sn_Puback_Msg *)cdata->data;

    if (msg->ret_code != EMQTT_SN_RETURN_CODE_ACCEPTED)
    {
        printf("Error : connection not accepted by server\n");
    }
    else
    {

        EINA_LIST_FOREACH(client->publishers, l, publisher)
        {
            if (publisher->topic->id == msg->topic_id)
	        publisher->suback_received_cb(client->data, client);
        }

    }

}


static void
_mqtt_sn_regack_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Regack_Msg *msg;
    EMqtt_Sn_Publisher *publisher;
    Eina_List *l;

    msg = (EMqtt_Sn_Regack_Msg *)cdata->data;

    if (msg->ret_code != EMQTT_SN_RETURN_CODE_ACCEPTED)
    {
        printf("Error : register not accepted by server\n");
    }
    else
    {

        EINA_LIST_FOREACH(client->publishers, l, publisher)
	{
	    if(publisher->msg_id == msg->msg_id ){
	        publisher->topic->id = msg->topic_id;
	        //printf("TOPIC -> ID: %d\t NAME: %s\n",publisher->topic->id, publisher->topic->name);
	        publisher->register_state = REGISTER_ACCEPTED;
	    }

	}

    }

}


static void
_mqtt_sn_suback_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Suback_Msg *msg;
    EMqtt_Sn_Subscriber *subscriber;
    Eina_List *l;

    msg = (EMqtt_Sn_Suback_Msg *)cdata->data;

    if (msg->ret_code != EMQTT_SN_RETURN_CODE_ACCEPTED)
    {
        printf("Error : publish not accepted by server\n");
        EINA_LIST_FOREACH(client->subscribers, l, subscriber)
        {
	    if (subscriber->msg_id == msg->msg_id)
	    {
	        if (subscriber->subscribe_error_cb)
		    subscriber->subscribe_error_cb(client->data, ERROR);
	    }
        }
        return;
    }
    else
    {
        EINA_LIST_FOREACH(client->subscribers, l, subscriber)
        {
	    if (subscriber->msg_id == msg->msg_id)
	    {
	        if (subscriber->subscribe_error_cb)
	        {
		    subscriber->topic->id = htons(msg->topic_id);
		    subscriber->subscribe_error_cb(client->data, ACCEPTED);
		}
	    }
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
    char *value;
    size_t s;

    msg = (EMqtt_Sn_Publish_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqtt_Sn_Publish_Msg));
    value = calloc(1, s + 1);

    memcpy(value, cdata->data + sizeof(EMqtt_Sn_Publish_Msg) , s);
    printf("value : %s\n", value);
    resp.header.len = sizeof(EMqtt_Sn_Puback_Msg);
    resp.header.msg_type = EMQTT_SN_PUBACK;
    resp.topic_id = msg->topic_id;
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;


    _mqtt_send_data(client->fd, &resp, resp.header.len);

    EINA_LIST_FOREACH(client->subscribers, l, subscriber)
    {
        if (subscriber->topic->id == htons(msg->topic_id))
        {
            if (subscriber->topic_received_cb)
                subscriber->topic_received_cb(subscriber->data, client, subscriber->topic->name, value);
        }
        else
        {
	    EMqtt_Sn_Topic *topic = emqtt_topic_id_get(htons(msg->topic_id), client->topics);
	    if (emqtt_topic_matches(subscriber->topic->name, topic->name))
	    {
	        if (subscriber->topic_received_cb)
                    subscriber->topic_received_cb(subscriber->data, client, topic->name, value);
	    }

        }
    }
    free(value);
}



static void
_mqtt_sn_register_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Register_Msg *msg;
    EMqtt_Sn_Regack_Msg resp;
    EMqtt_Sn_Topic *topic;
    char *topic_name;
    size_t s;
    EMqtt_Sn_Subscriber *subscriber;

    msg = (EMqtt_Sn_Register_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqtt_Sn_Register_Msg));
    topic_name = calloc(1, s + 1);
    memcpy(topic_name, cdata->data + sizeof(EMqtt_Sn_Register_Msg) , s);

    topic = emqtt_topic_name_get(topic_name, client->topics);
    if (!topic)
        /* Create the new topic */
    {
        topic = emqtt_topic_new(topic_name, NULL);
        topic->id = htons(msg->topic_id);
        client->topics = eina_list_append(client->topics, topic);
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;

    }
    else
    {
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;
    }

    resp.header.len = sizeof(EMqtt_Sn_Regack_Msg);
    resp.header.msg_type = EMQTT_SN_REGACK;
    resp.msg_id = msg->msg_id;

    _mqtt_send_data(client, (const char*)&resp, resp.header.len);

}


static Eina_Bool _mqtt_client_data_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Small_Header *header;
    socklen_t len;
    Mqtt_Client_Data *cdata;

    cdata = calloc(1, sizeof(Mqtt_Client_Data));

    len = sizeof(cdata->client_addr);
    cdata->len = recvfrom(client->fd, cdata->data,READBUFSIZ, 0, (struct sockaddr *)&cdata->client_addr, &len);

    header = (EMqtt_Sn_Small_Header *)cdata->data; 

    printf("[<-] %s[%d]\n", mqttsn_msg_desc[header->msg_type].name, header->msg_type);

    // Header
    if (header->len == 0x01)
    {
        printf("Error long header not handle yet !\n");
        free(cdata);
        return ECORE_CALLBACK_RENEW;
    }

    switch(header->msg_type)
    {
    case EMQTT_SN_CONNACK:
        _mqtt_sn_connack_msg(client, cdata);
        break;
    case EMQTT_SN_PUBACK:
        _mqtt_sn_puback_msg(client, cdata);
        break;
    case EMQTT_SN_REGISTER:
        _mqtt_sn_register_msg(client, cdata);
        break;
    case EMQTT_SN_REGACK:
        _mqtt_sn_regack_msg(client, cdata);
        break;
    case EMQTT_SN_SUBACK:
        _mqtt_sn_suback_msg(client, cdata);
        break;
    case EMQTT_SN_PUBLISH:
        _mqtt_sn_client_publish_msg(client, cdata);
        break;
    case EMQTT_SN_PINGRESP:
        break;
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
    client->connection_state = CONNECTION_CLOSED;

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
       printf("udpclient error for %s:%d: %s", client->addr, client->port, gai_strerror(ret));
    }
    else
    {
        printf("res : %p %s\n", res->ai_addr, res->ai_canonname);
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

    if(client->connection_state != CONNECTION_IN_PROGRESS)
    {
        msg = (EMqtt_Sn_Connect_Msg *)d;
        msg->header.msg_type = EMQTT_SN_CONNECT;
        msg->flags = 0;
        msg->protocol_id = 1;
        msg->duration = htons((uint16_t)keepalive);
        client->connected_received_cb = connected_cb;
        client->data = data;
        client->keepalive = keepalive;
        snprintf(d + sizeof(EMqtt_Sn_Connect_Msg), sizeof(d) - sizeof(EMqtt_Sn_Connect_Msg), "%s", client->name);
        msg->header.len = sizeof(EMqtt_Sn_Connect_Msg) + strlen(client->name);
        client->timeout = ecore_timer_add(EMQTT_TIMEOUT_CONNECT, _mqtt_timeout_connect_cb, client);
        _mqtt_send_data(client->fd, msg, msg->header.len);
    }
}

void emqtt_sn_client_subscribe(EMqtt_Sn_Client *client, const char *topic_name, EMqtt_Sn_Client_Topic_Received_Cb topic_received_cb, EMqtt_Sn_Client_Subscribe_Error_Cb subscribe_error_cb, void *data)
{
    char d[256];
    EMqtt_Sn_Subscribe_Msg *msg;
    EMqtt_Sn_Topic *topic;
    EMqtt_Sn_Subscriber *subscriber;

    if (!topic_name)
        return;

    msg = (EMqtt_Sn_Subscribe_Msg *)d;

    msg->header.msg_type = EMQTT_SN_SUBSCRIBE;
    msg->flags = 0;
    msg->msg_id = client->last_msg_id++;
    snprintf(d + ((sizeof(EMqtt_Sn_Subscribe_Msg) - sizeof(uint16_t))), sizeof(d) - ((sizeof(EMqtt_Sn_Subscribe_Msg) - sizeof(uint16_t))), "%s", topic_name);
    msg->header.len = ((sizeof(EMqtt_Sn_Subscribe_Msg) - sizeof(uint16_t))) + strlen(topic_name);

    topic = emqtt_topic_name_get(topic_name, client->topics);
    if (!topic)
    {
        topic = emqtt_topic_new(topic_name, NULL);
        client->topics = eina_list_append(client->topics, topic);
    }

    subscriber = calloc(1, sizeof(EMqtt_Sn_Subscriber));
    subscriber->topic = topic;
    subscriber->topic_received_cb = topic_received_cb;
    subscriber->subscribe_error_cb = subscribe_error_cb;
    subscriber->data = data;
    subscriber->msg_id = msg->msg_id;
    client->subscribers = eina_list_append(client->subscribers, subscriber);

    _mqtt_send_data(client->fd, msg, msg->header.len);
}

static Eina_Bool
 _emqtt_publisher_exist(EMqtt_Sn_Client *client, const char *topic_name)
{
    Eina_List *l;
    EMqtt_Sn_Publisher *publisher;

    EINA_LIST_FOREACH(client->publishers, l, publisher)
    {
        if (!strcmp(topic_name, publisher->topic->name))
	  return EINA_TRUE;
    }
    return EINA_FALSE;
}


static EMqtt_Sn_Publisher *
_emqtt_get_publisher(EMqtt_Sn_Client *client, const char *topic_name)
{
    Eina_List *l;
    EMqtt_Sn_Publisher *publisher;

    EINA_LIST_FOREACH(client->publishers, l, publisher)
    {
        if (!strcmp(topic_name, publisher->topic->name))
	  return publisher;
    }
    return NULL;
}


static void emqtt_sn_client_register(EMqtt_Sn_Client *client, const char *topic_name, EMqtt_Sn_Client_Suback_Received_Cb suback_received_cb, void *data)
{
    char d[256];
    EMqtt_Sn_Register_Msg *msg;
    EMqtt_Sn_Topic *topic;
    EMqtt_Sn_Publisher *publisher;

    if (!topic_name)
        return;

    if (_emqtt_publisher_exist(client,topic_name))
      return;


    msg = (EMqtt_Sn_Register_Msg *)d;
    msg->header.msg_type = EMQTT_SN_REGISTER;
    msg->topic_id = 0;
    msg->msg_id = (client->last_msg_id)++;
    snprintf(d + sizeof(EMqtt_Sn_Register_Msg), sizeof(d) - sizeof(EMqtt_Sn_Register_Msg), "%s", topic_name);
    msg->header.len = sizeof(EMqtt_Sn_Register_Msg) + strlen(topic_name);

    topic = emqtt_topic_name_get(topic_name, client->topics);
    if (!topic)
    {
        topic = emqtt_topic_new(topic_name, NULL);
        client->topics = eina_list_append(client->topics, topic);
    }

    publisher = calloc(1, sizeof(EMqtt_Sn_Publisher));
    publisher->topic = topic;
    publisher->data = data;
    publisher->msg_id = msg->msg_id;
    publisher->suback_received_cb = suback_received_cb;
    publisher->register_state = REGISTER_IN_PROGRESS;
    client->publishers = eina_list_append(client->publishers, publisher);

    _mqtt_send_data(client->fd, msg, msg->header.len);
}


void emqtt_sn_client_send_publish(EMqtt_Sn_Client *client, const char *topic_name, const char *publish_data, EMqtt_Sn_Client_Suback_Received_Cb suback_received_cb, void *data)
{
    char d[256];
    EMqtt_Sn_Publish_Msg *msg;

    Eina_List *l;
    EMqtt_Sn_Publisher *publisher;

    if (!topic_name)
        return;

    emqtt_sn_client_register(client, topic_name, suback_received_cb, NULL);

    EINA_LIST_FOREACH(client->publishers, l, publisher)
    {
      if (!strcmp(topic_name, publisher->topic->name) && publisher->register_state == REGISTER_ACCEPTED){
	  /* printf("PUBLISH ID: %d\t name: %s\n",publisher->topic->id, publisher->topic->name); */
	  msg = (EMqtt_Sn_Publish_Msg *)d;
	  msg->header.msg_type = EMQTT_SN_PUBLISH;
	  msg->flags = 0;
	  msg->topic_id = publisher->topic->id;
	  msg->msg_id = (client->last_msg_id)++;
	  snprintf(d + sizeof(EMqtt_Sn_Publish_Msg), sizeof(d) - sizeof(EMqtt_Sn_Publish_Msg), "%s", publish_data);
	  msg->header.len = sizeof(EMqtt_Sn_Publish_Msg) + strlen(publish_data);

	  _mqtt_send_data(client->fd, msg, msg->header.len);
      }
    }

}
