# PowerVR Graphics Native SDK #
![SDKExamples](http://powervr-graphics.github.io/Native_SDK/Documentation/SDKBrowser/images/ExamplesPageGraphic.png)
This repository contains the PowerVR Graphics cross-platform native (C++) SDK source code. The SDK includes optimized example applications to demonstrate the most efficient ways of implementing common 3D graphics effects on PowerVR graphics cores. It also includes a cross-platform OS and graphics API abstraction layer, and a library of helper code for resource loading, vector and matrix maths, text printing and more.

The SDK supports iOS, Android and Linux PowerVR devices. It also supports Windows, OS X and Linux PC emulation. Imagination's OpenGL ES emulation library, PVRVFrame, can be downloaded from our website [here](http://community.imgtec.com/developers/powervr/tools/pvrvframe/).

To keep up to date with changes to the SDK read the [what's new](http://community.imgtec.com/developers/powervr/whats-new/) page.

## About the SDK ##
### Framework ###
* The framework consists of separate libraries providing groups of functionality as modules. These modules are PVRCore, PVRAssets, PVRShell, PVRPlatformGlue, PVRApi, PVRUIRenderer and PVRCamera
	* PVRCore provides support for the other modules
	* PVRShell provides platform abstraction/entry point (replaces the old PVRShell)
	* PVRPlatformGlue provides platform/API joining functionality to PVRShell
	* PVRApi provides an API abstraction on top of which to build the example (replacing PVRTools)
	* PVRUIRenderer provides 2D element drawing/layout functions (replaces the old tools Print3D)
	* PVRCamera provides HW Camera abstractions (replaces the CameraModule)
	* Uses C++ (namespacing, smart pointers, OOP), the C++ standard library, GLM for maths

### Example Applications ###
The examples provided in the SDK are fully commented, highly optimized C++ applications that cover a variety of rendering techniques. They are designed in a step-by-step tutorial style to gradually guide the most inexperienced graphics developers from a simple render of a single triangle to complex scenes that incorporate many objects, animations and shader effects.

## Setup ##
The Getting Started guide of our SDK Browser provides step-by-step instructions to build and deploy the SDK example applications for the various support operating systems. You can find the SDK Browser [here](http://powervr-graphics.github.io/Native_SDK/SDKBrowser.html).

## Documentation ##
Our documentation (architecture guides, performance recommendations & white papers) can be found on the Imagination website [here](http://community.imgtec.com/developers/powervr/documentation/). They are also included in the SDK's Documentation/ directory.

## Release notes ##
The latest PowerVR Graphics Tools and SDK release notes can be found on the Imagination website [here](http://community.imgtec.com/developers/powervr/whats-new/).

## Support ##
If you have any questions about the SDK, PowerVR device optimization/debugging or our tools, please contact us through our [public forum](http://forum.imgtec.com/categories/powervr-graphics). We also recommend checking out our [FAQ](http://forum.imgtec.com/categories/powervr-faq) to see if your question has already been answered.
If you would prefer to contact us confidentially, you can file a support ticket [here](https://pvrsupport.imgtec.com/new-ticket).

## License ##
The SDK is distributed under the MIT license.
