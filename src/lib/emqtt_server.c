#include "EMqtt.h"
#include "emqtt_private.h"


const char *_get_port(const struct sockaddr *addr)
{
    char clientservice[NI_MAXSERV];

    getnameinfo(addr, sizeof(struct sockaddr_storage),
                NULL, 0,
                clientservice, sizeof(clientservice),
                NI_NUMERICHOST);

    return eina_stringshare_add(clientservice);
}

const char *_get_ip(const struct sockaddr *addr)
{
    char clienthost[NI_MAXHOST];

    getnameinfo(addr, sizeof(struct sockaddr_storage),
                clienthost, sizeof(clienthost),
                NULL, 0,
                NI_NUMERICHOST);

    return eina_stringshare_add(clienthost);
}


static Eina_Bool
_mqtt_send_data(EMqtt_Sn_Connected_Client *cl, const char *data, int len)
{
    EMqtt_Sn_Small_Header *header = (EMqtt_Sn_Small_Header *)data;

    if (!cl)
    {
        printf("Try to send data to unknown client\n");
        return EINA_FALSE;
    }

   /*printf("data[0][1] 0x%02X 0x%02X\n", data[0], data[1]);
    printf("HEader : %s %d\n", mqttsn_msg_desc[header->msg_type].name, header->len);*/

    printf("[->] %s[%d]\t\t %s [%s:%s]\n",
           mqttsn_msg_desc[header->msg_type].name, header->msg_type,
           cl ? cl->client_id : "Unknown client",
           _get_ip((struct sockaddr*)&cl->addr),
           _get_port((struct sockaddr*)&cl->addr));

    sendto(cl->fd, data, len, 0,
           (struct sockaddr*)&cl->addr, sizeof(cl->addr));
    return EINA_TRUE;
}

static void
_mqtt_sn_connect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Connect_Msg *msg;
    EMqtt_Sn_Connack_Msg resp;
    size_t s;

    msg = (EMqtt_Sn_Connect_Msg *)cdata->data;
    s = msg->header.len - (sizeof(EMqtt_Sn_Connect_Msg));

    if (!cl)
    {
        cl = calloc(1, sizeof(EMqtt_Sn_Connected_Client));
        srv->connected_clients = eina_list_append(srv->connected_clients, cl);
        cl->client_id = eina_stringshare_nprintf(s, "%s", cdata->data + sizeof(EMqtt_Sn_Connect_Msg));
        memcpy(&cl->addr, &cdata->client_addr, sizeof(struct sockaddr_storage));
        cl->fd = cdata->fd;
    }
    else
    {
        printf("Try to reconnect a known client ? \n");
    }

    resp.header.len = 0x03;
    resp.header.msg_type = EMqtt_Sn_CONNACK;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;  

    _mqtt_send_data(cl, (const char*)&resp, sizeof(resp));
}

static void
_mqtt_sn_register_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Register_Msg *msg;
    EMqtt_Sn_Regack_Msg resp;
    EMqtt_Sn_Topic *topic;
    char *topic_name;
    size_t s;    

    msg = (EMqtt_Sn_Register_Msg*)cdata->data;

    s = msg->header.len - (sizeof(EMqtt_Sn_Register_Msg));
    topic_name = calloc(1, s + 1);
    memcpy(topic_name, cdata->data + sizeof(EMqtt_Sn_Register_Msg) , s);

    topic = emqtt_topic_name_get(topic_name, cl->topics);
    if (!topic)
        /* Create the new topic */
    {
        topic = emqtt_topic_new(topic_name, &cl->last_topic);
        cl->topics = eina_list_append(cl->topics, topic);
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

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);
}

static void
_mqtt_sn_publish_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Publish_Msg *msg;
    EMqtt_Sn_Puback_Msg resp;
    Eina_List *l;
    EMqtt_Sn_Connected_Client *con_cli;
    EMqtt_Sn_Topic *topic;
    const char* topic_name = NULL;
    char *data;
    size_t s;

    msg = (EMqtt_Sn_Publish_Msg*)cdata->data;

    if (!cl)
    {
        printf("Error : this client is not connected");
        return;
    }

    s = msg->header.len - (sizeof(EMqtt_Sn_Publish_Msg));
    data = calloc(1, s + 1);
    memcpy(data, cdata->data + sizeof(EMqtt_Sn_Publish_Msg) , s);

    resp.header.len = sizeof(EMqtt_Sn_Puback_Msg);
    resp.header.msg_type = EMqtt_Sn_PUBACK;
    resp.topic_id = msg->topic_id;
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);

    topic = emqtt_topic_id_get(htons(msg->topic_id), cl->topics);
    if (!topic || !topic->name)
    {
        printf("Topic is NULL !");
    }
    else
    {
        printf("Publish %s with value %s\n", topic->name, data);
        topic_name = topic->name;
    }

    EINA_LIST_FOREACH(srv->connected_clients, l, con_cli)
    {
        EMqtt_Sn_Publish_Msg *pub_msg = msg;

        //pub_msg.header.len = sizeof(pub_msg);
        //pub_msg.header.msg_type = EMqtt_Sn_PUBLISH;
        //pub_msg.msg_id = 0x00;
        //pub_msg.flags = 0x00;

        topic = emqtt_topic_name_get(topic_name, con_cli->topics);
        if (!topic)
            continue;

        printf("Find topic : %s[%d] %d\n", topic->name, topic->id, topic->subscribed);
        if (topic->subscribed)
        {
            pub_msg->topic_id = htons(topic->id);
            _mqtt_send_data(con_cli, (const char*)pub_msg, pub_msg->header.len);
        }
    }

}

