LOCAL_PATH := $(call my-dir)/../../..

# Module OGLESHelloAPI
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESHelloAPI

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := OGLESHelloAPI_Android.cpp


LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv2

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
