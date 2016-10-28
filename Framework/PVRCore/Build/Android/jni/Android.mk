LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRFRAMEWORKDIR := $(realpath $(call my-dir)/../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRCore
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix Android/, $(notdir $(wildcard $(LOCAL_PATH)/Android/*.cpp))) \
                    ../../External/pugixml/pugixml.cpp

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS) -DPUGIXML_NO_EXCEPTIONS
LOCAL_C_INCLUDES := $(PVRFRAMEWORKDIR) $(PVRFRAMEWORKDIR)/../Builds/Include/

include $(BUILD_STATIC_LIBRARY)
