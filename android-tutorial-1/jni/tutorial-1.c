#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <gst/gst.h>
#include <pthread.h>
#include <gst/rtsp-server/rtsp-server.h>

GST_DEBUG_CATEGORY_STATIC (debug_category);
#define GST_CAT_DEFAULT debug_category

/*
 * These macros provide a way to store the native pointer to CustomData, which might be 32 or 64 bits, into
 * a jlong, which is always 64 bits, without warnings.
 */
#if GLIB_SIZEOF_VOID_P == 8
# define GET_CUSTOM_DATA(env, thiz, fieldID) (CustomData *)(*env)->GetLongField (env, thiz, fieldID)
# define SET_CUSTOM_DATA(env, thiz, fieldID, data) (*env)->SetLongField (env, thiz, fieldID, (jlong)data)
#else
# define GET_CUSTOM_DATA(env, thiz, fieldID) (CustomData *)(jint)(*env)->GetLongField (env, thiz, fieldID)
# define SET_CUSTOM_DATA(env, thiz, fieldID, data) (*env)->SetLongField (env, thiz, fieldID, (jlong)(jint)data)
#endif

#define DEFAULT_RTSP_PORT "8554"
#define DEFAULT_DISABLE_RTCP FALSE

static char *port = (char *) DEFAULT_RTSP_PORT;
static gboolean disable_rtcp = DEFAULT_DISABLE_RTCP;

static GOptionEntry entries[] = {
        {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
                "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
        {"disable-rtcp", '\0', 0, G_OPTION_ARG_NONE, &disable_rtcp,
                "Whether RTCP should be disabled (default false)", NULL},
        {NULL}
};

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData
{
    jobject app;                  /* Application instance, used to call its methods. A global reference is kept. */
    GstElement *pipeline;         /* The running pipeline */
    GMainContext *context;        /* GLib context used to run the main loop */
    GMainLoop *main_loop;         /* GLib main loop */
    gboolean initialized;         /* To avoid informing the UI multiple times about the initialization */
} CustomData;

/* These global variables cache values which are not changing during execution */
static pthread_t gst_app_thread;
static pthread_key_t current_jni_env;
static JavaVM *java_vm;
static jfieldID custom_data_field_id;
static jmethodID set_message_method_id;
static jmethodID on_gstreamer_initialized_method_id;

/*
 * Private methods
 */

/* Register this thread with the VM */
static JNIEnv *
attach_current_thread (void)
{
  JNIEnv *env;
  JavaVMAttachArgs args;

  GST_DEBUG ("Attaching thread %p", g_thread_self ());
  args.version = JNI_VERSION_1_4;
  args.name = NULL;
  args.group = NULL;

  if ((*java_vm)->AttachCurrentThread (java_vm, &env, &args) < 0) {
    GST_ERROR ("Failed to attach current thread");
    return NULL;
  }

  return env;
}

/* Unregister this thread from the VM */
static void
detach_current_thread (void *env)
{
  GST_DEBUG ("Detaching thread %p", g_thread_self ());
  (*java_vm)->DetachCurrentThread (java_vm);
}

/* Retrieve the JNI environment for this thread */
static JNIEnv *
get_jni_env (void)
{
  JNIEnv *env;

  if ((env = pthread_getspecific (current_jni_env)) == NULL) {
    env = attach_current_thread ();
    pthread_setspecific (current_jni_env, env);
  }

  return env;
}

/* Change the content of the UI's TextView */
static void
set_ui_message (const gchar * message, CustomData * data)
{
  JNIEnv *env = get_jni_env ();
  GST_DEBUG ("Setting message to: %s", message);
  jstring jmessage = (*env)->NewStringUTF (env, message);
  (*env)->CallVoidMethod (env, data->app, set_message_method_id, jmessage);
  if ((*env)->ExceptionCheck (env)) {
    GST_ERROR ("Failed to call Java method");
    (*env)->ExceptionClear (env);
  }
  (*env)->DeleteLocalRef (env, jmessage);
}

/* Retrieve errors from the bus and show them on the UI */
static void
error_cb (GstBus * bus, GstMessage * msg, CustomData * data)
{
  GError *err;
  gchar *debug_info;
  gchar *message_string;

  gst_message_parse_error (msg, &err, &debug_info);
  message_string =
          g_strdup_printf ("Error received from element %s: %s",
                           GST_OBJECT_NAME (msg->src), err->message);
  g_clear_error (&err);
  g_free (debug_info);
  set_ui_message (message_string, data);
  g_free (message_string);
  gst_element_set_state (data->pipeline, GST_STATE_NULL);
}

/* Notify UI about pipeline state changes */
static void
state_changed_cb (GstBus * bus, GstMessage * msg, CustomData * data)
{
  GstState old_state, new_state, pending_state;
  gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
  /* Only pay attention to messages coming from the pipeline, not its children */
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) {
    gchar *message = g_strdup_printf ("State changed to %s",
                                      gst_element_state_get_name (new_state));
    set_ui_message (message, data);
    g_free (message);
  }
}

