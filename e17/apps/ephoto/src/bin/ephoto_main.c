#include "ephoto.h"

/*Ewl Callbacks*/
static void destroy(Ewl_Widget *w, void *event, void *data);
static void populate_albums(Ewl_Widget *w, void *event, void *data);
static void populate_directories(Ewl_Widget *w, void *event, void *data);
static void window_fullscreen(Ewl_Widget *w, void *event, void *data);

/*Ephoto Create Callbacks*/
static Ewl_Widget *add_atree(Ewl_Widget *c);
static Ewl_Widget *add_dtree(Ewl_Widget *c);

/*Ephoto MVC Callbacks*/
static Ewl_Widget *album_view_new(void *data, unsigned int row, unsigned int column);
static Ewl_Widget *directory_view_new(void *data, unsigned int row, unsigned int column);
static Ewl_Widget *album_header_fetch(void *data, unsigned int column);
static Ewl_Widget *directory_header_fetch(void *data, unsigned int column);
static void *album_data_fetch(void *data, unsigned int row, unsigned int column);
static void *directory_data_fetch(void *data, unsigned int row, unsigned int column);
static unsigned int album_data_count(void *data);
static unsigned int directory_data_count(void *data);

/*Ephoto Global Variables*/
Ephoto_Main *em;

/*Destroy the Main Window*/
static void destroy(Ewl_Widget *w, void *event, void *data)
{
	ephoto_db_close(em->db);
        ewl_widget_destroy(em->win);
	ecore_list_destroy(em->albums);
	ecore_list_destroy(em->directories);
	ecore_dlist_destroy(em->images);
	free(em);
	ewl_main_quit();
	return;
}

static void window_fullscreen(Ewl_Widget *w, void *event, void *data)
{
	int fullscreen;

	fullscreen = ewl_window_fullscreen_get(EWL_WINDOW(em->win));
	if(!fullscreen)
	{
		ewl_window_fullscreen_set(EWL_WINDOW(em->win), 1);
	}
	else
	{
		ewl_window_fullscreen_set(EWL_WINDOW(em->win), 0);
	}
	return;
}

/*Show the main view*/
void show_main_view(Ewl_Widget *c, void *event, void *data)
{
	ewl_notebook_visible_page_set(EWL_NOTEBOOK(em->main_nb), em->main_vbox);
	ecore_dlist_goto_first(em->images);
}

