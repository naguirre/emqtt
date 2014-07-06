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
typedef struct _EMqtt_Sn_Connected_Client EMqtt_Sn_Connected_Client;

enum _EMqtt_Sn_MSG_TYPE
{
    EMQTT_SN_ADVERTISE,
    EMQTT_SN_SEARCHGW,
    EMQTT_SN_GWINFO,
    EMQTT_SN_RESERVED1,
    EMQTT_SN_CONNECT,
    EMQTT_SN_CONNACK,
    EMQTT_SN_WILLTOPICREQ,
    EMQTT_SN_WILLTOPIC,
    EMQTT_SN_WILLMSGREQ,
    EMQTT_SN_WILLMSG,
    EMQTT_SN_REGISTER,
    EMQTT_SN_REGACK,
    EMQTT_SN_PUBLISH,
    EMQTT_SN_PUBACK,
    EMQTT_SN_PUBCOMP,
    EMQTT_SN_PUBREC,
    EMQTT_SN_PUBREL,
    EMQTT_SN_RESERVED2,
    EMQTT_SN_SUBSCRIBE,
    EMQTT_SN_SUBACK,
    EMQTT_SN_UNSUBSCRIBE,
    EMQTT_SN_UNSUBACK,
    EMQTT_SN_PINGREQ,
    EMQTT_SN_PINGRESP,
    EMQTT_SN_DISCONNECT,
    EMQTT_SN_RESERVED3,
    EMQTT_SN_WILLTOPICUPD,
    EMQTT_SN_WILLTOPICRESP,
    EMQTT_SN_WILLMSGUPD,
    EMQTT_SN_WILLMSGRESP,
    EMQTT_SN_SENTINEL
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
    unsigned char gw_id;
    int fd6;
    int fd4;
    Eina_List *connected_clients;
    Ecore_Timer *advertise_timer;
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
    EMqtt_Sn_CONNECTION_STATE connection_state;
    int connection_retry;
    Ecore_Timer *timeout;
    void (*connected_received_cb) (void *data, EMqtt_Sn_Client *client, EMqtt_Sn_CONNECTION_STATE connection_state);
    void *data;
};

struct _EMqtt_Sn_Connected_Client
{
    const char *client_id;
    struct sockaddr_storage addr;
    int fd;
    Eina_List *subscribers;
    Eina_List *topics;
    uint16_t last_topic;
};

struct _Mqtt_Client_Data
{
    char data[65536];
    size_t len;
    int fd;
    struct sockaddr_storage client_addr;
};

struct _EMqtt_Sn_Subscriber
{
    EMqtt_Sn_Topic *topic;
    struct sockaddr_storage client_addr;
    uint16_t msg_id;
    void (*topic_received_cb) (void *data, EMqtt_Sn_Client *client, const char *topic, const char *value);
    void (*subscribe_error_cb) (void *data, EMQTT_SN_ERROR_TYPE state);
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
    EMQTT_SN_RETURN_CODE_ACCEPTED,
    EMQTT_SN_RETURN_CODE_CONGESTION,
    EMQTT_SN_RETURN_CODE_INVALID_TOPIC_ID,
    EMQTT_SN_RETURN_CODE_NOT_SUPPORTED,
    EMQTT_SN_RETURN_CODE_SENTINEL,
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
    uint8_t gw_id;
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
#define MQTT_T_ADV (15 * 60.0) /* greater than 15 minutes */

#endif /* _EMQTT_PRIVATE_H */
