SDK_BUILD_FLAGS := -Wno-switch -Wno-unused-value
APP_PLATFORM := android-15
APP_STL := c++_static
NDK_TOOLCHAIN_VERSION := clang
ifdef DEBUG
SDK_BUILD_FLAGS +=-DDEBUG=1
endif
APP_ABI := all