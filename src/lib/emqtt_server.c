/******************************************************************************
** Copyright (c) 2014, emqtt. All Rights Reserved.
**
** Authors: Nicolas Aguirre, Julien Masson
**
** This file is part of emqtt.
**
** emqtt is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** emqtt is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Foobar; if not, write to the Free Software
** Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
**
******************************************************************************/
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
        WRN("Try to send data to unknown client");
        return EINA_FALSE;
    }

    /*printf("data[0][1] 0x%02X 0x%02X\n", data[0], data[1]);
    printf("HEader : %s %d\n", mqttsn_msg_desc[header->msg_type].name, header->len);*/

    DBG("[->] %s[%d]\t\t %s [%s:%s]",
        mqttsn_msg_desc[header->msg_type].name, header->msg_type,
            cl ? cl->client_id : "Unknown client",
            _get_ip((struct sockaddr*)&cl->addr),
            _get_port((struct sockaddr*)&cl->addr));

    sendto(cl->fd, data, len, 0,
           (struct sockaddr*)&cl->addr, sizeof(cl->addr));
    return EINA_TRUE;
}

static void
_mqtt_sn_advertise_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Advertise_Msg *msg;

    msg = (EMqtt_Sn_Advertise_Msg *)cdata->data;
    if (msg->gw_id == srv->gw_id)
    {
        DBG("Receive ADVERTISE from myself");
    }
}

static void
_mqtt_sn_searchgw_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata)
{
    EMqtt_Sn_Gwinfo_Msg resp;

    resp.header.len = sizeof(resp);
    resp.header.msg_type = EMQTT_SN_GWINFO;
    resp.gw_id = srv->gw_id;

    DBG("[->] %s[%d]\t\t [%s:%s]",
        mqttsn_msg_desc[resp.header.msg_type].name, resp.header.msg_type,
            _get_ip((struct sockaddr*)&cdata->client_addr),
            _get_port((struct sockaddr*)&cdata->client_addr));

    sendto(cdata->fd, &resp, resp.header.len, 0,
           (struct sockaddr*)&cdata->client_addr, sizeof(cdata->client_addr));
}

