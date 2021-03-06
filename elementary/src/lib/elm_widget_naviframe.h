#ifndef ELM_WIDGET_NAVIFRAME_H
#define ELM_WIDGET_NAVIFRAME_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-naviframe-class The Elementary Naviframe Class
 *
 * Elementary, besides having the @ref Naviframe widget, exposes its
 * foundation -- the Elementary Naviframe Class -- in order to create other
 * widgets which are a naviframe with some more logic on top.
 */

/**
 * Base layout smart data extended with naviframe instance data.
 */
typedef struct _Elm_Naviframe_Smart_Data Elm_Naviframe_Smart_Data;
struct _Elm_Naviframe_Smart_Data
{
   Eina_Inlist          *stack; /* top item is the list's LAST item */
   Eina_List            *popping;
   Evas_Object          *dummy_edje;
   Evas_Display_Mode     dispmode;
   Elm_Object_Item      *compress_it;

   Eina_Bool             preserve : 1;
   Eina_Bool             on_deletion : 1;
   Eina_Bool             auto_pushed : 1;
   Eina_Bool             freeze_events : 1;
};

typedef struct _Elm_Naviframe_Item Elm_Naviframe_Item;
struct _Elm_Naviframe_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;

   Eina_Inlist *content_list;
   Eina_Inlist *text_list;
   Evas_Object *content;
   Evas_Object *title_prev_btn;
   Evas_Object *title_next_btn;
   Evas_Object *title_icon;
   Ecore_Animator *animator;
   const char  *style;
   const char  *title_label;
   const char  *subtitle_label;

   Evas_Coord   minw;
   Evas_Coord   minh;

   Eina_Bool    title_visible : 1;
   Eina_Bool    content_unfocusable : 1;
};

typedef struct _Elm_Naviframe_Content_Item_Pair Elm_Naviframe_Content_Item_Pair;
struct _Elm_Naviframe_Content_Item_Pair
{
   EINA_INLIST;
   const char *part;
   Evas_Object *content;
   Elm_Naviframe_Item *it;
};

typedef struct _Elm_Naviframe_Text_Item_Pair Elm_Naviframe_Text_Item_Pair;
struct _Elm_Naviframe_Text_Item_Pair
{
   EINA_INLIST;
   const char *part;
};

/**
 * @}
 */

#define ELM_NAVIFRAME_DATA_GET(o, sd) \
  Elm_Naviframe_Smart_Data * sd = eo_data_get(o, ELM_OBJ_NAVIFRAME_CLASS)

#define ELM_NAVIFRAME_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_NAVIFRAME_DATA_GET(o, ptr);                    \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_NAVIFRAME_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_NAVIFRAME_DATA_GET(o, ptr);                         \
  if (!ptr)                                               \
    {                                                     \
       CRITICAL("No widget data for object %p (%s)",      \
                o, evas_object_type_get(o));              \
       return val;                                        \
    }

#define ELM_NAVIFRAME_CHECK(obj)                                      \
  if (!eo_isa((obj), ELM_OBJ_NAVIFRAME_CLASS))    \
    return

#define ELM_NAVIFRAME_ITEM_CHECK(it)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_NAVIFRAME_CHECK(it->base.widget);

#define ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, ...)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_NAVIFRAME_CHECK(it->base.widget) __VA_ARGS__;

#endif
