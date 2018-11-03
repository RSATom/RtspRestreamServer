#pragma once

#include <gst/rtsp/gstrtspdefs.h>

#include "Config.h"
#include "Action.h"
#include "Log.h"


namespace RestreamServer
{

struct Callbacks
{
    std::function<bool (GstRTSPMethod method, const std::string& path)> authenticationRequired;
    std::function<bool (const std::string& user, const std::string& pass)> authenticate;
    std::function<bool (const std::string& user, Action, const std::string& path)> authorize;

    std::function<void (const std::string& path)> firstPlayerConnected;
    std::function<void (const std::string& path)> lastPlayerDisconnected;
    std::function<void (const std::string& path)> recorderConnected;
    std::function<void (const std::string& path)> recorderDisconnected;
};

class Server
{
public:
    Server(
        const Callbacks&,
        unsigned short staticPort,
        unsigned short restreamPort,
        unsigned maxPathsCount = 0,
        unsigned maxClientsPerPath = 0);
    ~Server();

    void serverMain();

private:
    static inline const std::shared_ptr<spdlog::logger>& Log();

    void initStaticServer();
    void initRestreamServer();

private:
    struct Private;
    std::unique_ptr<Private> _p;
};

}
