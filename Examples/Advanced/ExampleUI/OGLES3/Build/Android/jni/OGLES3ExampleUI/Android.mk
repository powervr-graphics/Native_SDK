LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3ExampleUI
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3ExampleUI

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/ExampleUI/OGLES3/OGLES3ExampleUI.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2 \
                    $(PVRSDKDIR)/Tools/OGLES3

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          ogles3tools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/container-corner.pvr \
	$(ASSETDIR)/container-vertical.pvr \
	$(ASSETDIR)/container-horizontal.pvr \
	$(ASSETDIR)/topbar.pvr \
	$(ASSETDIR)/background.pvr \
	$(ASSETDIR)/clock-face.pvr \
	$(ASSETDIR)/clock-face-small.pvr \
	$(ASSETDIR)/hand.pvr \
	$(ASSETDIR)/hand-small.pvr \
	$(ASSETDIR)/battery.pvr \
	$(ASSETDIR)/internet-web-browser.pvr \
	$(ASSETDIR)/mail-message-new.pvr \
	$(ASSETDIR)/network-wireless.pvr \
	$(ASSETDIR)/office-calendar.pvr \
	$(ASSETDIR)/weather-sun-cloud.pvr \
	$(ASSETDIR)/weather-storm.pvr \
	$(ASSETDIR)/weather-rain.pvr \
	$(ASSETDIR)/text1.pvr \
	$(ASSETDIR)/text2.pvr \
	$(ASSETDIR)/text-weather.pvr \
	$(ASSETDIR)/text-fri.pvr \
	$(ASSETDIR)/text-sat.pvr \
	$(ASSETDIR)/text-sun.pvr \
	$(ASSETDIR)/text-mon.pvr \
	$(ASSETDIR)/weather-sun-cloud-big.pvr \
	$(ASSETDIR)/window-bottom.pvr \
	$(ASSETDIR)/window-bottomcorner.pvr \
	$(ASSETDIR)/window-side.pvr \
	$(ASSETDIR)/window-top.pvr \
	$(ASSETDIR)/window-topleft.pvr \
	$(ASSETDIR)/window-topright.pvr \
	$(ASSETDIR)/loremipsum.pvr \
	$(ASSETDIR)/SpriteShaderF.fsh \
	$(ASSETDIR)/SpriteShaderV.vsh \
	$(ASSETDIR)/TexColShaderF.fsh \
	$(ASSETDIR)/TexColShaderV.vsh \
	$(ASSETDIR)/ColShaderF.fsh \
	$(ASSETDIR)/ColShaderV.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/container-corner.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/container-corner.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/container-vertical.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/container-vertical.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/container-horizontal.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/container-horizontal.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/topbar.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/topbar.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/background.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/background.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/clock-face.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/clock-face.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/clock-face-small.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/clock-face-small.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/hand.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/hand.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/hand-small.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/hand-small.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/battery.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/battery.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/internet-web-browser.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/internet-web-browser.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/mail-message-new.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/mail-message-new.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/network-wireless.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/network-wireless.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/office-calendar.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/office-calendar.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/weather-sun-cloud.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/weather-sun-cloud.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/weather-storm.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/weather-storm.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/weather-rain.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/weather-rain.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text1.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text1.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text2.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text2.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text-weather.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text-weather.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text-fri.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text-fri.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text-sat.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text-sat.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text-sun.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text-sun.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/text-mon.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/text-mon.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/weather-sun-cloud-big.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/weather-sun-cloud-big.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-bottom.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-bottom.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-bottomcorner.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-bottomcorner.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-side.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-side.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-top.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-top.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-topleft.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-topleft.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/window-topright.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/window-topright.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/loremipsum.pvr: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/loremipsum.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SpriteShaderF.fsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/SpriteShaderF.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SpriteShaderV.vsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/SpriteShaderV.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TexColShaderF.fsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/TexColShaderF.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TexColShaderV.vsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/TexColShaderV.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ColShaderF.fsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/ColShaderF.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ColShaderV.vsh: $(PVRSDKDIR)/Examples/Advanced/ExampleUI/OGLES3/ColShaderV.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


