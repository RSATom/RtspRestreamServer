#include "RtspMountPoints.h"

#include <cassert>

#include <map>

#include "Config.h"
#include "Common/GstRtspServerPtr.h"
#include "RtspMediaFactory.h"
#include "StaticSources.h"


namespace RestreamServer
{

namespace
{

struct CxxPrivate
{
    std::map<std::string, GstRTSPMediaFactory*> factories;
};

}

struct _RtspMountPoints
{
    GstRTSPMountPoints parent_instance;

    CxxPrivate* p;
};


G_DEFINE_TYPE(RtspMountPoints, rtsp_mount_points, GST_TYPE_RTSP_MOUNT_POINTS)


static gchar* make_path(GstRTSPMountPoints* mountPoints, const GstRTSPUrl* url);


RtspMountPoints* rtsp_mount_points_new()
{
    RtspMountPoints* instance =
        _RTSP_MOUNT_POINTS(g_object_new(TYPE_RTSP_MOUNT_POINTS, NULL));

    return instance;
}

static void rtsp_mount_points_class_init(RtspMountPointsClass* klass)
{
    GstRTSPMountPointsClass* gst_mount_points_klass =
        GST_RTSP_MOUNT_POINTS_CLASS(klass);

    gst_mount_points_klass->make_path = make_path;

    GObjectClass* object_klass = G_OBJECT_CLASS(klass);
    object_klass->finalize =
        [] (GObject* gobject) {
            RtspMountPoints* self = _RTSP_MOUNT_POINTS(gobject);
            delete self->p;
            self->p = nullptr;
        };
}

static void rtsp_mount_points_init(RtspMountPoints* self)
{
    self->p = new CxxPrivate;
}

static gchar* make_path(GstRTSPMountPoints* mountPoints, const GstRTSPUrl* url)
{
    RtspMountPoints* self = _RTSP_MOUNT_POINTS(mountPoints);

    GstRTSPContext* context = gst_rtsp_context_get_current();
    assert(context);
    if(!context)
        return nullptr;

    static const URL splashSource =
        "rtsp://localhost:" STATIC_SERVER_PORT_STR BLUE;

    CxxPrivate& p = *self->p;

    auto factoryIt = p.factories.find(url->abspath);
    if(p.factories.end() == factoryIt) {
        GstRTSPMediaFactory* factory =
            GST_RTSP_MEDIA_FACTORY(rtsp_media_factory_new(splashSource));
        gst_rtsp_mount_points_add_factory(mountPoints, url->abspath, factory);

        p.factories.emplace(url->abspath, factory);
    }

    return g_strdup(url->abspath);
}

}
