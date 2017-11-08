SDK_BUILD_FLAGS := -Wno-switch -Wno-unused-value -Wno-inconsistent-missing-override -fexceptions
APP_PLATFORM := android-15
APP_STL := c++_static
NDK_TOOLCHAIN_VERSION := clang
ifdef DEBUG
SDK_BUILD_FLAGS +=-DDEBUG=1
endif
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64 mips mips64