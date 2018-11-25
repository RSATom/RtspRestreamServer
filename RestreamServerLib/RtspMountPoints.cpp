#include "RtspMountPoints.h"

#include <cassert>

#include <set>
#include <map>

#include <CxxPtr/GlibPtr.h>
#include <CxxPtr/GstRtspServerPtr.h>

#include "Log.h"
#include "RtspRecordMediaFactory.h"
#include "RtspPlayMediaFactory.h"
#include "StaticSources.h"
#include "Private.h"


namespace RestreamServerLib
{

namespace
{

struct CxxPrivate
{
    MountPointsCallbacks callbacks;
    std::string splashSource;
    unsigned maxPathsCount;
    unsigned maxClientsPerPath;

    std::map<std::string, uint32_t> pathsRefs;
    std::map<GstRTSPClient*, std::set<std::string> > clientsToPaths;
};

}

struct _RtspMountPoints
{
    GstRTSPMountPoints parent_instance;

    uint64_t proxy;

    CxxPrivate* p;
};


G_DEFINE_TYPE(
    RtspMountPoints,
    rtsp_mount_points,
    GST_TYPE_RTSP_MOUNT_POINTS)


static gchar*
make_path(GstRTSPMountPoints* mountPoints, const GstRTSPUrl* url);


RtspMountPoints*
rtsp_mount_points_new(
    const MountPointsCallbacks& callbacks,
    const std::string& splashSource,
    unsigned maxPathsCount,
    unsigned maxClientsPerPath)
{
    RtspMountPoints* instance =
        _RTSP_MOUNT_POINTS(g_object_new(TYPE_RTSP_MOUNT_POINTS, NULL));

    if(instance) {
        instance->p->callbacks = callbacks;
        instance->p->splashSource = splashSource;
        instance->p->maxPathsCount = maxPathsCount;
        instance->p->maxClientsPerPath = maxPathsCount;
    }

    return instance;
}

static void
rtsp_mount_points_class_init(RtspMountPointsClass* klass)
{
    GstRTSPMountPointsClass* gst_mount_points_klass =
        GST_RTSP_MOUNT_POINTS_CLASS(klass);

    gst_mount_points_klass->make_path = make_path;

    GObjectClass* object_klass = G_OBJECT_CLASS(klass);
    object_klass->finalize =
        [] (GObject* object) {
            RtspMountPoints* self = _RTSP_MOUNT_POINTS(object);
            delete self->p;
            self->p = nullptr;

            G_OBJECT_CLASS(rtsp_mount_points_parent_class)->finalize(object);
        };
}

static void
rtsp_mount_points_init(RtspMountPoints* self)
{
    self->proxy = 0;
    self->p = new CxxPrivate;
}

static void
client_closed(GstRTSPClient* client, gpointer userData)
{
    RtspMountPoints* self = _RTSP_MOUNT_POINTS(userData);

    CxxPrivate& p = *self->p;

    auto clientPathsIt = p.clientsToPaths.find(client);
    if(clientPathsIt == p.clientsToPaths.end()) {
        Log()->debug(
            "Client didn't use any path. client: {}",
            static_cast<const void*>(client));
        return;
    } else {
        for(const std::string& path: clientPathsIt->second) {
            auto pathRefsIt = p.pathsRefs.find(path);
            if(pathRefsIt == p.pathsRefs.end())
                Log()->critical("Inconsistent data in mount points reference counting");
            else {
                --pathRefsIt->second;
                if(0 == pathRefsIt->second) {
                    Log()->debug(
                        "Removing unused mount point. last client: {}, path: {}",
                        static_cast<const void*>(client), path);
                    gst_rtsp_mount_points_remove_factory(
                        GST_RTSP_MOUNT_POINTS(self),
                        path.data());
                    const std::string recordPath = path + Private::RecordSuffix;
                    gst_rtsp_mount_points_remove_factory(
                        GST_RTSP_MOUNT_POINTS(self),
                        recordPath.data());
                    p.pathsRefs.erase(pathRefsIt);
                } else {
                    Log()->debug(
                        "Path ref count decreased. client: {}, path: {}, refs: {}",
                        static_cast<const void*>(client), path, pathRefsIt->second);
                }
            }
        }
        p.clientsToPaths.erase(clientPathsIt);
    }
}

static bool
authorize_access(
    RtspMountPoints* self,
    GstRTSPContext* context,
    const GstRTSPUrl* url)
{
    bool record = false;
    if(url->query != nullptr ) {
        if(0 == g_strcmp0(url->query, Private::RecordSuffix))
            record = true;
        else
            return false;
    }

    if(self->p->callbacks.authorizeAccess) {
        const gchar* user = nullptr;
        if(context->token) {
            user =
                gst_rtsp_token_get_string(
                    context->token,
                    GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE);
        }

        return
            self->p->callbacks.authorizeAccess(
                user ? user : "",
                url->abspath,
                record);
    } else
        return true;
}

static gchar*
make_path(GstRTSPMountPoints* mountPoints, const GstRTSPUrl* url)
{
    RtspMountPoints* self = _RTSP_MOUNT_POINTS(mountPoints);

    GstRTSPContext* context = gst_rtsp_context_get_current();
    assert(context);
    if(!context)
        return nullptr;

    if(!authorize_access(self, context, url))
        return nullptr;

    const std::string path = url->abspath;
    const bool isRecord = (g_strcmp0(url->query, "record") == 0);

    Log()->debug("make_path. client: {}, path: {}",
        static_cast<const void*>(context->client), path);

    CxxPrivate& p = *self->p;

    auto pathRefsIt = p.pathsRefs.find(path);
    if(self->p->maxPathsCount > 0 &&
       pathRefsIt == p.pathsRefs.end() &&
       p.pathsRefs.size() >= self->p->maxPathsCount)
    {
        Log()->info(
            "Max paths count reached. client: {}, path: {}, count {}",
            static_cast<const void*>(context->client), path, self->p->maxPathsCount);

        return nullptr;
    }

    if(self->p->maxClientsPerPath > 0 &&
       pathRefsIt != p.pathsRefs.end() &&
       pathRefsIt->second >= self->p->maxClientsPerPath)
    {
        Log()->info(
            "Max clients count per path reached. client: {}, path: {}, count {}",
            static_cast<const void*>(context->client), path, self->p->maxClientsPerPath);

        return nullptr;
    }

    bool addPathRef = false;
    auto clientPathsIt = p.clientsToPaths.find(context->client);
    if(clientPathsIt == p.clientsToPaths.end()) {
        Log()->debug(
            "Path request from new client. client: {}, path: {}",
            static_cast<const void*>(context->client), path);

        g_signal_connect(context->client, "closed", GCallback(client_closed), mountPoints);
        p.clientsToPaths.emplace(context->client, std::set<std::string>{path});
        addPathRef = true;
    } else {
        Log()->debug(
            "Client requesting path. client: {}, path: {}",
            static_cast<const void*>(context->client), path);
        addPathRef = clientPathsIt->second.insert(path).second;
    }

    if(p.pathsRefs.end() == pathRefsIt) {
        Log()->debug(
            "Creating mount point. client: {}, path: {}",
            static_cast<const void*>(context->client), path);

        assert(addPathRef);

        const std::string proxyName =
            fmt::format("proxy{}", self->proxy++);

        RtspPlayMediaFactory* playFactory =
            rtsp_play_media_factory_new(
                self->p->splashSource.c_str(),
                proxyName.c_str());
        RtspRecordMediaFactory* recordFactory =
            rtsp_record_media_factory_new(proxyName.c_str());

        gst_rtsp_mount_points_add_factory(
            mountPoints, url->abspath, GST_RTSP_MEDIA_FACTORY(playFactory));
        GCharPtr recordUrl(g_strconcat(url->abspath, "?", Private::RecordSuffix, nullptr));
        gst_rtsp_mount_points_add_factory(
            mountPoints, recordUrl.get(), GST_RTSP_MEDIA_FACTORY(recordFactory));

        p.pathsRefs.emplace(path, 1);
    } else {
        ++(pathRefsIt->second);
        Log()->debug(
            "Path ref count increased. client: {}, path: {}, refs: {}",
            static_cast<const void*>(context->client), path, pathRefsIt->second);
    }

    return
        isRecord ?
            g_strconcat(url->abspath, "?record", nullptr) :
            g_strdup(url->abspath);
}

}
