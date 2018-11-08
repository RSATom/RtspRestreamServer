#include "RtspAuth.h"

#include <cassert>

#include <CxxPtr/GstRtspServerPtr.h>

#include "Log.h"


namespace RestreamServerLib
{

struct CxxPrivate
{
    AuthCallbacks callbacks;
};

struct _RtspAuth
{
    GstRTSPAuth parent_instance;

    CxxPrivate* p;
};

G_DEFINE_TYPE(
    RtspAuth,
    rtsp_auth,
    GST_TYPE_RTSP_AUTH)

static void
finalize(GObject*);
static gboolean
check(GstRTSPAuth*, GstRTSPContext*, const gchar*);
static gboolean
authenticate(GstRTSPAuth*, GstRTSPContext*);

RtspAuth*
rtsp_auth_new(const AuthCallbacks& callbacks)
{
    RtspAuth* instance = (RtspAuth*)g_object_new(TYPE_RTSP_AUTH, NULL);

    if(instance)
        instance->p->callbacks= callbacks;

    return instance;
}

static void
rtsp_auth_class_init(RtspAuthClass* klass)
{
    GstRTSPAuthClass* gst_auth_klass = GST_RTSP_AUTH_CLASS(klass);

    gst_auth_klass->check = check;
    gst_auth_klass->authenticate = authenticate;

    GObjectClass* object_klass = G_OBJECT_CLASS(klass);
    object_klass->finalize = finalize;
}

static void
rtsp_auth_init(RtspAuth* self)
{
    self->p = new CxxPrivate;
}

static void
finalize(GObject* object)
{
    RtspAuth* self = _RTSP_AUTH(object);
    delete self->p;
    self->p = nullptr;

    G_OBJECT_CLASS(rtsp_auth_parent_class)->finalize(object);
}

static void
send_response(
    GstRTSPAuth* auth,
    GstRTSPStatusCode code,
    GstRTSPContext* ctx)
{
    gst_rtsp_message_init_response(
        ctx->response, code,
        gst_rtsp_status_as_text(code), ctx->request);

    if(code == GST_RTSP_STS_UNAUTHORIZED) {
        GstRTSPAuthClass *klass = GST_RTSP_AUTH_GET_CLASS(auth);

        if(klass->generate_authenticate_header)
            klass->generate_authenticate_header (auth, ctx);
    }

    gst_rtsp_client_send_message(ctx->client, ctx->session, ctx->response);
}

static bool
authentication_required(
    RtspAuth* auth,
    GstRTSPMethod method,
    const GstRTSPUrl* url)
{
    if(auth->p->callbacks.authenticationRequired)
        return auth->p->callbacks.authenticationRequired(method, url->abspath);
    else
        return false;
}

static bool
authenticate(
    RtspAuth* auth,
    const gchar* login,
    const gchar* pass)
{
    if(auth->p->callbacks.authenticate)
        return auth->p->callbacks.authenticate(login, pass);
    else if(login[0] == '\0')
        return true;
    else
        return false;
}

static bool
authorize(
    RtspAuth* auth,
    const gchar* userName,
    Action action,
    const GstRTSPUrl* url)
{
    if(auth->p->callbacks.authorize)
        return auth->p->callbacks.authorize(userName, action, url->abspath);
    else if(userName[0] == '\0')
        return true;
    else
        return false;
}

static void
basic_authenticate(
    RtspAuth* auth,
    GstRTSPAuthCredential* credential,
    GstRTSPContext* ctx)
{
    if(ctx->token) {
        Log()->debug("Already authenticated. Skipping.");
        return;
    }

    gchar* loginAndPass = NULL;

    gsize decodedLen;
    guchar* decoded = g_base64_decode(credential->authorization, &decodedLen);
    if(decoded) {
        loginAndPass = g_strndup((gchar*)decoded, decodedLen);
        g_free(decoded);
    }

    gchar** tokens = g_strsplit(loginAndPass, ":", 2);
    g_free(loginAndPass);

    const gchar* login = NULL;
    const gchar* pass = NULL;
    for(unsigned i = 0; i < 2 && tokens[i] != NULL; ++i) {
        switch(i) {
            case 0:
                login = tokens[i];
                break;
            case 1:
                pass = tokens[i];
                break;
        }
    }

    if(authenticate(auth, login, pass)) {
        ctx->token =
            gst_rtsp_token_new(
                GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE, G_TYPE_STRING,
                login, NULL);
    }

    g_strfreev(tokens);
}

static gboolean
authenticate(
    GstRTSPAuth* auth,
    GstRTSPContext* ctx)
{
    if(ctx->token) {
        Log()->debug("Already authenticated. Skipping.");
        return TRUE;
    }

    RtspAuth* self = _RTSP_AUTH(auth);

    GstRTSPToken* defaultToken = gst_rtsp_auth_get_default_token(auth);
    // FIXME! it looks like we shouldn't add ref to token. Look rtsp-auth.c:747
    gst_rtsp_token_unref(defaultToken);

    GstRTSPAuthCredential**credentials =
        gst_rtsp_message_parse_auth_credentials(ctx->request,
        GST_RTSP_HDR_AUTHORIZATION);
    if(!credentials) {
        ctx->token = defaultToken;
        goto no_auth;
    }

    for(GstRTSPAuthCredential** credential = credentials; *credential; ++credential) {
        if((*credential)->scheme == GST_RTSP_AUTH_BASIC) {
            basic_authenticate(self, *credential, ctx);
        } else if((*credential)->scheme == GST_RTSP_AUTH_DIGEST) {
        }

        if(ctx->token)
            break;
    }

    gst_rtsp_auth_credentials_free(credentials);

    if(!ctx->token) {
        ctx->token = defaultToken;
        Log()->debug("authenticate. Default token used");
    }

    return TRUE;

no_auth:
    {
        GST_DEBUG_OBJECT(auth, "no authorization header found");
        return TRUE;
    }
}

static gboolean
ensure_authenticated(
    GstRTSPAuth* auth,
    GstRTSPContext* ctx)
{
    GstRTSPAuthClass* klass = GST_RTSP_AUTH_GET_CLASS(auth);

    /* we need a token to check */
    if(ctx->token == NULL) {
        if(klass->authenticate) {
            if(!klass->authenticate(auth, ctx))
                return FALSE;
        }
    }

    if(!ctx->token)
        return FALSE;

    return TRUE;
}

static gboolean
check(
    GstRTSPAuth* auth,
    GstRTSPContext* ctx,
    const gchar* check)
{
    Log()->trace(">> RtspAuth.check. {}", check);

    RtspAuth* self = _RTSP_AUTH(auth);

    GstRTSPTokenPtr defaultTokenPtr(gst_rtsp_auth_get_default_token(auth));
    GstRTSPToken* defaultToken = defaultTokenPtr.get();

    gboolean success = FALSE;

    if(g_str_equal(check, GST_RTSP_AUTH_CHECK_URL)) {
        if(ensure_authenticated(auth, ctx) && ctx->token) {
            const bool authRequired = authentication_required(self, ctx->method, ctx->uri);
            const bool authenticated = (ctx->token != defaultToken);
            if(!authRequired || authenticated)
               success = TRUE;
        }
    } else if(g_str_has_prefix(check, "auth.check.media.factory.")) {
        if(ctx->token) {
            const gchar* user =
                gst_rtsp_token_get_string(ctx->token, GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE);

            if(g_str_equal(check, GST_RTSP_AUTH_CHECK_MEDIA_FACTORY_ACCESS))
                success = authorize(self, user, Action::ACCESS, ctx->uri);
            else if(g_str_equal(check, GST_RTSP_AUTH_CHECK_MEDIA_FACTORY_CONSTRUCT))
                success = authorize(self, user, Action::CONSTRUCT, ctx->uri);
        }
    } else
        return GST_RTSP_AUTH_CLASS(rtsp_auth_parent_class)->check(auth, ctx, check);

    if(success)
        return TRUE;
    else {
        send_response(auth, GST_RTSP_STS_UNAUTHORIZED, ctx);
        Log()->debug("\"{}\" unauthorized", ctx->uri->abspath);
        return FALSE;
    }

    return TRUE;
}

}
