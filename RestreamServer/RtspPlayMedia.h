#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>


namespace RestreamServer
{

struct _RtspPlayMedia;
typedef struct _RtspPlayMedia RtspPlayMedia;

G_BEGIN_DECLS

#define TYPE_RTSP_PLAY_MEDIA rtsp_play_media_get_type()
G_DECLARE_FINAL_TYPE(RtspPlayMedia, rtsp_play_media, , RTSP_PLAY_MEDIA, GstRTSPMedia)

GstElement* rtsp_play_media_create_element(const URL& splashSource);

void rtsp_play_media_set_proxy_sink(RtspPlayMedia*, GstElement*);

G_END_DECLS

struct RtspPlayMediaUnref
{
    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (RtspPlayMedia* media)
        { (*this)(G_OBJECT(media)); }
};

typedef
    std::unique_ptr<
        RtspPlayMedia,
        RtspPlayMediaUnref> RtspPlayMediaPtr;

}
