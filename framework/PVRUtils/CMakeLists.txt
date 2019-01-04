# Common source files
set(source_files
    ArialBoldFont.h
    MultiObject.h
    PowerVRLogo.h
    PVRUtilsGles.h
    PVRUtilsTypes.h
    PVRUtilsVk.h
    StructuredMemory.h
)

if(PVR_GRAPHICS_LIBRARY STREQUAL "OPENGLES")
    list(APPEND source_files
        OpenGLES/BindingsGles.h
        OpenGLES/ConvertToGlesTypes.cpp
        OpenGLES/ConvertToGlesTypes.h
        OpenGLES/ErrorsGles.cpp
        OpenGLES/ErrorsGles.h
        OpenGLES/HelperGles.h
        OpenGLES/ModelGles.cpp
        OpenGLES/ModelGles.h
        OpenGLES/ShaderUtilsGles.cpp
        OpenGLES/ShaderUtilsGles.h
        OpenGLES/SpriteGles.cpp
        OpenGLES/SpriteGles.h
        OpenGLES/TextureUtilsGles.cpp
        OpenGLES/TextureUtilsGles.h
        OpenGLES/UIRendererGles.cpp
        OpenGLES/UIRendererGles.h
        OpenGLES/UIRendererShaders_ES.h
    )

    if(IOS)
        list(APPEND source_files
            EAGL/EaglPlatformContext.mm
            EAGL/EaglPlatformHandles.h
        )
    else()
        list(APPEND source_files
            EGL/EglPlatformContext.h
            EGL/EglPlatformHandles.h
            EGL/EglPlatformContext.cpp
        )
    endif()
elseif(PVR_GRAPHICS_LIBRARY STREQUAL "VULKAN")
    list(APPEND source_files
        Vulkan/AsynchronousVk.h
        Vulkan/ConvertToPVRVkTypes.h
        Vulkan/HelperVk.cpp
        Vulkan/HelperVk.h
        Vulkan/MemoryAllocator.cpp
        Vulkan/MemoryAllocator.h
        Vulkan/SpriteVk.cpp
        Vulkan/SpriteVk.h
        Vulkan/UIRendererFragShader.h
        Vulkan/UIRendererVertShader.h
        Vulkan/UIRendererVk.cpp
        Vulkan/UIRendererVk.h
    )
endif()

add_library(PVRUtils STATIC ${source_files})

target_compile_definitions(PVRUtils PUBLIC
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>
)

target_include_directories(PVRUtils PUBLIC ..)
target_link_libraries(PVRUtils PUBLIC PowerVR_SDK)
