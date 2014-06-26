#include <unistd.h>
#include <fcntl.h>

#include "EMqtt.h"

typedef struct _EMQTTSN_Msg_Desc EMQTTSN_Msg_Desc;
typedef enum _EMQTTSN_MSG_TYPE EMQTTSN_MSG_TYPE;
typedef struct _EMqttSn_Subscriber EMqttSn_Subscriber;
typedef struct _EMqttSn_Topic EMqttSn_Topic;
typedef struct _Mqtt_Client_Data Mqtt_Client_Data;

enum _EMQTTSN_MSG_TYPE
{
    EMQTTSN_ADVERTISE,
    EMQTTSN_SEARCHGW,
    EMQTTSN_GWINFO,
    EMQTTSN_RESERVED1,
    EMQTTSN_CONNECT,
    EMQTTSN_CONNACK,
    EMQTTSN_WILLTOPICREQ,
    EMQTTSN_WILLTOPIC,
    EMQTTSN_WILLMSGREQ,
    EMQTTSN_WILLMSG,
    EMQTTSN_REGISTER,
    EMQTTSN_REGACK,
    EMQTTSN_PUBLISH,
    EMQTTSN_PUBACK,
    EMQTTSN_PUBCOMP,
    EMQTTSN_PUBREC,
    EMQTTSN_PUBREL,
    EMQTTSN_RESERVED2,
    EMQTTSN_SUBSCRIBE,
    EMQTTSN_SUBACK,
    EMQTTSN_UNSUBSCRIBE,
    EMQTTSN_UNSUBACK,
    EMQTTSN_PINGREQ,
    EMQTTSN_PINGRESP,
    EMQTTSN_DISCONNECT,
    EMQTTSN_RESERVED3,
    EMQTTSN_WILLTOPICUPD,
    EMQTTSN_WILLTOPICRESP,
    EMQTTSN_WILLMSGUPD,
    EMQTTSN_WILLMSGRESP
};

struct _EMQTTSN_Msg_Desc
{
    EMQTTSN_MSG_TYPE val;
    char *name;
};

const EMQTTSN_Msg_Desc mqttsn_msg_desc[] =
{
    {EMQTTSN_ADVERTISE,"ADVERTISE" },
    {EMQTTSN_SEARCHGW,"SEARCHGW"},
    {EMQTTSN_GWINFO,"GWINFO"},
    {EMQTTSN_RESERVED1,"RESERVED1"},
    {EMQTTSN_CONNECT,"CONNECT"},
    {EMQTTSN_CONNACK,"CONNACK"},
    {EMQTTSN_WILLTOPICREQ,"WILLTOPICREQ"},
    {EMQTTSN_WILLTOPIC, "WILLTOPIC"},
    {EMQTTSN_WILLMSGREQ,"WILLMSGREQ"},
    {EMQTTSN_WILLMSG,"WILLMSG"},
    {EMQTTSN_REGISTER,"REGISTER"},
    {EMQTTSN_REGACK,"REGACK"},
    {EMQTTSN_PUBLISH,"PUBLISH"},
    {EMQTTSN_PUBACK,"PUBACK"},
    {EMQTTSN_PUBCOMP,"PUBCOMP"},
    {EMQTTSN_PUBREC,"PUBREC"},
    {EMQTTSN_PUBREL,"PUBREL"},
    {EMQTTSN_RESERVED2,"RESERVED2"},
    {EMQTTSN_SUBSCRIBE,"SUBSCRIBE"},
    {EMQTTSN_SUBACK,"SUBACK"},
    {EMQTTSN_UNSUBSCRIBE,"UNSUBSCRIBE"},
    {EMQTTSN_UNSUBACK,"UNSUBACK"},
    {EMQTTSN_PINGREQ,"PINGREQ"},
    {EMQTTSN_PINGRESP,"PINGRESP"},
    {EMQTTSN_DISCONNECT,"DISCONNECT"},
    {EMQTTSN_RESERVED3,"RESERVED3"},
    {EMQTTSN_WILLTOPICUPD,"WILLTOPICUPD"},
    {EMQTTSN_WILLTOPICRESP,"WILLTOPICRESP"},
    {EMQTTSN_WILLMSGUPD,"WILLMSGUPD"},
    {EMQTTSN_WILLMSGRESP, "WILLMSGRESP"},
};