static void
_mqtt_sn_connect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Connect_Msg *msg;
    EMqtt_Sn_Connack_Msg resp;
    size_t s;

    msg = (EMqtt_Sn_Connect_Msg *)cdata->data;
    s = msg->header.len - (sizeof(EMqtt_Sn_Connect_Msg)) + 1;

    if (msg->flags & EMQTT_SN_FLAGS_WILL)
    {
        INF("Client asking for will topic and will message prompting\n");
    }
    if ((msg->flags & EMQTT_SN_FLAGS_CLEANSESSION) && cl)
    {
        EMqtt_Sn_Topic *topic;
        INF("Client asking for clean session, cleaning known topics for this client\n");
        EINA_LIST_FREE(cl->topics, topic)
        {
            emqtt_topic_free(topic);
        }
    }

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
        WRN("Try to reconnect a known client ? ");
    }

    resp.header.len = 0x03;
    resp.header.msg_type = EMQTT_SN_CONNACK;
    resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;

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
        ERR("Error : this client is not connected");
        return;
    }

    s = msg->header.len - (sizeof(EMqtt_Sn_Publish_Msg));
    data = calloc(1, s + 1);
    memcpy(data, cdata->data + sizeof(EMqtt_Sn_Publish_Msg) , s);

    resp.header.len = sizeof(EMqtt_Sn_Puback_Msg);
    resp.header.msg_type = EMQTT_SN_PUBACK;
    resp.topic_id = msg->topic_id;
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);

    topic = emqtt_topic_id_get(htons(msg->topic_id), cl->topics);
    if (!topic || !topic->name)
    {
        ERR("Topic is NULL !");
    }
    else
    {
        INF("Publish %s with value %s", topic->name, data);
        topic_name = topic->name;
    }

    EINA_LIST_FOREACH(srv->connected_clients, l, con_cli)
    {
        topic = emqtt_topic_name_get(topic_name, con_cli->topics);
        if (!topic)
        {
            Eina_List *l2, *l2_next;
            EMqtt_Sn_Topic *t;

            EINA_LIST_FOREACH_SAFE(con_cli->topics, l2, l2_next, t)
            {
                DBG("Test topic : %s | %s", t->name, topic_name);
                if (emqtt_topic_matches(t->name , topic_name))
                {
                    EMqtt_Sn_Subscriber *subscriber;
                    EMqtt_Sn_Register_Msg *reg_msg;
                    char d[256];
                    DBG("Topic matches");

                    topic = emqtt_topic_new(topic_name, &con_cli->last_topic);
                    con_cli->topics = eina_list_append(con_cli->topics, topic);
                    topic->subscribed = EINA_TRUE;

                    subscriber = calloc(1, sizeof(EMqtt_Sn_Subscriber));
                    subscriber->topic = topic;
                    //subscriber->topic_received_cb = _mqtt_topic_regack_cb;
                    //subscriber->data = cli;
                    con_cli->subscribers = eina_list_append(con_cli->subscribers,
                                                            subscriber);

                    reg_msg = (EMqtt_Sn_Register_Msg*)d;
                    reg_msg->header.len = sizeof(EMqtt_Sn_Register_Msg) + strlen(topic_name);
                    reg_msg->header.msg_type = EMQTT_SN_REGISTER;
                    reg_msg->msg_id = 0x00;
                    reg_msg->topic_id = htons(topic->id);
                    snprintf(d + sizeof(EMqtt_Sn_Register_Msg), sizeof(d) - sizeof(EMqtt_Sn_Register_Msg),
                             "%s", topic_name);
                    DBG("%s %d", d + sizeof(EMqtt_Sn_Register_Msg), topic->id);
                    _mqtt_send_data(con_cli, (void*)reg_msg, reg_msg->header.len);


                    // send publish to the client
                    EMqtt_Sn_Publish_Msg *pub_msg = msg;
                    pub_msg->topic_id = htons(topic->id);
                    _mqtt_send_data(con_cli, (const char*)pub_msg, pub_msg->header.len);
                    return;
                }
            }

        }
        else
        {
            EMqtt_Sn_Publish_Msg *pub_msg = msg;

            DBG("Find topic : %s[%d] %d", topic->name, topic->id, topic->subscribed);
            if (topic->subscribed)
            {
                pub_msg->topic_id = htons(topic->id);
                _mqtt_send_data(con_cli, (const char*)pub_msg, pub_msg->header.len);
            }
        }
    }

}

static void
_mqtt_sn_pingreq_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Pingresp_Msg resp;

    if (!cl)
    {
        ERR("Error : this client is not connected");
        return;
    }

    resp.header.len = 2;
    resp.header.msg_type = EMQTT_SN_PINGRESP;

    _mqtt_send_data(cl, (const char*)&resp, resp.header.len);
}