/* Check if all conditions are met to report GStreamer as initialized.
 * These conditions will change depending on the application */
static void
check_initialization_complete (CustomData * data)
{
  JNIEnv *env = get_jni_env ();
  if (!data->initialized && data->main_loop) {
    GST_DEBUG ("Initialization complete, notifying application. main_loop:%p",
               data->main_loop);
    (*env)->CallVoidMethod (env, data->app, on_gstreamer_initialized_method_id);
    if ((*env)->ExceptionCheck (env)) {
      GST_ERROR ("Failed to call Java method");
      (*env)->ExceptionClear (env);
    }
    data->initialized = TRUE;
  }
}
//==================================================================================//

/* called when a stream has received an RTCP packet from the client */
static void
on_ssrc_active (GObject * session, GObject * source, GstRTSPMedia * media)
{
  GstStructure *stats;

  GST_INFO ("source %p in session %p is active", source, session);

  g_object_get (source, "stats", &stats, NULL);
  if (stats) {
    gchar *sstr;

    sstr = gst_structure_to_string (stats);
    g_print ("structure: %s\n", sstr);
    g_free (sstr);

    gst_structure_free (stats);
  }
}

static void
on_sender_ssrc_active (GObject * session, GObject * source,
                       GstRTSPMedia * media)
{
  GstStructure *stats;

  GST_INFO ("source %p in session %p is active", source, session);

  g_object_get (source, "stats", &stats, NULL);
  if (stats) {
    gchar *sstr;

    sstr = gst_structure_to_string (stats);
    g_print ("Sender stats:\nstructure: %s\n", sstr);
    g_free (sstr);

    gst_structure_free (stats);
  }
}


/* signal callback when the media is prepared for streaming. We can get the
 * session manager for each of the streams and connect to some signals. */
static void
media_prepared_cb (GstRTSPMedia * media)
{
  guint i, n_streams;

  n_streams = gst_rtsp_media_n_streams (media);

  GST_INFO ("media %p is prepared and has %u streams", media, n_streams);

  for (i = 0; i < n_streams; i++) {
    GstRTSPStream *stream;
    GObject *session;

    stream = gst_rtsp_media_get_stream (media, i);
    if (stream == NULL)
      continue;

    session = gst_rtsp_stream_get_rtpsession (stream);
    GST_INFO ("watching session %p on stream %u", session, i);

    g_signal_connect (session, "on-ssrc-active",
                      (GCallback) on_ssrc_active, media);
    g_signal_connect (session, "on-sender-ssrc-active",
                      (GCallback) on_sender_ssrc_active, media);
  }
}

static void
media_configure_cb (GstRTSPMediaFactory * factory, GstRTSPMedia * media)
{
  /* connect our prepared signal so that we can see when this media is
   * prepared for streaming */
  g_signal_connect (media, "prepared", (GCallback) media_prepared_cb, factory);
}

/* Main method for the native code. This is executed on its own thread. */
static void *
app_function (void *userdata)
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;
  GOptionContext *optctx;
  GError *error = NULL;

  /*optctx = g_option_context_new ("<launch line> - Test RTSP Server, Launch\n\n"
                                 "Example: \"( videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96 )\"");
  g_option_context_add_main_entries (optctx, entries, NULL);
  g_option_context_add_group (optctx, gst_init_get_option_group ());
  g_option_context_free (optctx);
  loop = g_main_loop_new (NULL, FALSE);
  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);
   mounts = gst_rtsp_server_get_mount_points (server);
   factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory,
                                     "( videotestsrc is-live=1 ! x264enc ! rtph264pay name=pay0 pt=96 )");

  gst_rtsp_media_factory_set_shared (factory, TRUE);
  gst_rtsp_media_factory_set_enable_rtcp (factory, !disable_rtcp);
  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);
  g_object_unref (mounts);
  gst_rtsp_server_attach (server, NULL);
  g_print ("stream ready at rtsp://127.0.0.1:%s/test\n", port);
  g_main_loop_run (loop);*/


  gchar *str;

  optctx = g_option_context_new ("<filename.mp4> - Test RTSP Server, MP4");
  g_option_context_add_main_entries (optctx, entries, NULL);
  g_option_context_add_group (optctx, gst_init_get_option_group ());

  g_option_context_free (optctx);

  loop = g_main_loop_new (NULL, FALSE);

  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);

  mounts = gst_rtsp_server_get_mount_points (server);

  str = g_strdup_printf ("( "
                         "filesrc location=\"%s\" ! qtdemux name=d "
                         "d. ! queue ! rtph264pay pt=96 name=pay0 "
                         "d. ! queue ! rtpmp4apay pt=97 name=pay1 " ")",
                         "/sdcard/DCIM/ScreenRecorder/Screenrecorder-2021-12-04-10-29-00-468.mp4");

  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory, str);
  g_signal_connect (factory, "media-configure", (GCallback) media_configure_cb,
                    factory);
  g_free (str);

  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

  g_object_unref (mounts);

  gst_rtsp_server_attach (server, NULL);

  g_print ("stream ready at rtsp://127.0.0.1:%s/test\n", port);
  g_main_loop_run (loop);


  return NULL;
}

