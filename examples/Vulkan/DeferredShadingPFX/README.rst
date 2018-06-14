==================
DeferredShadingPFX
==================

.. figure:: ./DeferredShadingPFX.png

Duplicates the DeferredShading example using simple PFX scene description to describe this complex rendering scenario

Description
-----------
The DeferredShading example uses a multi-subpass technique (in Vulkan) or Pixel Local Storage (in OpenGL ES) to implement a Deferred Shading technique. In
this example, this exact same technique is described in a PFX file, and rendered with the RenderManager, effectively avoiding almost all the code in the example
except the most basics, demonstrating the power of the RenderManager combined with PFX files as a scene description.

APIS
----
* Vulkan

Controls
--------
- Action1- Pause
- Action2- Orbit camera
- Quit- Close the application
