==================
ImageBasedLighting
==================

.. figure:: ./ImageBasedLighting.png

This example demonstrates Physically Based Rendering (PBR) with Image Based Lighting (IBL). 

Description
-----------
This example has two scenes using Metallic-Roughness PBR workflow

* Helmet: A GLTF model rederered  with different material properties with albedo, metallic roughness and emissive map.

* Spheres: Showcasing different metallic and non metallic materials.

IBL
---
IBL Diffuse: This is pre-computed offline from the environment map, using the lambert diffuse BRDF.
IBL Specular: The Equation split into two sums. Each sum can be pre-calculated (Pre-Filtered Environment map and Environment BRDF)

Pre-Filtered Environment map:
Pre-calculated  for different roughness values and stored in the mip-map levels of the cubemap. The environment map is convolved  with CGX distribution using important sampling. 

Evironment BRDF: This sum includes everything else, it is same as integrating the specular BRDF with solid white environment. This takes two inputs (roughness and cos Θv (view angle)) and two outputs (scale and bias to F0) in the range [0,1].

IBL Layers

.. figure:: ./IBL_layers.png

APIs
----
* OpenGL ES 3.0+
* Vulkan

Controls
--------
- Quit- Close the application
- Left/ Right to change the scene
- Action1 to pause.