/*Create the Main Ephoto Window*/
void create_main_gui(void)
{
	Ewl_Widget *vbox, *hbox, *vsep, *button;
	Ewl_Widget *mb, *menu, *mi;

	em = NULL;
	em = calloc(1, sizeof(Ephoto_Main));
	em->albums = ecore_list_new();
	em->directories = ecore_list_new();
	em->images = ecore_dlist_new();

	em->win = add_window("Ephoto!", 775, 540, destroy, NULL);

	vbox = add_box(em->win, EWL_ORIENTATION_VERTICAL, 5);
	ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);

	mb = add_menubar(vbox);
	menu = add_menu(mb, "File");
	mi = add_menu_item(menu, "Exit", PACKAGE_DATA_DIR "/images/exit.png", destroy, NULL);

	hbox = add_box(vbox, EWL_ORIENTATION_HORIZONTAL, 2);
	ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_ALL);

	em->main_nb = ewl_notebook_new();
        ewl_notebook_tabbar_visible_set(EWL_NOTEBOOK(em->main_nb), 0);
        ewl_object_fill_policy_set(EWL_OBJECT(em->main_nb), EWL_FLAG_FILL_ALL);
        ewl_container_child_append(EWL_CONTAINER(hbox), em->main_nb);
        ewl_widget_show(em->main_nb);

	em->main_vbox = add_box(em->main_nb, EWL_ORIENTATION_VERTICAL, 2);
	ewl_object_fill_policy_set(EWL_OBJECT(em->main_vbox), EWL_FLAG_FILL_ALL);

	em->edit_vbox = add_box(em->main_nb, EWL_ORIENTATION_VERTICAL, 2);
	ewl_object_fill_policy_set(EWL_OBJECT(em->edit_vbox), EWL_FLAG_FILL_ALL);
	
	add_edit_view(em->edit_vbox);

	em->view = add_box(em->main_vbox, EWL_ORIENTATION_HORIZONTAL, 2);
	ewl_object_fill_policy_set(EWL_OBJECT(em->view), EWL_FLAG_FILL_ALL);

	em->browser = ewl_notebook_new();
	ewl_object_fill_policy_set(EWL_OBJECT(em->browser), EWL_FLAG_FILL_VFILL);
	ewl_object_preferred_inner_w_set(EWL_OBJECT(em->browser), 175);
	ewl_container_child_append(EWL_CONTAINER(em->view), em->browser);
	ewl_widget_show(em->browser);

        em->atree = add_atree(em->browser);
	ewl_object_maximum_w_set(EWL_OBJECT(em->atree), 175);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(em->browser), em->atree, "Albums");

	em->dtree = add_dtree(em->browser);
	ewl_object_maximum_w_set(EWL_OBJECT(em->dtree), 175);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(em->browser), em->dtree, "File System");

	em->view_box = ewl_notebook_new();
	ewl_notebook_tabbar_visible_set(EWL_NOTEBOOK(em->view_box), 0);
	ewl_object_fill_policy_set(EWL_OBJECT(em->view_box), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(em->view), em->view_box);
	ewl_widget_show(em->view_box);

	add_normal_view(em->view_box);

	add_single_view(em->view_box);

	add_list_view(em->view_box);	

	em->toolbar = ewl_hbox_new();
	ewl_box_spacing_set(EWL_BOX(em->toolbar), 5);
	ewl_object_alignment_set(EWL_OBJECT(em->toolbar), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(em->toolbar), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(vbox), em->toolbar);
	ewl_widget_show(em->toolbar);

        button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/normal_view.png", show_normal_view, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_attach_tooltip_text_set(button, "Normal View");
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

        button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/list_view.png", show_list_view, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_attach_tooltip_text_set(button, "List View");      
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

	button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/image.png", show_single_view, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_attach_tooltip_text_set(button, "Single View");
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

	vsep = ewl_vseparator_new();
	ewl_container_child_append(EWL_CONTAINER(em->toolbar), vsep);
	ewl_widget_show(vsep);

	button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/stock_fullscreen.png", window_fullscreen, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_attach_tooltip_text_set(button, "Toggle Fullscreen");
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

        button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/x-office-presentation.png", start_slideshow, NULL);
	ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_attach_tooltip_text_set(button, "Start a Slideshow");
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

	vsep = ewl_vseparator_new();
        ewl_container_child_append(EWL_CONTAINER(em->toolbar), vsep);
        ewl_widget_show(vsep);

        button = add_button(em->toolbar, NULL, PACKAGE_DATA_DIR "/images/edit_view.png", show_edit_view, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
        ewl_attach_tooltip_text_set(button, "Move to edit view");
        ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);

	em->albums = ecore_list_new();
	em->db = ephoto_db_init();
	em->albums = ephoto_db_list_albums(em->db);

	ewl_mvc_data_set(EWL_MVC(em->atree), em->albums);
	
	populate_albums(NULL, NULL, "Complete Library");
	populate_directories(NULL, NULL, getenv("HOME"));

	return;
}

