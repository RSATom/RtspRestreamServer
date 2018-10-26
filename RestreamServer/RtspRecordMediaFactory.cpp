#include "RtspRecordMediaFactory.h"

#include "Log.h"
#include "RtspPlayMediaFactory.h"


namespace RestreamServer
{

namespace
{

struct CxxPrivate
{
    std::string proxyName;
};

}

struct _RtspRecordMediaFactory
{
    GstRTSPMediaFactory parent_instance;

    CxxPrivate* p;
};

static GstElement*
create_element(
    GstRTSPMediaFactory* factory,
    const GstRTSPUrl* url);


G_DEFINE_TYPE(
    RtspRecordMediaFactory,
    rtsp_record_media_factory,
    GST_TYPE_RTSP_MEDIA_FACTORY)


RtspRecordMediaFactory*
rtsp_record_media_factory_new(
    const std::string& proxyName)
{
    RtspRecordMediaFactory* instance =
        _RTSP_RECORD_MEDIA_FACTORY(
            g_object_new(TYPE_RTSP_RECORD_MEDIA_FACTORY, NULL));

    if(instance)
        instance->p->proxyName = proxyName;

    return instance;
}

static void
rtsp_record_media_factory_class_init(
    RtspRecordMediaFactoryClass* klass)
{
    GstRTSPMediaFactoryClass* parent_klass =
        GST_RTSP_MEDIA_FACTORY_CLASS(klass);

    parent_klass->create_element = create_element;
}

static void
rtsp_record_media_factory_init(
    RtspRecordMediaFactory* self)
{
    self->p = new CxxPrivate;

    GstRTSPMediaFactory* parent = GST_RTSP_MEDIA_FACTORY(self);

    gst_rtsp_media_factory_set_transport_mode(
        parent, GST_RTSP_TRANSPORT_MODE_RECORD);
    gst_rtsp_media_factory_set_shared(parent, TRUE);

    gst_rtsp_media_factory_set_media_gtype(parent, TYPE_RTSP_RECORD_MEDIA);
}

static GstElement*
create_element(
    GstRTSPMediaFactory* factory,
    const GstRTSPUrl* url)
{
    RtspRecordMediaFactory* self = _RTSP_RECORD_MEDIA_FACTORY(factory);

    return
        rtsp_record_media_create_element(
            self->p->proxyName);
}

}
