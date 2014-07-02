#ifndef _EMQTT_PRIVATE_H
#define _EMQTT_PRIVATE_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "emqtt_topic.h"

typedef struct _EMqtt_Sn_Msg_Desc EMqtt_Sn_Msg_Desc;
typedef enum _EMqtt_Sn_MSG_TYPE EMqtt_Sn_MSG_TYPE;
typedef struct _EMqtt_Sn_Subscriber EMqtt_Sn_Subscriber;
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

extern const EMqtt_Sn_Msg_Desc mqttsn_msg_desc[];

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



struct _EMqtt_Sn_Subscriber
{
    EMqtt_Sn_Topic *topic;
    struct sockaddr_in6 client_addr;
    uint16_t msg_id;
    void (*topic_received_cb) (void *data, EMqtt_Sn_Client *client, const char *topic, const char *value);
    void (*subscribe_error_cb) (EMqtt_Sn_ERROR_TYPE state);
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

typedef struct _Server Server;

struct _Server
{
    int sdata;
};

#define READBUFSIZ 65536

#endif /* _EMQTT_PRIVATE_H */
