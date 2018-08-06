#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>


namespace RestreamServer
{

struct _RtspRecordMedia;
typedef struct _RtspRecordMedia RtspRecordMedia;

G_BEGIN_DECLS

#define TYPE_RTSP_RECORD_MEDIA rtsp_record_media_get_type()
G_DECLARE_FINAL_TYPE(RtspRecordMedia, rtsp_record_media, , RTSP_RECORD_MEDIA, GstRTSPMedia)

GstElement* rtsp_record_media_create_element();

GstElement* rtsp_record_media_get_proxy_sink(RtspRecordMedia*);

G_END_DECLS

struct RtspRecordMediaUnref
{
    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (RtspRecordMedia* media)
        { (*this)(G_OBJECT(media)); }
};

typedef
    std::unique_ptr<
        RtspRecordMedia,
        RtspRecordMediaUnref> RtspRecordMediaPtr;

}
