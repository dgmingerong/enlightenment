#include "engage.h"
#include "config.h"
#include <Ecore_X.h>
#include <Esmart/Esmart_Trans_X11.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

Ecore_Evas     *ee;
Evas           *evas;
Ecore_X_Window  od_window;
static Ecore_Timer *mouse_focus_timer = NULL;

static void     handle_delete_request(Ecore_Evas * _ee);
static void     handle_pre_render_cb(Ecore_Evas * _ee);
static void     handle_post_render_cb(Ecore_Evas * _ee);
static void     handle_mouse_in(Ecore_Evas * _ee);
static void     handle_mouse_out(Ecore_Evas * _ee);
static void     handle_focus_out(Ecore_Evas * _ee);
static void     handle_mouse_down(void *data, Evas * e, Evas_Object * obj,
                                  void *event);
static void     handle_mouse_move(void *data, Evas * e, Evas_Object * obj,
                                  void *event);
static void     handle_menu_draw(void *data, Evas * e, Evas_Object * obj,
                                 void *event);

int
od_window_hide_timer_cb(void *data)
{
  if (mouse_focus_timer) {
    if (dock.state == zooming || dock.state == zoomed) {
      od_dock_zoom_out();
      mouse_focus_timer = NULL;
      return (0);
    }
  }
  return (1);
}
static void
handle_pre_render_cb(Ecore_Evas * _ee)
{
  edje_thaw();
  od_dock_redraw(_ee);
}
static void
handle_post_render_cb(Ecore_Evas * _ee)
{
  edje_freeze();
}
static void
handle_delete_cb(Ecore_Evas * _ee)
{
  if (_ee != ee)
    return;
  ecore_main_loop_quit();
}
static void
handle_focus_out(Ecore_Evas * _ee)
{
  if (_ee != ee)
    return;
  if (mouse_focus_timer)
    ecore_timer_del(mouse_focus_timer);
  mouse_focus_timer = ecore_timer_add(0.75, od_window_hide_timer_cb, NULL);
}
static void
handle_mouse_in(Ecore_Evas * _ee)
{
  if (_ee != ee)
    return;
  if (mouse_focus_timer)
    ecore_timer_del(mouse_focus_timer);
  mouse_focus_timer = NULL;
}
static void
handle_mouse_out(Ecore_Evas * _ee)
{
  if (_ee != ee)
    return;
  if (mouse_focus_timer)
    ecore_timer_del(mouse_focus_timer);
  mouse_focus_timer = ecore_timer_add(0.5, od_window_hide_timer_cb, NULL);
}

void
od_window_move()
{
  int             x, y, w, h;
  Evas_Object    *o = NULL;

  ecore_evas_geometry_get(ee, &x, &y, &w, &h);

#ifdef HAVE_TRANS_BG
  if ((o = evas_object_name_find(ecore_evas_get(ee), "trans")))
    esmart_trans_x11_freshen(o, x, y, w, h);
#endif
}

void
od_window_resize()
{
  int             x, y, w, h;
  Evas_Object    *o = NULL;

  ecore_evas_geometry_get(ee, &x, &y, &w, &h);

#ifdef HAVE_TRANS_BG
  if ((o = evas_object_name_find(ecore_evas_get(ee), "trans"))) {
    evas_object_resize(o, w, h);
    esmart_trans_x11_freshen(o, x, y, w, h);
  }
#endif
  options.width = w;
  options.height = h;
  ecore_config_int_set("engage.options.width", w);
  ecore_config_int_set("engage.options.height", h);
}

