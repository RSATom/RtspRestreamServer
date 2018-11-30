#pragma once

#include <gio/gio.h>

#include <gst/rtsp/gstrtspdefs.h>

#include "Action.h"
#include "Log.h"


namespace RestreamServerLib
{

struct Callbacks
{
    std::function<bool (GTlsCertificate* peerCert, std::string* user)> tlsAuthenticate;
    std::function<bool (GstRTSPMethod method, const std::string& path, bool record)> authenticationRequired;
    std::function<bool (const std::string& user, const std::string& pass)> authenticate;
    std::function<bool (const std::string& user, Action, const std::string& path, bool record)> authorize;

    std::function<void (const std::string& user, const std::string& path)> firstPlayerConnected;
    std::function<void (const std::string& path)> lastPlayerDisconnected;
    std::function<void (const std::string& user, const std::string& path)> recorderConnected;
    std::function<void (const std::string& path)> recorderDisconnected;
};

class Server
{
public:
    Server(
        const Callbacks&,
        unsigned short staticPort,
        unsigned short restreamPort,
        bool useTls = false,
        unsigned maxPathsCount = 0,
        unsigned maxClientsPerPath = 0);
    ~Server();

    void serverMain();

    void setTlsCertificate(GTlsCertificate*);

private:
    static inline const std::shared_ptr<spdlog::logger>& Log();

    void initStaticServer();
    void initRestreamServer(bool useTls);

private:
    struct Private;
    std::unique_ptr<Private> _p;
};

}
