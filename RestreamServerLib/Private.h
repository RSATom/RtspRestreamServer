#pragma once

#include <glib.h>
#include <gst/rtsp/gstrtspdefs.h>
#include <gst/rtsp/gstrtspurl.h>


namespace RestreamServerLib
{
namespace Private
{

extern const gchar* RecordSuffix;

bool IsRecordUrl(GstRTSPMethod, const GstRTSPUrl*);

}
}
