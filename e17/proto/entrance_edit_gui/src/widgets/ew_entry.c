#include <Etk.h>
#include <Entrance_Widgets.h>

Entrance_Widget
ew_entry_new(const char *label, const char *text, int ispassword)
{
	Entrance_Widget ew = ew_new();
	if(!ew)
	{
		return NULL;
	}

	ew->owner = etk_hbox_new(0, 10);
	ew->entry_control = etk_entry_new();


	if(ispassword)
		ew_entry_password_set(ew);

	if(label)
	{
		ew->label = etk_label_new(label);
		etk_box_append(ETK_BOX(ew->owner), ew->label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
	}

	etk_box_append(ETK_BOX(ew->owner), ew->entry_control, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
	if(text)
	{
		ew_entry_set(ew, text);
	} 
	return ew;
}

const char*
ew_entry_get(Entrance_Widget ew)
{
	return etk_entry_text_get(ETK_ENTRY(ew->entry_control));
}

void
ew_entry_set(Entrance_Widget ew, const char *text)
{
	if(ew)
		etk_entry_text_set(ETK_ENTRY(ew->entry_control), text);
}

void
ew_entry_password_set(Entrance_Widget ew)
{
	if(ew)
		etk_entry_password_mode_set(ETK_ENTRY(ew->entry_control), ETK_TRUE);
}

void 
ew_entry_password_clear(Entrance_Widget ew)
{
	if(ew)
		etk_entry_password_mode_set(ETK_ENTRY(ew->entry_control),  ETK_FALSE);
}
