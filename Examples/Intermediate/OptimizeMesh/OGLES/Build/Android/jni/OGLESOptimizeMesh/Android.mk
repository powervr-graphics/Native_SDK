LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/OptimizeMesh/OGLES/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLESOptimizeMesh
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESOptimizeMesh

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/OptimizeMesh/OGLES/OGLESOptimizeMesh.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES

LOCAL_CFLAGS := -DBUILD_OGLES

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv1_CM

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          oglestools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/Satyr.pod \
	$(ASSETDIR)/SatyrOpt.pod

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Satyr.pod: $(PVRSDKDIR)/Examples/Intermediate/OptimizeMesh/OGLES/Satyr.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SatyrOpt.pod: $(PVRSDKDIR)/Examples/Intermediate/OptimizeMesh/OGLES/SatyrOpt.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif

