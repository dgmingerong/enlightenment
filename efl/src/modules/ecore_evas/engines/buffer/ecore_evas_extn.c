#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _AIX
#  define alloca __alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include <Ecore_Ipc.h>

#include "ecore_evas_buffer_private.h"


static const char *interface_extn_name = "extn";
static const int   interface_extn_version = 1;

typedef struct _Shmfile Shmfile;

struct _Shmfile
{
   void *addr, *addr2;
   const char *file, *file2;
   int fd, fd2;
   int size;
};

static int blank = 0x00000000;

static Ecore_Evas_Interface_Extn *_ecore_evas_extn_interface_new(void);

static Shmfile *
shmfile_new(const char *base, int id, int size, Eina_Bool sys)
{
   Shmfile *sf;
   char file[PATH_MAX], file2[PATH_MAX];

   sf = calloc(1, sizeof(Shmfile));
   if (!sf) return NULL;
   sf->fd = -1;
   sf->fd2 = -1;
   sf->addr = MAP_FAILED;
   sf->addr = MAP_FAILED;
   do
     {
        mode_t mode;
        int v1, v2, v3;

        v1 = (int)time(NULL);
        v2 = (int)getpid();
        v3 = (int)rand();
        mode = S_IRUSR | S_IWUSR;
        if (sys) mode |= S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        snprintf(file, sizeof(file), "/%s-%i-%i.%i.%i", base, id, v1, v2, v3);
        sf->fd = shm_open(file, O_RDWR | O_CREAT | O_EXCL, mode);
        if (sf->fd >= 0)
          {
             snprintf(file2, sizeof(file2), "/%s-%i-%i.%i.%i-b", base, id, v1, v2, v3);
             sf->fd2 = shm_open(file2, O_RDWR | O_CREAT | O_EXCL, mode);
             if (sf->fd2 < 0)
               {
                  close(sf->fd);
                  sf->fd = -1;
               }
          }
     }
   while (sf->fd < 0);

   sf->file = eina_stringshare_add(file);
   if (!sf->file) goto err;
   sf->file2 = eina_stringshare_add(file2);
   if (!sf->file2) goto err;
   sf->size = size;
   if (ftruncate(sf->fd, size) < 0) goto err;
   if (ftruncate(sf->fd2, size) < 0) goto err;
   sf->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sf->fd, 0);
   if (sf->addr == MAP_FAILED) goto err;
   sf->addr2 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sf->fd2, 0);
   if (sf->addr2 == MAP_FAILED) goto err;
   return sf;
err:
   if (sf->addr != MAP_FAILED) munmap(sf->addr, sf->size);
   if (sf->addr2 != MAP_FAILED) munmap(sf->addr2, sf->size);
   if (sf->fd >= 0)
     {
        close(sf->fd);
        shm_unlink(file);
     }
   if (sf->fd2 >= 0)
     {
        close(sf->fd2);
        shm_unlink(file2);
     }
   if (sf->file) eina_stringshare_del(sf->file);
   if (sf->file2) eina_stringshare_del(sf->file2);
   free(sf);
   return NULL;
}

void
shmfile_free(Shmfile *sf)
{
   munmap(sf->addr, sf->size);
   munmap(sf->addr2, sf->size);
   close(sf->fd);
   close(sf->fd2);
   shm_unlink(sf->file);
   shm_unlink(sf->file2);
   eina_stringshare_del(sf->file);
   eina_stringshare_del(sf->file2);
   free(sf);
}

static Shmfile *
shmfile_open(const char *ref, int size, Eina_Bool sys)
{
   Shmfile *sf;
   mode_t mode;
   char *s;
   int l;
   
   sf = calloc(1, sizeof(Shmfile));
   if (!sf) return NULL;
   sf->fd = -1;
   sf->fd2 = -1;
   sf->addr = MAP_FAILED;
   sf->addr = MAP_FAILED;
   mode = S_IRUSR | S_IWUSR;
   if (sys) mode |= S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
   l = strlen(ref);
   s = alloca(l + 3);
   strcpy(s, ref);
   s[l] = '-';
   s[l + 1] = 'b';
   s[l + 2] = 0;
   sf->file = eina_stringshare_add(ref);
   if (!sf->file) goto err;
   sf->file2 = eina_stringshare_add(s);
   if (!sf->file2) goto err;
   sf->fd = shm_open(sf->file, O_RDWR, mode);
   if (sf->fd < 0) goto err;
   sf->fd2 = shm_open(sf->file2, O_RDWR, mode);
   if (sf->fd2 < 0) goto err;
   sf->size = size;
   sf->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sf->fd, 0);
   if (sf->addr == MAP_FAILED) goto err;
   sf->addr2 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sf->fd2, 0);
   if (sf->addr2 == MAP_FAILED) goto err;
   return sf;
err:
   if (sf->addr != MAP_FAILED) munmap(sf->addr, sf->size);
   if (sf->addr2 != MAP_FAILED) munmap(sf->addr2, sf->size);
   if (sf->fd >= 0) close(sf->fd);
   if (sf->fd2 >= 0) close(sf->fd2);
   if (sf->file) eina_stringshare_del(sf->file);
   if (sf->file2) eina_stringshare_del(sf->file2);
   free(sf);
   return NULL;
}

void
shmfile_close(Shmfile *sf)
{
   munmap(sf->addr, sf->size);
   munmap(sf->addr2, sf->size);
   close(sf->fd);
   close(sf->fd2);
   eina_stringshare_del(sf->file);
   eina_stringshare_del(sf->file2);
   free(sf);
}

// procotol version - change this as needed
#define MAJOR 0x1011

enum // opcodes
{
   OP_RESIZE,
   OP_SHOW,
   OP_HIDE,
   OP_FOCUS,
   OP_UNFOCUS,
   OP_UPDATE,
   OP_UPDATE_DONE,
   OP_LOCK_FILE,
   OP_SHM_REF,
   OP_PROFILE_CHANGE_REQUEST,
   OP_PROFILE_CHANGE_DONE,
   OP_EV_MOUSE_IN,
   OP_EV_MOUSE_OUT,
   OP_EV_MOUSE_UP,
   OP_EV_MOUSE_DOWN,
   OP_EV_MOUSE_MOVE,
   OP_EV_MOUSE_WHEEL,
   OP_EV_MULTI_UP,
   OP_EV_MULTI_DOWN,
   OP_EV_MULTI_MOVE,
   OP_EV_KEY_UP,
   OP_EV_KEY_DOWN,
   OP_EV_HOLD,
   OP_LOCK_FILE2
};

enum
{
   MOD_SHIFT  = (1 << 0),
   MOD_CTRL   = (1 << 1),
   MOD_ALT    = (1 << 2),
   MOD_META   = (1 << 3),
   MOD_HYPER  = (1 << 4),
   MOD_SUPER  = (1 << 5),
   MOD_CAPS   = (1 << 6),
   MOD_NUM    = (1 << 7),
   MOD_SCROLL = (1 << 8),
};

typedef struct _Ipc_Data_Resize Ipc_Data_Resize;
typedef struct _Ipc_Data_Update Ipc_Data_Update;
typedef struct _Ipc_Data_Profile Ipc_Data_Profile;
typedef struct _Ipc_Data_Ev_Mouse_In Ipc_Data_Ev_Mouse_In;
typedef struct _Ipc_Data_Ev_Mouse_Out Ipc_Data_Ev_Mouse_Out;
typedef struct _Ipc_Data_Ev_Mouse_Up Ipc_Data_Ev_Mouse_Up;
typedef struct _Ipc_Data_Ev_Mouse_Down Ipc_Data_Ev_Mouse_Down;
typedef struct _Ipc_Data_Ev_Mouse_Move Ipc_Data_Ev_Mouse_Move;
typedef struct _Ipc_Data_Ev_Mouse_Wheel Ipc_Data_Ev_Mouse_Wheel;
typedef struct _Ipc_Data_Ev_Hold Ipc_Data_Ev_Hold;
typedef struct _Ipc_Data_Ev_Multi_Up Ipc_Data_Ev_Multi_Up;
typedef struct _Ipc_Data_Ev_Multi_Down Ipc_Data_Ev_Multi_Down;
typedef struct _Ipc_Data_Ev_Multi_Move Ipc_Data_Ev_Multi_Move;
typedef struct _Ipc_Data_Ev_Key_Up Ipc_Data_Ev_Key_Up;
typedef struct _Ipc_Data_Ev_Key_Down Ipc_Data_Ev_Key_Down;

struct _Ipc_Data_Resize
{
   int w, h;
};

struct _Ipc_Data_Update
{
   int x, w, y, h;
};

struct _Ipc_Data_Profile
{
   const char *name;
};

