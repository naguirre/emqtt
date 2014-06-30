
//#define READBUFSIZ 65536

#include "EMqtt.h"
#include "emqtt_private.h"

static int _EMqttinit_count = 0;

const EMqtt_Sn_Msg_Desc mqttsn_msg_desc[]=
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








