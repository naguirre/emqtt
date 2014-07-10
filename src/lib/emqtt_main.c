
//#define READBUFSIZ 65536

#include "EMqtt.h"
#include "emqtt_private.h"

static int _emqtt_init_count = 0;

int _emqtt_log_dom_global = -1;

const EMqtt_Sn_Msg_Desc mqttsn_msg_desc[]=
{
    {EMQTT_SN_ADVERTISE,"ADVERTISE" },
    {EMQTT_SN_SEARCHGW,"SEARCHGW"},
    {EMQTT_SN_GWINFO,"GWINFO"},
    {EMQTT_SN_RESERVED1,"RESERVED1"},
    {EMQTT_SN_CONNECT,"CONNECT"},
    {EMQTT_SN_CONNACK,"CONNACK"},
    {EMQTT_SN_WILLTOPICREQ,"WILLTOPICREQ"},
    {EMQTT_SN_WILLTOPIC, "WILLTOPIC"},
    {EMQTT_SN_WILLMSGREQ,"WILLMSGREQ"},
    {EMQTT_SN_WILLMSG,"WILLMSG"},
    {EMQTT_SN_REGISTER,"REGISTER"},
    {EMQTT_SN_REGACK,"REGACK"},
    {EMQTT_SN_PUBLISH,"PUBLISH"},
    {EMQTT_SN_PUBACK,"PUBACK"},
    {EMQTT_SN_PUBCOMP,"PUBCOMP"},
    {EMQTT_SN_PUBREC,"PUBREC"},
    {EMQTT_SN_PUBREL,"PUBREL"},
    {EMQTT_SN_RESERVED2,"RESERVED2"},
    {EMQTT_SN_SUBSCRIBE,"SUBSCRIBE"},
    {EMQTT_SN_SUBACK,"SUBACK"},
    {EMQTT_SN_UNSUBSCRIBE,"UNSUBSCRIBE"},
    {EMQTT_SN_UNSUBACK,"UNSUBACK"},
    {EMQTT_SN_PINGREQ,"PINGREQ"},
    {EMQTT_SN_PINGRESP,"PINGRESP"},
    {EMQTT_SN_DISCONNECT,"DISCONNECT"},
    {EMQTT_SN_RESERVED3,"RESERVED3"},
    {EMQTT_SN_WILLTOPICUPD,"WILLTOPICUPD"},
    {EMQTT_SN_WILLTOPICRESP,"WILLTOPICRESP"},
    {EMQTT_SN_WILLMSGUPD,"WILLMSGUPD"},
    {EMQTT_SN_WILLMSGRESP, "WILLMSGRESP"},
};

int emqtt_init(void)
{
    if (++_emqtt_init_count != 1)
        return _emqtt_init_count;

    if (!eina_init ())
        return --_emqtt_init_count;

    _emqtt_log_dom_global =
        eina_log_domain_register("emqtt", EMQTT_DEFAULT_LOG_COLOR);
    if (_emqtt_log_dom_global < 0)
    {
        EINA_LOG_ERR("Enna-Server Can not create a general log domain.");
        goto shutdown_eina;
    }
    else
      INF("EMqtt Init");

    if (!ecore_init ())
        goto shutdown_eina;

    return _emqtt_init_count;

shutdown_eina:
    eina_shutdown ();
    return --_emqtt_init_count;
}

int emqtt_shutdown(void)
{
    INF("EMqtt Shutdown");

    if (--_emqtt_init_count != 0)
        return _emqtt_init_count;

    ecore_shutdown();
    eina_shutdown();

    return _emqtt_init_count;
}
