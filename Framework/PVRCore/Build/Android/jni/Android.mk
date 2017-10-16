LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRSDKDIR := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRCore
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix Android/, $(notdir $(wildcard $(LOCAL_PATH)/Android/*.cpp))) \
                    $(addprefix Base/, $(notdir $(wildcard $(LOCAL_PATH)/Base/*.cpp))) \
                    $(addprefix DataStructures/, $(notdir $(wildcard $(LOCAL_PATH)/DataStructures/*.cpp))) \
                    $(addprefix Interfaces/, $(notdir $(wildcard $(LOCAL_PATH)/Interfaces/*.cpp))) \
                    $(addprefix IO/, $(notdir $(wildcard $(LOCAL_PATH)/IO/*.cpp))) \
                    $(addprefix Logging/, $(notdir $(wildcard $(LOCAL_PATH)/Logging/*.cpp))) \
                    $(addprefix Math/, $(notdir $(wildcard $(LOCAL_PATH)/Math/*.cpp))) \
                    $(addprefix Strings/, $(notdir $(wildcard $(LOCAL_PATH)/Strings/*.cpp))) \
                    $(addprefix Texture/, $(notdir $(wildcard $(LOCAL_PATH)/Texture/*.cpp))) \
                    ../../External/pugixml/pugixml.cpp

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS) -DPUGIXML_NO_EXCEPTIONS
LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/

include $(BUILD_STATIC_LIBRARY)
