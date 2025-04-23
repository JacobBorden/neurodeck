#include <wayland-server.h>
#include <cstdio>
#include <cstring>

// Global Wayland objects
static struct wl_display* display = nullptr;
static struct wl_event_loop* loop = nullptr;

// Wayland globals (interfaces)
// Resource destroy callback (no-op for now)
static void handle_destroy(struct wl_resource* resource) {
    // cleanup if needed
}

// Bind callback for compositor interface
static void bind_compositor(struct wl_client* client,
                            void* data,
                            uint32_t version,
                            uint32_t id) {
    struct wl_resource* resource =
        wl_resource_create(client, &wl_compositor_interface, version, id);
    wl_resource_set_implementation(resource, nullptr, nullptr, handle_destroy);
    printf("Client bound to wl_compositor (id=%u)\n", id);
}

int main(int argc, char* argv[]) {
    // 1. Create the Wayland display
    display = wl_display_create();
    if (!display) {
        fprintf(stderr, "Failed to create Wayland display\n");
        return 1;
    }

    // 2. Add a UNIX socket at WAYLAND_DISPLAY=wayland-0
    const char* sock = wl_display_add_socket_auto(display);
    if (!sock) {
        fprintf(stderr, "Failed to add socket\n");
        wl_display_destroy(display);
        return 1;
    }
    printf("Wayland socket created: %s\n", sock);

    // 3. Create a global for wl_compositor
    wl_global_create(display, &wl_compositor_interface, 4 /*version*/, nullptr, bind_compositor);
    printf("wl_compositor global advertised\n");

    // 4. Optional: advertise wl_shm for shared memory
    // wl_global_create(display, &wl_shm_interface, 1, nullptr, bind_shm);

    // 5. Run the event loop
    printf("Entering Wayland event loop...\n");
    loop = wl_display_get_event_loop(display);
    wl_event_loop_dispatch(loop, -1);

    // Cleanup
    wl_display_destroy(display);
    return 0;
}
