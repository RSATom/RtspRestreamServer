#include "RestreamServer/Server.h"

#include <gst/gst.h>

#include "Config.h"

extern "C" {
GST_PLUGIN_STATIC_DECLARE(interpipe);
}

bool authenticationRequired(GstRTSPMethod method, const std::string& path)
{
    const std::string publicPath = "/test";

    // on RTSP SETUP path could contain control id of the stream
    if(path == publicPath ||
        (method == GST_RTSP_SETUP &&
         0 == path.compare(0, publicPath.length(), publicPath) &&
         0 == path.compare(publicPath.length(), 1, "/")))
    {
         return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    gst_init(0, nullptr);

    GST_PLUGIN_STATIC_REGISTER(interpipe);

    RestreamServer::Server restreamServer(
        { .authenticationRequired = authenticationRequired },
        STATIC_SERVER_PORT, RESTREAM_SERVER_PORT,
        MAX_PATHS_COUNT, MAX_CLIENTS_PER_PATH);

    restreamServer.serverMain();

    return 0;
}
