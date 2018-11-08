#include "RtspPlayMedia.h"

#include <glib.h>

#include <CxxPtr/GlibPtr.h>
#include <CxxPtr/GstPtr.h>

#include "Log.h"


namespace RestreamServerLib
{

struct _RtspPlayMedia
{
    GstRTSPMedia parent_instance;

    bool sourceSelected;
    GstElement* selector;

    GstPad* selectorTestCardPad;
    GstPad* selectorSourcePad;

    GstPad* sourcePad;

    gulong sourcePadProbe;
    guint checkTimeout;

    GstClockTime lastBufferTime;
};


G_DEFINE_TYPE(
    RtspPlayMedia,
    rtsp_play_media,
    GST_TYPE_RTSP_MEDIA)


GstElement*
rtsp_play_media_create_element(
    const URL& splashSource,
    const std::string& listenTo)
{
    const std::string pipeline =
        fmt::format(
           "rtspsrc name=src ! rtph264depay ! h264parse name=testCardParse ! selector. "
           "interpipesrc format=time listen-to={} ! selector. "
           "input-selector cache-buffers=true sync-mode=1 name=selector "
           "selector. ! rtph264pay config-interval=-1 pt=96 name=pay0 ",
           listenTo);

    GError* error = nullptr;
    GstElement* element =
        gst_parse_launch_full(
            pipeline.c_str(), NULL, GST_PARSE_FLAG_PLACE_IN_BIN,
            &error);
    GErrorPtr errorPtr(error);

    if(errorPtr)
        Log()->critical(
            "Fail to create play pipeline: {}",
            errorPtr->message);

    if(element) {
        GstElementPtr rtspsrcPtr(gst_bin_get_by_name(GST_BIN(element), "src"));
        GstElement* rtspsrc = rtspsrcPtr.get();
        if(rtspsrc)
            g_object_set(rtspsrc, "location", splashSource.c_str(), NULL);
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

    GstElementPtr selectorPtr(gst_bin_get_by_name(GST_BIN(pipeline), "selector"));
    self->selector = selectorPtr.get(); // FIXME! should we keep ref?

    GstElementPtr testCardParsePtr(gst_bin_get_by_name(GST_BIN(pipeline), "testCardParse"));
    GstElement* testCardParse = testCardParsePtr.get();

    GstPadPtr testCardParseSrcPadPtr(gst_element_get_static_pad(testCardParse, "src"));
    GstPad* testCardParseSrcPad = testCardParseSrcPadPtr.get();

    GstPadPtr selectorTestCardPadPtr(gst_pad_get_peer(testCardParseSrcPad));
    self->selectorTestCardPad = selectorTestCardPadPtr.get(); // FIXME! should we keep ref?

    gst_element_foreach_sink_pad(self->selector,
        [] (GstElement* /*element*/,
            GstPad* pad,
            gpointer userData) -> gboolean
        {
            RtspPlayMedia* selfMedia = static_cast<RtspPlayMedia*>(userData);
            if(pad != selfMedia->selectorTestCardPad) {
                selfMedia->selectorSourcePad = pad;
                return FALSE;
            }
            return TRUE;
        }, self);

    GstPadPtr sourcePadPtr(gst_pad_get_peer(self->selectorSourcePad));
    self->sourcePad = sourcePadPtr.get(); // FIXME! should we keep ref?

    Log()->trace("<< RtspPlayMedia.constructed");
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


    GstClockPtr clockPtr(gst_element_get_clock(self->selector));
    GstClock* clock = clockPtr.get();
    if(clock) {
        const GstClockTime bufferTime = gst_clock_get_time(clock);

        // Log()->debug("Buffer. Pts: {}, clock: {}", GST_BUFFER_PTS(buffer), bufferTime);

        self->lastBufferTime = bufferTime;
    }

    Log()->trace("<< RtspPlayMedia.onSourcePadData");

    return GST_PAD_PROBE_OK;
}

static void
switchSelector(
    RtspPlayMedia* self,
    bool selectSource)
{
    GstElement* selector = self->selector;

    if(selectSource && !self->sourceSelected) {
        Log()->debug("RtspPlayMedia. Switching to source.");
        self->sourceSelected = true;
        g_object_set(G_OBJECT(selector), "active-pad", self->selectorSourcePad, NULL);
    } else if(!selectSource && self->sourceSelected) {
        Log()->debug("RtspPlayMedia. Switching to splash screen.");
        self->sourceSelected = false;
        g_object_set(G_OBJECT(self->selector), "active-pad", self->selectorTestCardPad, NULL);
    }
}

static gboolean
checkSourceTimeout(
    gpointer userData)
{
    Log()->trace(">> RtspPlayMedia.checkSourceTimeout");

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(userData);

    GstClockPtr clockPtr(gst_element_get_clock(self->selector));
    GstClock* clock = clockPtr.get();
    if(clock) {
        GstClockTime currentTime = gst_clock_get_time(clock);

        const bool timeout =
            GST_CLOCK_DIFF(self->lastBufferTime, currentTime) > (2 * GST_SECOND);

        Log()->debug("Buffer timeout: {}", timeout);

        switchSelector(self, !timeout);
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

    self->sourceSelected = false;
    g_object_set(G_OBJECT(self->selector), "active-pad", self->selectorTestCardPad, NULL);

    self->sourcePadProbe =
        gst_pad_add_probe(
            self->sourcePad,
            GST_PAD_PROBE_TYPE_BUFFER, onSourcePadData,
            self, NULL);

    self->checkTimeout =
        g_timeout_add(500, checkSourceTimeout, self);

    Log()->trace("<< RtspPlayMedia.prepared");
}

static void
unprepared(
    GstRTSPMedia* media,
    gpointer /*userData*/)
{
    Log()->trace(">> RtspPlayMedia.unprepared");

    RtspPlayMedia* self = _RTSP_PLAY_MEDIA(media);

    gst_pad_remove_probe(self->sourcePad, self->sourcePadProbe);
    self->sourcePadProbe = 0;

    g_source_remove(self->checkTimeout);
    self->checkTimeout = 0;

    Log()->trace("<< RtspPlayMedia.unprepared");
}

static void
rtsp_play_media_class_init(
    RtspPlayMediaClass* klass)
{
    // GstRTSPMediaClass* parent_klass = GST_RTSP_MEDIA_CLASS(klass);

    GObjectClass* objectKlass = G_OBJECT_CLASS(klass);

    objectKlass->constructed = constructed;
}

static void
rtsp_play_media_init(
    RtspPlayMedia* self)
{
    // GstRTSPMedia* parent = GST_RTSP_MEDIA(self);

    self->selector = nullptr;

    self->selectorTestCardPad = nullptr;

    self->sourcePad = nullptr;
    self->selectorSourcePad = nullptr;

    self->sourcePadProbe = 0;
    self->checkTimeout = 0;

    self->lastBufferTime = 0;
    self->sourceSelected = false;

    g_signal_connect(self, "prepared", G_CALLBACK(prepared), nullptr);
    g_signal_connect(self, "unprepared", G_CALLBACK(unprepared), nullptr);
}

}
