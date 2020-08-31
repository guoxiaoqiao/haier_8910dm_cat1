/**************************************************************************************
 * Copyright (c) 2016, Tomoaki Yamaguchi
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Tomoaki Yamaguchi - initial API and implementation and/or initial documentation
 **************************************************************************************/

#include "MQTTGWPublishHandler.h"
#include "MQTTSNGateway.h"
#include "MQTTSNPacket.h"
#include <string>

using namespace std;
using namespace MQTTSNGW;

char* currentDateTime(void);

MQTTGWPublishHandler::MQTTGWPublishHandler(Gateway* gateway)
{
    _gateway = gateway;
}

MQTTGWPublishHandler::~MQTTGWPublishHandler()
{

}

void MQTTGWPublishHandler::handlePublish(Client* client, MQTTGWPacket* packet)
{
    if ( !client->isActive() && !client->isSleep() && !client->isAwake())
    {
        WRITELOG("%s     The client is neither active nor sleep %s%s\n", ERRMSG_HEADER, client->getStatus(), ERRMSG_FOOTER);
        return;
    }

    /* client is sleeping. save PUBLISH */
    if ( client->isSleep() )
    {
        Publish pub;
        packet->getPUBLISH(&pub);

        WRITELOG(FORMAT_Y_G_G, currentDateTime(), packet->getName(),
        RIGHTARROW, client->getClientId(), "is sleeping. a message was saved.");

        if (pub.header.bits.qos == 1)
        {
            replyACK(client, &pub, PUBACK);
        }
        else if ( pub.header.bits.qos == 2)
        {
            replyACK(client, &pub, PUBREC);
        }

        MQTTGWPacket* msg = new MQTTGWPacket();
        *msg = *packet;
        if ( msg->getType() == 0 )
        {
            WRITELOG("%s MQTTGWPublishHandler::handlePublish can't allocate memories for Packet.%s\n", ERRMSG_HEADER,ERRMSG_FOOTER);
            delete msg;
            return;
        }
        client->setClientSleepPacket(msg);
        return;
    }

    Publish pub;
    packet->getPUBLISH(&pub);

    MQTTSNPacket* snPacket = new MQTTSNPacket();

    /* create MQTTSN_topicid */
    MQTTSN_topicid topicId;
    uint16_t id = 0;

    if (pub.topiclen == 2)
    {
        topicId.type = MQTTSN_TOPIC_TYPE_SHORT;
        *(topicId.data.short_name) = *pub.topic;
        *(topicId.data.short_name + 1) = *(pub.topic + 1);
    }
    else
    {
        topicId.type = MQTTSN_TOPIC_TYPE_NORMAL;
        topicId.data.long_.len = pub.topiclen;
        topicId.data.long_.name = pub.topic;
        id = client->getTopics()->getTopicId(&topicId);

        if ( id > 0 )
        {
            topicId.data.id = id;
        }
        else
        {
            /* This message might be subscribed with wild card. */
            Topic* topic = client->getTopics()->match(&topicId);
            if (topic == 0)
            {
                WRITELOG(" Invalid Topic. PUBLISH message is canceled.\n");
                if (pub.header.bits.qos == 1)
                {
                    replyACK(client, &pub, PUBACK);
                }
                else if ( pub.header.bits.qos == 2 )
                {
                    replyACK(client, &pub, PUBREC);
                }
                return;
            }

            /* add the Topic and get a TopicId */
            topic = client->getTopics()->add(&topicId);
            id = topic->getTopicId();

            if (id > 0)
            {
                /* create REGACK */
                MQTTSNPacket* regPacket = new MQTTSNPacket();

                MQTTSNString topicName;
                topicName.lenstring.len = topicId.data.long_.len;
                topicName.lenstring.data = topicId.data.long_.name;

                uint16_t regackMsgId = client->getNextSnMsgId();
                regPacket->setREGISTER(id, regackMsgId, &topicName);

                /* send REGISTER */
                Event* evrg = new Event();
                evrg->setClientSendEvent(client, regPacket);
                _gateway->getClientSendQue()->post(evrg);

                /* send PUBLISH */
                topicId.data.id = id;
                snPacket->setPUBLISH((uint8_t) pub.header.bits.dup, (int) pub.header.bits.qos,
                        (uint8_t) pub.header.bits.retain, (uint16_t) pub.msgId, topicId, (uint8_t*) pub.payload,
                        pub.payloadlen);
                client->getWaitREGACKPacketList()->setPacket(snPacket, regackMsgId);
            }
            else
            {
                WRITELOG("%sMQTTGWPublishHandler Can't create a Topic.%s\n", ERRMSG_HEADER,ERRMSG_FOOTER);
                delete snPacket;
                return;
            }
        }
    }

    snPacket->setPUBLISH((uint8_t) pub.header.bits.dup, (int) pub.header.bits.qos, (uint8_t) pub.header.bits.retain,
            (uint16_t) pub.msgId, topicId, (uint8_t*) pub.payload, pub.payloadlen);
    Event* ev1 = new Event();
    ev1->setClientSendEvent(client, snPacket);
    _gateway->getClientSendQue()->post(ev1);

}

void MQTTGWPublishHandler::replyACK(Client* client, Publish* pub, int type)
{
    MQTTGWPacket* pubAck = new MQTTGWPacket();
    pubAck->setAck(type, (uint16_t)pub->msgId);
    Event* ev1 = new Event();
    ev1->setBrokerSendEvent(client, pubAck);
    _gateway->getBrokerSendQue()->post(ev1);
}

void MQTTGWPublishHandler::handlePuback(Client* client, MQTTGWPacket* packet)
{
    Ack ack;
    packet->getAck(&ack);
    uint16_t topicId = client->getWaitedPubTopicId((uint16_t)ack.msgId);
    if (topicId)
    {
        MQTTSNPacket* mqttsnPacket = new MQTTSNPacket();
        mqttsnPacket->setPUBACK(topicId, (uint16_t)ack.msgId, 0);

        client->eraseWaitedPubTopicId((uint16_t)ack.msgId);
        Event* ev1 = new Event();
        ev1->setClientSendEvent(client, mqttsnPacket);
        _gateway->getClientSendQue()->post(ev1);
        return;
    }
    WRITELOG(" PUBACK from the Broker is invalid. PacketID : %04X  ClientID : %s \n", (uint16_t)ack.msgId, client->getClientId());
}

void MQTTGWPublishHandler::handleAck(Client* client, MQTTGWPacket* packet, int type)
{
    Ack ack;
    packet->getAck(&ack);

    if ( client->isActive() || client->isAwake() )
    {
        MQTTSNPacket* mqttsnPacket = new MQTTSNPacket();
        if (type == PUBREC)
        {
            mqttsnPacket->setPUBREC((uint16_t) ack.msgId);
        }
        else if (type == PUBREL)
        {
            mqttsnPacket->setPUBREL((uint16_t) ack.msgId);
        }
        else if (type == PUBCOMP)
        {
            mqttsnPacket->setPUBCOMP((uint16_t) ack.msgId);
        }

        Event* ev1 = new Event();
        ev1->setClientSendEvent(client, mqttsnPacket);
        _gateway->getClientSendQue()->post(ev1);
    }
    else if ( client->isSleep() )
    {
        if (type == PUBREL)
        {
            MQTTGWPacket* pubComp = new MQTTGWPacket();
            pubComp->setAck(PUBCOMP, (uint16_t)ack.msgId);
            Event* ev1 = new Event();
            ev1->setBrokerSendEvent(client, pubComp);
            _gateway->getBrokerSendQue()->post(ev1);
        }
    }
}

