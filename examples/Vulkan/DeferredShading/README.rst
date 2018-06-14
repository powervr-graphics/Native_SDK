===============
DeferredShading
===============

.. figure:: ./DeferredShading.png

Demonstrates optimal deferred shading using Pixel Local Storage (PLS), or Subpasses with Transient Attachments (Vulkan)

Description
-----------	
Traditional rendering algorithms submit geometry and immediately apply shading effects to the rasterized primitives. Complex shading effects may require multiple draw calls (one per object per light) or render passes to produce the final pixel color, with the geometry submitted every pass. Deferred shading is an alternative rendering technique that submits the scene geometry once, storing per-pixel attributes into local video memory to be used in the subsequent rendering passes. 
In these later passes, light volume primitives are rendered, and the per-pixel attributes contained in the buffer are retrieved at a 1:1 mapping ratio so that each pixel is shaded individually.
In the PowerVR architecture, the user can use fast on-chip memory instead of the render target, by utilising the Pixel Local Storage OpenGL ES extension, or correctly defining Subpasses for those operations.
Requires the extension GL_EXT_shader_pixel_local_storage for PLS.

APIS
----
* Vulkan
* OpenGL ES 3.0

Controls
--------
- Action1- Pause
- Action2- Orbit camera
- Quit- Close the application