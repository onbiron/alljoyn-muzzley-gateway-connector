#include "app/ConfigServiceListenerImpl.h"

#include "app/SrpKeyXListener.h"
#include "app/ConfigParser.h"
#include "common/xmppconnutil.h"
#include <AboutObjApi.h>
#include <iostream>

using namespace ajn;
using namespace services;

ConfigServiceListenerImpl::ConfigServiceListenerImpl(ConfigDataStore& store,
                                                     BusAttachment& bus,
                                                     CommonBusListener* busListener,
                                                     void(*func)(),
                                                     const std::string& configFilePath ) :
ConfigService::Listener(), m_ConfigDataStore(&store), m_Bus(&bus), m_BusListener(busListener),
m_onRestartCallback(func), m_ConfigFilePath(configFilePath), m_KeyListener(new SrpKeyXListener())
{
    ConfigParser parser(m_ConfigFilePath);
    std::string passcode = parser.GetField("AllJoynPasscode");
    if ( passcode.empty() && parser.GetErrors().size() > 0 )
    {
        // Assume the error is that the passcode hasn't been set
        passcode = "000000";
        parser.SetField("AllJoynPasscode", passcode.c_str());
    }
    m_KeyListener->setPassCode(passcode.c_str());
    QStatus status = m_Bus->EnablePeerSecurity("ALLJOYN_PIN_KEYX ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", dynamic_cast<AuthListener*>(m_KeyListener));
    if ( ER_OK != status )
    {
        LOG_RELEASE("Failed to enable AllJoyn Peer Security! Reason: %s", QCC_StatusText(status));
    }
}

QStatus ConfigServiceListenerImpl::Restart()
{
    m_onRestartCallback();
    return ER_OK;
}

QStatus ConfigServiceListenerImpl::FactoryReset()
{
    QStatus status = ER_OK;
    m_ConfigDataStore->FactoryReset();
    m_Bus->ClearKeyStore();

    AboutObjApi* aboutObjApi = AboutObjApi::getInstance();
    if (aboutObjApi) {
        status = aboutObjApi->Announce();
    }

    return status;
}

QStatus ConfigServiceListenerImpl::SetPassphrase(const char* daemonRealm,
                                                 size_t passcodeSize,
                                                 const char* passcode,
                                                 ajn::SessionId sessionId)
{
    qcc::String passCodeString(passcode, passcodeSize);

    PersistPassword(daemonRealm, passCodeString.c_str());

    // TODO: What exactly is the purpose of the rest of this function?
    m_Bus->ClearKeyStore();
    m_Bus->EnableConcurrentCallbacks();

    std::vector<SessionId> sessionIds = m_BusListener->getSessionIds();
    for (size_t i = 0; i < sessionIds.size(); i++) {
        if (sessionIds[i] == sessionId) {
            continue;
        }
        m_Bus->LeaveSession(sessionIds[i]);
    }
    return ER_OK;
}

ConfigServiceListenerImpl::~ConfigServiceListenerImpl()
{
    delete m_KeyListener;
}

void ConfigServiceListenerImpl::PersistPassword(const char* /*daemonRealm*/, const char* passcode)
{
    ConfigParser parser(m_ConfigFilePath);
    parser.SetField("AllJoynPasscode", passcode);
    m_KeyListener->setPassCode(passcode);
}