void
od_window_init()
{
  Ecore_X_Display *dsp;
  Screen         *scr;
  int             def;
  int             res_x, res_y;
  Evas_Object    *o;

  // determine the desktop size
  dsp = ecore_x_display_get();
  def = DefaultScreen(dsp);
  scr = ScreenOfDisplay(dsp, def);
  res_x = scr->width;
  res_y = scr->height;

  if (!(strcmp(options.engine, "gl")))
    ee = ecore_evas_gl_x11_new(NULL, 0,
                               (int) ((res_x - options.width) / 2.0),
                               (int) (res_y - options.height),
                               options.width, options.height);
  else {
    if (strcmp(options.engine, "software")) {
      fprintf(stderr,
              "Warning: Invalid engine type \"%s\" specified in config.\n");
      fprintf(stderr, "         Defaulting to software engine.\n");
    }
    ee = ecore_evas_software_x11_new(NULL, 0,
                                     (int) ((res_x - options.width) / 2.0),
                                     (int) (res_y - options.height),
                                     options.width, options.height);
  }

  ecore_evas_title_set(ee, "Engage");
  ecore_evas_name_class_set(ee, "engage", "engage");
  ecore_evas_borderless_set(ee, 1);
  if (options.mode == OM_ONTOP)
    ecore_evas_shaped_set(ee, 1);
  else
    ecore_evas_shaped_set(ee, 0);
  ecore_evas_callback_post_render_set(ee, handle_post_render_cb);
  ecore_evas_callback_pre_render_set(ee, handle_pre_render_cb);
  ecore_evas_callback_delete_request_set(ee, handle_delete_cb);
  ecore_evas_callback_mouse_out_set(ee, handle_mouse_out);
  ecore_evas_callback_mouse_in_set(ee, handle_mouse_in);
  ecore_evas_callback_focus_out_set(ee, handle_focus_out);

  evas = ecore_evas_get(ee);
  Evas_Object    *eventer = evas_object_rectangle_add(evas);

  evas_object_color_set(eventer, 0, 0, 0, 0);
  evas_object_resize(eventer, options.width, options.height);
  evas_object_move(eventer, 0.0, 0.0);
  evas_object_layer_set(eventer, 9999);
  evas_object_repeat_events_set(eventer, 1);
  evas_object_show(eventer);
#if 1
  evas_object_event_callback_add(eventer, EVAS_CALLBACK_MOUSE_DOWN,
                                 handle_menu_draw, NULL);
#endif
  evas_object_event_callback_add(eventer, EVAS_CALLBACK_MOUSE_MOVE,
                                 handle_mouse_move, NULL);

  od_window = ecore_evas_software_x11_window_get(ee);
  ecore_x_window_prop_xy_set(od_window,
                             (int) ((res_x - options.width) / 2.0),
                             (int) (res_y - options.height));
  ecore_x_window_prop_window_type_set(od_window, ECORE_X_WINDOW_TYPE_DOCK);
  ecore_x_window_prop_sticky_set(od_window, 1);
  if (options.mode == OM_ONTOP)
    ecore_x_window_prop_layer_set(od_window, ECORE_X_WINDOW_LAYER_ABOVE);
  else
    ecore_x_window_prop_layer_set(od_window, ECORE_X_WINDOW_LAYER_BELOW);

  ecore_evas_show(ee);
  ecore_evas_callback_move_set(ee, od_window_move);
  ecore_evas_callback_resize_set(ee, od_window_resize);

  if (options.mode == OM_BELOW) {
#ifdef HAVE_TRANS_BG
    o = esmart_trans_x11_new(evas);
    evas_object_layer_set(o, 0);
    evas_object_move(o, 0, 0);
    evas_object_resize(o, options.width, options.height);
    evas_object_name_set(o, "trans");

    esmart_trans_x11_freshen(o, (int) ((res_x - options.width) / 2.0),
                             (int) (res_y - options.height),
                             options.width, options.height);
    evas_object_show(o);
#else
    o = evas_object_rectangle_add(evas);
    evas_object_color_set(o, 0, 0, 0, 255);
    evas_object_layer_set(o, 0);
    evas_object_move(o, 0, 0);
    evas_object_resize(o, options.width, options.height);
    evas_object_name_set(o, "trans");
    evas_object_show(o);
#endif
  }
}

static void
handle_mouse_down(void *data, Evas * e, Evas_Object * obj, void *event)
{
  Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event;
  OD_Icon        *icon = NULL;

  {
    Evas_List      *objects =
      evas_objects_at_xy_get(evas, ev->canvas.x, ev->canvas.y, 0, 0);
    Evas_List      *item = objects;
    bool            done = false;

    while (item && !done) {
      const char     *name = evas_object_name_get((Evas_Object *) item->data);

      if (name && strncmp("icon", name, 4) == 0) {
        Evas_List      *i2 = dock.icons;

        while (i2 && !done) {
          if (((OD_Icon *) (i2->data))->pic == (Evas_Object *) item->data) {
            icon = (OD_Icon *) i2->data;
            done = true;
          }
          i2 = i2->next;
        }
      }
      item = item->next;
    }
  }
  if (!icon || !(icon->state & OD_ICON_STATE_USEABLE))
    return;

  if (icon->type == application_link) {
    if ((icon->data.applnk.count == 0 && ev->button == 1) || ev->button == 2) {
      // then make one...
      system(icon->data.applnk.command);
    } else if (icon->data.applnk.count > 0 && ev->button == 1) {
      Evas_List      *item = clients;

      while (item) {
        OD_Window      *window = (OD_Window *) item->data;

        if (window->applnk == icon)
          od_wm_activate_window(window->id);
        item = evas_list_next(item);
      }
    }
  } else if (icon->type == docked_icon) {
    if (ev->button == 1) {
      system(icon->data.dicon.command);
    }
  } else if (icon->type == minimised_window) {
    if (ev->button == 1) {
      od_wm_activate_window(icon->data.minwin.window);
    }
  }
}

static void
handle_mouse_move(void *data, Evas * e, Evas_Object * obj, void *event)
{
  Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event;

  if (ev->cur.canvas.y >=
      (options.height - options.arrow_size - options.size * dock.zoom) &&
      ev->cur.canvas.y <= options.height && ev->cur.canvas.x > dock.left_end &&
      ev->cur.canvas.x < dock.right_end) {
    dock.x = ev->cur.canvas.x;
    if (dock.state == unzoomed || dock.state == unzooming)
      od_dock_zoom_in();
    need_redraw = true;
  } else if (dock.state == zoomed || dock.state == zooming)
    od_dock_zoom_out();
}

static void
handle_menu_draw(void *data, Evas * e, Evas_Object * obj, void *event)
{
  Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event;

#ifdef HAVE_EWL
  if (ev->button == 3)
    od_config_menu_draw(ev->canvas.x, ev->canvas.y);
#endif
}
