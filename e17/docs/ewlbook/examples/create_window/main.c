#include <Ewl.h>

void destroy_cb(Ewl_Widget *w, void *event, void *data) {
    ewl_widget_destroy(w);
    ewl_main_quit();
}

int main(int argc, char ** argv) {
    Ewl_Widget *win = NULL;

    if (!ewl_init(&argc, argv)) {
        printf("Unable to init ewl\n");
        return 1;
    }

    win = ewl_window_new();
    ewl_window_set_title(EWL_WINDOW(win), "EWL Window");
    ewl_window_set_name(EWL_WINDOW(win), "EWL_WINDOW");
    ewl_window_set_class(EWL_WINDOW(win), "EWLWindow");
    ewl_object_request_size(EWL_OBJECT(win), 200, 100);

    ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW, destroy_cb, NULL);
    ewl_widget_show(win);

    ewl_main();

    return 0;
}

