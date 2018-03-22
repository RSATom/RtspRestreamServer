#pragma once

#include <gst/rtsp-server/rtsp-server.h>


namespace RestreamServer
{

G_BEGIN_DECLS

#define TYPE_RTSP_MOUNT_POINTS rtsp_mount_points_get_type()
G_DECLARE_FINAL_TYPE(RtspMountPoints, rtsp_mount_points, , RTSP_MOUNT_POINTS, GstRTSPMountPoints)

RtspMountPoints* rtsp_mount_points_new();

G_END_DECLS

}