struct _Ipc_Data_Ev_Mouse_In
{
   unsigned int timestamp;
   int mask;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Out
{
   unsigned int timestamp;
   int mask;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Up
{
   int b;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Down
{
   int b;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Move
{
   int x, y;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Wheel
{
   int direction, z;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Hold
{
   int hold;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Up
{
   Evas_Button_Flags flags;
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Down
{
   Evas_Button_Flags flags;
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Move
{
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Key_Up
{
   const char *keyname, *key, *string, *compose;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Key_Down
{
   const char *keyname, *key, *string, *compose;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

typedef struct _Extn Extn;

struct _Extn
{
   struct {
        Ecore_Ipc_Server *server;
        Eina_List *clients;
        Eina_List *handlers;
        Eina_Bool am_server : 1;
   } ipc;
   struct {
        const char *name;
        int         num;
        Eina_Bool   sys : 1;
   } svc;
   struct {
        const char *lock, *lock2;
        int         lockfd, lockfd2;
        const char *shm;
        int         w, h;
        Shmfile    *shmfile;
        Eina_List  *updates;
        Eina_Bool   have_lock : 1;
        Eina_Bool   have_real_lock : 1;
        Eina_Bool   have_lock2 : 1;
        Eina_Bool   have_real_lock2 : 1;
   } file;
   struct {
        Eina_Bool   done : 1; /* need to send change done event to the client(plug) */
   } profile;
};

static Eina_List *extn_ee_list = NULL;

EAPI int ECORE_EVAS_EXTN_CLIENT_ADD = 0;
EAPI int ECORE_EVAS_EXTN_CLIENT_DEL = 0;

Eina_Bool
_ecore_evas_extn_init(void)
{
   if (!ECORE_EVAS_EXTN_CLIENT_ADD)
     {
	ECORE_EVAS_EXTN_CLIENT_ADD = ecore_event_type_new();
	ECORE_EVAS_EXTN_CLIENT_DEL = ecore_event_type_new();
     }

   return EINA_TRUE;
}

void
_ecore_evas_extn_shutdown(void)
{
}

static void
_ecore_evas_extn_event_free(void *data, void *ev EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image)
     evas_object_unref(bdata->image);
   _ecore_evas_unref(ee);
}

static void
_ecore_evas_extn_event(Ecore_Evas *ee, int event)
{
    Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   _ecore_evas_ref(ee);

   if (bdata->image)
     evas_object_ref(bdata->image);
   ecore_event_add(event, bdata->image,
                   _ecore_evas_extn_event_free, ee);
}

static Eina_Bool
_ecore_evas_lock_other_have(Ecore_Evas *ee, int buf)
{
   Eina_List *l;
   Ecore_Evas *ee2;
   Extn *extn, *extn2;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Ecore_Evas_Engine_Buffer_Data *bdata2;

   extn = bdata->data;
   if (!extn) return EINA_FALSE;
   // brute force - i know. i expect extn_ee_list to be fairly short. could
   // be improved with a hash of lockfiles
   EINA_LIST_FOREACH(extn_ee_list, l, ee2)
     {
        if (ee == ee2) continue;
	bdata2 = ee2->engine.data;
        extn2 = bdata2->data;
        if (!extn2) continue;
        if (buf == 0)
          {
             if ((extn->file.lock) && (extn2->file.lock) &&
                 (!strcmp(extn->file.lock, extn2->file.lock)) &&
                 (extn2->file.have_real_lock))
               return EINA_TRUE;
          }
        else if (buf == 1)
          {
             if ((extn->file.lock2) && (extn2->file.lock2) &&
                 (!strcmp(extn->file.lock2, extn2->file.lock2)) &&
                 (extn2->file.have_real_lock2))
               return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static void
_ecore_evas_socket_lock(Ecore_Evas *ee, int buf)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = bdata->data;
   if (!extn) return;
   if (buf == 0)
     {
        if (extn->file.lockfd < 0) return;
        if (extn->file.have_lock) return;
        extn->file.have_lock = EINA_TRUE;
        if (_ecore_evas_lock_other_have(ee, buf)) return;
        if (lockf(extn->file.lockfd, F_ULOCK, 0) < 0)
          {
             extn->file.have_lock = EINA_FALSE;
             return;
          }
        extn->file.have_real_lock = EINA_TRUE;
     }
   else if (buf == 1)
     {
        if (extn->file.lockfd2 < 0) return;
        if (extn->file.have_lock2) return;
        extn->file.have_lock2 = EINA_TRUE;
        if (_ecore_evas_lock_other_have(ee, buf)) return;
        if (lockf(extn->file.lockfd2, F_ULOCK, 0) < 0)
          {
             extn->file.have_lock2 = EINA_FALSE;
             return;
          }
        extn->file.have_real_lock2 = EINA_TRUE;
     }
}

static void
_ecore_evas_socket_unlock(Ecore_Evas *ee, int buf)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = bdata->data;
   if (!extn) return;
   if (buf == 0)
     {
        if (extn->file.lockfd < 0) return;
        if (!extn->file.have_lock) return;
        extn->file.have_lock = EINA_FALSE;
        if (!extn->file.have_real_lock) return;
        if (lockf(extn->file.lockfd, F_ULOCK, 0) < 0) return;
     }
   else if (buf == 1)
     {
        if (extn->file.lockfd2 < 0) return;
        if (!extn->file.have_lock2) return;
        extn->file.have_lock2 = EINA_FALSE;
        if (!extn->file.have_real_lock2) return;
        if (lockf(extn->file.lockfd2, F_ULOCK, 0) < 0) return;
     }
}

static void
_ecore_evas_extn_plug_targer_render_pre(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   if (ee) _ecore_evas_socket_lock(ee, 0); // XXX choose right buffer to lock
}

static void
_ecore_evas_extn_plug_targer_render_post(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   if (ee) _ecore_evas_socket_unlock(ee, 0); // XXX choose right buffer to lock
}

static void
_ecore_evas_extn_plug_image_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   ecore_evas_free(ee);
}

static void
_ecore_evas_extn_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   evas_object_geometry_get(bdata->image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(bdata->image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   if (fh < 1) fh = 1;

   if (evas_object_map_get(bdata->image) &&
       evas_object_map_enable_get(bdata->image))
     {
        fx = 0; fy = 0;
        fw = ee->w; fh = ee->h;
        ww = ee->w; hh = ee->h;
     }

   if ((fx == 0) && (fy == 0) && (fw == ww) && (fh == hh))
     {
        *x = (ee->w * (*x - xx)) / fw;
        *y = (ee->h * (*y - yy)) / fh;
     }
   else
     {
        xx = (*x - xx) - fx;
        while (xx < 0) xx += fw;
        while (xx > fw) xx -= fw;
        *x = (ee->w * xx) / fw;

        yy = (*y - yy) - fy;
        while (yy < 0) yy += fh;
        while (yy > fh) yy -= fh;
        *y = (ee->h * yy) / fh;
     }
}

static void
_ecore_evas_extn_free(Ecore_Evas *ee)
{
   Extn *extn;
   Ecore_Ipc_Client *client;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = bdata->data;
   if (extn)
     {
        Ecore_Event_Handler *hdl;
        Ipc_Data_Update *ipc;

        if (extn->file.have_lock) _ecore_evas_socket_unlock(ee, 0);
        if (extn->file.have_lock2) _ecore_evas_socket_unlock(ee, 1);
        if (extn->file.lockfd >= 0)
          {
             close(extn->file.lockfd);
             if (extn->ipc.am_server)
               {
                  if (extn->file.lock) unlink(extn->file.lock);
               }
          }
        if (extn->file.lockfd2 >= 0)
          {
             close(extn->file.lockfd2);
             if (extn->ipc.am_server)
               {
                  if (extn->file.lock2) unlink(extn->file.lock2);
               }
          }
        if (extn->svc.name) eina_stringshare_del(extn->svc.name);
        if (extn->ipc.clients)
          {
             EINA_LIST_FREE(extn->ipc.clients, client)
               ecore_ipc_client_del(client);
          }
        if (extn->ipc.server) ecore_ipc_server_del(extn->ipc.server);
        if (extn->file.lock) eina_stringshare_del(extn->file.lock);
        if (extn->file.lock2) eina_stringshare_del(extn->file.lock2);
        if (extn->file.shm) eina_stringshare_del(extn->file.shm);
        if (extn->file.shmfile)
          {
             if (extn->ipc.am_server)
               shmfile_free(extn->file.shmfile);
             else
               shmfile_close(extn->file.shmfile);
          }

        EINA_LIST_FREE(extn->file.updates, ipc)
          free(ipc);

        EINA_LIST_FREE(extn->ipc.handlers, hdl)
          ecore_event_handler_del(hdl);
        free(extn);
        ecore_ipc_shutdown();
        bdata->data = NULL;
     }
   if (bdata->image)
     {
        Ecore_Evas *ee2;

        evas_object_event_callback_del_full(bdata->image,
                                            EVAS_CALLBACK_DEL,
                                            _ecore_evas_extn_plug_image_obj_del,
                                            ee);
        evas_event_callback_del_full(evas_object_evas_get(bdata->image),
                                     EVAS_CALLBACK_RENDER_PRE,
                                     _ecore_evas_extn_plug_targer_render_pre,
                                     ee);
        evas_event_callback_del_full(evas_object_evas_get(bdata->image),
                                     EVAS_CALLBACK_RENDER_POST,
                                     _ecore_evas_extn_plug_targer_render_post,
                                     ee);
        evas_object_del(bdata->image);
        ee2 = evas_object_data_get(bdata->image, "Ecore_Evas_Parent");
        if (ee2)
          {
             ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
          }
     }
   free(bdata);
   extn_ee_list = eina_list_remove(extn_ee_list, ee);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   /*
    * No need for it if not used later.
   Extn *extn;

   extn = bdata->data;
   */
   if (bdata->image)
     evas_object_image_size_set(bdata->image, ee->w, ee->h);
   /* Server can have many plugs, so I block resize comand from client to server *
      if ((extn) && (extn->ipc.server))
      {
      Ipc_Data_Resize ipc;

      ipc.w = ee->w;
      ipc.h = ee->h;
      ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_RESIZE, 0, 0, 0, &ipc, sizeof(ipc));
      }*/
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_resize(ee, w, h);
}

static int
_ecore_evas_modifiers_locks_mask_get(Evas *e)
{
   int mask = 0;

   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Shift"))
     mask |= MOD_SHIFT;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     mask |= MOD_CTRL;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     mask |= MOD_ALT;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     mask |= MOD_META;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     mask |= MOD_HYPER;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     mask |= MOD_SUPER;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     mask |= MOD_SCROLL;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     mask |= MOD_NUM;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     mask |= MOD_CAPS;
   return mask;
}

static void
_ecore_evas_modifiers_locks_mask_set(Evas *e, int mask)
{
   if (mask & MOD_SHIFT) evas_key_modifier_on (e, "Shift");
   else                  evas_key_modifier_off(e, "Shift");
   if (mask & MOD_CTRL)  evas_key_modifier_on (e, "Control");
   else                  evas_key_modifier_off(e, "Control");
   if (mask & MOD_ALT)   evas_key_modifier_on (e, "Alt");
   else                  evas_key_modifier_off(e, "Alt");
   if (mask & MOD_META)  evas_key_modifier_on (e, "Meta");
   else                  evas_key_modifier_off(e, "Meta");
   if (mask & MOD_HYPER) evas_key_modifier_on (e, "Hyper");
   else                  evas_key_modifier_off(e, "Hyper");
   if (mask & MOD_SUPER) evas_key_modifier_on (e, "Super");
   else                  evas_key_modifier_off(e, "Super");
   if (mask & MOD_SCROLL) evas_key_lock_on (e, "Scroll_Lock");
   else                   evas_key_lock_off(e, "Scroll_Lock");
   if (mask & MOD_NUM)    evas_key_lock_on (e, "Num_Lock");
   else                   evas_key_lock_off(e, "Num_Lock");
   if (mask & MOD_CAPS)   evas_key_lock_on (e, "Caps_Lock");
   else                   evas_key_lock_off(e, "Caps_Lock");
}

static void
_ecore_evas_extn_cb_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_In *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_In ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_IN, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Out *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Out ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_OUT, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
       /* We have send mouse move event before mouse down event */
       {
          Ipc_Data_Ev_Mouse_Move ipc_move;
          memset(&ipc_move, 0, sizeof(ipc_move));
          Evas_Coord x, y;

          x = ev->canvas.x;
          y = ev->canvas.y;
          _ecore_evas_extn_coord_translate(ee, &x, &y);
          ipc_move.x = x;
          ipc_move.y = y;
          ipc_move.timestamp = ev->timestamp;
          ipc_move.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
          ipc_move.event_flags = ev->event_flags;
          ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_MOVE, 0, 0, 0, &ipc_move, sizeof(ipc_move));
       }
       {
          Ipc_Data_Ev_Mouse_Down ipc;
          memset(&ipc, 0, sizeof(ipc));
          ipc.b = ev->button;
          ipc.flags = ev->flags;
          ipc.timestamp = ev->timestamp;
          ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
          ipc.event_flags = ev->event_flags;
          ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_DOWN, 0, 0, 0, &ipc, sizeof(ipc));
       }
     }
}

static void
_ecore_evas_extn_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Up ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.b = ev->button;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_UP, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Move *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Move ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        x = ev->cur.canvas.x;
        y = ev->cur.canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_MOVE, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Wheel *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Wheel ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.direction = ev->direction;
        ipc.z = ev->z;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_WHEEL, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_multi_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Down *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Down ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->canvas.x;
        y = ev->canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->canvas.xsub;
        ipc.fy = ev->canvas.ysub;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_DOWN, 0, 0, 0, &ipc, sizeof(ipc));
     }
}


static void
_ecore_evas_extn_cb_multi_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Up ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->canvas.x;
        y = ev->canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->canvas.xsub;
        ipc.fy = ev->canvas.ysub;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_UP, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_multi_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Move *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Move ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->cur.canvas.x;
        y = ev->cur.canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->cur.canvas.xsub;
        ipc.fy = ev->cur.canvas.ysub;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_MOVE, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Key_Down *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Key_Down *ipc;
        char *st, *p;
        int len = 0;

        len = sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key) len += strlen(ev->key) + 1;
        if (ev->keyname) len += strlen(ev->keyname) + 1;
        if (ev->string) len += strlen(ev->string) + 1;
        if (ev->compose) len += strlen(ev->compose) + 1;
        len += 1;
        st = alloca(len);
        ipc = (Ipc_Data_Ev_Key_Down *)st;
        memset(st, 0, len);
        p = st + sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key)
          {
             strcpy(p, ev->key);
             ipc->key = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->keyname)
          {
             strcpy(p, ev->keyname);
             ipc->keyname = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->string)
          {
             strcpy(p, ev->string);
             ipc->string = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->compose)
          {
             strcpy(p, ev->compose);
             ipc->compose = p - (long)st;
             p += strlen(p) + 1;
          }
        ipc->timestamp = ev->timestamp;
        ipc->mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc->event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_KEY_DOWN, 0, 0, 0, ipc, len);
     }
}

