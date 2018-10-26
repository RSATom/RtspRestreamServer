#pragma once

#include <memory>

#include <glib.h>


struct GlibUnref
{
    void operator() (GError* error)
        { g_error_free(error); }

    void operator() (GObject* object)
        { g_object_unref(object); }

    void operator() (GMainContext* context)
        { g_main_context_unref(context); }

    void operator() (GMainLoop* loop)
        { g_main_loop_unref(loop); }

    void operator() (GSource* source)
        { g_source_unref(source); }

    void operator() (GList* list)
        { g_list_free(list); }

    void operator() (gchar* str)
        { g_free(str); }
};

typedef
    std::unique_ptr<
        GError,
        GlibUnref> GErrorPtr;
typedef
    std::unique_ptr<
        GObject,
        GlibUnref> GObjectPtr;
typedef
    std::unique_ptr<
        GMainContext,
        GlibUnref> GMainContextPtr;
typedef
    std::unique_ptr<
        GMainLoop,
        GlibUnref> GMainLoopPtr;
typedef
    std::unique_ptr<
        GSource,
        GlibUnref> GSourcePtr;
typedef
    std::unique_ptr<
        GList,
        GlibUnref> GListPtr;
typedef
    std::unique_ptr<
        gchar,
        GlibUnref> GCharPtr;


struct GObjectWeakRef
{
    GObjectWeakRef(GObject* object = nullptr)
        { g_weak_ref_init(&ref, object); }

    ~GObjectWeakRef()
        { g_weak_ref_clear(&ref); }

    GObjectPtr get()
        { return GObjectPtr(G_OBJECT(g_weak_ref_get(&ref))); }

    void reset(GObject* object)
        { g_weak_ref_set(&ref, object); }

    GWeakRef ref;
};
