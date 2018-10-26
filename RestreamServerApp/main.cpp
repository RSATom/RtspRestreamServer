#include "RestreamServer/Server.h"

#include <gst/gst.h>

extern "C" {
GST_PLUGIN_STATIC_DECLARE(interpipe);
}

int main(int argc, char *argv[])
{
    gst_init(0, nullptr);

    GST_PLUGIN_STATIC_REGISTER(interpipe);

    RestreamServer::Server restreamServer;

    restreamServer.serverMain();

    return 0;
}
