#pragma once

#include <functional>

#include <gst/rtsp-server/rtsp-server.h>


namespace RestreamServerLib
{

struct MountPointsCallbacks
{
    std::function<bool (const std::string& user, const std::string& path)> authorizeAccess;
};

G_BEGIN_DECLS

#define TYPE_RTSP_MOUNT_POINTS rtsp_mount_points_get_type()
G_DECLARE_FINAL_TYPE(
    RtspMountPoints,
    rtsp_mount_points,
    ,
    RTSP_MOUNT_POINTS,
    GstRTSPMountPoints)

RtspMountPoints*
rtsp_mount_points_new(
    const MountPointsCallbacks&,
    const std::string& splashSource,
    unsigned maxPathsCount,
    unsigned maxClientsPerPath);

G_END_DECLS

}
