/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

#define E_MODULE_API_VERSION 4

typedef struct _E_Module     E_Module;
typedef struct _E_Module_Api E_Module_Api;

#else
#ifndef E_MODULE_H
#define E_MODULE_H

#define E_MODULE_TYPE 0xE0b0100b

struct _E_Module
{
   E_Object             e_obj_inherit;

   E_Module_Api        *api;
   
   const char          *name;
   const char          *dir;
   void                *handle;
   
   struct {
      void * (*init)        (E_Module *m);
      int    (*shutdown)    (E_Module *m);
      int    (*save)        (E_Module *m);
      int    (*about)       (E_Module *m);
      int    (*config)      (E_Module *m);
   } func;
   
   unsigned char        enabled : 1;
   unsigned char        error : 1;
   
   /* the module is allowed to modify these */
   void                *data;
};

struct _E_Module_Api
{
   int         version;
   const char *name;
};

EAPI int          e_module_init(void);
EAPI int          e_module_shutdown(void);

EAPI E_Module    *e_module_new(const char *name);
EAPI int          e_module_save(E_Module *m);
EAPI const char  *e_module_dir_get(E_Module *m);
EAPI int          e_module_enable(E_Module *m);
EAPI int          e_module_disable(E_Module *m);
EAPI int          e_module_enabled_get(E_Module *m);
EAPI int          e_module_save_all(void);
EAPI E_Module    *e_module_find(const char *name);
EAPI Evas_List   *e_module_list(void);
EAPI void         e_module_dialog_show(E_Module *m, const char *title, const char *body);

#endif
#endif
