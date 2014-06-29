#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define READBUFSIZ 65536

#include "EMqtt.h"

typedef struct _EMqtt_Sn_Msg_Desc EMqtt_Sn_Msg_Desc;
typedef enum _EMqtt_Sn_MSG_TYPE EMqtt_Sn_MSG_TYPE;
typedef struct _EMqtt_Sn_Subscriber EMqtt_Sn_Subscriber;
typedef struct _EMqtt_Sn_Topic EMqtt_Sn_Topic;
typedef struct _Mqtt_Client_Data Mqtt_Client_Data;

enum _EMqtt_Sn_MSG_TYPE
{
    EMqtt_Sn_ADVERTISE,
    EMqtt_Sn_SEARCHGW,
    EMqtt_Sn_GWINFO,
    EMqtt_Sn_RESERVED1,
    EMqtt_Sn_CONNECT,
    EMqtt_Sn_CONNACK,
    EMqtt_Sn_WILLTOPICREQ,
    EMqtt_Sn_WILLTOPIC,
    EMqtt_Sn_WILLMSGREQ,
    EMqtt_Sn_WILLMSG,
    EMqtt_Sn_REGISTER,
    EMqtt_Sn_REGACK,
    EMqtt_Sn_PUBLISH,
    EMqtt_Sn_PUBACK,
    EMqtt_Sn_PUBCOMP,
    EMqtt_Sn_PUBREC,
    EMqtt_Sn_PUBREL,
    EMqtt_Sn_RESERVED2,
    EMqtt_Sn_SUBSCRIBE,
    EMqtt_Sn_SUBACK,
    EMqtt_Sn_UNSUBSCRIBE,
    EMqtt_Sn_UNSUBACK,
    EMqtt_Sn_PINGREQ,
    EMqtt_Sn_PINGRESP,
    EMqtt_Sn_DISCONNECT,
    EMqtt_Sn_RESERVED3,
    EMqtt_Sn_WILLTOPICUPD,
    EMqtt_Sn_WILLTOPICRESP,
    EMqtt_Sn_WILLMSGUPD,
    EMqtt_Sn_WILLMSGRESP
};

struct _EMqtt_Sn_Msg_Desc
{
    EMqtt_Sn_MSG_TYPE val;
    char *name;
};

const EMqtt_Sn_Msg_Desc mqttsn_msg_desc[] =
{
    {EMqtt_Sn_ADVERTISE,"ADVERTISE" },
    {EMqtt_Sn_SEARCHGW,"SEARCHGW"},
    {EMqtt_Sn_GWINFO,"GWINFO"},
    {EMqtt_Sn_RESERVED1,"RESERVED1"},
    {EMqtt_Sn_CONNECT,"CONNECT"},
    {EMqtt_Sn_CONNACK,"CONNACK"},
    {EMqtt_Sn_WILLTOPICREQ,"WILLTOPICREQ"},
    {EMqtt_Sn_WILLTOPIC, "WILLTOPIC"},
    {EMqtt_Sn_WILLMSGREQ,"WILLMSGREQ"},
    {EMqtt_Sn_WILLMSG,"WILLMSG"},
    {EMqtt_Sn_REGISTER,"REGISTER"},
    {EMqtt_Sn_REGACK,"REGACK"},
    {EMqtt_Sn_PUBLISH,"PUBLISH"},
    {EMqtt_Sn_PUBACK,"PUBACK"},
    {EMqtt_Sn_PUBCOMP,"PUBCOMP"},
    {EMqtt_Sn_PUBREC,"PUBREC"},
    {EMqtt_Sn_PUBREL,"PUBREL"},
    {EMqtt_Sn_RESERVED2,"RESERVED2"},
    {EMqtt_Sn_SUBSCRIBE,"SUBSCRIBE"},
    {EMqtt_Sn_SUBACK,"SUBACK"},
    {EMqtt_Sn_UNSUBSCRIBE,"UNSUBSCRIBE"},
    {EMqtt_Sn_UNSUBACK,"UNSUBACK"},
    {EMqtt_Sn_PINGREQ,"PINGREQ"},
    {EMqtt_Sn_PINGRESP,"PINGRESP"},
    {EMqtt_Sn_DISCONNECT,"DISCONNECT"},
    {EMqtt_Sn_RESERVED3,"RESERVED3"},
    {EMqtt_Sn_WILLTOPICUPD,"WILLTOPICUPD"},
    {EMqtt_Sn_WILLTOPICRESP,"WILLTOPICRESP"},
    {EMqtt_Sn_WILLMSGUPD,"WILLMSGUPD"},
    {EMqtt_Sn_WILLMSGRESP, "WILLMSGRESP"},
};

