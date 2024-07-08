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
	if (caps & WL_SEAT_CAPABILITY_POINTER) { wl_pointer_add_listener(wl_seat_get_pointer(seat), &pointerListener, data); }
}

static void seat_handle_name(void* data, struct wl_seat* seat, const char* name) {}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdgWmBase, uint32_t serial) { xdg_wm_base_pong(xdgWmBase, serial); }
static const struct xdg_wm_base_listener xdgWmBaseListener = {
    .ping = xdg_wm_base_ping,
};

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial) { }

static const struct wl_seat_listener seatListener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	SurfaceData* surfaceData = (SurfaceData*)data;

	if (strcmp(interface, "wl_compositor") == 0) { surfaceData->wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1); }
	else if (strcmp(interface, "wl_seat") == 0)
	{
		surfaceData->wlSeat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(surfaceData->wlSeat, &seatListener, data);
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		surfaceData->xdgWmBase = (xdg_wm_base *)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(surfaceData->xdgWmBase, &xdgWmBaseListener, surfaceData);
	}
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name) {}

static const struct xdg_surface_listener xdgSurfaceListener = { .configure = xdg_surface_configure };

static const struct wl_registry_listener registryListener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
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

	vulkanExample.surfaceData.xdgSurface = xdg_wm_base_get_xdg_surface(vulkanExample.surfaceData.xdgWmBase, vulkanExample.surfaceData.surface);
	if (!vulkanExample.surfaceData.xdgSurface)
	{
		LOGE("Could create Wayland shell surface\n");
		exit(1);
	}

	xdg_surface_add_listener(vulkanExample.surfaceData.xdgSurface, &xdgSurfaceListener, &vulkanExample.surfaceData);

	vulkanExample.surfaceData.xdgToplevel = xdg_surface_get_toplevel(vulkanExample.surfaceData.xdgSurface);
	xdg_toplevel_set_title(vulkanExample.surfaceData.xdgToplevel, "HelloApiVk");
}

void releaseWaylandConnection(VulkanHelloAPI& vulkanExample)
{
	xdg_surface_destroy(vulkanExample.surfaceData.xdgSurface);
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