struct _EMqttSn_Server
{
    const char *addr;
    unsigned short port;
    Eina_List *subscribers;
    Eina_List *topics;
    uint16_t last_topic;
    int fd;
};

struct _Mqtt_Client_Data
{
    char data[65536];
    size_t len;
    struct sockaddr_in6 client_addr;

};

struct _EMqttSn_Topic
{
    const char *name;
    uint16_t id;
};

struct _EMqttSn_Subscriber
{
    EMqttSn_Topic *topic;
    struct sockaddr_in6 client_addr;
};


typedef enum _EMqttSN_RETURN_CODE EMqttSN_RETURN_CODE;
typedef struct _EMqttSn_Small_Header EMqttSn_Small_Header;
typedef struct _EMqttSn_Advertise_Msg EMqttSn_Advertise_Msg;
typedef struct _EMqttSn_Searchgw_Msg EMqttSn_Searchgw_Msg;
typedef struct _EMqttSn_Gwinfo_Msg EMqttSn_Gwinfo_Msg;
typedef struct _EMqttSn_Connect_Msg EMqttSn_Connect_Msg;
typedef struct _EMqttSn_Connack_Msg EMqttSn_Connack_Msg;
typedef struct _EMqttSn_Willtopicreq_Msg EMqttSn_Willtopicreq_Msg;
typedef struct _EMqttSn_Willtopic_Msg EMqttSn_Willtopic_Msg;
typedef struct _EMqttSn_Willmsgreq_Msg EMqttSn_Willmsgreq_Msg;
typedef struct _EMqttSn_Willmsg_Msg EMqttSn_Willmsg_Msg;
typedef struct _EMqttSn_Register_Msg EMqttSn_Register_Msg;
typedef struct _EMqttSn_Regack_Msg EMqttSn_Regack_Msg;
typedef struct _EMqttSn_Publish_Msg EMqttSn_Publish_Msg;
typedef struct _EMqttSn_Puback_Msg EMqttSn_Puback_Msg;
typedef struct _EMqttSn_Pubcomp_Msg EMqttSn_Pubcomp_Msg;
typedef struct _EMqttSn_Pubrec_Msg EMqttSn_Pubrec_Msg;
typedef struct _EMqttSn_Pubrel_Msg EMqttSn_Pubrel_Msg;
typedef struct _EMqttSn_Subscribe_Msg EMqttSn_Subscribe_Msg;
typedef struct _EMqttSn_Suback_Msg EMqttSn_Suback_Msg;
typedef struct _EMqttSn_Unsbuback_Msg EMqttSn_Unsbuback_Msg;
typedef struct _EMqttSn_Pingreq_Msg EMqttSn_Pingreq_Msg;
typedef struct _EMqttSn_Pingresp_Msg EMqttSn_Pingresp_Msg;
typedef struct _EMqttSn_Disconnect_Msg EMqttSn_Disconnect_Msg;
typedef struct _EMqttSn_Willtopicupd_Msg EMqttSn_Willtopicupd_Msg;
typedef struct _EMqttSn_Willtopicresp_Msg EMqttSn_Willtopicresp_Msg;
typedef struct _EMqttSn_Willmsgupd_Msg EMqttSn_Willmsgupd_Msg;
typedef struct _EMqttSn_Willmsgresp_Msg EMqttSn_Willmsgresp_Msg;

enum _EMqttSN_RETURN_CODE
{
    EMqttSN_RETURN_CODE_ACCEPTED,
    EMqttSN_RETURN_CODE_CONGESTION,
    EMqttSN_RETURN_CODE_INVALID_TOPIC_ID,
    EMqttSN_RETURN_CODE_NOT_SUPPORTED,
    EMqttSN_RETURN_CODE_SENTINEL,
};

struct _EMqttSn_Small_Header
{
    uint8_t len;
    uint8_t msg_type;
} __attribute__((packed));

struct _EMqttSn_Advertise_Msg
{
    EMqttSn_Small_Header header;
    uint8_t gw_id;
    uint16_t duration;
} __attribute__((packed));