static void
_ecore_evas_extn_cb_key_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Key_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Key_Up *ipc;
        char *st, *p;
        int len = 0;

        len = sizeof(Ipc_Data_Ev_Key_Up);
        if (ev->key) len += strlen(ev->key) + 1;
        if (ev->keyname) len += strlen(ev->keyname) + 1;
        if (ev->string) len += strlen(ev->string) + 1;
        if (ev->compose) len += strlen(ev->compose) + 1;
        len += 1;
        st = alloca(len);
        ipc = (Ipc_Data_Ev_Key_Up *)st;
        memset(st, 0, len);
        p = st + sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key)
          {
             strcpy(p, ev->key);
             ipc->key = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->keyname)
          {
             strcpy(p, ev->keyname);
             ipc->keyname = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->string)
          {
             strcpy(p, ev->string);
             ipc->string = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->compose)
          {
             strcpy(p, ev->compose);
             ipc->compose = p - (long)st;
             p += strlen(p) + 1;
          }
        ipc->timestamp = ev->timestamp;
        ipc->mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc->event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_KEY_UP, 0, 0, 0, ipc, len);
     }
}

static void
_ecore_evas_extn_cb_hold(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Hold *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Hold ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.hold = ev->hold;
        ipc.timestamp = ev->timestamp;
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_HOLD, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->prop.focused = 1;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_FOCUS, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_focus_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->prop.focused = 0;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_UNFOCUS, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->visible = 1;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_SHOW, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->visible = 0;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_HIDE, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_plug_profile_set(Ecore_Evas *ee, const char *profile)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   extn = bdata->data;
   if (!extn) return;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);
        if (extn->ipc.server)
          {
             Ipc_Data_Profile *ipc;
             char *st, *p;
             int len = 0;

             len += sizeof(Ipc_Data_Profile);
             len += strlen(ee->prop.profile.name) + 1;
             len += 1;

             st = alloca(len);
             ipc = (Ipc_Data_Profile *)st;
             memset(st, 0, len);
             p = st + sizeof(Ipc_Data_Profile);

             strcpy(p, ee->prop.profile.name);
             ipc->name = p - (long)st;
             p += strlen(p) + 1;

             /* send window profile change request to the server(socket)
              * and wait for profile change done from the server
              */
             ecore_ipc_server_send(extn->ipc.server, MAJOR,
                                   OP_PROFILE_CHANGE_REQUEST,
                                   0, 0, 0, ipc, len);
          }
     }
}

static const Ecore_Evas_Engine_Func _ecore_extn_plug_engine_func =
{
   _ecore_evas_extn_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_resize,
   _ecore_evas_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL, //transparent
   NULL, // profiles_set
   _ecore_evas_extn_plug_profile_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL, // render
   NULL, // screen_geometry_get
   NULL  // screen_dpi_get
};

