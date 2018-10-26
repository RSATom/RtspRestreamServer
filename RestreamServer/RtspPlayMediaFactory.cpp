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
    std::string listenTo;
};

}

struct _RtspPlayMediaFactory
{
    GstRTSPMediaFactory parent_instance;

    CxxPrivate* p;
};

static GstElement*
create_element(
    GstRTSPMediaFactory* factory,
    const GstRTSPUrl* url);


G_DEFINE_TYPE(
    RtspPlayMediaFactory,
    rtsp_play_media_factory,
    GST_TYPE_RTSP_MEDIA_FACTORY)


RtspPlayMediaFactory*
rtsp_play_media_factory_new(
    const URL& splashSource,
    const std::string& listenTo)
{
    RtspPlayMediaFactory* instance =
        _RTSP_PLAY_MEDIA_FACTORY(
            g_object_new(TYPE_RTSP_PLAY_MEDIA_FACTORY, NULL));

    if(instance) {
        instance->p->splashSource = splashSource;
        instance->p->listenTo = listenTo;
    }

    return instance;
}

static void
rtsp_play_media_factory_class_init(
    RtspPlayMediaFactoryClass* klass)
{
    GstRTSPMediaFactoryClass* parent_klass =
        GST_RTSP_MEDIA_FACTORY_CLASS(klass);

    parent_klass->create_element = create_element;
}

static void
rtsp_play_media_factory_init(
    RtspPlayMediaFactory* self)
{
    self->p = new CxxPrivate;

    GstRTSPMediaFactory* parent = GST_RTSP_MEDIA_FACTORY(self);

    gst_rtsp_media_factory_set_transport_mode(
        parent, GST_RTSP_TRANSPORT_MODE_PLAY);
    gst_rtsp_media_factory_set_shared(parent, TRUE);

    gst_rtsp_media_factory_set_media_gtype(parent, TYPE_RTSP_PLAY_MEDIA);
}

static GstElement*
create_element(
    GstRTSPMediaFactory* factory,
    const GstRTSPUrl* url)
{
    RtspPlayMediaFactory* self = _RTSP_PLAY_MEDIA_FACTORY(factory);

    return
        rtsp_play_media_create_element(
            self->p->splashSource,
            self->p->listenTo);
}

}
