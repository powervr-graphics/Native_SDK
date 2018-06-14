========
Skinning
========

.. figure:: ./Skinning.png

This demo shows a scene with a combination of a skinned, bumpmapped character with non-skinned, non-bumpmapped objects.

Description
-----------
The Skinning demo shows a Skinned Character in combination with bump mapping. Skinning is the act of animating a vertex over time given a set (palette) of matrices and a known set of blend weights assigned to those matrices. For each frame the Matrix Palette is recomputed based on time. PVRAssets and POD files support skinning. either full transformation Matrices, or Quaternion rotation with Scaling and Translation vectors. The provided POD file contains matrix animation. Uses a Shader Storage Buffer object to support a dynamic number of bones.

APIS
----
* Vulkan
* OpenGL ES 3.0

Controls
--------
- Action1/2/3- Pause
- Esc- Close