struct _EMqttSn_Searchgw_Msg
{
    EMqttSn_Small_Header header;
    uint8_t radius;
} __attribute__((packed));

struct _EMqttSn_Gwinfo_Msg
{
    EMqttSn_Small_Header header;
    uint8_t radius;
    char *data;
} __attribute__((packed));

struct _EMqttSn_Connect_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    uint8_t protocol_id;
    uint16_t duration;
} __attribute__((packed));


struct _EMqttSn_Connack_Msg
{
    EMqttSn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqttSn_Willtopicreq_Msg
{
    EMqttSn_Small_Header header;
} __attribute__((packed));

struct _EMqttSn_Willtopic_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    char *will_topic;
} __attribute__((packed));

struct _EMqttSn_Willmsgreq_Msg
{
    EMqttSn_Small_Header header;
} __attribute__((packed));

struct _EMqttSn_Willmsg_Msg
{
    EMqttSn_Small_Header header;
    char *will_msg;
} __attribute__((packed));


struct _EMqttSn_Register_Msg
{
    EMqttSn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Regack_Msg
{
    EMqttSn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqttSn_Publish_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Puback_Msg
{
    EMqttSn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqttSn_Pubrec_Msg
{
    EMqttSn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Pubrel_Msg
{
    EMqttSn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Pubcomp_Msg
{
    EMqttSn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Subscribe_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    uint16_t msg_id;
    uint16_t topic_id;
} __attribute__((packed));

struct _EMqttSn_Suback_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));

struct _EMqttSn_Unubscribe_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    uint16_t msg_id;
    char *topic_name;
} __attribute__((packed));

struct _EMqttSn_Unsuback_Msg
{
    EMqttSn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqttSn_Pingreq_Msg
{
    EMqttSn_Small_Header header;
    char* client_id;
} __attribute__((packed));

struct _EMqttSn_Pingresp_Msg
{
    EMqttSn_Small_Header header;
} __attribute__((packed));

struct _EMqttSn_Disconnect_Msg
{
    EMqttSn_Small_Header header;
    uint16_t duration;
} __attribute__((packed));

struct _EMqttSn_Willtopicupd_Msg
{
    EMqttSn_Small_Header header;
    uint8_t flags;
    char* will_topic;
} __attribute__((packed));

struct _EMqttSn_Willmsgcupd_Msg
{
    EMqttSn_Small_Header header;
    char* will_msg;
} __attribute__((packed));

struct _EMqttSn_Willtopicresp_Msg
{
    EMqttSn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));

struct _EMqttSn_Willmsgresp_Msg
{
    EMqttSn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));

static int _EMqttinit_count = 0;


typedef struct _Server Server;

struct _Server
{
    int sdata;
};

static EMqttSn_Topic *
_mqtt_topic_name_get(const char *topic_name, EMqttSn_Server *srv)
{
    Eina_List *l;
    EMqttSn_Topic *topic;

    EINA_LIST_FOREACH(srv->topics, l, topic)
    {
        if (topic && !strcmp(topic_name, topic->name))
            return topic;
    }
    return NULL;
}

static EMqttSn_Topic *
_mqtt_topic_id_get(uint16_t topic_id, EMqttSn_Server *srv)
{
    Eina_List *l;
    EMqttSn_Topic *topic = NULL;

    EINA_LIST_FOREACH(srv->topics, l, topic)
    {
        if (topic_id == topic->id)
            return topic;
    }
    return NULL;
}

static Eina_Bool
_mqtt_subscriber_name_exists(Mqtt_Client_Data *cdata, const char *name, EMqttSn_Server *srv)
{
    Eina_List *l;
    EMqttSn_Subscriber *subscriber;

    EINA_LIST_FOREACH(srv->subscribers, l, subscriber)
    {
        if (memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
                !strcmp(name, subscriber->topic->name))
            return EINA_TRUE;
    }
    return EINA_FALSE;
}

static Eina_Bool
_mqtt_subscriber_id_exists(Mqtt_Client_Data *cdata, uint16_t id, EMqttSn_Server *srv)
{
    Eina_List *l;
    EMqttSn_Subscriber *subscriber;

    EINA_LIST_FOREACH(srv->subscribers, l, subscriber)
    {
        if (memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
                id == subscriber->topic->id)
            return EINA_TRUE;
    }
    return EINA_FALSE;
}

static EMqttSn_Topic *
_mqtt_topic_new(const char *name, EMqttSn_Server *srv)
{
    EMqttSn_Topic *topic;

    topic = calloc(1, sizeof(EMqttSn_Topic));
    topic->id = srv->last_topic++;
    topic->name = eina_stringshare_add(name);
    return topic;
}


static void
_mqtt_sn_connect_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Connect_Msg *msg;
    EMqttSn_Connack_Msg resp;
    char *client_id;
    size_t s;

