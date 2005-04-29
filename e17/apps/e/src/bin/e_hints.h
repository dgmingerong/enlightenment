/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_HINTS_H
#define E_HINTS_H

EAPI void e_hints_init(void);
EAPI void e_hints_client_list_set(void);
EAPI void e_hints_client_stacking_set(void);

EAPI void e_hints_active_window_set(E_Manager *man, E_Border *bd);

EAPI void e_hints_window_name_set(E_Border *bd);
EAPI void e_hints_window_name_get(E_Border *bd);

EAPI void e_hints_desktop_config_set(void);

EAPI void e_hints_window_init(E_Border *bd);
EAPI void e_hints_window_state_set(E_Border *bd);
EAPI void e_hints_window_state_get(E_Border *bd);
EAPI void e_hints_window_type_set(E_Border *bd);
EAPI void e_hints_window_type_get(E_Border *bd);

EAPI void e_hints_window_visible_set(E_Border *bd);
EAPI void e_hints_window_iconic_set(E_Border *bd);
EAPI void e_hints_window_hidden_set(E_Border *bd);

EAPI void e_hints_window_shade_direction_set(E_Border *bd, E_Direction dir);
EAPI E_Direction e_hints_window_shade_direction_get(E_Border *bd);

EAPI void e_hints_window_shaded_set(E_Border *bd, int on);
EAPI void e_hints_window_maximized_set(E_Border *bd, int on);
EAPI void e_hints_window_sticky_set(E_Border *bd, int on);
EAPI void e_hints_window_fullscreen_set(E_Border *bd, int on);

EAPI void e_hints_window_icon_name_get(E_Border *bd);
    
#endif
#endif
