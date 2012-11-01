#include "main.h"

#include <string.h>
#include <assert.h>

#include <Evas_Engine_Wayland_Egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>

/* Global struct */
struct _engine_wayland_egl_display
{
   struct wl_display *display;
   struct wl_registry *registry;
   struct wl_compositor *compositor;
   struct wl_surface *surface;
   struct wl_shell *shell;
   struct wl_shell_surface *shell_surface;
};

static struct _engine_wayland_egl_display wl;

/*
 * Function prototypes
 */
/* Registry handler */
static void _registry_handle_global(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int  version __UNUSED__);
static const struct wl_registry_listener _registry_listener =
{
   _registry_handle_global,
};

/* Shell Surface handler */
static void _shell_surface_handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);

static const struct wl_shell_surface_listener _shell_surface_listener =
{
   _shell_surface_handle_ping,
};

/*
 * API
 */
Eina_Bool
engine_wayland_egl_args(const char *engine __UNUSED__, int width, int height)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   evas_output_method_set(evas, evas_render_method_lookup("wayland_egl"));
   einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(evas);
   if (!einfo)
     {
        printf("Evas does not support the Wayland EGL Engine\n");
        return EINA_FALSE;
     }

   wl.display = wl_display_connect(NULL);
   wl.registry = wl_display_get_registry(wl.display);
   wl_registry_add_listener(wl.registry, &_registry_listener, NULL);
   wl_display_roundtrip(wl.display);

   assert(wl.compositor != NULL);
   assert(wl.shell != NULL);

   wl.surface = wl_compositor_create_surface(wl.compositor);
   wl.shell_surface = wl_shell_get_shell_surface(wl.shell, wl.surface);
   wl_shell_surface_set_title(wl.shell_surface, "Expedite Wayland EGL");
   wl_shell_surface_add_listener(wl.shell_surface, &_shell_surface_listener, NULL);
   wl_shell_surface_set_toplevel(wl.shell_surface);

   einfo->info.display = wl.display;
   einfo->info.surface = wl.surface;
   if (!evas_engine_info_set(evas, (Evas_Engine_Info *) einfo))
     {
        printf("Evas can not setup the informations of the Wayland EGL Engine\n");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
engine_wayland_egl_loop(void)
{
   assert(wl_display_dispatch_pending(wl.display) != -1);
   wl_display_flush(wl.display);
}

void
engine_wayland_egl_shutdown(void)
{
   wl_shell_surface_destroy(wl.shell_surface);
   wl_surface_destroy(wl.surface);
   wl_shell_destroy(wl.shell);
   wl_compositor_destroy(wl.compositor);
   wl_display_flush(wl.display);
   wl_display_disconnect(wl.display);
}

/*
 * Function implementation
 */
static void
_registry_handle_global(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int  version __UNUSED__)
{
   if (!strcmp(interface, "wl_compositor"))
      wl.compositor = wl_registry_bind(wl.registry, id, &wl_compositor_interface, 1);
   else if (!strcmp(interface, "wl_shell"))
      wl.shell = wl_registry_bind(wl.registry, id, &wl_shell_interface, 1);
}

static void
_shell_surface_handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
   wl_shell_surface_pong(shell_surface, serial);
}
