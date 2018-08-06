#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>

#include "RtspPlayMedia.h"


namespace RestreamServer
{

struct _RtspRecordMediaFactory;
typedef struct _RtspRecordMediaFactory RtspRecordMediaFactory;

G_BEGIN_DECLS

#define TYPE_RTSP_PLAY_MEDIA_FACTORY rtsp_play_media_factory_get_type()
G_DECLARE_FINAL_TYPE(RtspPlayMediaFactory, rtsp_play_media_factory, , RTSP_PLAY_MEDIA_FACTORY, GstRTSPMediaFactory)

RtspPlayMediaFactory* rtsp_play_media_factory_new(const URL& splashSource);
void rtsp_play_media_factory_set_record_factory(RtspPlayMediaFactory*, RtspRecordMediaFactory*);
RtspPlayMedia* rtsp_play_media_factory_get_media(RtspPlayMediaFactory*);

G_END_DECLS

struct RtspPlayMediaFactoryUnref
{
    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (RtspPlayMediaFactory* factory)
        { (*this)(G_OBJECT(factory)); }
};

typedef
    std::unique_ptr<
        RtspPlayMediaFactory,
        RtspPlayMediaFactoryUnref> RtspPlayMediaFactoryPtr;

}
