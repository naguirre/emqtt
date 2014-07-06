#include "EMqtt.h"
#include "emqtt_private.h"
#include "emqtt_topic.h"



Eina_Bool emqtt_topic_matches(const char *wild_topic, const char *topic)
{
    int rc = EINA_FALSE;
    char *last1 = NULL, *last2 = NULL;
    char *pwild = NULL, *pmatch = NULL;
    char *wild_topic_rw;
    char *topic_rw;

    if (strcmp(wild_topic, "#") == 0 || /* Hash matches anything... */
        strcmp(wild_topic, topic) == 0)
        return EINA_TRUE;


    if (strcmp(wild_topic, "/#") == 0)  /* Special case for /# matches anything starting with / */
    {
        if (topic[0] == '/')
            return EINA_TRUE;
        else
            return EINA_FALSE;
    }

    /* because strtok will return bill when matching /bill/ or bill in a topic name for the first time,
         * we have to check whether the first character is / explicitly.
         */
    if ((wild_topic[0] == '/') && (topic[0] != '/'))
            return EINA_FALSE;

    if ((wild_topic[0] == '+') && (topic[0] == '/'))
            return EINA_FALSE;

    wild_topic_rw = strdup(wild_topic);
    topic_rw = strdup(topic);
    pwild = strtok_r(wild_topic_rw, "/", &last1);
    pmatch = strtok_r(topic_rw, "/", &last2);

    /* Step through the subscription, level by level */
    while (pwild != NULL)
    {
        /* Have we got # - if so, it matches anything. */
        if (strcmp(pwild, "#") == 0)
        {
            rc = EINA_TRUE;
            break;
        }
        /* Nope - check for matches... */
        if (pmatch != NULL)
        {
            if (strcmp(pwild, "+") != 0 && strcmp(pwild, pmatch) != 0)
                /* The two levels simply don't match... */
                break;
        }
        else
            break; /* No more tokens to match against further tokens in the wildcard stream... */
        pwild = strtok_r(NULL, "/", &last1);
        pmatch = strtok_r(NULL, "/", &last2);
    }

    /* All tokens up to here matched, and we didn't end in #. If there
        are any topic tokens remaining, the match is bad, otherwise it was
        a good match. */
    if (pmatch == NULL && pwild == NULL)
        rc = EINA_TRUE;

    /* Now free the memory allocated in strdup() */
    free(wild_topic_rw);
    free(topic_rw);

    return rc;
}


EMqtt_Sn_Topic *
emqtt_topic_new(const char *name, uint16_t *next_topic)
{
    EMqtt_Sn_Topic *topic;

    topic = calloc(1, sizeof(EMqtt_Sn_Topic));
    if (next_topic)
	     topic->id = (*next_topic)++;
    topic->name = eina_stringshare_add(name);
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
