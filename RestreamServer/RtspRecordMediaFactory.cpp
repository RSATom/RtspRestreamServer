#include "RtspRecordMediaFactory.h"

#include "Log.h"
#include "RtspPlayMediaFactory.h"


namespace RestreamServer
{

namespace
{

struct CxxPrivate
{
    RtspPlayMediaFactoryPtr dstFactory;

    RtspRecordMediaPtr media;
};

}

struct _RtspRecordMediaFactory
{
    GstRTSPMediaFactory parent_instance;

    CxxPrivate* p;
};

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url);


G_DEFINE_TYPE(RtspRecordMediaFactory, rtsp_record_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)


RtspRecordMediaFactory* rtsp_record_media_factory_new()
{
    RtspRecordMediaFactory* instance =
        _RTSP_RECORD_MEDIA_FACTORY(g_object_new(TYPE_RTSP_RECORD_MEDIA_FACTORY, NULL));

    return instance;
}

static void rtsp_record_media_factory_class_init(RtspRecordMediaFactoryClass* klass)
{
    GstRTSPMediaFactoryClass* parent_klass = GST_RTSP_MEDIA_FACTORY_CLASS(klass);

    parent_klass->create_element = create_element;
}

static void media_constructed(
    GstRTSPMediaFactory* /*mediaFactory*/,
    GstRTSPMedia* /*media*/,
    gpointer /*userData*/)
{
}

static void media_configure(
    GstRTSPMediaFactory* mediaFactory,
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    RtspRecordMediaFactory* self = _RTSP_RECORD_MEDIA_FACTORY(mediaFactory);

    RtspRecordMedia* recordMedia = _RTSP_RECORD_MEDIA(media);

    gst_object_ref(recordMedia);

    self->p->media.reset(recordMedia);

    RtspPlayMedia* playMedia =
        rtsp_play_media_factory_get_media(self->p->dstFactory.get());
    if(!playMedia)
        return;

    GstElement* proxySink = rtsp_record_media_get_proxy_sink(recordMedia);
    rtsp_play_media_set_proxy_sink(playMedia, proxySink);
}

static void rtsp_record_media_factory_init(RtspRecordMediaFactory* self)
{
    self->p = new CxxPrivate;

    GstRTSPMediaFactory* parent = GST_RTSP_MEDIA_FACTORY(self);

    gst_rtsp_media_factory_set_transport_mode(
        parent, GST_RTSP_TRANSPORT_MODE_RECORD);
    gst_rtsp_media_factory_set_shared(parent, TRUE);

    gst_rtsp_media_factory_set_media_gtype(parent, TYPE_RTSP_RECORD_MEDIA);

    g_signal_connect(self, "media-constructed", G_CALLBACK(media_constructed), self);
    g_signal_connect(self, "media-configure", G_CALLBACK(media_configure), self);
}

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url)
{
    // RtspRecordMediaFactory* self = _RTSP_RECORD_MEDIA_FACTORY(factory);

    return rtsp_record_media_create_element();
}

void rtsp_record_media_factory_set_play_factory(
    RtspRecordMediaFactory* factory, RtspPlayMediaFactory* dstFactory)
{
    gst_object_ref(dstFactory);
    factory->p->dstFactory.reset(dstFactory);
}

RtspRecordMedia* rtsp_record_media_factory_get_media(RtspRecordMediaFactory* self)
{
    return self->p->media.get();
}

}