static void
_mqtt_sn_disconnect_msg(EMqtt_Sn_Server *srv, Mqtt_Client_Data *cdata, EMqtt_Sn_Connected_Client *cl)
{
    EMqtt_Sn_Disconnect_Msg resp;

    if (!cl)
    {
        ERR("Error : this client is not connected");
        return;
    }

    resp.header.len = 2;
    resp.header.msg_type = EMQTT_SN_DISCONNECT;

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

    INF("%s[%s:%s] subscribe to topic %s[%d]",
        cl->client_id,
        _get_ip( (struct sockaddr *)&cl->addr),
        _get_port( (struct sockaddr *)&cl->addr),
        topic->name, topic->id);

    resp.header.len = sizeof(EMqtt_Sn_Suback_Msg);
    resp.header.msg_type = EMQTT_SN_SUBACK;
    resp.flags = 0x00;
    resp.topic_id = htons(topic->id);
    resp.msg_id = msg->msg_id;
    resp.ret_code = EMQTT_SN_RETURN_CODE_ACCEPTED;

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
        CRIT("Error long header not handle yet !");
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

    if (header->msg_type > EMQTT_SN_SENTINEL)
    {
        free(cdata);
        ERR("Error malformed message ?");
        return ECORE_CALLBACK_RENEW;

    }

    DBG("[<-] %s[%d]\t\t %s [%s:%s]", mqttsn_msg_desc[header->msg_type].name, header->msg_type,
            cl ? cl->client_id : "Unknown client ",
            _get_ip((struct sockaddr*)&cdata->client_addr), _get_port((struct sockaddr*)&cdata->client_addr));


    switch(header->msg_type)
    {
    case EMQTT_SN_ADVERTISE:
        _mqtt_sn_advertise_msg(srv, cdata);
        break;
    case EMQTT_SN_SEARCHGW:
        _mqtt_sn_searchgw_msg(srv, cdata);
        break;
    case EMQTT_SN_CONNECT:
        _mqtt_sn_connect_msg(srv, cdata, cl);
        break;
    case EMQTT_SN_REGISTER:
        _mqtt_sn_register_msg(srv, cdata, cl);
        break;
    case EMQTT_SN_PUBLISH:
        _mqtt_sn_publish_msg(srv, cdata, cl);
        break;
    case EMQTT_SN_PINGREQ:
        _mqtt_sn_pingreq_msg(srv, cdata, cl);
        break;
    case EMQTT_SN_SUBSCRIBE:
        _mqtt_sn_subscribe_msg(srv, cdata, cl);
        break;
    case EMQTT_SN_DISCONNECT:
        _mqtt_sn_disconnect_msg(srv, cdata, cl);
        break;
    default:
        ERR("Unknown message");
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
        ERR("Error");
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
        ERR("Error");

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(optval)) < 0)
        ERR("Error");

    if (bind(fd,  (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR("Error");

    ecore_main_fd_handler_add(fd, ECORE_FD_READ, _mqtt_server_data_cb, srv, NULL, NULL);

    return fd;
}

static Eina_Bool
_mqtt_advertise_cb(void *data)
{
    EMqtt_Sn_Advertise_Msg msg;
    EMqtt_Sn_Server *srv = data;
    int fd;
    struct sockaddr_in sock_in;
    int broadcast = 1;

    memset(&sock_in, 0, sizeof(struct sockaddr_in));

    fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_in.sin_port = htons(0);
    sock_in.sin_family = PF_INET;

    bind(fd, (struct sockaddr *)&sock_in, sizeof(struct sockaddr_in));
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int));

    sock_in.sin_addr.s_addr = htonl(-1); /* send message to 255.255.255.255 */
    sock_in.sin_port = htons(srv->port); /* same port number as server ?*/
    sock_in.sin_family = PF_INET;

    msg.header.msg_type = EMQTT_SN_ADVERTISE;
    msg.header.len = sizeof(msg);
    msg.duration = htons(MQTT_T_ADV);
    msg.gw_id = srv->gw_id;

    sendto(fd, &msg, msg.header.len, 0, (struct sockaddr *)&sock_in, sizeof(struct sockaddr_in));

    return ECORE_CALLBACK_RENEW;
}

EMqtt_Sn_Server *emqtt_sn_server_add(char *addr, unsigned short port, unsigned char gw_id)
{
    EMqtt_Sn_Server *srv;

    if (!addr || !port)
        return NULL;

    srv = calloc(1, sizeof(EMqtt_Sn_Server));
    srv->addr = eina_stringshare_add(addr);
    srv->port = port;

    srv->fd6 = _create_server_udp_socket(PF_INET6, port, srv);
    srv->fd4 = _create_server_udp_socket(PF_INET, port, srv);
    srv->gw_id = gw_id;
    srv->advertise_timer = ecore_timer_add(MQTT_T_ADV, _mqtt_advertise_cb, srv);

    return srv;
}

void emqtt_sn_server_del(EMqtt_Sn_Server *srv)
{
    if (!srv)
        return;

    if (srv->addr)
        eina_stringshare_del(srv->addr);
    if (srv->advertise_timer)
        ecore_timer_del(srv->advertise_timer);

    free(srv);
}