struct _EMqtt_Sn_Server
{
    const char *addr;
    unsigned short port;
    int fd;
    Eina_List *subscribers;
    Eina_List *topics;
    uint16_t last_topic;

};

struct _EMqtt_Sn_Client
{
    const char *addr;
    unsigned short port;
    int fd;
    struct sockaddr server_addr;
    const char *name;
    Ecore_Timer *keepalive_timer;
    double keepalive;
    Eina_List *subscribers;
    Eina_List *topics;
    uint16_t last_msg_id;
};

struct _Mqtt_Client_Data
{
    char data[65536];
    size_t len;
    struct sockaddr_in6 client_addr;

};

struct _EMqtt_Sn_Topic
{
    const char *name;
    uint16_t id;
};

struct _EMqtt_Sn_Subscriber
{
    EMqtt_Sn_Topic *topic;
    struct sockaddr_in6 client_addr;
    uint16_t msg_id;
    void (*topic_received_cb) (void *data, EMqtt_Sn_Client *client, const char *topic, const char *value);
    void *data;
};


typedef enum _EMqtt_Sn_RETURN_CODE EMqtt_Sn_RETURN_CODE;
typedef struct _EMqtt_Sn_Small_Header EMqtt_Sn_Small_Header;
typedef struct _EMqtt_Sn_Advertise_Msg EMqtt_Sn_Advertise_Msg;
typedef struct _EMqtt_Sn_Searchgw_Msg EMqtt_Sn_Searchgw_Msg;
typedef struct _EMqtt_Sn_Gwinfo_Msg EMqtt_Sn_Gwinfo_Msg;
typedef struct _EMqtt_Sn_Connect_Msg EMqtt_Sn_Connect_Msg;
typedef struct _EMqtt_Sn_Connack_Msg EMqtt_Sn_Connack_Msg;
typedef struct _EMqtt_Sn_Willtopicreq_Msg EMqtt_Sn_Willtopicreq_Msg;
typedef struct _EMqtt_Sn_Willtopic_Msg EMqtt_Sn_Willtopic_Msg;
typedef struct _EMqtt_Sn_Willmsgreq_Msg EMqtt_Sn_Willmsgreq_Msg;
typedef struct _EMqtt_Sn_Willmsg_Msg EMqtt_Sn_Willmsg_Msg;
typedef struct _EMqtt_Sn_Register_Msg EMqtt_Sn_Register_Msg;
typedef struct _EMqtt_Sn_Regack_Msg EMqtt_Sn_Regack_Msg;
typedef struct _EMqtt_Sn_Publish_Msg EMqtt_Sn_Publish_Msg;
typedef struct _EMqtt_Sn_Puback_Msg EMqtt_Sn_Puback_Msg;
typedef struct _EMqtt_Sn_Pubcomp_Msg EMqtt_Sn_Pubcomp_Msg;
typedef struct _EMqtt_Sn_Pubrec_Msg EMqtt_Sn_Pubrec_Msg;
typedef struct _EMqtt_Sn_Pubrel_Msg EMqtt_Sn_Pubrel_Msg;
typedef struct _EMqtt_Sn_Subscribe_Msg EMqtt_Sn_Subscribe_Msg;
typedef struct _EMqtt_Sn_Suback_Msg EMqtt_Sn_Suback_Msg;
typedef struct _EMqtt_Sn_Unsbuback_Msg EMqtt_Sn_Unsbuback_Msg;
typedef struct _EMqtt_Sn_Pingreq_Msg EMqtt_Sn_Pingreq_Msg;
typedef struct _EMqtt_Sn_Pingresp_Msg EMqtt_Sn_Pingresp_Msg;
typedef struct _EMqtt_Sn_Disconnect_Msg EMqtt_Sn_Disconnect_Msg;
typedef struct _EMqtt_Sn_Willtopicupd_Msg EMqtt_Sn_Willtopicupd_Msg;
typedef struct _EMqtt_Sn_Willtopicresp_Msg EMqtt_Sn_Willtopicresp_Msg;
typedef struct _EMqtt_Sn_Willmsgupd_Msg EMqtt_Sn_Willmsgupd_Msg;
typedef struct _EMqtt_Sn_Willmsgresp_Msg EMqtt_Sn_Willmsgresp_Msg;

