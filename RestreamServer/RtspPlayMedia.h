#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <CxxPtr/GlibPtr.h>

#include "Types.h"


namespace RestreamServer
{

G_BEGIN_DECLS

#define TYPE_RTSP_PLAY_MEDIA rtsp_play_media_get_type()
G_DECLARE_FINAL_TYPE(RtspPlayMedia, rtsp_play_media, , RTSP_PLAY_MEDIA, GstRTSPMedia)

GstElement*
rtsp_play_media_create_element(
    const URL& splashSource,
    const std::string& listenTo);

G_END_DECLS

}
