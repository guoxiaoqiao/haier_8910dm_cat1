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

#include "MQTTGWSubscribeHandler.h"
#include "MQTTGWPacket.h"

using namespace std;
using namespace MQTTSNGW;

MQTTGWSubscribeHandler::MQTTGWSubscribeHandler(Gateway* gateway)
{
    _gateway = gateway;
}

MQTTGWSubscribeHandler::~MQTTGWSubscribeHandler()
{

}

void MQTTGWSubscribeHandler::handleSuback(Client* client, MQTTGWPacket* packet)
{
    uint16_t msgId;
    uint8_t rc;
    uint8_t returnCode;
    int qos = 0;

    packet->getSUBACK(&msgId, &rc);
    uint16_t topicId = client->getWaitedSubTopicId(msgId);

    if (topicId)
    {
        MQTTSNPacket* snPacket = new MQTTSNPacket();
        client->eraseWaitedSubTopicId(msgId);

        if (rc == 0x80)
        {
            returnCode = MQTTSN_RC_REJECTED_INVALID_TOPIC_ID;
        }
        else
        {
            returnCode = MQTTSN_RC_ACCEPTED;
            qos = rc;
        }
        snPacket->setSUBACK(qos, topicId, msgId, returnCode);
        Event* evt = new Event();
        evt->setClientSendEvent(client, snPacket);
        _gateway->getClientSendQue()->post(evt);
    }
}

void MQTTGWSubscribeHandler::handleUnsuback(Client* client, MQTTGWPacket* packet)
{
    Ack ack;
    packet->getAck(&ack);
    MQTTSNPacket* snPacket = new MQTTSNPacket();
    snPacket->setUNSUBACK(ack.msgId);
    Event* evt = new Event();
    evt->setClientSendEvent(client, snPacket);
    _gateway->getClientSendQue()->post(evt);
}

