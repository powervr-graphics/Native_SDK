/*!*********************************************************************************************************************
\File         MainLinuxX11.cpp
\Title        Main Linux X11
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a Linux X11 platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_XLIB_KHR

void createXLIBWIndowSurface(VulkanHelloAPI& VulkanExample)
{
	VulkanExample.surfaceData.width = 1280.0f;
	VulkanExample.surfaceData.height = 800.0f;
	VulkanExample.surfaceData.display = XOpenDisplay(0);

	if (VulkanExample.surfaceData.display != nullptr)
	{
		// Get the default screen for the display
		int defaultScreen = XDefaultScreen(VulkanExample.surfaceData.display);

		// Get the default depth of the display
		int defaultDepth = DefaultDepth(VulkanExample.surfaceData.display, defaultScreen);

		// Select a visual info
		std::auto_ptr<XVisualInfo> visualInfo(new XVisualInfo);
		XMatchVisualInfo(VulkanExample.surfaceData.display, defaultScreen, defaultDepth, TrueColor, visualInfo.get());
		if (!visualInfo.get())
		{
			printf("Error: Unable to acquire visual\n");
			exit(0);
		}

		// Get the root window for the display and default screen
		Window rootWindow = RootWindow(VulkanExample.surfaceData.display, defaultScreen);

		// Create a color map from the display, root window and visual info
		Colormap colorMap = XCreateColormap(VulkanExample.surfaceData.display, rootWindow, visualInfo->visual, AllocNone);

		// Now setup the final window by specifying some attributes
		XSetWindowAttributes windowAttributes;

		// Set the color map that was just created
		windowAttributes.colormap = colorMap;

		// Set events that will be handled by the app, add to these for other events.
		windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;

		// Create the window
        VulkanExample.surfaceData.window = XCreateWindow(VulkanExample.surfaceData.display, // The display used to create the window
            rootWindow, // The parent (root) window - the desktop
            0, // The horizontal (x) origin of the window
            0, // The vertical (y) origin of the window
            VulkanExample.surfaceData.width, // The width of the window
            VulkanExample.surfaceData.height, // The height of the window
            0, // Border size - set it to zero
            visualInfo->depth, // Depth from the visual info
            InputOutput, // Window type - this specifies InputOutput.
            visualInfo->visual, // Visual to use
            CWEventMask | CWColormap, // Mask specifying these have been defined in the window attributes
            &windowAttributes); // Pointer to the window attribute structure

		// Make the window viewable by mapping it to the display
		XMapWindow(VulkanExample.surfaceData.display, VulkanExample.surfaceData.window);

		// Set the window title
		XStoreName(VulkanExample.surfaceData.display, VulkanExample.surfaceData.window, "VulkanHelloAPI");

		// Setup the window manager protocols to handle window deletion events
		Atom windowManagerDelete = XInternAtom(VulkanExample.surfaceData.display, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(VulkanExample.surfaceData.display, VulkanExample.surfaceData.window, &windowManagerDelete, 1);
	}
}

int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI VulkanExample;
	createXLIBWIndowSurface(VulkanExample);
	VulkanExample.initialize();
	VulkanExample.recordCommandBuffer();

    for (uint32_t i = 0; i < 800; ++i)
	{
        // Check for messages from the windowing system.
		int numberOfMessages = XPending(VulkanExample.surfaceData.display);
		for (int i = 0; i < numberOfMessages; i++)
		{
			XEvent event;
			XNextEvent(VulkanExample.surfaceData.display, &event);

			switch (event.type)
			{
            // Exit on window close
			case ClientMessage:
            // Exit on mouse click
			case ButtonPress:
			case DestroyNotify:
				return false;
			default:
				break;
			}
		}
		VulkanExample.drawFrame();
    }

    VulkanExample.deinitialize();

    // Destroy the window
    if (VulkanExample.surfaceData.window)
    {
        XDestroyWindow(VulkanExample.surfaceData.display, VulkanExample.surfaceData.window);
    }

    // Release the display.
    if (VulkanExample.surfaceData.display)
    {
        XCloseDisplay(VulkanExample.surfaceData.display);
    }

    // Clean up our instance.
    // Vulkan can register a callback with Xlib. When the application calls
    // XCloseDisplay, this callback is called and will segfault if the driver had already been unloaded,
    // which could happen when the Vulkan instance is destroyed.
    vk::DestroyInstance(VulkanExample.appManager.instance, nullptr);
	return 0;
}

#endif
