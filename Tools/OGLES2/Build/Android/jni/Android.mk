LOCAL_PATH := $(call my-dir)/../../../..

PVRSDKDIR := $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_MODULE    := ogles2tools

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := 	OGLES2/PVRTPrint3DAPI.cpp \
					OGLES2/PVRTBackground.cpp \
					OGLES2/PVRTPFXParserAPI.cpp \
					OGLES2/PVRTPFXSemantics.cpp \
					OGLES2/PVRTShader.cpp \
					OGLES2/PVRTgles2Ext.cpp \
					OGLES2/PVRTTextureAPI.cpp

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
					PVRTPFXParser.cpp \
					PVRTShadowVol.cpp \
					PVRTVector.cpp \
					PVRTError.cpp \
					PVRTUnicode.cpp \
					PVRTQuaternionF.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Tools/OGLES2 $(PVRSDKDIR)/Tools $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES2

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
