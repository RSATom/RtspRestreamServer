#pragma once

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>


namespace RestreamServer
{

G_BEGIN_DECLS

#define TYPE_RESTREAM_RTSP_MEDIA restream_rtsp_media_get_type()
G_DECLARE_FINAL_TYPE(RestreamRtspMedia, restream_rtsp_media, , RESTREAM_RTSP_MEDIA, GstRTSPMedia)

GstElement* restream_rtsp_media_create_element(const URL& splashSource);

G_END_DECLS

}
