LOCAL_PATH := $(call my-dir)/../../../..

PVRSDKDIR := $(realpath $(LOCAL_PATH))

include $(CLEAR_VARS)

LOCAL_MODULE    := ogles2tools

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := 	Tools/OGLES2/PVRTPrint3DAPI.cpp \
					Tools/OGLES2/PVRTgles2Ext.cpp \
					Tools/OGLES2/PVRTTextureAPI.cpp \
					Tools/OGLES2/PVRTBackground.cpp \
					Tools/OGLES2/PVRTPFXParserAPI.cpp \
					Tools/OGLES2/PVRTPFXSemantics.cpp \
					Tools/OGLES2/PVRTShader.cpp

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
					Tools/PVRTPFXParser.cpp \
					Tools/PVRTShadowVol.cpp \
					Tools/PVRTVector.cpp \
					Tools/PVRTError.cpp \
					Tools/PVRTUnicode.cpp \
					Tools/PVRTQuaternionF.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Tools/OGLES2 $(PVRSDKDIR)/Tools $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES2

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector 
endif

include $(BUILD_STATIC_LIBRARY)
