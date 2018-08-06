#include "RtspPlayMediaFactory.h"

#include "Log.h"
#include "RtspRecordMediaFactory.h"


namespace RestreamServer
{

namespace
{

struct CxxPrivate
{
    URL splashSource;

    RtspRecordMediaFactoryPtr srcFactory;

    RtspPlayMediaPtr media;
};

}

struct _RtspPlayMediaFactory
{
    GstRTSPMediaFactory parent_instance;

    CxxPrivate* p;
};

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url);


G_DEFINE_TYPE(RtspPlayMediaFactory, rtsp_play_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)


RtspPlayMediaFactory* rtsp_play_media_factory_new(const URL& splashSource)
{
    RtspPlayMediaFactory* instance =
        _RTSP_PLAY_MEDIA_FACTORY(g_object_new(TYPE_RTSP_PLAY_MEDIA_FACTORY, NULL));

    if(instance)
        instance->p->splashSource = splashSource;

    return instance;
}

static void rtsp_play_media_factory_class_init(RtspPlayMediaFactoryClass* klass)
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
    RtspPlayMediaFactory* self = _RTSP_PLAY_MEDIA_FACTORY(mediaFactory);

    RtspPlayMedia* playMedia = _RTSP_PLAY_MEDIA(media);

    gst_object_ref(playMedia);

    self->p->media.reset(playMedia);

    RtspRecordMedia* recordMedia =
        rtsp_record_media_factory_get_media(self->p->srcFactory.get());
    if(!recordMedia)
        return;

    GstElement* proxySink = rtsp_record_media_get_proxy_sink(recordMedia);
    rtsp_play_media_set_proxy_sink(playMedia, proxySink);
}

static void rtsp_play_media_factory_init(RtspPlayMediaFactory* self)
{
    self->p = new CxxPrivate;

    GstRTSPMediaFactory* parent = GST_RTSP_MEDIA_FACTORY(self);

    gst_rtsp_media_factory_set_transport_mode(
        parent, GST_RTSP_TRANSPORT_MODE_PLAY);
    gst_rtsp_media_factory_set_shared(parent, TRUE);

    gst_rtsp_media_factory_set_media_gtype(parent, TYPE_RTSP_PLAY_MEDIA);

    g_signal_connect(self, "media-constructed", G_CALLBACK(media_constructed), self);
    g_signal_connect(self, "media-configure", G_CALLBACK(media_configure), self);
}

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url)
{
    RtspPlayMediaFactory* self = _RTSP_PLAY_MEDIA_FACTORY(factory);

    return rtsp_play_media_create_element(self->p->splashSource);
}

void rtsp_play_media_factory_set_record_factory(
    RtspPlayMediaFactory* factory, RtspRecordMediaFactory* srcFactory)
{
    gst_object_ref(srcFactory);
    factory->p->srcFactory.reset(srcFactory);
}

RtspPlayMedia* rtsp_play_media_factory_get_media(RtspPlayMediaFactory* self)
{
    return self->p->media.get();
}

}