static Eina_Bool
_ipc_server_add(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Add *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(e->server))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   //FIXME: find a way to let app know server there
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_server_del(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Del *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (extn->ipc.server != e->server) return ECORE_CALLBACK_PASS_ON;
   evas_object_image_data_set(bdata->image, NULL);
   bdata->pixels = NULL;
   if (extn->file.shmfile)
     {
        shmfile_close(extn->file.shmfile);
        extn->file.shmfile = NULL;
     }
   if (extn->file.shm)
     {
        eina_stringshare_del(extn->file.shm);
        extn->file.shm = NULL;
     }
   extn->ipc.server = NULL;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_server_data(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Data *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(e->server))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (e->major != MAJOR)
     return ECORE_CALLBACK_PASS_ON;
   switch (e->minor)
     {
      case OP_UPDATE:
         // add rect to update list
         if (e->size >= (int)sizeof(Ipc_Data_Update))
           {
              Ipc_Data_Update *ipc = malloc(sizeof(Ipc_Data_Update));
              if (ipc)
                {
                   memcpy(ipc, e->data, sizeof(Ipc_Data_Update));
                   extn->file.updates = eina_list_append(extn->file.updates, ipc);
                }
           }
         break;
      case OP_UPDATE_DONE:
         // updates finished being sent - done now. frame ready
           {
              Ipc_Data_Update *ipc;

              EINA_LIST_FREE(extn->file.updates, ipc)
                {
                   if (bdata->image)
                     evas_object_image_data_update_add(bdata->image,
                                                       ipc->x, ipc->y,
                                                       ipc->w, ipc->h);
                   free(ipc);
                }
           }
         break;
      case OP_LOCK_FILE:
         if ((e->data) && (e->size > 0) &&
             (((unsigned char *)e->data)[e->size - 1] == 0))
           {
              if (extn->file.have_lock) _ecore_evas_socket_unlock(ee, 0);
              if (extn->file.lockfd) close(extn->file.lockfd);
              if (extn->file.lock) eina_stringshare_del(extn->file.lock);
              extn->file.lock = eina_stringshare_add(e->data);
              extn->file.lockfd = open(extn->file.lock, O_RDONLY);
           }
         break;
      case OP_LOCK_FILE2:
         if ((e->data) && (e->size > 0) &&
             (((unsigned char *)e->data)[e->size - 1] == 0))
           {
              if (extn->file.have_lock2) _ecore_evas_socket_unlock(ee, 1);
              if (extn->file.lockfd2 >= 0) close(extn->file.lockfd2);
              if (extn->file.lock2) eina_stringshare_del(extn->file.lock2);
              extn->file.lock2 = eina_stringshare_add(e->data);
              extn->file.lockfd2 = open(extn->file.lock2, O_RDONLY);
           }
         break;
      case OP_SHM_REF:
         // e->ref == w
         // e->ref_to == h
         // e->response == alpha
         // e->data = shm ref string + nul byte
         if ((e->data) && ((unsigned char *)e->data)[e->size - 1] == 0)
           {
              bdata->pixels = NULL;
              if (extn->file.shmfile)
                {
                   shmfile_close(extn->file.shmfile);
                   extn->file.shmfile = NULL;
                }
              if (extn->file.shm)
                {
                   eina_stringshare_del(extn->file.shm);
                   extn->file.shm = NULL;
                }
              if ((e->ref > 0) && (e->ref_to > 0))
                {
                   extn->file.w = e->ref;
                   extn->file.h = e->ref_to;
                   extn->file.shm = eina_stringshare_add(e->data);
                   extn->file.shmfile = shmfile_open(extn->file.shm,
                                                     extn->file.w *
                                                     extn->file.h * 4,
                                                     EINA_TRUE);
                   if (extn->file.shmfile)
                     {
                        bdata->pixels = extn->file.shmfile->addr;
                        if (bdata->image)
                          {
                             if (e->response)
                               evas_object_image_alpha_set(bdata->image,
                                                           EINA_TRUE);
                             else
                               evas_object_image_alpha_set(bdata->image,
                                                           EINA_FALSE);
                             evas_object_image_size_set(bdata->image,
                                                        extn->file.w,
                                                        extn->file.h);
                             evas_object_image_data_set(bdata->image,
                                                        bdata->pixels);
                             evas_object_image_data_update_add(bdata->image,
                                                               0, 0,
                                                               extn->file.w,
                                                               extn->file.h);
                             _ecore_evas_resize(ee,
                                                extn->file.w,
                                                extn->file.h);
                          }
                        else
                          evas_object_image_data_set(bdata->image, NULL);
                     }
                   else
                     evas_object_image_data_set(bdata->image, NULL);
                }
              else
                evas_object_image_data_set(bdata->image, NULL);
           }
         break;
      case OP_RESIZE:
         if ((e->data) && (e->size >= (int)sizeof(Ipc_Data_Resize)))
           {
              Ipc_Data_Resize *ipc = e->data;
              _ecore_evas_resize(ee, ipc->w, ipc->h);
           }
         break;
      case OP_PROFILE_CHANGE_DONE:
         /* profile change finished being sent - done now. */
         /* do something here */
         break;
      default:
         break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

EAPI Evas_Object *
ecore_evas_extn_plug_new_internal(Ecore_Evas *ee_target)
{
   Evas_Object *o;
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas_Interface_Extn *iface;
   int w = 1, h = 1;

   if (!ee_target) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ee->engine.data = bdata;
   o = evas_object_image_filled_add(ee_target->evas);
   /* this make problem in gl engine, so I'll block this until solve problem 
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);*/
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_alpha_set(o, 1);
   evas_object_image_size_set(o, 1, 1);
   evas_object_image_data_set(o, &blank);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_extn_plug_engine_func;

   ee->driver = "extn_plug";

   iface = _ecore_evas_extn_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 0;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 0;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   bdata->image = o;
   evas_object_data_set(bdata->image, "Ecore_Evas", ee);
   evas_object_data_set(bdata->image, "Ecore_Evas_Parent", ee_target);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_IN,
                                  _ecore_evas_extn_cb_mouse_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_OUT,
                                  _ecore_evas_extn_cb_mouse_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _ecore_evas_extn_cb_mouse_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _ecore_evas_extn_cb_mouse_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_MOVE,
                                  _ecore_evas_extn_cb_mouse_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _ecore_evas_extn_cb_mouse_wheel, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_extn_cb_multi_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_extn_cb_multi_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_extn_cb_multi_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  _ecore_evas_extn_cb_key_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_UP,
                                  _ecore_evas_extn_cb_key_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HOLD,
                                  _ecore_evas_extn_cb_hold, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_IN,
                                  _ecore_evas_extn_cb_focus_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_OUT,
                                  _ecore_evas_extn_cb_focus_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_SHOW,
                                  _ecore_evas_extn_cb_show, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HIDE,
                                  _ecore_evas_extn_cb_hide, ee);

   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_DEL,
                                  _ecore_evas_extn_plug_image_obj_del, ee);


   extn_ee_list = eina_list_append(extn_ee_list, ee);
   ee_target->sub_ecore_evas = eina_list_append(ee_target->sub_ecore_evas, ee);

   evas_event_callback_add(ee_target->evas, EVAS_CALLBACK_RENDER_PRE,
                           _ecore_evas_extn_plug_targer_render_pre, ee);
   evas_event_callback_add(ee_target->evas, EVAS_CALLBACK_RENDER_POST,
                           _ecore_evas_extn_plug_targer_render_post, ee);
   return o;
}

