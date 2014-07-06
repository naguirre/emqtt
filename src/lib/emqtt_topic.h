#ifndef _EMQTT_TOPIC_H
#define _EMQTT_TOPIC_H

typedef struct _EMqtt_Sn_Topic EMqtt_Sn_Topic;

struct _EMqtt_Sn_Topic
{
    const char *name;
    uint16_t id;
    Eina_Bool subscribed;
};

EMqtt_Sn_Topic *emqtt_topic_new(const char *name, uint16_t *next_topic);
EMqtt_Sn_Topic *emqtt_topic_name_get(const char *topic_name, Eina_List *topics);
EMqtt_Sn_Topic *emqtt_topic_id_get(uint16_t topic_id, Eina_List *topics);
Eina_Bool emqtt_topic_matches(const char *wild_topic, const char *topic);

#endif /* _EMQTT_TOPIC_H */