/*
 * Java Bindings
 */

/* Instruct the native code to create its internal data structure, pipeline and thread */
static void
gst_native_init (JNIEnv * env, jobject thiz)
{
  CustomData *data = g_new0 (CustomData, 1);
  SET_CUSTOM_DATA (env, thiz, custom_data_field_id, data);
  GST_DEBUG_CATEGORY_INIT (debug_category, "tutorial-2", 0,
                           "Android tutorial 2");
  gst_debug_set_threshold_for_name ("tutorial-2", GST_LEVEL_DEBUG);
  GST_DEBUG ("Created CustomData at %p", data);
  data->app = (*env)->NewGlobalRef (env, thiz);
  GST_DEBUG ("Created GlobalRef for app object at %p", data->app);
  pthread_create (&gst_app_thread, NULL, &app_function, data);
}

/* Quit the main loop, remove the native thread and free resources */
static void
gst_native_finalize (JNIEnv * env, jobject thiz)
{
  CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
  if (!data)
    return;
  GST_DEBUG ("Quitting main loop...");
  g_main_loop_quit (data->main_loop);
  GST_DEBUG ("Waiting for thread to finish...");
  pthread_join (gst_app_thread, NULL);
  GST_DEBUG ("Deleting GlobalRef for app object at %p", data->app);
  (*env)->DeleteGlobalRef (env, data->app);
  GST_DEBUG ("Freeing CustomData at %p", data);
  g_free (data);
  SET_CUSTOM_DATA (env, thiz, custom_data_field_id, NULL);
  GST_DEBUG ("Done finalizing");
}

/* Set pipeline to PLAYING state */
static void
gst_native_play (JNIEnv * env, jobject thiz)
{
  CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
  if (!data)
    return;
  GST_DEBUG ("Setting state to PLAYING");
  gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
}

/* Set pipeline to PAUSED state */
static void
gst_native_pause (JNIEnv * env, jobject thiz)
{
  CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
  if (!data)
    return;
  GST_DEBUG ("Setting state to PAUSED");
  gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
}

/* Static class initializer: retrieve method and field IDs */
static jboolean
gst_native_class_init (JNIEnv * env, jclass klass)
{
  custom_data_field_id =
          (*env)->GetFieldID (env, klass, "native_custom_data", "J");
  set_message_method_id =
          (*env)->GetMethodID (env, klass, "setMessage", "(Ljava/lang/String;)V");
  on_gstreamer_initialized_method_id =
          (*env)->GetMethodID (env, klass, "onGStreamerInitialized", "()V");

  if (!custom_data_field_id || !set_message_method_id
      || !on_gstreamer_initialized_method_id) {
    /* We emit this message through the Android log instead of the GStreamer log because the later
     * has not been initialized yet.
     */
    __android_log_print (ANDROID_LOG_ERROR, "tutorial-2",
                         "The calling class does not implement all necessary interface methods");
    return JNI_FALSE;
  }
  return JNI_TRUE;
}

/* List of implemented native methods */
static JNINativeMethod native_methods[] = {
        {"nativeInit", "()V", (void *) gst_native_init},
        {"nativeFinalize", "()V", (void *) gst_native_finalize},
        {"nativePlay", "()V", (void *) gst_native_play},
        {"nativePause", "()V", (void *) gst_native_pause},
        {"nativeClassInit", "()Z", (void *) gst_native_class_init}
};

/* Library initializer */
jint
JNI_OnLoad (JavaVM * vm, void *reserved)
{
  JNIEnv *env = NULL;

  java_vm = vm;

  if ((*vm)->GetEnv (vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK) {
    __android_log_print (ANDROID_LOG_ERROR, "tutorial-2",
                         "Could not retrieve JNIEnv");
    return 0;
  }
  jclass klass = (*env)->FindClass (env,
                                    "org/freedesktop/gstreamer/tutorials/tutorial_2/Tutorial2");
  (*env)->RegisterNatives (env, klass, native_methods,
                           G_N_ELEMENTS (native_methods));

  pthread_key_create (&current_jni_env, detach_current_thread);

  return JNI_VERSION_1_4;
}
