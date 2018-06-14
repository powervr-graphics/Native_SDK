============
Navigation2D
============

.. figure:: ./Navigation2D.png

Demonstrates parsing, processing and rendering of raw OSM data as a 2D navigational map.

Description
-----------
The 2D navigation example demonstrates the entire process of creating a navigational map from raw data, in this 
case the Open Street Map data. The example demonstrates loading and parsing of the XML, the processing of the raw 
data, triangulation with the ear clipping algorithm to generate triangles, defining the roads, and batching all 
of that into tiles as renderable polygons.
This example also shows several rendering techniques such as; anti-aliased lines with outline for roads, 
UI elements for road names and places of interest and an effective tile based approach to batching 
and culling the geometry. 

APIS
----
* OpenGL ES 2.0+, Vulkan

Controls
--------
Q- Quit- Close the application