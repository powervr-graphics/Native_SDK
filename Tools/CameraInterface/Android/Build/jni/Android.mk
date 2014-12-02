LOCAL_PATH := $(call my-dir)/../..

include $(CLEAR_VARS)

LOCAL_MODULE    := CameraInterface

LOCAL_SRC_FILES := PVRTCameraInterface_Android.cpp

LOCAL_CFLAGS := -DBUILD_OGLES2

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
