LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/Build/Android/assets
LIBDIR := $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/Build/Android/libs

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3TextureStreaming
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3TextureStreaming

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/TextureStreaming/OGLES3/OGLES3TextureStreaming.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2 \
                    $(PVRSDKDIR)/Tools/OGLES3 \
                    $(PVRSDKDIR)/Tools/CameraInterface

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          ogles3tools \
                          CameraInterface

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

# Copy the .jar file for the CameraInterface
# Remember to build it ("ant debug") first!
CAMERA_JAR_LIB_SRC := $(PVRSDKDIR)/Tools/CameraInterface/Android/Build/bin/classes.jar
CAMERA_JAR_LIB_DST := $(LIBDIR)/cameralib.jar
$(shell mkdir $(subst /,$(SEPARATOR),"$(LIBDIR)"))
$(shell ($(CPY) $(subst /,$(SEPARATOR),"$(CAMERA_JAR_LIB_SRC)" "$(CAMERA_JAR_LIB_DST)")))


all:  \
	$(ASSETDIR)/tvscene.pod \
	$(ASSETDIR)/camera.pvr \
	$(ASSETDIR)/concrete.pvr \
	$(ASSETDIR)/concretelod.pvr \
	$(ASSETDIR)/rand.pvr \
	$(ASSETDIR)/recordred.pvr \
	$(ASSETDIR)/silver.pvr \
	$(ASSETDIR)/tv.pvr \
	$(ASSETDIR)/TVVertexShader.vsh \
	$(ASSETDIR)/TVFragmentShader.fsh \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/camera.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/camera.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/concrete.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/concrete.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/concretelod.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/concretelod.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/rand.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/rand.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/recordred.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/recordred.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/silver.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/silver.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/tv.pvr: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/tv.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVVertexShader.vsh: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/TVVertexShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVFragmentShader.fsh: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/TVFragmentShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/tvscene.pod: $(PVRSDKDIR)/Examples/Advanced/TextureStreaming/OGLES3/tvscene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


