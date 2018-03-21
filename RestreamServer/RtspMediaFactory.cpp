#include "RtspMediaFactory.h"

#include "Log.h"
#include "RestreamRtspMedia.h"


namespace RestreamServer
{

namespace
{

struct CxxPrivate
{
    URL splashSource;
};

}

struct _RtspMediaFactory
{
    GstRTSPMediaFactory parent_instance;

    CxxPrivate* p;
};

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url);


G_DEFINE_TYPE(RtspMediaFactory, rtsp_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)


RtspMediaFactory* rtsp_media_factory_new(const URL& splashSource)
{
    RtspMediaFactory* instance =
        _RTSP_MEDIA_FACTORY(g_object_new(TYPE_RTSP_MEDIA_FACTORY, NULL));

    if(instance)
        instance->p->splashSource = splashSource;

    return instance;
}

static void rtsp_media_factory_class_init(RtspMediaFactoryClass* klass)
{
    GstRTSPMediaFactoryClass* parent_klass = GST_RTSP_MEDIA_FACTORY_CLASS(klass);

    parent_klass->create_element = create_element;
}

static void rtsp_media_factory_init(RtspMediaFactory* self)
{
    self->p = new CxxPrivate;

    GstRTSPMediaFactory* parent = GST_RTSP_MEDIA_FACTORY(self);

    gst_rtsp_media_factory_set_transport_mode(
        parent, GST_RTSP_TRANSPORT_MODE_PLAY_RECORD);
    gst_rtsp_media_factory_set_shared(parent, TRUE);

    gst_rtsp_media_factory_set_media_gtype(parent, TYPE_RESTREAM_RTSP_MEDIA);
}

static GstElement* create_element(GstRTSPMediaFactory* factory, const GstRTSPUrl* url)
{
    RtspMediaFactory* self = _RTSP_MEDIA_FACTORY(factory);

    return restream_rtsp_media_create_element(self->p->splashSource);
}

}
