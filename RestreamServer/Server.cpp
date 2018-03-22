#include "Server.h"

#include "Common/Types.h"
#include "Common/GstRtspServerPtr.h"

#include "Log.h"
#include "StaticSources.h"
#include "RtspMountPoints.h"


namespace RestreamServer
{

struct Server::Private
{
    GstRTSPServerPtr staticServer;

    GstRTSPServerPtr restreamServer;
    GstRTSPMountPointsPtr mountPoints;
};


const std::shared_ptr<spdlog::logger>& Server::Log()
{
    return RestreamServer::Log();
}

Server::Server() :
    _p(new Private())
{
    gst_init(0, nullptr);

    initStaticServer();
    initRestreamServer();
}

Server::~Server()
{
    _p.reset();
}

void Server::initStaticServer()
{
    _p->staticServer.reset(gst_rtsp_server_new());
    GstRTSPMountPointsPtr mountPointsPtr(gst_rtsp_mount_points_new());

    GstRTSPServer* server = _p->staticServer.get();
    GstRTSPMountPoints* mountPoints = mountPointsPtr.get();

    gst_rtsp_server_set_service(server, STATIC_SERVER_PORT_STR);

    gst_rtsp_server_set_mount_points(server, mountPoints);

    {
        GstRTSPMediaFactory* barsFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            barsFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(barsFactory,
            "( videotestsrc pattern=smpte100 ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(barsFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, BARS, barsFactory);
    }

    {
        GstRTSPMediaFactory* whiteScreenFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            whiteScreenFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(whiteScreenFactory,
            "( videotestsrc pattern=white ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(whiteScreenFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, WHITE, whiteScreenFactory);
    }

    {
        GstRTSPMediaFactory* blackScreenFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            blackScreenFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(blackScreenFactory,
            "( videotestsrc pattern=black ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(blackScreenFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, BLACK, blackScreenFactory);
    }

    {
        GstRTSPMediaFactory* redScreenFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            redScreenFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(redScreenFactory,
            "( videotestsrc pattern=red ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(redScreenFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, RED, redScreenFactory);
    }

    {
        GstRTSPMediaFactory* greenScreenFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            greenScreenFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(greenScreenFactory,
            "( videotestsrc pattern=green ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(greenScreenFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, GREEN, greenScreenFactory);
    }

    {
        GstRTSPMediaFactory* blueScreenFactory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_transport_mode(
            blueScreenFactory, GST_RTSP_TRANSPORT_MODE_PLAY);
        gst_rtsp_media_factory_set_launch(blueScreenFactory,
            "( videotestsrc pattern=blue ! x264enc ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(blueScreenFactory, TRUE);
        gst_rtsp_mount_points_add_factory(mountPoints, BLUE, blueScreenFactory);
    }
}

void Server::initRestreamServer()
{
    _p->restreamServer.reset(gst_rtsp_server_new());
    _p->mountPoints.reset(GST_RTSP_MOUNT_POINTS(rtsp_mount_points_new()));

    GstRTSPServer* server = _p->restreamServer.get();
    GstRTSPMountPoints* mountPoints = _p->mountPoints.get();

    gst_rtsp_server_set_service(server, RESTREAM_SERVER_PORT_STR);

    gst_rtsp_server_set_mount_points(server, mountPoints);
}

void Server::serverMain()
{
    GstRTSPServer* staticServer = _p->staticServer.get();
    GstRTSPServer* restreamServer = _p->restreamServer.get();

    if(!staticServer) {
        Log()->critical("RTSP static server not initialized");
        return;
    }
    if(!restreamServer) {
        Log()->critical("RTSP restream server not initialized");
        return;
    }

    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);

    gst_rtsp_server_attach(staticServer, nullptr);
    gst_rtsp_server_attach(restreamServer, nullptr);

    Log()->info(
        "RTSP static server running on port {}",
        gst_rtsp_server_get_bound_port(staticServer));
    Log()->info(
        "RTSP restream server running on port {}",
        gst_rtsp_server_get_bound_port(restreamServer));

    g_main_loop_run(loop);
}

}