static Eina_Bool
_ecore_evas_extn_plug_connect(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS)) return EINA_FALSE;

   bdata = ee->engine.data;
   if (!svcname)
     {
        bdata->data = NULL;
        return EINA_FALSE;
     }

   extn = calloc(1, sizeof(Extn));
   if (!extn) return EINA_FALSE;



   Ecore_Ipc_Type ipctype = ECORE_IPC_LOCAL_USER;

   ecore_ipc_init();
   extn->svc.name = eina_stringshare_add(svcname);
   extn->svc.num = svcnum;
   extn->svc.sys = svcsys;

   if (extn->svc.sys) ipctype = ECORE_IPC_LOCAL_SYSTEM;
   extn->ipc.server = ecore_ipc_server_connect(ipctype, (char *)extn->svc.name,
                                               extn->svc.num, ee);
   if (!extn->ipc.server)
     {
        bdata->data = NULL;
        eina_stringshare_del(extn->svc.name);
        free(extn);
        ecore_ipc_shutdown();
        return EINA_FALSE;
     }
   bdata->data = extn;
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD,
                               _ipc_server_add, ee));
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL,
                               _ipc_server_del, ee));
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA,
                               _ipc_server_data, ee));
   return EINA_TRUE;
}

static void
_ecore_evas_extn_plug_object_data_lock(Ecore_Evas *ee)
{
   if (!ee) return;
   _ecore_evas_socket_lock(ee, 0); // XXX lock correct buffer
}

static void
_ecore_evas_extn_plug_object_data_unlock(Ecore_Evas *ee)
{
   if (!ee) return;
   _ecore_evas_socket_unlock(ee, 0); // XXX lock correct buffer
}

static void
_ecore_evas_socket_resize(Ecore_Evas *ee, int w, int h)
{
   Extn *extn;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   int stride = 0;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   extn = bdata->data;
   if (extn)
     {
        if (extn->file.shmfile)
          shmfile_free(extn->file.shmfile);
        bdata->pixels = NULL;
        extn->file.shmfile = shmfile_new(extn->svc.name, extn->svc.num,
                                         ee->w * ee->h * 4, extn->svc.sys);
        if (extn->file.shmfile)
          bdata->pixels = extn->file.shmfile->addr;

        stride = ee->w * 4;
        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             einfo->info.dest_buffer = bdata->pixels;
             einfo->info.dest_buffer_row_bytes = stride;
             einfo->info.use_color_key = 0;
             einfo->info.alpha_threshold = 0;
             einfo->info.func.new_update_region = NULL;
             einfo->info.func.free_update_region = NULL;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }

        if (extn->ipc.clients && extn->file.shmfile)
          {
             Ipc_Data_Resize ipc;
             Eina_List *l;
             Ecore_Ipc_Client *client;

             EINA_LIST_FOREACH(extn->ipc.clients, l, client)
                ecore_ipc_client_send(client, MAJOR, OP_SHM_REF,
                                      ee->w, ee->h, ee->alpha,
                                      extn->file.shmfile->file,
                                      strlen(extn->file.shmfile->file) + 1);
             ipc.w = ee->w;
             ipc.h = ee->h;
             EINA_LIST_FOREACH(extn->ipc.clients, l, client)
                ecore_ipc_client_send(client, MAJOR, OP_RESIZE,
                                      0, 0, 0, &ipc, sizeof(ipc));
          }
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_socket_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_socket_resize(ee, w, h);
}

static void
_ecore_evas_extn_socket_window_profile_change_done_send(Ecore_Evas *ee)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Ecore_Ipc_Client *client;
   Eina_List *l = NULL;
   Ipc_Data_Profile *ipc;
   char *st, *p;
   int len = 0;

   extn = bdata->data;
   if (!extn) return;

   len += sizeof(Ipc_Data_Profile);
   len += strlen(ee->prop.profile.name) + 1;
   len += 1;

   st = alloca(len);
   ipc = (Ipc_Data_Profile *)st;
   memset(st, 0, len);
   p = st + sizeof(Ipc_Data_Profile);

   strcpy(p, ee->prop.profile.name);
   ipc->name = p - (long)st;
   p += strlen(p) + 1;

   EINA_LIST_FOREACH(extn->ipc.clients, l, client)
     {
        ecore_ipc_client_send(client, MAJOR,
                              OP_PROFILE_CHANGE_DONE,
                              0, 0, 0, ipc, len);
     }
}

int
_ecore_evas_extn_socket_render(Ecore_Evas *ee)
{
   Eina_List *updates = NULL, *l, *ll;
   Ecore_Evas *ee2;
   int rend = 0;
   Eina_Rectangle *r;
   Extn *extn;
   Ecore_Ipc_Client *client;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = bdata->data;
   if (!extn) return rend;
   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   if (bdata->pixels)
     {
        _ecore_evas_socket_lock(ee, 0); // XXX lock correct buffer
        updates = evas_render_updates(ee->evas);
        _ecore_evas_socket_unlock(ee, 0); // XXX lock correct buffer
     }
   EINA_LIST_FOREACH(updates, l, r)
     {
        Ipc_Data_Update ipc;


        ipc.x = r->x;
        ipc.y = r->y;
        ipc.w = r->w;
        ipc.h = r->h;
        EINA_LIST_FOREACH(extn->ipc.clients, ll, client)
           ecore_ipc_client_send(client, MAJOR, OP_UPDATE, 0, 0, 0, &ipc, sizeof(ipc));
     }
   if (updates)
     {
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
        EINA_LIST_FOREACH(extn->ipc.clients, ll, client)
           ecore_ipc_client_send(client, MAJOR, OP_UPDATE_DONE, 0, 0, 0, NULL, 0);
        if (extn->profile.done)
          {
             _ecore_evas_extn_socket_window_profile_change_done_send(ee);
             extn->profile.done = EINA_FALSE;
          }
     }

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
   return updates ? 1 : rend;
}