static void
_mqtt_sn_pingreq_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Pingresp_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMqtt_Sn_PINGRESP;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);
}

static void
_mqtt_sn_disconnect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Disconnect_Msg resp;

    resp.header.len = 2;
    resp.header.msg_type = EMqtt_Sn_DISCONNECT;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);    
}

static void
_mqtt_sn_subscribe_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Subscribe_Msg *msg;
    EMqtt_Sn_Suback_Msg resp;
    uint8_t topic_id_type;
    char *topic_name = NULL;  
    EMqtt_Sn_Topic *topic;

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

    topic = emqtt_topic_name_get(topic_name, cl->topics);
    if (!topic)
    {
        topic = emqtt_topic_new(topic_name, &cl->last_topic);
        cl->topics = eina_list_append(cl->topics, topic);
        topic->subscribed = EINA_TRUE;
    }

    printf("%s[%s:%s] subscribe to topic %s[%d]\n",
           cl->client_id,
           _get_ip( (struct sockaddr *)&cl->addr),
           _get_port( (struct sockaddr *)&cl->addr),
           topic->name, topic->id);

    resp.header.len = sizeof(EMqtt_Sn_Suback_Msg);
    resp.header.msg_type = EMqtt_Sn_SUBACK;
    resp.flags = 0x00;
    resp.topic_id = htons(topic->id);
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMqtt_Sn_RETURN_CODE_ACCEPTED;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);
}

static Eina_Bool _mqtt_server_data_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
    EMqtt_Sn_Server *srv = data;
    EMqtt_Sn_Small_Header *header;
    socklen_t len;
    Mqtt_Client_Data *cdata;
    EMqtt_Sn_Connected_Client *cl_temp, *cl = NULL;
    Eina_List *l;


    cdata = calloc(1, sizeof(Mqtt_Client_Data));
    len = sizeof(cdata->client_addr);
    cdata->fd = ecore_main_fd_handler_fd_get(fd_handler);
    cdata->len = recvfrom(cdata->fd, cdata->data, READBUFSIZ, 0,
                          (struct sockaddr *)&cdata->client_addr, &len);

    header = (EMqtt_Sn_Small_Header *)cdata->data;

    // Header
    if (header->len == 0x01)
    {
        printf("Error long header not handle yet !\n");
        free(cdata);
        return ECORE_CALLBACK_RENEW;
    }

    EINA_LIST_FOREACH(srv->connected_clients, l, cl_temp)
    {
        if (!memcmp(&cl_temp->addr, &cdata->client_addr, sizeof(struct sockaddr_storage)))
        {
            cl = cl_temp;
            break;
        }
    }


    printf("[<-] %s[%d]\t\t %s [%s:%s]\n", mqttsn_msg_desc[header->msg_type].name, header->msg_type,
            cl ? cl->client_id : "Unknown client ",
            _get_ip((struct sockaddr*)&cdata->client_addr), _get_port((struct sockaddr*)&cdata->client_addr));


    switch(header->msg_type)
    {
    case EMqtt_Sn_CONNECT:
        _mqtt_sn_connect_msg(srv, cdata, cl);
        break;
    case EMqtt_Sn_REGISTER:
        _mqtt_sn_register_msg(srv, cdata, cl);
        break;
    case EMqtt_Sn_PUBLISH:
        _mqtt_sn_publish_msg(srv, cdata, cl);
        break;
    case EMqtt_Sn_PINGREQ:
        _mqtt_sn_pingreq_msg(srv, cdata, cl);
        break;
    case EMqtt_Sn_SUBSCRIBE:
        _mqtt_sn_subscribe_msg(srv, cdata, cl);
        break;
    case EMqtt_Sn_DISCONNECT:
        _mqtt_sn_disconnect_msg(srv, cdata, cl);
        break;
    default:
        printf("Unknown message\n");
        break;
    }

    free(cdata);

    return ECORE_CALLBACK_RENEW;
}

int _create_server_udp_socket(sa_family_t sa_family, unsigned short port, EMqtt_Sn_Server *srv)
{
    struct sockaddr_storage servaddr;
    int optval;
    int fd;

    fd = socket(sa_family, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    switch(sa_family)
    {
    case AF_INET:
    {
        struct sockaddr_in *addr;
        addr = (struct sockaddr_in*)&servaddr;
        addr->sin_family = sa_family;
        addr->sin_addr.s_addr=htonl(INADDR_ANY);
        addr->sin_port = htons(port);
        break;
    }

    case AF_INET6:
    {
        struct sockaddr_in6 *addr;
        addr = (struct sockaddr_in6*)&servaddr;
        addr->sin6_family = sa_family;
        addr->sin6_addr = in6addr_any;
        addr->sin6_port = htons(port);
        break;
    }

    default:
        return -1;
    }

    optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
        printf("Error\n");
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
        printf("Error\n");

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(optval)) < 0)
      printf("Error\n");

    if (bind(fd,  (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
      printf("Error");

    ecore_main_fd_handler_add(fd, ECORE_FD_READ, _mqtt_server_data_cb, srv, NULL, NULL);

    return fd;
}

EMqtt_Sn_Server *emqtt_sn_server_add(char *addr, unsigned short port)
{
    EMqtt_Sn_Server *srv;

    if (!addr || !port)
        return NULL;

    srv = calloc(1, sizeof(EMqtt_Sn_Server));
    srv->addr = eina_stringshare_add(addr);
    srv->port = port;

    srv->fd6 = _create_server_udp_socket(PF_INET6, port, srv);
    srv->fd4 = _create_server_udp_socket(PF_INET, port, srv);

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
