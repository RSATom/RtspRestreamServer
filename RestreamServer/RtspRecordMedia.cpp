#include "RtspRecordMedia.h"

#include <glib.h>

#include <CxxPtr/GstPtr.h>

#include "Log.h"


namespace RestreamServer
{

struct _RtspRecordMedia
{
    GstRTSPMedia parent_instance;
};


G_DEFINE_TYPE(
    RtspRecordMedia,
    rtsp_record_media,
    GST_TYPE_RTSP_MEDIA)


GstElement*
rtsp_record_media_create_element(
    const std::string& proxyName)
{
    Log()->trace(">> rtsp_record_media_create_element");

    const std::string pipeline =
        fmt::format(
            "rtph264depay name=depay0 ! h264parse ! "
            "interpipesink name={} sync=true allow-negotiation=false",
            proxyName);

    GError* error = nullptr;
    GstElement* element =
        gst_parse_launch_full(
            pipeline.c_str(), NULL, GST_PARSE_FLAG_PLACE_IN_BIN,
            &error);
    GErrorPtr errorPtr(error);

    if(errorPtr)
        Log()->critical(
            "Fail to create record pipeline: {}",
            errorPtr->message);

    return element;
}

static void
constructed(
    GObject* object)
{
    Log()->trace(">> RtspRecordMedia.constructed");

    // RtspRecordMedia* self = _RTSP_RECORD_MEDIA(object);

    // GstRTSPMedia* selfMedia = GST_RTSP_MEDIA(object);
}

static void
finalize(
    GObject* object)
{
    Log()->trace(">> RtspRecordMedia.finalize");

    // RtspRecordMedia* self = _RTSP_RECORD_MEDIA(object);

    G_OBJECT_CLASS(rtsp_record_media_parent_class)->finalize(object);
}

static void
prepared(
    GstRTSPMedia* media,
    gpointer userData)
{
    Log()->trace(">> RtspRecordMedia.prepared");
}

static void
unprepared(
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    Log()->trace(">> RtspRecordMedia.unprepared");

    // RtspRecordMedia* self = _RTSP_RECORD_MEDIA(media);
}

static void
rtsp_record_media_class_init(
    RtspRecordMediaClass* klass)
{
    Log()->trace(">> RtspRecordMedia.class_init");

    // GstRTSPMediaClass* parent_klass = GST_RTSP_MEDIA_CLASS(klass);

    GObjectClass* objectKlass = G_OBJECT_CLASS(klass);

    objectKlass->constructed = constructed;

    GObjectClass* object_klass = G_OBJECT_CLASS(klass);
    object_klass->finalize = finalize;
}

static void
rtsp_record_media_init(
    RtspRecordMedia* self)
{
    Log()->trace(">> RtspRecordMedia.init");

    // GstRTSPMedia* parent = GST_RTSP_MEDIA(self);

    g_signal_connect(self, "prepared", G_CALLBACK(prepared), nullptr);
    g_signal_connect(self, "unprepared", G_CALLBACK(unprepared), nullptr);
}

}
