#include <wayland-server.h>
#include <cstdio>
#include <cstring>
#include <EGL/egl.h>
#include <wayland-egl.h> // Needs to be included after EGL/egl.h
#include <GLES2/gl2.h>

// Global Wayland objects
static struct wl_display *display = nullptr;
static struct wl_event_loop *loop = nullptr;

// Global EGL objects
static EGLDisplay egl_display = EGL_NO_DISPLAY;
static EGLConfig egl_config = nullptr;
static EGLContext egl_context = EGL_NO_CONTEXT;

// Global client surface resources (for the first client that creates a surface)
static struct wl_surface *g_client_surface = nullptr;
static struct wl_egl_window *g_client_egl_window = nullptr;
static EGLSurface g_client_egl_rendering_surface = EGL_NO_SURFACE;

// Timer for rendering
static struct wl_event_source *g_render_timer_source = nullptr;


// Wayland globals (interfaces)
// Generic resource destroy callback
static void handle_destroy(struct wl_resource *resource)
{
    // cleanup if needed
    printf("Resource %p destroyed\n", resource);
}

// Specific destroy handler for our global surface resource
static void handle_surface_destroy(struct wl_resource *resource) {
    printf("Surface resource %p destroyed\n", resource);
    if (g_client_surface == (struct wl_surface *)resource) {
        g_client_surface = nullptr;
        // Note: g_client_egl_window and g_client_egl_rendering_surface should be cleaned up
        // before or around the time the wl_surface is actually destroyed.
        // For this example, we'll do it in main's cleanup.
    }
}


// Forward declaration for compositor interface
static void compositor_interface_create_surface(struct wl_client *client, struct wl_resource *compositor_resource, uint32_t id);

static const struct wl_compositor_interface app_compositor_implementation = {
    .create_surface = compositor_interface_create_surface,
    // .create_region = NULL, // create_region is also part of wl_compositor_interface, but can be NULL if not implemented
};

// Implementation of wl_compositor.create_surface
static void compositor_interface_create_surface(struct wl_client *client,
                                            struct wl_resource *compositor_resource, // This is the client's wl_compositor resource
                                            uint32_t id)
{
    struct wl_resource *surface_resource = wl_resource_create(client, &wl_surface_interface, wl_resource_get_version(compositor_resource), id);
    if (!surface_resource) {
        wl_client_post_no_memory(client);
        fprintf(stderr, "Failed to create surface resource for client.\n");
        return;
    }
    // Set a specific destroy handler if we want to track our global surface
    wl_resource_set_implementation(surface_resource, NULL /* no methods needed for wl_surface itself from server side for EGL */, 
                                   NULL /* no server-side data associated with wl_surface directly */, 
                                   handle_surface_destroy);
    printf("wl_surface (id=%u) created for client %p\n", id, client);

    // HACK: Store the first created surface globally for EGL window creation.
    // A real compositor would manage a list of surfaces or associate EGL parts differently.
    if (!g_client_surface) {
        g_client_surface = (struct wl_surface*)surface_resource; // wl_resource is the server-side representation of wl_surface
        printf("Stored as g_client_surface: %p\n", g_client_surface);

        // Now that we have a wl_surface, create the wl_egl_window and EGLSurface
        if (egl_display != EGL_NO_DISPLAY && egl_config != NULL) {
            printf("Attempting to create wl_egl_window with surface %p\n", g_client_surface);
            g_client_egl_window = wl_egl_window_create(g_client_surface, 640, 480); // Fixed size for now
            if (!g_client_egl_window) {
                fprintf(stderr, "Failed to create wl_egl_window\n");
                // Don't clear g_client_surface here, its lifecycle is managed by Wayland.
                // But we can't proceed to create EGL rendering surface.
                return;
            }
            printf("wl_egl_window created: %p\n", g_client_egl_window);

            g_client_egl_rendering_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)g_client_egl_window, NULL);
            if (g_client_egl_rendering_surface == EGL_NO_SURFACE) {
                fprintf(stderr, "Failed to create EGL window surface, EGL error %x\n", eglGetError());
                wl_egl_window_destroy(g_client_egl_window);
                g_client_egl_window = NULL;
                // Don't clear g_client_surface here.
                return;
            }
            printf("EGL rendering surface created: %p\n", g_client_egl_rendering_surface);
            printf("Created wl_surface, wl_egl_window, and EGL surface for the first client.\n");
        } else {
            fprintf(stderr, "EGL not ready or surface not created properly, cannot create EGL window/surface.\n");
        }
    } else {
        printf("Another client created a surface, not using for global EGL window.\n");
    }
}