enum _EMqtt_Sn_RETURN_CODE
{
    EMqtt_Sn_RETURN_CODE_ACCEPTED,
    EMqtt_Sn_RETURN_CODE_CONGESTION,
    EMqtt_Sn_RETURN_CODE_INVALID_TOPIC_ID,
    EMqtt_Sn_RETURN_CODE_NOT_SUPPORTED,
    EMqtt_Sn_RETURN_CODE_SENTINEL,
};

struct _EMqtt_Sn_Small_Header
{
    uint8_t len;
    uint8_t msg_type;
} __attribute__((packed));

struct _EMqtt_Sn_Advertise_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t gw_id;
    uint16_t duration;
} __attribute__((packed));

struct _EMqtt_Sn_Searchgw_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t radius;
} __attribute__((packed));

struct _EMqtt_Sn_Gwinfo_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t radius;
    char *data;
} __attribute__((packed));

struct _EMqtt_Sn_Connect_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    uint8_t protocol_id;
    uint16_t duration;
} __attribute__((packed));


struct _EMqtt_Sn_Connack_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqtt_Sn_Willtopicreq_Msg
{
    EMqtt_Sn_Small_Header header;
} __attribute__((packed));

struct _EMqtt_Sn_Willtopic_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    char *will_topic;
} __attribute__((packed));

struct _EMqtt_Sn_Willmsgreq_Msg
{
    EMqtt_Sn_Small_Header header;
} __attribute__((packed));

struct _EMqtt_Sn_Willmsg_Msg
{
    EMqtt_Sn_Small_Header header;
    char *will_msg;
} __attribute__((packed));


struct _EMqtt_Sn_Register_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Regack_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqtt_Sn_Publish_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Puback_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));


struct _EMqtt_Sn_Pubrec_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Pubrel_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Pubcomp_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Subscribe_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    uint16_t msg_id;
    uint16_t topic_id;
} __attribute__((packed));

struct _EMqtt_Sn_Suback_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t msg_id;
    uint8_t ret_code;
} __attribute__((packed));

struct _EMqtt_Sn_Unubscribe_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    uint16_t msg_id;
    char *topic_name;
} __attribute__((packed));

struct _EMqtt_Sn_Unsuback_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t msg_id;
} __attribute__((packed));

struct _EMqtt_Sn_Pingreq_Msg
{
    EMqtt_Sn_Small_Header header;
    char* client_id;
} __attribute__((packed));

struct _EMqtt_Sn_Pingresp_Msg
{
    EMqtt_Sn_Small_Header header;
} __attribute__((packed));

struct _EMqtt_Sn_Disconnect_Msg
{
    EMqtt_Sn_Small_Header header;
    uint16_t duration;
} __attribute__((packed));

struct _EMqtt_Sn_Willtopicupd_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t flags;
    char* will_topic;
} __attribute__((packed));

struct _EMqtt_Sn_Willmsgcupd_Msg
{
    EMqtt_Sn_Small_Header header;
    char* will_msg;
} __attribute__((packed));

struct _EMqtt_Sn_Willtopicresp_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));

struct _EMqtt_Sn_Willmsgresp_Msg
{
    EMqtt_Sn_Small_Header header;
    uint8_t ret_code;
} __attribute__((packed));

static int _EMqttinit_count = 0;


typedef struct _Server Server;

struct _Server
{
    int sdata;
};

static EMqtt_Sn_Topic *
_mqtt_topic_name_get(const char *topic_name, Eina_List *topics)
{
    Eina_List *l;
    EMqtt_Sn_Topic *topic;

    EINA_LIST_FOREACH(topics, l, topic)
    {
        if (topic && !strcmp(topic_name, topic->name))
            return topic;
    }
    return NULL;
}

