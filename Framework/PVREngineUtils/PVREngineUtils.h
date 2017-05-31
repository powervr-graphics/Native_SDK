/*!
\brief Include this file if you wish to use the PVREngineUtils functionality.
\file PVREngineUtils/PVREngineUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVREngineUtils/UIRenderer.h"
#include "PVREngineUtils/AssetStore.h"
#include "PVREngineUtils/AssetUtils.h"
#include "PVREngineUtils/RenderManager.h"

/*****************************************************************************/
/*! \mainpage PVREngineUtils
******************************************************************************

\tableofcontents 
 
\section overview Overview
*****************************

PVREngineUtils leverages PVRApi to provide a simple, platform- and API-agnostic library for rendering 2D elements in a 3D environment. Specifically, PVREngineUtils was written with text and sprites in mind.

A wealth of tools is provided to import fonts and create glyphs for specific or full parts of them. In most circumstances, PVREngineUtils will use a Command Buffer provided to it to queue up the "recipe" for a render, allowing the user to defer it for later using inside complex engines.

PVREngineUtils source can be found in the <a href="../../">PVREngineUtils</a> folder in the SDK package.

<ul>
<li>
<ol>
<li>Depending on the platform, add the module in by adding either:</li>
<li>
<ol>
<li>A project dependency (windows/osx/ios/android/...) (project file in <span class="code">Framework/PVREngineUtils/Build/[Platform]/...</span>)</li>
<li>The library to link against directly (windows/android/linux makefiles) (.so etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVREngineUtils[.lib/.so]</span>)</li>
</ol>
</li>
<li>Also link against the PowerVR Framework dependencies of PVREngineUtils: (PVRApi and its dependencies(PVRCore, PVRAssets, PVRShell))</li>
<li>Include the PVREngineUtils header file (<span class="code">PVREngineUtils/PVREngineUtils.h</span>)</li>
</ol>
</li>
<li>Create and use 2D elements through a pvr::ui::UIRenderer object, and pvr::ui::[Text/Image/Font] objects</li>
</ul>

\section code Code Examples
*****************************

\code
using namespace pvr::ui;
UIRenderer spriteEngine; Text niceCustomText; Font myFont; Image myImage; Group myGroup;
pvr::api::SecondaryCommandBuffer uiRendererBuffer;

spriteEngine->init(this->getGraphicsContext()); //“this” is a pvr::Shell, a.k.a. our demo class
myText= spriteEngine->createText(“Hello, world”); 
myText->setAnchor(Anchor::TopLeft, glm::vec2(-1, 0)); //"Pin" the top-left of the sprite to the center-left of the screen
myText->setPixelOffset(30, -20); // Additionally move the text 30 pixels right and 20 pixels down from the anchor point we set

myGroup = spriteEngine->createMatrixGroup(); //Create a group that we can transform with matrices.
myGroup->setMatrix(glm::rotate(…)); myGroup->add(myText); //Whenever we render it, all sprites will be contained

spriteEngine->beginRendering(uiRendererBuffer);
myGroup->render(); //Everything is positioned relative to the screen, and then transformed with the Group
myText->render(); //But we can still render it OUTSIDE of the group. It’s still a sprite.
spriteEngine->endRendering();

//The Command Buffer now contains all the drawing commands for the UI elements we rendered (minus the renderpass).
//By not containing the render pass, we can use the command buffer to render into an FBO or anything else we want.
//It is used by submitting into a main command buffer.
\endcode
*/