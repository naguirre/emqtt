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
void emqtt_topic_free(EMqtt_Sn_Topic *topic);

#endif /* _EMQTT_TOPIC_H */