static EMqtt_Sn_Topic *
_mqtt_topic_id_get(uint16_t topic_id, Eina_List *topics)
{
    Eina_List *l;
    EMqtt_Sn_Topic *topic = NULL;

    EINA_LIST_FOREACH(topics, l, topic)
    {
        if (topic_id == topic->id)
            return topic;
    }
    return NULL;
}

static Eina_Bool
_mqtt_subscriber_name_exists(Mqtt_Client_Data *cdata, const char *name, Eina_List *subscribers)
{
    Eina_List *l;
    EMqtt_Sn_Subscriber *subscriber;

    EINA_LIST_FOREACH(subscribers, l, subscriber)
    {
        if (memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
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
        if (memcmp((void*)&cdata->client_addr, (void*)&subscriber->client_addr, sizeof(cdata->client_addr)) &&
                id == subscriber->topic->id)
            return EINA_TRUE;
    }
    return EINA_FALSE;
}

static EMqtt_Sn_Topic *
_mqtt_topic_new(const char *name, uint16_t *next_topic)
{
    EMqtt_Sn_Topic *topic;

    topic = calloc(1, sizeof(EMqtt_Sn_Topic));
    if (next_topic)
            topic->id = *next_topic++;
    topic->name = eina_stringshare_add(name);
    return topic;
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

    topic = _mqtt_topic_name_get(topic_name, srv->topics);
    if (!topic)
        /* Create the new topic */
    {
        topic = _mqtt_topic_new(topic_name, &srv->last_topic);
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

    topic = _mqtt_topic_name_get(topic_name, srv->topics);
    if (!topic)
    {
        topic = _mqtt_topic_new(topic_name, &srv->last_topic);
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

static Eina_Bool
_mqtt_keepalive_timer_cb(void *data)
{
    EMqtt_Sn_Client *client = data;
    EMqtt_Sn_Pingreq_Msg msg;

    msg.header.len = 2;
    msg.header.msg_type = EMqtt_Sn_PINGREQ;

    send(client->fd, &msg, msg.header.len, 0);
}

static void
_mqtt_sn_connack_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Connack_Msg *msg;

    msg = (EMqtt_Sn_Connack_Msg *)cdata->data;

    if (msg->ret_code != EMqtt_Sn_RETURN_CODE_ACCEPTED)
    {
        printf("Error : connection not accepted by server\n");
        return;
    }

    /* Client now accepted, create a timer to launch Ping request each keepalive seconds */
    client->keepalive_timer = ecore_timer_add(client->keepalive, _mqtt_keepalive_timer_cb, client);
}

static void
_mqtt_sn_suback_msg(EMqtt_Sn_Client *client, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Suback_Msg *msg;
    EMqtt_Sn_Subscriber *subscriber;
    Eina_List *l;

    msg = (EMqtt_Sn_Suback_Msg *)cdata->data;

    if (msg->ret_code != EMqtt_Sn_RETURN_CODE_ACCEPTED)
    {
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

    msg = (EMqtt_Sn_Connect_Msg *)d;
    msg->header.msg_type = EMqtt_Sn_CONNECT;
    msg->flags = 0;
    msg->protocol_id = 1;
    msg->duration = htons((uint16_t)keepalive);
    client->keepalive = keepalive;
    snprintf(d + sizeof(msg), sizeof(d) - sizeof(msg), "%s", client->name);
    msg->header.len = sizeof(msg) + strlen(client->name);
    send(client->fd, msg, msg->header.len, 0);
    printf("Send %d bytes to %s\n", msg->header.len, client->server_addr.sa_data);
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

    topic = _mqtt_topic_name_get(topic_name, client->topics);
    if (!topic)
    {
        topic = _mqtt_topic_new(topic_name, NULL);
        client->topics = eina_list_append(client->topics, topic);
    }

    subscriber = calloc(1, sizeof(EMqtt_Sn_Subscriber));
    subscriber->topic = topic;
    subscriber->topic_received_cb = topic_received_cb;
    subscriber->data = data;
    subscriber->msg_id = msg->msg_id;
    client->subscribers = eina_list_append(client->subscribers, subscriber);

    send(client->fd, msg, msg->header.len, 0);
}

