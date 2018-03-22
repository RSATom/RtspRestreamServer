#pragma once

#include "Config.h"
#include "Log.h"


namespace RestreamServer
{

class Server
{
public:
    Server();
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
