#pragma once

#include <gio/gio.h>

#include <gst/rtsp-server/rtsp-server.h>

#include "GstPtr.h"


struct GstRtspServerUnref
{
    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (GstRTSPServer* server)
        { (*this)(G_OBJECT(server)); }
    void operator() (GstRTSPMountPoints* mountPoints)
        { (*this)(G_OBJECT(mountPoints)); }
    void operator() (GstRTSPAuth* auth)
        { (*this)(G_OBJECT(auth)); }
    void operator() (GstRTSPToken* token)
        { gst_rtsp_token_unref(token); }

    void operator() (GTlsCertificate* certificate)
        { (*this)(G_OBJECT(certificate)); }
};

typedef
    std::unique_ptr<
        GstRTSPServer,
        GstRtspServerUnref> GstRTSPServerPtr;
typedef
    std::unique_ptr<
        GstRTSPMountPoints,
        GstRtspServerUnref> GstRTSPMountPointsPtr;
typedef
    std::unique_ptr<
        GstRTSPAuth,
        GstRtspServerUnref> GstRTSPAuthPtr;
typedef
    std::unique_ptr<
        GstRTSPToken,
        GstRtspServerUnref> GstRTSPTokenPtr;

typedef
    std::unique_ptr<
        GTlsCertificate,
        GstRtspServerUnref> GTlsCertificatePtr;