// Bind callback for compositor interface
static void bind_compositor(struct wl_client *client,
                            void *data, // This 'data' is from wl_global_create.
                            uint32_t version,
                            uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wl_compositor_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        fprintf(stderr, "Failed to create compositor resource for client.\n");
        return;
    }
    // Set the compositor implementation that handles create_surface
    // The 'data' here could be a pointer to our main compositor struct if we had one.
    wl_resource_set_implementation(resource, &app_compositor_implementation, data, handle_destroy);
    printf("Client %p bound to wl_compositor (id=%u)\n", client, id);
    // The actual surface creation will happen when the client calls wl_compositor.create_surface,
    // which will trigger compositor_interface_create_surface above.
}

// Basic OpenGL ES v2 rendering function
static void render_solid_color() {
    if (egl_display == EGL_NO_DISPLAY || 
        g_client_egl_rendering_surface == EGL_NO_SURFACE || 
        egl_context == EGL_NO_CONTEXT) {
        // fprintf(stderr, "render_solid_color: EGL resources not ready.\n");
        return;
    }

    if (!eglMakeCurrent(egl_display, g_client_egl_rendering_surface, g_client_egl_rendering_surface, egl_context)) {
        fprintf(stderr, "Failed to make EGL context current, EGL error %x\n", eglGetError());
        return;
    }

    // Using fixed size as per wl_egl_window_create
    int width = 640;
    int height = 480;
    glViewport(0, 0, width, height);

    // Clear to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!eglSwapBuffers(egl_display, g_client_egl_rendering_surface)) {
        fprintf(stderr, "Failed to swap EGL buffers, EGL error %x\n", eglGetError());
    }

    // It's good practice to unbind the context.
    if (!eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        // This might fail if the surface is already destroyed, not necessarily a critical error for unbinding.
        // fprintf(stderr, "Failed to unbind EGL context, EGL error %x\n", eglGetError());
    }
    // printf("Rendered solid color.\n"); // Can be noisy if called frequently
}

// Timer callback for rendering
static int render_timer_callback(void *data) {
    render_solid_color();
    if (g_render_timer_source) { // Re-arm the timer for continuous rendering
        wl_event_source_timer_update(g_render_timer_source, 16); // ~60 FPS
    }
    return 0; // Success
}

static void bind_shm(struct wl_client *client,
                     void *data,
                     uint32_t version,
                     uint32_t id)
{
    wl_resource *resource =
        wl_resource_create(client, &wl_shm_interface, version, id);
    wl_resource_set_implementation(resource, nullptr, nullptr, handle_destroy);
    printf("Client bound to wl_shm (id=%u)\n", id);
}