static Eina_Bool
_ipc_client_add(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Add *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(ecore_ipc_client_server_get(e->client)))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;

   extn->ipc.clients = eina_list_append(extn->ipc.clients, e->client);
   ecore_ipc_client_send(e->client, MAJOR, OP_LOCK_FILE, 0, 0, 0, extn->file.lock, strlen(extn->file.lock) + 1);
   ecore_ipc_client_send(e->client, MAJOR, OP_LOCK_FILE2, 0, 0, 0, extn->file.lock2, strlen(extn->file.lock2) + 1);

   if (extn->file.shmfile)
     {
        Ipc_Data_Resize ipc;

        ecore_ipc_client_send(e->client, MAJOR, OP_SHM_REF,
                              ee->w, ee->h, ee->alpha,
                              extn->file.shmfile->file,
                              strlen(extn->file.shmfile->file) + 1);
        ipc.w = ee->w;
        ipc.h = ee->h;

        ecore_ipc_client_send(e->client, MAJOR, OP_RESIZE,
                              0, 0, 0, &ipc, sizeof(ipc));
     }
   _ecore_evas_extn_event(ee, ECORE_EVAS_EXTN_CLIENT_ADD);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_client_del(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Del *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn->ipc.clients, e->client)) return ECORE_CALLBACK_PASS_ON;

   extn->ipc.clients = eina_list_remove(extn->ipc.clients, e->client);

   _ecore_evas_extn_event(ee, ECORE_EVAS_EXTN_CLIENT_DEL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_client_data(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Data *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(ecore_ipc_client_server_get(e->client)))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (e->major != MAJOR)
     return ECORE_CALLBACK_PASS_ON;
   switch (e->minor)
     {
      case OP_RESIZE:
         if ((e->data) && (e->size >= (int)sizeof(Ipc_Data_Resize)))
           {

              Ipc_Data_Resize *ipc = e->data;
              /* create callbacke data size changed */
              _ecore_evas_socket_resize(ee, ipc->w, ipc->h);
           }
         break;
      case OP_SHOW:
         if (!ee->visible)
           {
              ee->visible = 1;
              if (ee->func.fn_show) ee->func.fn_show(ee);
           }
         break;
      case OP_HIDE:
         if (ee->visible)
           {
              ee->visible = 0;
              if (ee->func.fn_hide) ee->func.fn_hide(ee);
           }
         break;
      case OP_FOCUS:
         if (!ee->prop.focused)
           {
              ee->prop.focused = 1;
              evas_focus_in(ee->evas);
              if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
           }
         break;
      case OP_UNFOCUS:
         if (ee->prop.focused)
           {
              ee->prop.focused = 0;
              evas_focus_out(ee->evas);
              if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
           }
         break;
      case OP_EV_MOUSE_IN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_In))
           {
              Ipc_Data_Ev_Mouse_In *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_in(ee->evas, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_OUT:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Out))
           {
              Ipc_Data_Ev_Mouse_Out *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_out(ee->evas, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Up))
           {
              Ipc_Data_Ev_Mouse_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_up(ee->evas, ipc->b, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Down))
           {
              Ipc_Data_Ev_Mouse_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_down(ee->evas, ipc->b, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_MOVE:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Move))
           {
              Ipc_Data_Ev_Mouse_Move *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_move(ee->evas, ipc->x, ipc->y, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_WHEEL:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Wheel))
           {
              Ipc_Data_Ev_Mouse_Wheel *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_wheel(ee->evas, ipc->direction, ipc->z, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Up))
           {
              Ipc_Data_Ev_Multi_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_up(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Down))
           {
              Ipc_Data_Ev_Multi_Down *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_down(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_MOVE:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Move))
           {
              Ipc_Data_Ev_Multi_Move *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_move(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;

#define STRGET(val) \
         do { \
              if ((ipc->val) && (ipc->val < (char *)(long)(e->size - 1))) \
              ipc->val = ((char *)ipc) + (long)ipc->val; \
              else \
              ipc->val = NULL; \
         } while (0)

      case OP_EV_KEY_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Key_Up))
           {
              if ((e->data) && (e->size > 0) &&
                  (((unsigned char *)e->data)[e->size - 1] == 0))
                {
                   Ipc_Data_Ev_Key_Up *ipc = e->data;
                   Evas_Event_Flags flags;

                   STRGET(keyname);
                   STRGET(key);
                   STRGET(string);
                   STRGET(compose);
                   flags = evas_event_default_flags_get(ee->evas);
                   evas_event_default_flags_set(ee->evas, ipc->event_flags);
                   _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
                   evas_event_feed_key_up(ee->evas, ipc->keyname, ipc->key, ipc->string, ipc->compose, ipc->timestamp, NULL);
                   evas_event_default_flags_set(ee->evas, flags);
                }
           }
         break;
      case OP_EV_KEY_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Key_Down))
           {
              if ((e->data) && (e->size > 0) &&
                  (((unsigned char *)e->data)[e->size - 1] == 0))
                {
                   Ipc_Data_Ev_Key_Down *ipc = e->data;
                   Evas_Event_Flags flags;

                   STRGET(keyname);
                   STRGET(key);
                   STRGET(string);
                   STRGET(compose);
                   flags = evas_event_default_flags_get(ee->evas);
                   evas_event_default_flags_set(ee->evas, ipc->event_flags);
                   _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
                   evas_event_feed_key_down(ee->evas, ipc->keyname, ipc->key, ipc->string, ipc->compose, ipc->timestamp, NULL);
                   evas_event_default_flags_set(ee->evas, flags);
                }
           }
         break;
      case OP_EV_HOLD:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Hold))
           {
              Ipc_Data_Ev_Hold *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              evas_event_feed_hold(ee->evas, ipc->hold, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_PROFILE_CHANGE_REQUEST:
         if (e->size >= (int)sizeof(Ipc_Data_Profile))
           {
              if ((e->data) && (e->size > 0) &&
                  (((unsigned char *)e->data)[e->size - 1] == 0))
                {
                   Ipc_Data_Profile *ipc = e->data;
                   STRGET(name);
                   if (ipc->name)
                     {
                        _ecore_evas_window_profile_free(ee);
                        ee->prop.profile.name = (char *)eina_stringshare_add(ipc->name);

                        if (ee->func.fn_state_change)
                          ee->func.fn_state_change(ee);

                        extn->profile.done = EINA_TRUE;
                     }
                }
           }
         break;
      default:
         break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
_ecore_evas_extn_socket_alpha_set(Ecore_Evas *ee, int alpha)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   Eina_List *l;
   Ecore_Ipc_Client *client;

   if (((ee->alpha) && (alpha)) || ((!ee->alpha) && (!alpha))) return;
   ee->alpha = alpha;

   extn = bdata->data;
   if (extn)
     {
        Evas_Engine_Info_Buffer *einfo;

        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        EINA_LIST_FOREACH(extn->ipc.clients, l, client)
           ecore_ipc_client_send(client, MAJOR, OP_SHM_REF,
                                 ee->w, ee->h, ee->alpha,
                                 extn->file.shmfile->file,
                                 strlen(extn->file.shmfile->file) + 1);
     }
}

static void
_ecore_evas_extn_socket_profile_set(Ecore_Evas *ee, const char *profile)
{
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);

        if (ee->func.fn_state_change)
          ee->func.fn_state_change(ee);
     }
}

