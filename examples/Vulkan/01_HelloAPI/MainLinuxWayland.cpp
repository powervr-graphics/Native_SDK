/*!*********************************************************************************************************************
\File         MainWayland.cpp
\Title        Main Linux Wayland
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on an Linux Wayland platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

static void pointer_handle_enter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {}
static void pointer_handle_leave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface) {}
static void pointer_handle_motion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {}
static void pointer_handle_button(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {}
static void pointer_handle_axis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointerListener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void seat_handle_capabilities(void* data, struct wl_seat* seat, uint32_t caps)
{
	SurfaceData* surfaceData = static_cast<SurfaceData*>(data);

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && (surfaceData->wlPointer == NULL))
	{
		surfaceData->wlPointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(surfaceData->wlPointer, &pointerListener, NULL);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && (surfaceData->wlPointer != NULL))
	{
		wl_pointer_destroy(surfaceData->wlPointer);
		surfaceData->wlPointer = NULL;
	}
}

static void seat_handle_name(void* data, struct wl_seat* seat, const char* name) {}

static void handle_ping(void* data, struct wl_shell_surface* surface, uint32_t serial) { wl_shell_surface_pong(surface, serial); }

static void handle_configure(void* data, struct wl_shell_surface* surface, uint32_t edges, int32_t width, int32_t height) {}
static void handle_popup_done(void* data, struct wl_shell_surface* surface) {}

static const struct wl_seat_listener seatListener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void xdg_wm_base_ping(void* data, struct xdg_wm_base* shell, uint32_t serial) { xdg_wm_base_pong(shell, serial); }

static const struct xdg_wm_base_listener xdgWmBaseListener = {
	xdg_wm_base_ping,
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	SurfaceData* surfaceData = static_cast<SurfaceData*>(data);

	if (strcmp(interface, "wl_compositor") == 0) { surfaceData->wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1); }
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		surfaceData->xdgShell = (reinterpret_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)));
		xdg_wm_base_add_listener(surfaceData->xdgShell, &xdgWmBaseListener, NULL);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		surfaceData->wlSeat = (reinterpret_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1)));
		wl_seat_add_listener(surfaceData->wlSeat, &seatListener, data);
	}
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name) {}

static const struct wl_shell_surface_listener shellSurfaceListener = { .ping = handle_ping, .configure = handle_configure, .popup_done = handle_popup_done };

static const struct wl_registry_listener registryListener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static void xdg_surface_handle_configure(void* data, struct xdg_surface* surface, uint32_t serial) { xdg_surface_ack_configure(surface, serial); }

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	SurfaceData* surfaceData = static_cast<SurfaceData*>(data);

	if (!width && !height) return;

	if (surfaceData->width != width || surfaceData->height != height)
	{
		surfaceData->width = width;
		surfaceData->height = height;

		wl_surface_commit(surfaceData->surface);
	}
}

static void xdg_toplevel_handle_close(void* data, struct xdg_toplevel* xdg_toplevel) {}

static const struct xdg_toplevel_listener xdgToplevelListener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};

void createWaylandWindowSurface(VulkanHelloAPI& vulkanExample)
{
	vulkanExample.surfaceData.width = 1280;
	vulkanExample.surfaceData.height = 800;

	vulkanExample.surfaceData.display = wl_display_connect(NULL);
	if (!vulkanExample.surfaceData.display)
	{
		LOGE("Could not open Wayland display connection\n");
		exit(1);
	}

	vulkanExample.surfaceData.wlRegistry = wl_display_get_registry(vulkanExample.surfaceData.display);
	if (!vulkanExample.surfaceData.wlRegistry)
	{
		LOGE("Could not get Wayland registry\n");
		exit(1);
	}

	wl_registry_add_listener(vulkanExample.surfaceData.wlRegistry, &registryListener, &vulkanExample.surfaceData);
	wl_display_dispatch(vulkanExample.surfaceData.display);
	vulkanExample.surfaceData.surface = wl_compositor_create_surface(vulkanExample.surfaceData.wlCompositor);

	if (!vulkanExample.surfaceData.surface)
	{
		LOGE("Could create Wayland compositor surface\n");
		exit(1);
	}

	vulkanExample.surfaceData.xdgShellSurface = xdg_wm_base_get_xdg_surface(vulkanExample.surfaceData.xdgShell, vulkanExample.surfaceData.surface);
	if (!vulkanExample.surfaceData.xdgShellSurface)
	{
		LOGE("Could create Wayland shell surface\n");
		exit(1);
	}

	xdg_surface_add_listener(vulkanExample.surfaceData.xdgShellSurface, &xdg_surface_listener, &vulkanExample.surfaceData);
	vulkanExample.surfaceData.xdgToplevel = xdg_surface_get_toplevel(vulkanExample.surfaceData.xdgShellSurface);
	xdg_toplevel_add_listener(vulkanExample.surfaceData.xdgToplevel, &xdgToplevelListener, &vulkanExample.surfaceData);
	xdg_toplevel_set_title(vulkanExample.surfaceData.xdgToplevel, "HelloApiVk");
	xdg_toplevel_set_app_id(vulkanExample.surfaceData.xdgToplevel, "OpenGLESHelloAPI");
	wl_surface_commit(vulkanExample.surfaceData.surface);
}

void releaseWaylandConnection(VulkanHelloAPI& vulkanExample)
{
	xdg_surface_destroy(vulkanExample.surfaceData.xdgShellSurface);
	xdg_wm_base_destroy(vulkanExample.surfaceData.xdgShell);
	xdg_toplevel_destroy(vulkanExample.surfaceData.xdgToplevel);
	wl_surface_destroy(vulkanExample.surfaceData.surface);
	if (vulkanExample.surfaceData.wlPointer) { wl_pointer_destroy(vulkanExample.surfaceData.wlPointer); }
	wl_seat_destroy(vulkanExample.surfaceData.wlSeat);
	wl_compositor_destroy(vulkanExample.surfaceData.wlCompositor);
	wl_registry_destroy(vulkanExample.surfaceData.wlRegistry);
	wl_display_disconnect(vulkanExample.surfaceData.display);
}

int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI vulkanExample;
	createWaylandWindowSurface(vulkanExample);
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i)
	{
		wl_display_dispatch_pending(vulkanExample.surfaceData.display);
		vulkanExample.drawFrame();
	}

	vulkanExample.deinitialize();

	// Clean up the Wayland
	releaseWaylandConnection(vulkanExample);

	return 0;
}
#endif
