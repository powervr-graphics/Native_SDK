=======
AntiAliasing
=======

.. figure:: ./Antialiasing.png

This example demonstrates different kind of shader-based anti-aliasing techniques.

API
---
* Vulkan

Description
-----------

- in MSAA, if any of the multi sample locations in a pixel is covered by the triangle being rendered, a shading computation must be performed for that triangle.

- FXAA uses a high contrast filter to find edges before sampling those edges and blending them

- TXAA is similar to FXAA. It’s a post-process form of anti-aliasing that samples each pixel in a frame. However, it samples a different location within each frame, and it uses past frames to blend the samples together.

These techniques are used extensively in graphics applications to remove aliasing.

Controls
--------
- Tap to screen to change between various AA techniques.

