LOCAL_PATH := $(call my-dir)/../../../..

PVRSDKDIR := $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_MODULE    := oglestools

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := 	OGLES/PVRTPrint3DAPI.cpp \
					OGLES/PVRTglesExt.cpp \
					OGLES/PVRTTextureAPI.cpp \
					OGLES/PVRTBackground.cpp

LOCAL_SRC_FILES += 	PVRTFixedPoint.cpp \
					PVRTMatrixF.cpp \
					PVRTMisc.cpp \
					PVRTTrans.cpp \
					PVRTVertex.cpp \
					PVRTModelPOD.cpp \
					PVRTDecompress.cpp \
					PVRTTriStrip.cpp \
					PVRTTexture.cpp \
					PVRTPrint3D.cpp \
					PVRTResourceFile.cpp \
					PVRTString.cpp \
					PVRTStringHash.cpp \
					PVRTVector.cpp \
					PVRTError.cpp \
					PVRTUnicode.cpp \
					PVRTQuaternionF.cpp \
					PVRTShadowVol.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Tools/OGLES $(PVRSDKDIR)/Tools $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
