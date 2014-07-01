#include "EMqtt.h"
#include "emqtt_private.h"
#include "emqtt_topic.h"

EMqtt_Sn_Topic *
emqtt_topic_new(const char *name, uint16_t *next_topic)
{
    EMqtt_Sn_Topic *topic;

    topic = calloc(1, sizeof(EMqtt_Sn_Topic));

    if (next_topic)
      topic->id = (*next_topic)++;

    topic->name = eina_stringshare_add(name);

    printf("Topic: %s [%d]\n",topic->name,topic->id);
    return topic;
}

EMqtt_Sn_Topic *
emqtt_topic_name_get(const char *topic_name, Eina_List *topics)
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

EMqtt_Sn_Topic *
emqtt_topic_id_get(uint16_t topic_id, Eina_List *topics)
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
