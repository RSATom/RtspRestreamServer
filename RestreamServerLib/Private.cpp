#include "Private.h"


namespace RestreamServerLib
{
namespace Private
{

#define RECORD_SUFFIX "record"

const gchar* RecordSuffix= RECORD_SUFFIX;

bool IsRecordUrl(GstRTSPMethod method, const GstRTSPUrl* url)
{
    bool record = false;
    if(url->query != nullptr) {
        if(0 == g_strcmp0(url->query, RecordSuffix) ||
            (GST_RTSP_SETUP == method &&
             g_str_has_prefix(url->query, RECORD_SUFFIX "/"))) {
            record = true;
        }
    }

    return record;
}

}
}
