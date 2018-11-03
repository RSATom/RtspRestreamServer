#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include "RtspRecordMedia.h"


namespace RestreamServer
{

G_BEGIN_DECLS

#define TYPE_RTSP_RECORD_MEDIA_FACTORY rtsp_record_media_factory_get_type()
G_DECLARE_FINAL_TYPE(
    RtspRecordMediaFactory,
    rtsp_record_media_factory,
    ,
    RTSP_RECORD_MEDIA_FACTORY,
    GstRTSPMediaFactory)

RtspRecordMediaFactory*
rtsp_record_media_factory_new(
    const std::string& proxyName);

G_END_DECLS

}
