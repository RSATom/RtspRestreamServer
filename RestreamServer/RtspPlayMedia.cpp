#include "RtspPlayMedia.h"

#include <glib.h>

#include "Common/GstPtr.h"

#include "Log.h"


namespace RestreamServer
{

struct _RtspPlayMedia
{
    GstRTSPMedia parent_instance;

    bool sourceSelected;
    GstElement* selector;
    GstElement* proxy;

    GstPad* selectorTestCardPad;

    GstPad* sourceParseSrcPad;
    GstPad* selectorSourcePad;

    gulong sourcePadProbe;
    guint checkTimeout;

    GstClockTime lastBufferTime;
};


G_DEFINE_TYPE(RtspPlayMedia, rtsp_play_media, GST_TYPE_RTSP_MEDIA)


GstElement* rtsp_play_media_create_element(const URL& splashSource)
{
    const gchar* pipeline =
        "rtspsrc name=src ! rtph264depay ! h264parse name=testCardParse ! selector. "
        "proxysrc name=proxy ! h264parse name=sourceParse ! selector. "
        "input-selector cache-buffers=true sync-mode=1 name=selector "
        "selector. ! rtph264pay config-interval=-1 pt=96 name=pay0 ";

    GError* error = nullptr;
    GstElement* element =
        gst_parse_launch_full(
            pipeline, NULL, GST_PARSE_FLAG_PLACE_IN_BIN,
            &error);
    GstGErrorPtr errorPtr(error);

    if(errorPtr)
        Log()->critical(
            "Fail to create play pipeline: {}",
            errorPtr->message);

    if(element) {
        GstElement* rtspsrc = gst_bin_get_by_name(GST_BIN(element), "src");
        if(rtspsrc) {
            g_object_set(rtspsrc, "location", splashSource.c_str(), NULL);
            gst_object_unref(rtspsrc);
        }
    }

    return element;
}

static void
constructed(GObject* object)
{
    Log()->trace(">> RtspPlayMedia.constructed");

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(object);

    GstRTSPMedia* selfMedia = GST_RTSP_MEDIA(object);

    GstElementPtr pipelinePtr(gst_rtsp_media_get_element(selfMedia));
    GstElement* pipeline = pipelinePtr.get();

    GstElementPtr testCardParsePtr(gst_bin_get_by_name(GST_BIN(pipeline), "testCardParse"));
    GstElement* testCardParse = testCardParsePtr.get();

    GstPadPtr testCardParseSrcPadPtr(gst_element_get_static_pad(testCardParse, "src"));
    GstPad* testCardParseSrcPad = testCardParseSrcPadPtr.get();

    GstPadPtr selectorTestCardPadPtr(gst_pad_get_peer(testCardParseSrcPad));
    self->selectorTestCardPad = selectorTestCardPadPtr.get(); // FIXME! should we keep ref?

    GstElementPtr sourceParsePtr(gst_bin_get_by_name(GST_BIN(pipeline), "sourceParse"));
    GstElement* sourceParse = sourceParsePtr.get();

    GstPadPtr parseSrcPadPtr(gst_element_get_static_pad(sourceParse, "src"));
    self->sourceParseSrcPad = parseSrcPadPtr.get();

    GstPadPtr selectorSourcePadPtr(gst_pad_get_peer(self->sourceParseSrcPad));
    self->selectorSourcePad = selectorSourcePadPtr.get(); // FIXME! should we keep ref?

    GstElementPtr selectorPtr(gst_bin_get_by_name(GST_BIN(pipeline), "selector"));
    self->selector = selectorPtr.get(); // FIXME! should we keep ref?

    GstElementPtr proxyPtr(gst_bin_get_by_name(GST_BIN(pipeline), "proxy"));
    self->proxy = proxyPtr.get(); // FIXME! should we keep ref?

    Log()->trace("<< RtspPlayMedia.constructed");
}

static void
switchSelector(RtspPlayMedia* self, bool selectSource)
{
    GstElement* selector = self->selector;

    if(selectSource) {
        Log()->debug("RtspPlayMedia. Switching to source.");
        self->sourceSelected = true; // FIXME! should we do it thread safe?
        g_object_set(G_OBJECT(selector), "active-pad", self->selectorSourcePad, NULL);
    } else {
        Log()->debug("RtspPlayMedia. Switchng to splash screen.");
        self->sourceSelected = false; // FIXME! should we do it thread safe?
        g_object_set(G_OBJECT(self->selector), "active-pad", self->selectorTestCardPad, NULL);
    }
}

static GstPadProbeReturn
onSourcePadData(GstPad* pad,
                GstPadProbeInfo* info,
                gpointer userData)
{
    Log()->trace(
        ">> RtspPlayMedia.onSourcePadData. pad: {}",
        static_cast<void*>(pad));

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(userData);

    GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if(!buffer)
        return GST_PAD_PROBE_OK;

    if(!self->sourceSelected)
        switchSelector(self, true);

    GstClock* clock = gst_element_get_clock(self->selector);
    if(clock) {
        self->lastBufferTime = gst_clock_get_time(clock);
        gst_object_unref(clock);
    }

    Log()->trace("<< RtspPlayMedia.onSourcePadData");

    return GST_PAD_PROBE_OK;
}

static gboolean
checkSourceTimeout(gpointer userData)
{
    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(userData);

    Log()->trace(">> RtspPlayMedia.checkSourceTimeout");

    if(self->sourceSelected) {
        GstClock* clock = gst_element_get_clock(self->selector);
        if(clock) {
            GstClockTime currentTime = gst_clock_get_time(clock);

            if(GST_CLOCK_DIFF(self->lastBufferTime, currentTime) > GST_SECOND)
                switchSelector(self, false);

            gst_object_unref(clock);
        }
    }

    Log()->trace("<< RtspPlayMedia.checkSourceTimeout");

    return GST_PAD_PROBE_OK;
}

static void
prepared(
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    Log()->trace(">> RtspPlayMedia.prepared");

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(media);

    switchSelector(self, false);

    self->sourcePadProbe =
        gst_pad_add_probe(
            self->sourceParseSrcPad,
            GST_PAD_PROBE_TYPE_BUFFER, onSourcePadData,
            self, NULL);

    self->checkTimeout =
        g_timeout_add_seconds(1, checkSourceTimeout, self);

    Log()->trace("<< RtspPlayMedia.prepared");
}

static void
unprepared(
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    Log()->trace(">> RtspPlayMedia.unprepared");

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(media);

    gst_pad_remove_probe(self->sourceParseSrcPad, self->sourcePadProbe);
    self->sourcePadProbe = 0;

    g_source_remove(self->checkTimeout);
    self->checkTimeout = 0;

    Log()->trace("<< RtspPlayMedia.unprepared");
}

static void
rtsp_play_media_class_init(RtspPlayMediaClass* klass)
{
    // GstRTSPMediaClass* parent_klass = GST_RTSP_MEDIA_CLASS(klass);

    GObjectClass* objectKlass = G_OBJECT_CLASS(klass);

    objectKlass->constructed = constructed;
}

static void
rtsp_play_media_init(RtspPlayMedia* self)
{
    // GstRTSPMedia* parent = GST_RTSP_MEDIA(self);

    self->selector = nullptr;
    self->proxy = nullptr;

    self->selectorTestCardPad = nullptr;

    self->sourceParseSrcPad = nullptr;
    self->selectorSourcePad = nullptr;

    self->sourcePadProbe = 0;
    self->checkTimeout = 0;

    self->lastBufferTime = 0;
    self->sourceSelected = false;

    g_signal_connect(self, "prepared", G_CALLBACK(prepared), nullptr);
    g_signal_connect(self, "unprepared", G_CALLBACK(unprepared), nullptr);
}

void rtsp_play_media_set_proxy_sink(RtspPlayMedia* self, GstElement* proxySink)
{
    g_object_set(self->proxy, "proxysink", proxySink, NULL);
}

}
