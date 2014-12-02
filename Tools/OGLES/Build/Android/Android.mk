LOCAL_PATH := $(call my-dir)/../../../..

PVRSDKDIR := $(realpath $(LOCAL_PATH))

include $(CLEAR_VARS)

LOCAL_MODULE    := oglestools

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := 	Tools/OGLES/PVRTPrint3DAPI.cpp \
					Tools/OGLES/PVRTglesExt.cpp \
					Tools/OGLES/PVRTTextureAPI.cpp \
					Tools/OGLES/PVRTBackground.cpp

LOCAL_SRC_FILES += 	Tools/PVRTFixedPoint.cpp \
					Tools/PVRTMatrixF.cpp \
					Tools/PVRTMisc.cpp \
					Tools/PVRTTrans.cpp \
					Tools/PVRTVertex.cpp \
					Tools/PVRTModelPOD.cpp \
					Tools/PVRTDecompress.cpp \
					Tools/PVRTTriStrip.cpp \
					Tools/PVRTTexture.cpp \
					Tools/PVRTPrint3D.cpp \
					Tools/PVRTResourceFile.cpp \
					Tools/PVRTString.cpp \
					Tools/PVRTStringHash.cpp \
					Tools/PVRTVector.cpp \
					Tools/PVRTError.cpp \
					Tools/PVRTQuaternionF.cpp \
					Tools/PVRTShadowVol.cpp \
					Tools/PVRTUnicode.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Tools/OGLES $(PVRSDKDIR)/Tools $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
