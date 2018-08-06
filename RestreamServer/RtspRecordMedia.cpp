#include "RtspRecordMedia.h"

#include <glib.h>

#include "Common/GstPtr.h"

#include "Log.h"


namespace RestreamServer
{

struct _RtspRecordMedia
{
    GstRTSPMedia parent_instance;

    GstElement* proxy;
};


G_DEFINE_TYPE(RtspRecordMedia, rtsp_record_media, GST_TYPE_RTSP_MEDIA)


GstElement* rtsp_record_media_create_element()
{
    const gchar* pipeline =
        "rtph264depay name=depay0 ! proxysink name=proxy";

    GError* error = nullptr;
    GstElement* element =
        gst_parse_launch_full(
            pipeline, NULL, GST_PARSE_FLAG_PLACE_IN_BIN,
            &error);
    GstGErrorPtr errorPtr(error);

    if(errorPtr)
        Log()->critical(
            "Fail to create record pipeline: {}",
            errorPtr->message);

    return element;
}

static void
constructed(GObject* object)
{
    Log()->trace(">> RtspRecordMedia.constructed");

    RtspRecordMedia* self = _RTSP_RECORD_MEDIA(object);

    GstRTSPMedia* selfMedia = GST_RTSP_MEDIA(object);

    GstElementPtr pipelinePtr(gst_rtsp_media_get_element(selfMedia));
    GstElement* pipeline = pipelinePtr.get();

    GstElementPtr proxyPtr(gst_bin_get_by_name(GST_BIN(pipeline), "proxy"));
    self->proxy = proxyPtr.get(); // FIXME! should we keep ref?

    Log()->trace("<< RtspRecordMedia.constructed");
}

static void
prepared(
    GstRTSPMedia* media,
    gpointer userData)
{
    Log()->trace(">> RtspRecordMedia.prepared");
    Log()->trace("<< RtspRecordMedia.prepared");
}

static void
unprepared(
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    Log()->trace(">> RtspRecordMedia.unprepared");

    // RtspRecordMedia* self = _RTSP_RECORD_MEDIA(media);

    Log()->trace("<< RtspRecordMedia.unprepared");
}

static void
rtsp_record_media_class_init(RtspRecordMediaClass* klass)
{
    // GstRTSPMediaClass* parent_klass = GST_RTSP_MEDIA_CLASS(klass);

    GObjectClass* objectKlass = G_OBJECT_CLASS(klass);

    objectKlass->constructed = constructed;
}

static void
rtsp_record_media_init(RtspRecordMedia* self)
{
    // GstRTSPMedia* parent = GST_RTSP_MEDIA(self);

    self->proxy = nullptr;

    g_signal_connect(self, "prepared", G_CALLBACK(prepared), nullptr);
    g_signal_connect(self, "unprepared", G_CALLBACK(unprepared), nullptr);
}

GstElement* rtsp_record_media_get_proxy_sink(RtspRecordMedia* self)
{
    return self->proxy;
}

}
