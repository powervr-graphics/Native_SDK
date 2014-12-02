LOCAL_PATH := $(call my-dir)/../../..

# Module OGLES3HelloAPI
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3HelloAPI

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := OGLES3HelloAPI_Android.cpp


LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)


