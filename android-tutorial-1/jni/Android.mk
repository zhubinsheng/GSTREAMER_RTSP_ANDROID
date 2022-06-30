LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := tutorial-1
LOCAL_SRC_FILES := tutorial-1.c dummy.cpp
LOCAL_SHARED_LIBRARIES := gstreamer_android
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)


GSTREAMER_ROOT        := Z:/gst/arm64
GSTREAMER_NDK_BUILD_PATH  := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/
#GSTREAMER_PLUGINS         := coreelements
GSTREAMER_EXTRA_LIBS      := -liconv
GSTREAMER_NDK_BUILD_PATH  := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/
include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk
GSTREAMER_PLUGINS         := $(GSTREAMER_PLUGINS_CORE) $(GSTREAMER_PLUGINS_EFFECTS) \
							 $(GSTREAMER_PLUGINS_PLAYBACK) $(GSTREAMER_PLUGINS_CODECS) \
							 $(GSTREAMER_PLUGINS_CODECS_RESTRICTED) $(GSTREAMER_PLUGINS_NET) \
							 $(GSTREAMER_PLUGINS_SYS) $(GSTREAMER_CODECS_GPL)        \
                             $(GSTREAMER_PLUGINS_ENCODING) $(GSTREAMER_PLUGINS_VIS)       \

G_IO_MODULES              := openssl
GSTREAMER_EXTRA_DEPS      := gstreamer-player-1.0 gstreamer-video-1.0 glib-2.0 gstreamer-rtsp-server-1.0
include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk