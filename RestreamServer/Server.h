#pragma once

#include "Config.h"
#include "Log.h"


namespace RestreamServer
{

struct Callbacks
{
    std::function<void (const std::string& path)> firstPlayerConnected;
    std::function<void (const std::string& path)> lastPlayerDisconnected;
    std::function<void (const std::string& path)> recorderConnected;
    std::function<void (const std::string& path)> recorderDisconnected;
};

class Server
{
public:
    Server(const Callbacks&);
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
