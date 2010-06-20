/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Panes panes
 *
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *panes;

   struct
     {
	Evas_Object *left;
	Evas_Object *right;
     } contents;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_theme_object_set(obj, wd->panes, "panes", "vertical", "default");
   edje_object_scale_set(wd->panes, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->panes, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, minw, minh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->contents.left)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
	      _changed_size_hints, obj);
	wd->contents.left = NULL;
	_sizing_eval(obj);
     }
   else if (sub == wd->contents.right)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
	      _changed_size_hints, obj);
	wd->contents.right= NULL;
	_sizing_eval(obj);
     }
}

static void
_clicked(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_press(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, "press", NULL);
}

   static void
_unpress(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, "unpress", NULL);
}

/**
 * Add a new panes to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Panel
 */
EAPI Evas_Object *
elm_panes_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "panes");
   elm_widget_type_set(obj, "panes");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->panes = edje_object_add(e);
   _elm_theme_object_set(obj, wd->panes, "panes", "vertical", "default");
   elm_widget_resize_object_set(obj, wd->panes);
   evas_object_show(wd->panes);

   edje_object_signal_callback_add(wd->panes, "elm,action,click", "", _clicked, obj);
   edje_object_signal_callback_add(wd->panes, "elm,action,press", "", _press, obj);
   edje_object_signal_callback_add(wd->panes, "elm,action,unpress", "", _unpress, obj);



   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI void elm_panes_content_left_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(wd->contents.left)
     {
	evas_object_del(wd->contents.left);
	wd->contents.left = NULL;
     }

   if(content)
     {
	wd->contents.left = content;
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->panes, "elm.swallow.left", content);
     }
}


EAPI void elm_panes_content_right_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(wd->contents.right)
     {
	evas_object_del(wd->contents.right);
	wd->contents.right = NULL;
     }

   if(content)
     {
	wd->contents.right = content;
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->panes, "elm.swallow.right", content);
     }
}