static void
_ecore_evas_extn_socket_available_profiles_set(Ecore_Evas *ee, const char **plist, int n)
{
   int i;
   _ecore_evas_window_available_profiles_free(ee);
   ee->prop.profile.available_list = NULL;

   if ((plist) && (n >= 1))
     {
        ee->prop.profile.available_list = calloc(n, sizeof(char *));
        if (ee->prop.profile.available_list)
          {
             for (i = 0; i < n; i++)
                ee->prop.profile.available_list[i] = (char *)eina_stringshare_add(plist[i]);
             ee->prop.profile.count = n;

             if (ee->func.fn_state_change)
               ee->func.fn_state_change(ee);
          }
     }
}

static const Ecore_Evas_Engine_Func _ecore_extn_socket_engine_func =
{
   _ecore_evas_extn_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_socket_resize,
   _ecore_evas_socket_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_extn_socket_alpha_set,
   NULL, //transparent
   _ecore_evas_extn_socket_available_profiles_set,
   _ecore_evas_extn_socket_profile_set,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   
   _ecore_evas_extn_socket_render, // render
   NULL,  // screen_geometry_get
   NULL,  // screen_dpi_get
};

EAPI Ecore_Evas *
ecore_evas_extn_socket_new_internal(int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Interface_Extn *iface;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas *ee;
   int rmethod;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_extn_socket_engine_func;
   ee->engine.data = bdata;

   ee->driver = "extn_socket";

   iface = _ecore_evas_extn_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1; /* to accept the profile change request from the client(plug) */

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 0;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 0;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (ee->alpha)
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        else
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = NULL;
        einfo->info.dest_buffer_row_bytes = 0;
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }
   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");

   extn_ee_list = eina_list_append(extn_ee_list, ee);

   _ecore_evas_register(ee);

   return ee;
}

Eina_Bool
_ecore_evas_extn_socket_listen(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = calloc(1, sizeof(Extn));
   if (!extn)
     {
        return EINA_FALSE;
     }
   else
     {
        Ecore_Ipc_Type ipctype = ECORE_IPC_LOCAL_USER;
        char buf[PATH_MAX];

        ecore_ipc_init();
        extn->svc.name = eina_stringshare_add(svcname);
        extn->svc.num = svcnum;
        extn->svc.sys = svcsys;

        snprintf(buf, sizeof(buf), "/tmp/ee-lock-XXXXXX");
        extn->file.lockfd = mkstemp(buf);
        if (extn->file.lockfd >= 0)
          extn->file.lock = eina_stringshare_add(buf);
        if ((extn->file.lockfd < 0) || (!extn->file.lock))
          {
             if (extn->file.lockfd)
               {
                  close(extn->file.lockfd >= 0);
                  unlink(buf);
               }
             eina_stringshare_del(extn->svc.name);
             if (extn->file.lock) eina_stringshare_del(extn->file.lock);
             free(extn);
             ecore_ipc_shutdown();
             return EINA_FALSE;
          }

        snprintf(buf, sizeof(buf), "/tmp/ee-lock-XXXXXX");
        extn->file.lockfd2 = mkstemp(buf);
        if (extn->file.lockfd2 >= 0)
          extn->file.lock2 = eina_stringshare_add(buf);
        if ((extn->file.lockfd2 < 0) || (!extn->file.lock2))
          {
             if (extn->file.lockfd >= 0)
               {
                  close(extn->file.lockfd);
                  if (extn->file.lock) unlink(extn->file.lock);
               }
             if (extn->file.lockfd2 >= 0)
               {
                  close(extn->file.lockfd2);
                  unlink(buf);
               }
             eina_stringshare_del(extn->svc.name);
             if (extn->file.lock) eina_stringshare_del(extn->file.lock);
             if (extn->file.lock2) eina_stringshare_del(extn->file.lock2);
             free(extn);
             ecore_ipc_shutdown();
             return EINA_FALSE;
          }
        
        if (extn->svc.sys) ipctype = ECORE_IPC_LOCAL_SYSTEM;
        extn->ipc.am_server = EINA_TRUE;
        extn->ipc.server = ecore_ipc_server_add(ipctype,
                                                (char *)extn->svc.name,
                                                extn->svc.num, ee);
        if (!extn->ipc.server)
          {
             if (extn->file.lockfd >= 0)
               {
                  close(extn->file.lockfd);
                  if (extn->file.lock) unlink(extn->file.lock);
               }
             if (extn->file.lockfd2 >= 0)
               {
                  close(extn->file.lockfd2);
                  if (extn->file.lock2) unlink(extn->file.lock2);
               }
             eina_stringshare_del(extn->svc.name);
             eina_stringshare_del(extn->file.lock);
             eina_stringshare_del(extn->file.lock2);
             free(extn);
             ecore_ipc_shutdown();
             return EINA_FALSE;
          }
        bdata->data = extn;
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD,
                                    _ipc_client_add, ee));
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL,
                                    _ipc_client_del, ee));
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,
                                    _ipc_client_data, ee));
     }
   return EINA_TRUE;
}

static Ecore_Evas_Interface_Extn *
_ecore_evas_extn_interface_new(void)
{
   Ecore_Evas_Interface_Extn *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Extn));
   if (!iface) return NULL;

   iface->base.name = interface_extn_name;
   iface->base.version = interface_extn_version;

   iface->data_lock = _ecore_evas_extn_plug_object_data_lock;
   iface->data_unlock = _ecore_evas_extn_plug_object_data_unlock;
   iface->connect = _ecore_evas_extn_plug_connect;
   iface->listen = _ecore_evas_extn_socket_listen;

   return iface;
}

EINA_MODULE_INIT(_ecore_evas_extn_init);
EINA_MODULE_SHUTDOWN(_ecore_evas_extn_shutdown);
