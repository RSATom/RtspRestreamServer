#pragma once

#include <memory>


struct GstUnref
{
    void operator() (GError* error)
        { g_error_free(error); }

    void operator() (GObject* object)
        { gst_object_unref(object); }

    void operator() (GstCaps* caps)
        { gst_caps_unref(caps); }

    void operator() (GstElement* element)
        { (*this)(G_OBJECT(element)); }

    void operator() (GstPad* pad)
        { (*this)(G_OBJECT(pad)); }

};

typedef
    std::unique_ptr<
        GError,
        GstUnref> GstGErrorPtr;

typedef
    std::unique_ptr<
        GstCaps,
        GstUnref> GstCapsPtr;
typedef
    std::unique_ptr<
        GstElement,
        GstUnref> GstElementPtr;
typedef
    std::unique_ptr<
        GstPad,
        GstUnref> GstPadPtr;