    msg = (EMqttSn_Connect_Msg *)cdata->data;
    s = msg->header.len - (sizeof(EMqttSn_Connect_Msg));
    client_id = calloc(1, s + 1);
    memcpy(client_id, cdata->data + sizeof(EMqttSn_Connect_Msg) , s);

    resp.header.len = 0x03;
    resp.header.msg_type = EMQTTSN_CONNACK;
    resp.ret_code = EMqttSN_RETURN_CODE_ACCEPTED;

    sendto(srv->fd, &resp, sizeof(resp), 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_register_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Register_Msg *msg;
    EMqttSn_Regack_Msg resp;
    EMqttSn_Topic *topic;
    Eina_List *l;
    char *topic_name;
    size_t s;
    Eina_Bool found = EINA_FALSE;

    msg = (EMqttSn_Register_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqttSn_Register_Msg));
    topic_name = calloc(1, s + 1);
    memcpy(topic_name, cdata->data + sizeof(EMqttSn_Register_Msg) , s);

    topic = _mqtt_topic_name_get(topic_name, srv);
    if (!topic)
        /* Create the new topic */
    {
        topic = _mqtt_topic_new(topic_name, srv);
        srv->topics = eina_list_append(srv->topics, topic);
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMqttSN_RETURN_CODE_ACCEPTED;
    }
    else
    {
        resp.topic_id = htons(topic->id);
        resp.ret_code = EMqttSN_RETURN_CODE_ACCEPTED;
    }

    resp.header.len = sizeof(EMqttSn_Regack_Msg);
    resp.header.msg_type = EMQTTSN_REGACK;
    resp.msg_id = msg->msg_id;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_publish_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Publish_Msg *msg;
    EMqttSn_Puback_Msg resp;
    Eina_List *l;
    EMqttSn_Subscriber *subscriber;
    char *data;
    size_t s;

