#include "Log.h"


namespace RestreamServerLib
{

static std::shared_ptr<spdlog::logger> RestreamServer;

void InitLoggers()
{
    spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::stderr_sink_st>();

    RestreamServer = spdlog::create("RestreamServer", { sink });

#ifndef NDEBUG
    RestreamServer->set_level(spdlog::level::debug);
#else
    RestreamServer->set_level(spdlog::level::info);
#endif
}

const std::shared_ptr<spdlog::logger>& Log()
{
    if(!RestreamServer)
        InitLoggers();

    return RestreamServer;
}

}
