LOCAL_PATH := $(call my-dir)/../..

include $(CLEAR_VARS)

LOCAL_MODULE    := PVRCamera

LOCAL_SRC_FILES := ../CameraInterface_Android.cpp

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

LOCAL_C_INCLUDES := ../../../../Builds/Include ../../..

include $(BUILD_STATIC_LIBRARY)
