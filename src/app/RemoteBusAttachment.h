/**
 * Copyright (c) 2015, Affinegy, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef REMOTEBUSATTACHMENT_H_
#define REMOTEBUSATTACHMENT_H_

#include <string>
#include <vector>
#include <pthread.h>
#include "common/xmppconnutil.h"
#include "alljoyn/BusAttachment.h"
#include "AboutPropertyStore.h"
#include "AboutBusObject.h"
#include "RemoteBusListener.h"
#include "RemoteBusAttachment.h"
#include "RemoteBusObject.h"

class XMPPConnector;

class RemoteBusAttachment :
    public ajn::BusAttachment
{
public:
    RemoteBusAttachment(
        const std::string&  remoteName,
        XMPPConnector* connector
        );

    ~RemoteBusAttachment();

    QStatus
    BindSessionPort(
        ajn::SessionPort port
        );

    QStatus
    JoinSession(
        const std::string& host,
        ajn::SessionPort   port,
        ajn::SessionId&    id
        );

    QStatus
    RegisterSignalHandler(
        const InterfaceDescription::Member* member
        );

    void
    AllJoynSignalHandler(
        const InterfaceDescription::Member* member,
        const char*                         srcPath,
        ajn::Message&                       message
        );

    QStatus
    AddRemoteObject(
        const std::string&                       path,
        std::vector<const InterfaceDescription*> interfaces
        );

    std::string
    RequestWellKnownName(
        const std::string& remoteName
        );

    void
    AddSession(
        ajn::SessionId     localId,
        const std::string& peer,
        ajn::SessionPort   port,
        ajn::SessionId     remoteId
        );

    void
    RemoveSession(
        ajn::SessionId localId
        );

    ajn::SessionId
    GetLocalSessionId(
        ajn::SessionId remoteId
        );

    ajn::SessionId
    GetSessionIdByPeer(
        const std::string& peer
        );

    std::string
    GetPeerBySessionId(
        ajn::SessionId id
        );

    void
    RelayAnnouncement(
        uint16_t                                                  version,
        uint16_t                                                  port,
        const std::string&                                        busName,
        const ajn::services::AnnounceHandler::ObjectDescriptions& objectDescs,
        const ajn::services::AnnounceHandler::AboutData&          aboutData
        );

    void
    RelaySignal(
        const std::string&         destination,
        ajn::SessionId             sessionId,
        const std::string&         objectPath,
        const std::string&         ifaceName,
        const std::string&         ifaceMember,
        const std::vector<MsgArg>& msgArgs
        );

    void
    SignalReplyReceived(
        const std::string& objectPath,
        const std::string& replyStr
        );

    void
    SignalSessionJoined(
        ajn::SessionId result
        );

    std::string WellKnownName() const;
    std::string RemoteName() const;

private:

    XMPPConnector*           m_connector;
    std::string              m_remoteName;
    std::string              m_wellKnownName;
    RemoteBusListener        m_listener;
    std::vector<RemoteBusObject*> m_objects;

    struct SessionInfo
    {
        std::string      peer;
        ajn::SessionPort port;
        ajn::SessionId   remoteId;
    };
    map<ajn::SessionId, SessionInfo> m_activeSessions;
    pthread_mutex_t                  m_activeSessionsMutex;

    AboutPropertyStore*       m_aboutPropertyStore;
    AboutBusObject*           m_aboutBusObject;
};


#endif // REMOTEBUSATTACHMENT_H_