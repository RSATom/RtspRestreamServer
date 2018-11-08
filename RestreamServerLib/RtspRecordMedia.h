#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <CxxPtr/GlibPtr.h>


namespace RestreamServerLib
{

G_BEGIN_DECLS

#define TYPE_RTSP_RECORD_MEDIA rtsp_record_media_get_type()
G_DECLARE_FINAL_TYPE(
    RtspRecordMedia,
    rtsp_record_media,
    ,
    RTSP_RECORD_MEDIA,
    GstRTSPMedia)

GstElement*
rtsp_record_media_create_element(
    const std::string& proxyName);

G_END_DECLS

}
