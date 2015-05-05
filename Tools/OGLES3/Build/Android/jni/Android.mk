LOCAL_PATH := $(call my-dir)/../../../..

PVRSDKDIR := $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_MODULE    := ogles3tools

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := 	OGLES2/PVRTPrint3DAPI.cpp \
					OGLES2/PVRTBackground.cpp \
					OGLES2/PVRTPFXParserAPI.cpp \
					OGLES2/PVRTPFXSemantics.cpp \
					OGLES2/PVRTShader.cpp \
					OGLES3/PVRTgles3Ext.cpp \
					OGLES3/PVRTTextureAPI.cpp

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
					PVRTPFXParser.cpp \
					PVRTQuaternionF.cpp \
					PVRTShadowVol.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Tools/OGLES3 $(PVRSDKDIR)/Tools/OGLES2 $(PVRSDKDIR)/Tools $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES3

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