    msg = (EMqttSn_Publish_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqttSn_Publish_Msg));
    data = calloc(1, s + 1);
    memcpy(data, cdata->data + sizeof(EMqttSn_Publish_Msg) , s);

    resp.header.len = sizeof(EMqttSn_Puback_Msg);
    resp.header.msg_type = EMQTTSN_PUBACK;
    resp.topic_id = msg->topic_id;
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqttSN_RETURN_CODE_ACCEPTED;


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
_mqtt_sn_pingreq_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Pingresp_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMQTTSN_PINGRESP;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_disconnect_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Disconnect_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMQTTSN_DISCONNECT;

    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_subscribe_msg(EMqttSn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqttSn_Subscribe_Msg *msg;
    EMqttSn_Suback_Msg resp;
    uint8_t topic_id_type;
    char *topic_name = NULL;
    uint16_t topic_id;
    Eina_List *l;
    EMqttSn_Subscriber *subscriber;
    EMqttSn_Topic *topic;
    Eina_Bool found = EINA_FALSE;

    msg = (EMqttSn_Subscribe_Msg *)cdata->data;
    topic_id_type = msg->flags & 0x03;
    switch(topic_id_type)
    {
    case 0:
    {
        /* Topic name */
        size_t s;

        s = msg->header.len - ((sizeof(EMqttSn_Subscribe_Msg) - sizeof(uint16_t)));
        topic_name = calloc(1, s + 1);
        memcpy(topic_name, cdata->data + sizeof(EMqttSn_Subscribe_Msg) - sizeof(uint16_t) , s);
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

    topic = _mqtt_topic_name_get(topic_name, srv);
    if (!topic)
    {
        topic = _mqtt_topic_new(topic_name, srv);
        srv->topics = eina_list_append(srv->topics, topic);
    }

    if (!_mqtt_subscriber_name_exists(cdata, topic_name, srv))
    {
        subscriber = calloc(1, sizeof(EMqttSn_Subscriber));
        subscriber->topic = topic;
        subscriber->client_addr = cdata->client_addr;
        srv->subscribers = eina_list_append(srv->subscribers, subscriber);
    }

    resp.header.len = sizeof(EMqttSn_Suback_Msg);
    resp.header.msg_type = EMQTTSN_SUBACK;
    resp.flags = 0x00;
    resp.topic_id = htons(topic->id);
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqttSN_RETURN_CODE_ACCEPTED;
    sendto(srv->fd, &resp, resp.header.len, 0, (struct sockaddr *)&cdata->client_addr, sizeof(cdata->client_addr));
}


int emqtt_init(void)
{
    if (++_EMqttinit_count != 1)
        return _EMqttinit_count;


    if (!eina_init ())
        return --_EMqttinit_count;

    if (!ecore_init ())
        return --_EMqttinit_count;

    if (!ecore_con_init ())
        return --_EMqttinit_count;

    return _EMqttinit_count;

}

int emqtt_shutdown(void)
{
    if (--_EMqttinit_count != 0)
        return _EMqttinit_count;

    ecore_con_shutdown();
    ecore_shutdown();
    eina_shutdown();

    return _EMqttinit_count;
}

static Eina_Bool
_timer_cb(void *data)
{
    EMqttSn_Server *srv = data;
    Eina_List *l;
    EMqttSn_Subscriber *subscriber;


    printf("timer\n");
    EINA_LIST_FOREACH(srv->subscribers, l, subscriber)
    {
        EMqttSn_Pingreq_Msg msg;
        msg.header.len = 2;
        msg.header.msg_type = EMQTTSN_PINGREQ;

        sendto(srv->fd, &msg, msg.header.len, 0, (struct sockaddr *)&subscriber->client_addr, sizeof(subscriber->client_addr));

    }
    return EINA_TRUE;
}

static Eina_Bool _mqtt_server_data_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
    EMqttSn_Server *srv = data;
    EMqttSn_Small_Header *header;
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


    header = (EMqttSn_Small_Header *)cdata->data;

    d = cdata->data;

    printf("Receive Message : %s[%d]\n", mqttsn_msg_desc[header->msg_type].name, header->msg_type);

    // Header
    if (header->len == 0x01)
    {
        printf("Error long header not handle yet !\n");
        return ECORE_CALLBACK_RENEW;
    }

    switch(header->msg_type)
    {
    case EMQTTSN_CONNECT:
        _mqtt_sn_connect_msg(srv, cdata);
        break;
    case EMQTTSN_REGISTER:
        _mqtt_sn_register_msg(srv, cdata);
        break;
    case EMQTTSN_PUBLISH:
        _mqtt_sn_publish_msg(srv, cdata);
        break;
    case EMQTTSN_PINGREQ:
        _mqtt_sn_pingreq_msg(srv, cdata);
        break;
    case EMQTTSN_SUBSCRIBE:
        _mqtt_sn_subscribe_msg(srv, cdata);
        break;
    case EMQTTSN_DISCONNECT:
        _mqtt_sn_disconnect_msg(srv, cdata);
        break;
    default:
        printf("Unknown message\n");
        break;
    }

    free(cdata);

    return ECORE_CALLBACK_RENEW;
}


EMqttSn_Server *emqtt_sn_server_add(char *addr, unsigned short port)
{
    EMqttSn_Server *srv;
    struct sockaddr_in6 servaddr;
    int optval;
    int flags;

    if (!addr || !port)
        return NULL;

    srv = calloc(1, sizeof(EMqttSn_Server));
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

void emqtt_sn_server_del(EMqttSn_Server *srv)
{
    if (!srv)
        return;

    if (srv->addr)
        eina_stringshare_del(srv->addr);

    free(srv);
}
