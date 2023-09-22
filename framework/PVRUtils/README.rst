PVRUtils
========

Overview
--------

PVRUtilsVk provides several useful utilities for rendering with Vulkan through PVRVk. PVRUtilsGles provides several useful utilities for rendering with OpenGL ES.

UIRenderer is a simple, platform agnostic library for rendering 2D elements in a 3D environment, written with text and sprites in mind, providing very useful capabilities. A wealth of tools is provided to import fonts and create glyphs for specific or full parts of them. For PVRUtilsVk, UIRenderer will use a Command Buffer provided to it to queue up the “recipe” for a render, allowing the user to defer it for later for using it inside complex engines. For PVRUtilsGles, UIRenderer will execute the rendering commands immediately at the point of calling “render”.

Other utilities include:

- A ton of helpers like instance/devices/swapchain creation for Vulkan and EGLContexts for OpenGLES, eliminating the overhead of configuring everything every time
- Uploading textures (easier said than done on Vulkan)
- For Vulkan, Multithreading tools, like the Asynchronous Texture Uploader
- For Vulkan, the very cool Render Manager rendering engine (you can see it in action in the Skinning example).

You can find the PVRUtils source in the SDK package's ``framework/PVRUtils`` folder.

Using PVRUtils
--------------

To use PVRUtils:

1. Depending on the platform, add the module in by adding either:

   - A project dependency (windows/macOS/ios/android/…) (project file in Framework/PVRUtils/[Vulkan/OpenGLES]/Build/[Platform]/…).
   - The library to link against directly (windows/android/linux makefiles) (.so etc. in Framework/Bin/[Platform]/[lib?]PVRUtils[Vk/Gles][.lib/.so]).

2. Link against the PowerVR Framework dependencies of PVRUtils: PVRVk PVRCore PVRAssets and PVRShell.
3. Include the PVRUtils header file (PVRUtils/PVRUtils.h).
4. Create and use 2D elements through a ``pvr::ui::UIRenderer`` object, and ``pvr::ui::[Text/Image/Font]`` objects

Code Examples
-------------

.. code:: cpp

   using namespace pvr::ui;
   UIRenderer spriteEngine; Text niceCustomText; Font myFont; Image myImage; Group myGroup;
   pvrvk::SecondaryCommandBuffer uiRendererBuffer;

   spriteEngine->init(screenWidth, screenHeight, isFullScreen(), renderPass, *subpass* 0, cmdPool, cmdQueue);
   myText= spriteEngine->createText("Hello, world");
   myText->setAnchor(Anchor::TopLeft, glm::vec2(-1, 0)); //"Pin" the top-left of the sprite to the center-left of the screen
   myText->setPixelOffset(30, -20); // Additionally move the text 30 pixels right and 20 pixels down from the anchor point we set

   myGroup = spriteEngine->createMatrixGroup(); //Create a group that we can transform with matrices.
   myGroup->setMatrix(glm::rotate(...)); myGroup->add(myText); //Whenever we render it, all sprites will be contained

   spriteEngine->beginRendering(uiRendererBuffer);
   myGroup->render(); //Everything is positioned relative to the screen, and then transformed with the Group
   myText->render(); //But we can still render it OUTSIDE of the group. It is still a sprite.
   spriteEngine->endRendering();

   //The Command Buffer now contains all the drawing commands for the UI elements we rendered (minus the renderpass).
   //By not containing the render pass, we can use the command buffer to render into an Framebuffer or anything else we want.
   //It is used by submitting into a main command buffer.