/*Update the Image List*/
static void populate_albums(Ewl_Widget *w, void *event, void *data)
{
	const char *album;
	char *imagef;
	Ewl_Widget *thumb;

	if (w)
	{
		album = ewl_widget_name_get(w);
	}
	else
	{
		album = data;
	}

	if (!ecore_list_is_empty(em->images))
	{
		ecore_dlist_destroy(em->images);
	}

	em->images = ecore_dlist_new();
	em->images = ephoto_db_list_images(em->db, album);

	ecore_dlist_goto_first(em->images);
	ewl_container_reset(EWL_CONTAINER(em->fbox));
        while (ecore_dlist_current(em->images))
        {
                imagef = ecore_dlist_current(em->images);

                thumb = add_image(em->fbox, imagef, 1, freebox_image_clicked, NULL);
		ewl_image_constrain_set(EWL_IMAGE(thumb), 81);
		ewl_object_alignment_set(EWL_OBJECT(thumb), EWL_FLAG_ALIGN_CENTER);
		ewl_widget_name_set(thumb, imagef);

		ecore_dlist_next(em->images);
        }
	ecore_dlist_goto_first(em->images);
	ewl_image_file_path_set(EWL_IMAGE(em->simage), ecore_dlist_current(em->images));

        ewl_mvc_data_set(EWL_MVC(em->ltree), em->images);
	ewl_mvc_dirty_set(EWL_MVC(em->ltree), 1);

	return;
}

/*Update the Image List*/
static void populate_directories(Ewl_Widget *w, void *event, void *data)
{
	const char *directory;
	char *imagef;
        Ewl_Widget *thumb;

	if (w)
	{
		directory = ewl_widget_name_get(w);
	}
	else
	{
		directory = data;
	}
	if (!ecore_list_is_empty(em->directories))
	{
		ecore_list_destroy(em->directories);
	}

	if(em->current_directory) free(em->current_directory);
	em->current_directory = strdup(directory);

	em->directories = ecore_list_new();
	em->directories = get_directories(directory);

	ecore_dlist_goto_first(em->directories);
        ewl_mvc_data_set(EWL_MVC(em->dtree), em->directories);
        ewl_mvc_dirty_set(EWL_MVC(em->dtree), 1);

	if (w)
	{
        	if (!ecore_list_is_empty(em->images))
        	{
                	ecore_dlist_destroy(em->images);
        	}

	        em->images = ecore_dlist_new();
		em->images = get_images(directory);
	
		ecore_dlist_goto_first(em->images);
		
		ewl_container_reset(EWL_CONTAINER(em->fbox));
		while (ecore_dlist_current(em->images))
		{
			imagef = ecore_dlist_current(em->images);
	
      		        thumb = add_image(em->fbox, imagef, 1, freebox_image_clicked, NULL);
	      	        ewl_image_constrain_set(EWL_IMAGE(thumb), 81);
       		        ewl_object_alignment_set(EWL_OBJECT(thumb), EWL_FLAG_ALIGN_CENTER);
	                ewl_widget_name_set(thumb, imagef);

        	        ecore_dlist_next(em->images);
		}
		ecore_dlist_goto_first(em->images);
		ewl_image_file_path_set(EWL_IMAGE(em->simage), ecore_dlist_current(em->images));
		ewl_mvc_data_set(EWL_MVC(em->ltree), em->images);
		ewl_mvc_dirty_set(EWL_MVC(em->ltree), 1);
	}
	return;
} 

/*Create and Add a Tree to the Container c*/
static Ewl_Widget *add_atree(Ewl_Widget *c)
{
	Ewl_Widget *tree;
	Ewl_Model *model;
	Ewl_View *view;

	model = ewl_model_new();
	ewl_model_data_fetch_set(model, album_data_fetch);
	ewl_model_data_count_set(model, album_data_count);

        view = ewl_view_new();
        ewl_view_widget_fetch_set(view, album_view_new);
        ewl_view_header_fetch_set(view, album_header_fetch);

	tree = ewl_tree2_new();
	ewl_tree2_headers_visible_set(EWL_TREE2(tree), 0);
	ewl_tree2_fixed_rows_set(EWL_TREE2(tree), 1);
	ewl_tree2_column_count_set(EWL_TREE2(tree), 1);
	ewl_mvc_model_set(EWL_MVC(tree), model);
	ewl_mvc_view_set(EWL_MVC(tree), view);
	ewl_mvc_selection_mode_set(EWL_MVC(tree), EWL_SELECTION_MODE_SINGLE);
	ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(c), tree);
	ewl_widget_show(tree);

	return tree;
}

