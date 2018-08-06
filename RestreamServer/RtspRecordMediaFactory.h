#pragma once

#include <memory>

#include <gst/rtsp-server/rtsp-server.h>

#include <Common/Types.h>

#include "RtspRecordMedia.h"


namespace RestreamServer
{

struct _RtspPlayMediaFactory;
typedef struct _RtspPlayMediaFactory RtspPlayMediaFactory;

G_BEGIN_DECLS

#define TYPE_RTSP_RECORD_MEDIA_FACTORY rtsp_record_media_factory_get_type()
G_DECLARE_FINAL_TYPE(RtspRecordMediaFactory, rtsp_record_media_factory, , RTSP_RECORD_MEDIA_FACTORY, GstRTSPMediaFactory)

RtspRecordMediaFactory* rtsp_record_media_factory_new();
void rtsp_record_media_factory_set_play_factory(RtspRecordMediaFactory*, RtspPlayMediaFactory*);
RtspRecordMedia* rtsp_record_media_factory_get_media(RtspRecordMediaFactory*);

G_END_DECLS

struct RtspRecordMediaFactoryUnref
{
    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (RtspRecordMediaFactory* factory)
        { (*this)(G_OBJECT(factory)); }
};

typedef
    std::unique_ptr<
        RtspRecordMediaFactory,
        RtspRecordMediaFactoryUnref> RtspRecordMediaFactoryPtr;

}
