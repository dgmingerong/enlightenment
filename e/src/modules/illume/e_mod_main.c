#include "e.h"

#include "e_cfg.h"
#include "e_kbd.h"
#include "e_slipshelf.h"
#include "e_slipwin.h"
#include "e_pwr.h"
#include "e_busywin.h"
#include "e_simplelock.h"
#include "e_flaunch.h"
#include "e_slipwin.h"
#include "e_appwin.h"
#include "e_syswin.h"

#include "e_mod_main.h"
#include "e_mod_layout.h"
#include "e_mod_win.h"

#include "e_mod_gad_wifi.h"
#include "e_mod_gad_gsm.h"
#include "e_mod_gad_usb.h"
#include "e_mod_gad_bluetooth.h"

/* this is needed to advertise a label for the module IN the code (not just
 * the .desktop file) but more specifically the api version it was compiled
 * for so E can skip modules that are compiled for an incorrect API version
 * safely) */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Illume"
};

/* called first thing when E inits the module */
EAPI void *
e_modapi_init(E_Module *m) 
{
   /* set up the virtual keyboard */
   e_cfg_init(m);
   e_kbd_init(m);
   e_busywin_init();
   e_slipshelf_init();
   e_slipwin_init();
   e_pwr_init();
   e_flaunch_init();
   e_simplelock_init(m);
   e_appwin_init();
   e_syswin_init();
   _e_mod_layout_init(m);
   _e_mod_win_init(m);
   
   _e_mod_gad_wifi_init(m);
   _e_mod_gad_gsm_init(m);
   _e_mod_gad_usb_init(m);
   _e_mod_gad_bluetooth_init(m);
   
   return m; /* return NULL on failure, anything else on success. the pointer
	      * returned will be set as m->data for convenience tracking */
}

/* called on module shutdown - should clean up EVERYTHING or we leak */
EAPI int
e_modapi_shutdown(E_Module *m) 
{
   _e_mod_gad_bluetooth_shutdown();
   _e_mod_gad_usb_shutdown();
   _e_mod_gad_gsm_shutdown();
   _e_mod_gad_wifi_shutdown();
   
   _e_mod_win_shutdown();
   _e_mod_layout_shutdown();
   e_syswin_shutdown();
   e_appwin_shutdown();
   e_simplelock_shutdown();
   e_flaunch_shutdown();
   e_pwr_shutdown();
   e_slipwin_shutdown();
   e_slipshelf_shutdown();
   e_busywin_shutdown();
   e_kbd_shutdown();
   e_cfg_shutdown();
   return 1; /* 1 for success, 0 for failure */
}

/* called by E when it thinks this module shoudl go save any config it has */
EAPI int
e_modapi_save(E_Module *m) 
{
   return e_cfg_save(); /* 1 for success, 0 for failure */
}
