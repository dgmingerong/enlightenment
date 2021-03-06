#include "evas_common.h"
#include "evas_private.h"

/* local calls */

static void evas_object_intercept_init(Evas_Object *eo_obj);
static void evas_object_intercept_deinit(Evas_Object *eo_obj);

static void
evas_object_intercept_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj->interceptors)
     obj->interceptors = evas_mem_calloc(sizeof(Evas_Intercept_Func));
}

static void
evas_object_intercept_deinit(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj->interceptors) return;
   if ((obj->interceptors->show.func) ||
       (obj->interceptors->hide.func) ||
       (obj->interceptors->move.func) ||
       (obj->interceptors->resize.func) ||
       (obj->interceptors->raise.func) ||
       (obj->interceptors->lower.func) ||
       (obj->interceptors->stack_above.func) ||
       (obj->interceptors->stack_below.func) ||
       (obj->interceptors->layer_set.func) ||
       (obj->interceptors->color_set.func) ||
       (obj->interceptors->clip_set.func) ||
       (obj->interceptors->clip_unset.func))
     return;
   free(obj->interceptors);
   obj->interceptors = NULL;
}

/* private calls */

void
evas_object_intercept_cleanup(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (obj->interceptors) free(obj->interceptors);
}

int
evas_object_intercept_call_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->show.func);
   if (ret)
     obj->interceptors->show.func(obj->interceptors->show.data, eo_obj);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_hide(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->hide.func);
   if (ret)
     obj->interceptors->hide.func(obj->interceptors->hide.data, eo_obj);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_move(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->move.func);
   if (ret)
     obj->interceptors->move.func(obj->interceptors->move.data, eo_obj, x, y);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_resize(Evas_Object *eo_obj, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->resize.func);
   if (ret)
     obj->interceptors->resize.func(obj->interceptors->resize.data, eo_obj, w, h);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_raise(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->raise.func);
   if (ret)
     obj->interceptors->raise.func(obj->interceptors->raise.data, eo_obj);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_lower(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->lower.func);
   if (ret)
     obj->interceptors->lower.func(obj->interceptors->lower.data, eo_obj);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_stack_above(Evas_Object *eo_obj, Evas_Object *above)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->stack_above.func);
   if (ret)
     obj->interceptors->stack_above.func(obj->interceptors->stack_above.data, eo_obj, above);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_stack_below(Evas_Object *eo_obj, Evas_Object *below)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->stack_below.func);
   if (ret)
     obj->interceptors->stack_below.func(obj->interceptors->stack_below.data, eo_obj, below);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_layer_set(Evas_Object *eo_obj, int l)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->layer_set.func);
   if (ret)
     obj->interceptors->layer_set.func(obj->interceptors->layer_set.data, eo_obj, l);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_color_set(Evas_Object *eo_obj, int r, int g, int b, int a)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->color_set.func);
   if (ret)
     obj->interceptors->color_set.func(obj->interceptors->color_set.data, eo_obj, r, g, b, a);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_clip_set(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *clip)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->clip_set.func);
   if (ret)
     obj->interceptors->clip_set.func(obj->interceptors->clip_set.data, eo_obj, clip);
   obj->intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_clip_unset(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->clip_unset.func);
   if (ret)
     obj->interceptors->clip_unset.func(obj->interceptors->clip_unset.data, eo_obj);
   obj->intercepted = EINA_FALSE;
   return ret;
}

/* public calls */

EAPI void
evas_object_intercept_show_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Show_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->show.func = func;
   obj->interceptors->show.data = (void *)data;
}

EAPI void *
evas_object_intercept_show_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Show_Cb func)
{
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->show.func = NULL;
   data = obj->interceptors->show.data;
   obj->interceptors->show.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_hide_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Hide_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->hide.func = func;
   obj->interceptors->hide.data = (void *)data;
}

EAPI void *
evas_object_intercept_hide_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Hide_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->hide.func = NULL;
   data = obj->interceptors->hide.data;
   obj->interceptors->hide.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_move_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Move_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->move.func = func;
   obj->interceptors->move.data = (void *)data;
}

EAPI void *
evas_object_intercept_move_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Move_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->move.func = NULL;
   data = obj->interceptors->move.data;
   obj->interceptors->move.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_resize_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Resize_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->resize.func = func;
   obj->interceptors->resize.data = (void *)data;
}

EAPI void *
evas_object_intercept_resize_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Resize_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->resize.func = NULL;
   data = obj->interceptors->resize.data;
   obj->interceptors->resize.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_raise_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Raise_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->raise.func = func;
   obj->interceptors->raise.data = (void *)data;
}

EAPI void *
evas_object_intercept_raise_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Raise_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->raise.func = NULL;
   data = obj->interceptors->raise.data;
   obj->interceptors->raise.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_lower_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Lower_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->lower.func = func;
   obj->interceptors->lower.data = (void *)data;
}

EAPI void *
evas_object_intercept_lower_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Lower_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->lower.func = NULL;
   data = obj->interceptors->lower.data;
   obj->interceptors->lower.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_stack_above_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->stack_above.func = func;
   obj->interceptors->stack_above.data = (void *)data;
}

EAPI void *
evas_object_intercept_stack_above_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Stack_Above_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->stack_above.func = NULL;
   data = obj->interceptors->stack_above.data;
   obj->interceptors->stack_above.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_stack_below_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->stack_below.func = func;
   obj->interceptors->stack_below.data = (void *)data;
}

EAPI void *
evas_object_intercept_stack_below_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Stack_Below_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->stack_below.func = NULL;
   data = obj->interceptors->stack_below.data;
   obj->interceptors->stack_below.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_layer_set_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->layer_set.func = func;
   obj->interceptors->layer_set.data = (void *)data;
}

EAPI void *
evas_object_intercept_layer_set_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Layer_Set_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->layer_set.func = NULL;
   data = obj->interceptors->layer_set.data;
   obj->interceptors->layer_set.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_color_set_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->color_set.func = func;
   obj->interceptors->color_set.data = (void *)data;
}

EAPI void *
evas_object_intercept_color_set_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Color_Set_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->color_set.func = NULL;
   data = obj->interceptors->color_set.data;
   obj->interceptors->color_set.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_clip_set_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->clip_set.func = func;
   obj->interceptors->clip_set.data = (void *)data;
}

EAPI void *
evas_object_intercept_clip_set_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Clip_Set_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->clip_set.func = NULL;
   data = obj->interceptors->clip_set.data;
   obj->interceptors->clip_set.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}

EAPI void
evas_object_intercept_clip_unset_callback_add(Evas_Object *eo_obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!func) return;
   evas_object_intercept_init(eo_obj);
   if (!obj->interceptors) return;
   obj->interceptors->clip_unset.func = func;
   obj->interceptors->clip_unset.data = (void *)data;
}

EAPI void *
evas_object_intercept_clip_unset_callback_del(Evas_Object *eo_obj, Evas_Object_Intercept_Clip_Unset_Cb func)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   void *data;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->clip_unset.func = NULL;
   data = obj->interceptors->clip_unset.data;
   obj->interceptors->clip_unset.data = NULL;
   evas_object_intercept_deinit(eo_obj);
   return data;
}
