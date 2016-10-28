/*!*********************************************************************************************************************
\file         PVRApi\PVRApi.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Include for the functionality of the PVRApi library. Contains the full PVRApi functionality, with core plus helpers
              and libraries.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/Api.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRApi/AssetStore.h"
#include "PVRApi/AssetUtils.h"
#include "PVRApi/ApiObjects.h"

/*****************************************************************************/
/*! \mainpage PVRApi
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRApi leverages PVRAssets and PVRShell in order to abstract the graphics API. PVRApi is a very ambitious project that aims to provide a common codebase across different graphics APIs. Combined with PVRShell (which provides a common codebase across platforms), PVRApi supports a cross-platform, cross-API Framework to develop your applications in.

PVRApi becomes especially important with the emergence of new and powerful APIs like Vulkan, Metal and Mantle, where a lot of complexity is now revealed. This threatens to complicate simpler tasks and bloat application code as a fair exchange for their power, customizability and performance.

Inspired by these modern paradigms, PVRApi provides a common interface that follows them closely, but is still able to provide the convenience and power that C++ allows (such as overloads, namespacing, default arguments, etc.). You can expect to find thin abstractions on top of Command Buffers, Textures, Buffers, etc., designed so as to provide an implementation on top of Vulkan or OpenGL ES.

Different APIs are normally provided link-time, but if the user chooses to build PVRApi as a runtime library, the choice could be delegated to execution-time or even runtime. PVRApi will in many cases be the majority of application code. PVRApi also features the ability to abstract most of the renderstate - making state an object now. You can create a pipeline object with minimal input: Vertex+Fragment shader, Set shader attributes, non-default things.

PVRApi source can be found in the <a href="../../">PVRApi</a> folder in the SDK package.

\section usage Using PVRApi
*****************************

To use PVRApi:
<ul>
<li>Depending on the platform, add the module in by adding either:</li>
<ol>
<li>A project dependency (windows/osx/ios/android/...) (project file in <span class="code">Framework/PVRApi/[API]/Build/[Platform]/...</span>)</li>
<li>The library to link against directly (windows/android/linux makefiles) (.so etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVR[API][.lib/.so]</span>)</li>
</ol>
<li>Include the header file (most commonly this file, <span class="code">PVRApi/PVRApi.h</span>)</li>
<li>Also link against the PowerVR Framework dependencies of PVRApi: (PVRCore, PVRAssets, PVRPlatformGlue, PVRShell)</li>
<li>Use the library!</li>
</ul>

\section code Code Examples
*****************************

Pipeline Creation
\code
GraphicsPipelineCreateParam pipeCreate;
pipeCreate.addDescriptorSetLayout(myDescriptorSet); //Pipeline needs to know what will be bound
pipeCreate.vertexShader = myCookedVertexShader; //Add shaders...
pipeCreate.vertexInput.addVertexAttribute(0, 0/ *buffer index* /, posLayout, “inVertex”); //vertex attributes...
\endcode

One-Shot Generation
\code
GraphicsPipeline skinnedPipeline = getGraphicsContext().createGraphicsPipeline(pipeCreate);
\endcode

Buffer Creation
\code
Buffer vbo = getGraphicsContext().createBuffer(mesh.getDataSize(), BufferBindingUse::VertexBuffer);
buffer.update(myMesh.getDataPtr(0), 0, myMesh.getDataSize(0));
commandBuffer->bindVertexBuffer(vbo, 0/ *offset* /, 0/ *index, see above!* /);
\endcode
*/