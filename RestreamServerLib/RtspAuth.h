#pragma once

#include <functional>

#include <gst/rtsp-server/rtsp-server.h>

#include "Action.h"


namespace RestreamServerLib
{

struct AuthCallbacks
{
    std::function<bool (GstRTSPMethod method, const std::string& path)> authenticationRequired;
    std::function<bool (const std::string& user, const std::string& pass)> authenticate;
    std::function<bool (const std::string& user, Action, const std::string& path)> authorize;
};

G_BEGIN_DECLS

#define TYPE_RTSP_AUTH rtsp_auth_get_type()
G_DECLARE_FINAL_TYPE(
    RtspAuth,
    rtsp_auth,
    ,
    RTSP_AUTH,
    GstRTSPAuth)

RtspAuth*
rtsp_auth_new(const AuthCallbacks&);

G_END_DECLS

}
