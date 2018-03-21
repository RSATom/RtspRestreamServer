#pragma once

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>


namespace RestreamServer
{

G_BEGIN_DECLS

#define TYPE_RTSP_MEDIA_FACTORY rtsp_media_factory_get_type()
G_DECLARE_FINAL_TYPE(RtspMediaFactory, rtsp_media_factory, , RTSP_MEDIA_FACTORY, GstRTSPMediaFactory)

RtspMediaFactory* rtsp_media_factory_new(const URL& splashSource);

G_END_DECLS

}