/*Create and Add a Tree to the Container c*/
static Ewl_Widget *add_dtree(Ewl_Widget *c)
{
        Ewl_Widget *tree;
        Ewl_Model *model;
        Ewl_View *view;

        model = ewl_model_new();
        ewl_model_data_fetch_set(model, directory_data_fetch);
        ewl_model_data_count_set(model, directory_data_count);

        view = ewl_view_new();
        ewl_view_widget_fetch_set(view, directory_view_new);
        ewl_view_header_fetch_set(view, directory_header_fetch);

        tree = ewl_tree2_new();
        ewl_tree2_headers_visible_set(EWL_TREE2(tree), 0);
        ewl_tree2_fixed_rows_set(EWL_TREE2(tree), 1);
        ewl_tree2_column_count_set(EWL_TREE2(tree), 1);
        ewl_mvc_model_set(EWL_MVC(tree), model);
        ewl_mvc_view_set(EWL_MVC(tree), view);
        ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
        ewl_container_child_append(EWL_CONTAINER(c), tree);
        ewl_widget_show(tree);

        return tree;
}

/* The view of the users albums */
static Ewl_Widget *album_view_new(void *data, unsigned int row, unsigned int column)
{
	const char *album;
	Ewl_Widget *icon;

	album = data;

	icon = add_icon(NULL, album, PACKAGE_DATA_DIR "/images/image.png", 0, populate_albums, NULL);

        ewl_icon_constrain_set(EWL_ICON(icon), 25);
	ewl_box_orientation_set(EWL_BOX(icon), EWL_ORIENTATION_HORIZONTAL);
	ewl_object_alignment_set(EWL_OBJECT(icon), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(icon), EWL_FLAG_FILL_ALL);
	ewl_widget_name_set(icon, album);

	return icon;
}

/* The view of the users directories */
static Ewl_Widget *directory_view_new(void *data, unsigned int row, unsigned int column)
{
        const char *directory;
        Ewl_Widget *icon;

        directory = data;

	icon = add_icon(NULL, basename((char *)directory), PACKAGE_DATA_DIR "/images/folder.png", 0, populate_directories, NULL);

        ewl_icon_constrain_set(EWL_ICON(icon), 25);
        ewl_box_orientation_set(EWL_BOX(icon), EWL_ORIENTATION_HORIZONTAL);
        ewl_object_alignment_set(EWL_OBJECT(icon), EWL_FLAG_ALIGN_LEFT);
        ewl_object_fill_policy_set(EWL_OBJECT(icon), EWL_FLAG_FILL_ALL);
	if (!strncmp(directory, "..", 2))
	{
		ewl_widget_name_set(icon, dirname(em->current_directory));
	}
	else
	{
        	ewl_widget_name_set(icon, directory);
        }

        return icon;
}


/* The header for the tree */
static Ewl_Widget *album_header_fetch(void *data, unsigned int column)
{
	Ewl_Widget *label;

	label = add_label(NULL, "Albums");

	return label;
}

/* The header for the tree */
static Ewl_Widget *directory_header_fetch(void *data, unsigned int column)
{
        Ewl_Widget *label;

        label = add_label(NULL, "Directories");

        return label;
}

/* The albums that will be displayed*/ 
static void *album_data_fetch(void *data, unsigned int row, unsigned int column)
{
	const char *album;
	void *val = NULL;

	album = ecore_list_goto_index(em->albums, row);
	if (album)
	{
		val = (void *)album;
	}

	return val;
}

/* The directories that will be displayed*/
static void *directory_data_fetch(void *data, unsigned int row, unsigned int column)
{
        const char *directory;
        void *val = NULL;

        directory = ecore_list_goto_index(em->directories, row);
        if (directory)
        {
                val = (void *)directory;
        }

        return val;
}

/* The number of albums the view is displaying */
static unsigned int album_data_count(void *data)
{
	int val;

	val = ecore_list_nodes(em->albums);

	return val;
}

/* The number of directories the view is displaying */
static unsigned int directory_data_count(void *data)
{
        int val;

        val = ecore_list_nodes(em->directories);

        return val;
}