int main(int argc, char *argv[])
{
    // 1. Create the Wayland display
    display = wl_display_create();
    if (!display)
    {
        fprintf(stderr, "Failed to create Wayland display\n");
        return 1;
    }

    // Initialize EGL
    egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        wl_display_destroy(display);
        return 1;
    }

    EGLint major, minor;
    if (!eglInitialize(egl_display, &major, &minor)) {
        fprintf(stderr, "Failed to initialize EGL: %x\n", eglGetError());
        wl_display_destroy(display);
        return 1;
    }
    printf("EGL Initialized. Version: %d.%d\n", major, minor);

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    EGLint num_configs;
    if (!eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &num_configs)) {
        fprintf(stderr, "Failed to choose EGL config: %x\n", eglGetError());
        eglTerminate(egl_display);
        wl_display_destroy(display);
        return 1;
    }
    if (num_configs == 0) {
        fprintf(stderr, "No suitable EGL config found\n");
        eglTerminate(egl_display);
        wl_display_destroy(display);
        return 1;
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, // Request OpenGL ES 2.0
        EGL_NONE
    };
    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context: %x\n", eglGetError());
        eglTerminate(egl_display);
        wl_display_destroy(display);
        return 1;
    }
    printf("EGL context created.\n");

    // 2. Add a UNIX socket at WAYLAND_DISPLAY=wayland-0
    const char *sock = wl_display_add_socket_auto(display);
    if (!sock)
    {
        fprintf(stderr, "Failed to add socket\n");
        wl_display_destroy(display);
        return 1;
    }
    printf("Wayland socket created: %s\n", sock);

    // 3. Create a global for wl_compositor
    wl_global_create(display, &wl_compositor_interface, 4 /*version*/, nullptr, bind_compositor);
    printf("wl_compositor global advertised\n");
    wl_global_create(display, &wl_shm_interface, 1 /*version*/, nullptr, bind_shm);
    printf("wl_shm global advertised\n");

    // 4. Optional: advertise wl_shm for shared memory
    // wl_global_create(display, &wl_shm_interface, 1, nullptr, bind_shm);

    // 5. Run the event loop
    printf("Entering Wayland event loop...\n");
    loop = wl_display_get_event_loop(display);

    // Setup render timer
    g_render_timer_source = wl_event_loop_add_timer(loop, render_timer_callback, nullptr);
    if (g_render_timer_source) {
        wl_event_source_timer_update(g_render_timer_source, 16); // Initial fire after 16ms
        printf("Render timer initialized.\n");
    } else {
        fprintf(stderr, "Failed to create render timer source.\n");
        // Consider cleanup and exit if timer is critical
    }

    wl_event_loop_dispatch(loop, -1);

    // Cleanup
    // Remove timer first
    if (g_render_timer_source) {
        printf("Removing render timer source %p\n", g_render_timer_source);
        wl_event_source_remove(g_render_timer_source);
        g_render_timer_source = nullptr;
    }

    // Destroy EGL surface and wl_egl_window first
    if (g_client_egl_rendering_surface != EGL_NO_SURFACE) {
        printf("Destroying EGL rendering surface %p\n", g_client_egl_rendering_surface);
        eglDestroySurface(egl_display, g_client_egl_rendering_surface);
        g_client_egl_rendering_surface = EGL_NO_SURFACE;
    }
    if (g_client_egl_window != NULL) {
        printf("Destroying wl_egl_window %p\n", g_client_egl_window);
        wl_egl_window_destroy(g_client_egl_window);
        g_client_egl_window = NULL;
    }
    // g_client_surface is a wl_resource, it will be destroyed when the client disconnects
    // or explicitly via wl_resource_destroy. handle_surface_destroy will clear the global.
    // If g_client_surface is not null at this point, it means the client did not disconnect
    // or the surface was not explicitly destroyed. For this example, we assume client disconnect
    // handles it or that it's already null due to handle_surface_destroy.

    if (egl_display != EGL_NO_DISPLAY) {
        if (egl_context != EGL_NO_CONTEXT) {
            printf("Destroying EGL context %p\n", egl_context);
            eglDestroyContext(egl_display, egl_context);
            egl_context = EGL_NO_CONTEXT;
        }
        printf("Terminating EGL display %p\n", egl_display);
        eglTerminate(egl_display);
        egl_display = EGL_NO_DISPLAY;
    }

    printf("Destroying Wayland display %p\n", display);
    wl_display_destroy(display);
    display = nullptr;
    return 0;
}
